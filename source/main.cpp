
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <iostream>

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>

// #include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

using namespace SKVMOIP;

#define SERVER_PORT "2000"
#define SERVER_IP "192.168.1.113"

#define THIS_PORT 3000
#define THIS_IP "192.168.1.17"

#define NETWORK_THREAD_BUFFER_SIZE 512

struct InputData
{
	Win32::RawInputDeviceType deviceType;
	union
	{
		Win32::MouseInput mouseInput;
		Win32::KeyboardInput keyboardInput;
	};
};

struct PackedInputData
{
	u8 deviceType : 1;
	union
	{
		u32 usbHIDUsageID;
		struct
		{
			/* 1 Byte */
			u8 middleMBPressed : 1;
			u8 middleMBReleased : 1;
			u8 leftMBPressed : 1;
			u8 leftMBReleased : 1;
			u8 rightMBPressed : 1;
			u8 rightMBReleased : 1;
			u8 bfMBPressed : 1;
			u8 bbMBReleased : 1;

			/* 4 Bytes */
			s8 mouse_point_x;
			s8 mouse_point_y;
			s8 mouse_wheel_x;
			s8 mouse_wheel_y;
		};
	};
};

static std::mutex gMutex;
static std::condition_variable gCv;

static std::deque<InputData> gInputQueue;

static std::array<InputData, NETWORK_THREAD_BUFFER_SIZE> gNetworkThreadBuffer;

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
			const InputData& inputData = gNetworkThreadBuffer[index];
			switch(inputData.deviceType)
			{
				case Win32::RawInputDeviceType::Mouse:
				{
					Win32::DumpMouseInput(reinterpret_cast<const Win32::MouseInput*>(&inputData.mouseInput));
					break;
				}
				case Win32::RawInputDeviceType::Keyboard:
				{
					Win32::DumpKeyboardInput(reinterpret_cast<const Win32::KeyboardInput*>(&inputData.keyboardInput));
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
	InputData inputData = { Win32::RawInputDeviceType::Mouse };
	memcpy(&inputData.mouseInput, mouseInputData, sizeof(Win32::MouseInput));
	gInputQueue.push_front(inputData);
	lock.unlock();
	gCv.notify_one();
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	std::unique_lock<std::mutex> lock(gMutex);
	InputData inputData = { Win32::RawInputDeviceType::Keyboard };
	memcpy(&inputData.keyboardInput, keyboardInputData, sizeof(Win32::KeyboardInput));
	gInputQueue.push_front(inputData);
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

