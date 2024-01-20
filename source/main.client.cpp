#ifdef BUILD_CLIENT

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>

#include <SKVMOIP/HDMIDecodeNetStream.hpp>
#include <SKVMOIP/Win32/Win32DrawSurface.hpp>

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>
#include <memory>

#include <bufferlib/buffer.h>

using namespace SKVMOIP;

#define SERVER_IP_ADDRESS "localhost"
#define SERVER_PORT_NUMBER "2020"

#define NETWORK_THREAD_BUFFER_SIZE 1024

static std::mutex gMutex;
static std::condition_variable gCv;

static std::deque<Win32::KMInputData> gInputQueue;

static std::array<Win32::KMInputData, NETWORK_THREAD_BUFFER_SIZE> gNetworkThreadBuffer;

static u8 gModifierKeys = 0;

static void NetworkHandler(Network::Socket& networkStream)
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
			const Network::NetworkPacket netPacket = Network::GetNetworkPacketFromKMInputData(kmInputData, gModifierKeys);
			// Network::DumpNetworkPacket(netPacket);
			Network::Result result = networkStream.send(reinterpret_cast<const u8*>(&netPacket), sizeof(netPacket));
			if(result == Network::Result::SocketError)
			{
				debug_log_error("Failed to send Network Packet over Network Stream, error code: %d", WSAGetLastError());
				exit(-1);
			}
			++index;
		}
	}
}

static std::unordered_map<u32, Win32::KeyStatus> gPressedKeys;

static void MouseInputHandler(void* mouseInputData, void* userData)
{
	std::unique_lock<std::mutex> lock(gMutex);
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Mouse };
	memcpy(&kmInputData.mouseInput, mouseInputData, sizeof(Win32::MouseInput));
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

static HIDUsageIDModifierBits GetModifierBitFromMakeCode(PS2Set1MakeCode makeCode)
{
	switch(makeCode)
	{
		case PS2Set1MakeCode::LeftControl: return HIDUsageIDModifierBits::LEFTCTRL_BIT;
		case PS2Set1MakeCode::LeftShift: return HIDUsageIDModifierBits::LEFTSHIFT_BIT;
		case PS2Set1MakeCode::LeftAlt: return HIDUsageIDModifierBits::LEFTALT_BIT;
		case PS2Set1MakeCode::LeftGUI: return HIDUsageIDModifierBits::LEFTGUI_BIT; 
		case PS2Set1MakeCode::RightControl: return HIDUsageIDModifierBits::RIGHTCTRL_BIT; 
		case PS2Set1MakeCode::RightShift: return HIDUsageIDModifierBits::RIGHTSHIFT_BIT;
		case PS2Set1MakeCode::RightAlt: return HIDUsageIDModifierBits::RIGHTALT_BIT; 
		case PS2Set1MakeCode::RightGUI: return HIDUsageIDModifierBits::RIGHTGUI_BIT;
		default: return IntToEnumClass<HIDUsageIDModifierBits>(static_cast<u8>(0));
	}
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	Win32::KeyboardInput* kInput = reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData);
	if(kInput->keyStatus == Win32::KeyStatus::Pressed)
	{
		switch(kInput->makeCode)
		{
			case PS2Set1MakeCode::LeftControl:
			case PS2Set1MakeCode::LeftShift:
			case PS2Set1MakeCode::LeftAlt:
			case PS2Set1MakeCode::LeftGUI: 
			case PS2Set1MakeCode::RightControl: 
			case PS2Set1MakeCode::RightShift:
			case PS2Set1MakeCode::RightAlt: 
			case PS2Set1MakeCode::RightGUI: 
			{
				u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(kInput->makeCode));
				_assert(modifierKey != 0);
				gModifierKeys |= modifierKey;
				break;
			}
		}

		if(gPressedKeys.find(kInput->makeCode) != gPressedKeys.end())
			/* skip as the key is already pressed */
			return;
		else
			gPressedKeys.insert({kInput->makeCode, Win32::KeyStatus::Pressed});
	}
	else if(kInput->keyStatus == Win32::KeyStatus::Released)
	{
		switch(kInput->makeCode)
		{
			case PS2Set1MakeCode::LeftControl:
			case PS2Set1MakeCode::LeftShift:
			case PS2Set1MakeCode::LeftAlt:
			case PS2Set1MakeCode::LeftGUI: 
			case PS2Set1MakeCode::RightControl: 
			case PS2Set1MakeCode::RightShift:
			case PS2Set1MakeCode::RightAlt: 
			case PS2Set1MakeCode::RightGUI: 
			{
				u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(kInput->makeCode));
				_assert(modifierKey != 0);
				_assert((gModifierKeys & modifierKey) == modifierKey);
				gModifierKeys &= ~(modifierKey);
				break;
			}
		}
		_assert(gPressedKeys.erase(kInput->makeCode) == 1);
	}
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Keyboard };
	memcpy(&kmInputData.keyboardInput, keyboardInputData, sizeof(Win32::KeyboardInput));
	
	std::unique_lock<std::mutex> lock(gMutex);
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

static std::unique_ptr<Win32::Win32DrawSurface> gWin32DrawSurfaceUPtr;
/* NOTE: HDMIDecodeNetStream creates 2 threads - one for Receiving data from network and Another for Decoding/Converting 
 * Typically we would need 3 threads for HDMIDecodeNetStream 
 * 		1. One for receiving data from network 
 * 		2. Second for decoding the data
 * 		3. Third for converting the decoded data into RGB data */
static std::unique_ptr<HDMIDecodeNetStream> gHDMIDecodeNetStream;

static void WindowPaintHandler(void* paintInfo, void* userData)
{
	if(std::optional<FIFOPool<HDMIDecodeNetStream::FrameData>::ItemType> frame = gHDMIDecodeNetStream->borrowFrameData())
	{
		HDMIDecodeNetStream::FrameData& frameData = frame->first;
		_assert(frameData.getSize() == gWin32DrawSurfaceUPtr->getBufferSize());
		memcpy(gWin32DrawSurfaceUPtr->getPixels(), frameData.getPtr(), frameData.getSize());
		gHDMIDecodeNetStream->returnFrameData(*frame);
	}

	auto drawSurfaceSize = gWin32DrawSurfaceUPtr->getSize();
	Win32::WindowPaintInfo* winPaintInfo = reinterpret_cast<Win32::WindowPaintInfo*>(paintInfo);
	BitBlt(winPaintInfo->deviceContext, 0, 0, drawSurfaceSize.first, drawSurfaceSize.second, gWin32DrawSurfaceUPtr->getHDC(), 0, 0, SRCCOPY);
}

int main(int argc, const char* argv[])
{
	Win32::InitializeMediaFundationAndCOM();

	debug_log_info("Platform is Windows");

	/* dispatches 2 threads, one (decode thread) waiting on another (network thread) to get data from network  */
	gHDMIDecodeNetStream = std::move(std::unique_ptr<HDMIDecodeNetStream>(new HDMIDecodeNetStream(1920, 1080, 60, 1, 32)));

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });

	Window window(1920, 1080, "Scalable KVM Over IP");

	gWin32DrawSurfaceUPtr = std::move(std::unique_ptr<Win32::Win32DrawSurface>(new Win32::Win32DrawSurface(window.getNativeHandle(), window.getSize().first, window.getSize().second, 32)));

	Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);
	Event::SubscriptionHandle windowPaintHandle = window.getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, NULL);

	debug_log_info("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	/* pauses (acquires mutex from) the network thread and waiting for connecting with the server */
	if(gHDMIDecodeNetStream->connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
		debug_log_info("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// Network::Socket networkStream(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	// debug_log_info("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	// if(networkStream.connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
	// 	debug_log_info("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// std::thread networkThread(NetworkHandler, std::ref(networkStream));

	window.runGameLoop(static_cast<u32>(60));

	// networkThread.join();

	window.getEvent(Window::EventType::Paint).unsubscribe(windowPaintHandle);
	window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	Win32::DeinitializeMediaFoundationAndCOM();
	return 0;
}


#endif /* Client */
