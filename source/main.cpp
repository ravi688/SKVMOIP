
#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>

using namespace SKVMOIP;

#define SERVER_PORT "2000"
#define SERVER_IP "192.168.1.113"

#define THIS_PORT 3000
#define THIS_IP "192.168.1.17"

#define NETWORK_THREAD_BUFFER_SIZE 512

static std::mutex gMutex;
static std::condition_variable gCv;

static std::deque<Win32::KMInputData> gInputQueue;

static std::array<Win32::KMInputData, NETWORK_THREAD_BUFFER_SIZE> gNetworkThreadBuffer;

static void NetworkHandler()
{
	while(1)
	{
		std::size_t index = 0;
		{
			std::unique_lock<std::mutex> lock(gMutex);
			gCv.wait(lock, [] { return !gInputQueue.empty(); });
			while(!gInputQueue.empty() && (index < NETWORK_THREAD_BUFFER_SIZE))
			{
				gNetworkThreadBuffer[index++] = gInputQueue.back();
				gInputQueue.pop_back();
			}
		}

		std::size_t count = index;

		if(count == 0) continue;

#ifdef GLOBAL_DEBUG
		if(count == NETWORK_THREAD_BUFFER_SIZE)
			debug_log_warning("Consider increasing the value of NETWORK_THREAD_BUFFER_SIZE, which is currently set at %lu, but should be at least %lu", 
							 NETWORK_THREAD_BUFFER_SIZE, gInputQueue.size() + NETWORK_THREAD_BUFFER_SIZE);
#endif /* Debug Mode */

		index = 0;
		while(index < count)
		{
			const Win32::KMInputData& kmInputData = gNetworkThreadBuffer[index];
			Network::NetworkPacket netPacket = Network::GetNetworkPacketFromKMInputData(kmInputData);
			Network::DumpNetworkPacket(netPacket);
			switch(kmInputData.deviceType)
			{
				case Win32::RawInputDeviceType::Mouse:
				{
					// Win32::DumpMouseInput(reinterpret_cast<const Win32::MouseInput*>(&kmInputData.mouseInput));
					break;
				}
				case Win32::RawInputDeviceType::Keyboard:
				{
					// Win32::DumpKeyboardInput(reinterpret_cast<const Win32::KeyboardInput*>(&kmInputData.keyboardInput));
					break;
				}
				default:
				{		
					_assert(false);
					break;
				}
			}
			++index;
		}
	}
}

static void MouseInputHandler(void* mouseInputData, void* userData)
{
	std::unique_lock<std::mutex> lock(gMutex);
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Mouse };
	memcpy(&kmInputData.mouseInput, mouseInputData, sizeof(Win32::MouseInput));
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	std::unique_lock<std::mutex> lock(gMutex);
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Keyboard };
	memcpy(&kmInputData.keyboardInput, keyboardInputData, sizeof(Win32::KeyboardInput));
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });


	Window window(500, 500, "Scalable KVM Over IP");

	Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);

	std::thread networkThread(NetworkHandler);

	while(!window.shouldClose())
	{
		window.pollEvents();
	}

	networkThread.join();

	window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	return 0;
}

