
#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/Win32/Win32DrawSurface.hpp>

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>
#include <memory>

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>


using namespace SKVMOIP;

#define SERVER_IP_ADDRESS "192.168.1.113"
#define SERVER_PORT_NUMBER "2000"

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
static std::unique_ptr<VideoSourceStream> gHDMIStream;

static u32 counter = 0;

static void WindowPaintHandler(void* paintInfo, void* userData)
{
	u8* pixels = gWin32DrawSurfaceUPtr->getPixels();
	memset(pixels, 255, gWin32DrawSurfaceUPtr->getBufferSize());

	auto drawSurfaceSize = gWin32DrawSurfaceUPtr->getSize();
	// for(u32 i = 0; i < drawSurfaceSize.second; i++)
	// {
	// 	for(u32 j = 0; j < drawSurfaceSize.first; j++)
	// 	{
	// 		u32 offset = i * drawSurfaceSize.first * 4;
	// 		pixels[offset + j * 4 + 0] = 0; // B
	// 		pixels[offset + j * 4 + 1] = 0;   // G
	// 		pixels[offset + j * 4 + 2] = 255; // R
	// 		pixels[offset + j * 4 + 3] = 255;
	// 	}
	// }

	if(!gHDMIStream->readRGBFrameToBuffer(pixels, gWin32DrawSurfaceUPtr->getBufferSize()))
	{
		return;
	}
	// debug_log_info("Reading frame : %u", ++counter);

	Win32::WindowPaintInfo* winPaintInfo = reinterpret_cast<Win32::WindowPaintInfo*>(paintInfo);
	BitBlt(winPaintInfo->deviceContext, 0, 0, drawSurfaceSize.first, drawSurfaceSize.second, gWin32DrawSurfaceUPtr->getHDC(), 0, 0, SRCCOPY);
}

#ifdef BUILD_SERVER

static void HandleHDMIStream(Network::Protocols::USBToHDMIStreamControlProtocol& controlProtocol, u32 clientID, u32 usbPortNumber, std::condition_variable& sync)
{
	USBToHDMIStream hdmiStream(usbPortNumber);

	if(!hdmiStream.isValid())
		controlProtocol.sendMessage(STATUS_FAILED);
	else
		controlProtocol.sendMessage(STATUS_OK, clientID);

	Network::Socket listenSocket;
	while(listenSocket.listen())
	{
		Network::Socket streamSocket = listenSocket.accpet();
		Network::Protocols::USBToHDMIStreamProtocol streamProtocol(streamSocket);
		
		std::vector<u8> buffer;
		buffer.reserve(hdmiStream.getFrameSizeInBytes());

		while(!streamProtocol.shouldClose())
		{
			Encoding::ImageEncode(hdmiStream.getLatestFrame(), buffer);
			streamProtocol.sendFrame(STATUS_OK, buffer);
		}
	}
}

static void HandleNetworkConnection(Network::Socket& connectionSocket)
{
	// UHSCP Protocol
	Network::Protocols::USBToHDMIStreamControlProtocol controlSocket(connectionSocket);
	while(!controlSocket.shouldClose())
	{
		Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket messagePacket;
		if(controlSocket.peekMessage(messagePacket))
		{
			switch(messagePacket.message)
			{
				case Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket::MessageType::AvailableHDMIConnection:
				{
					std::optional<std::vector<u32>> usbPortNumbers = GetAvailableUSBToHDMIConnections();
					if(!usbPortNumbers)
						controlSocket.sendMessage(STATUS_FAILED);
					else
						controlSocket.sendMessage(STATUS_OK, usbPortNumbers);
					break;
				}
				case Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket::MessageType::StartHDMIStream:
				{
					u32 usbPortNumber = message.getUSBPortNumber();
					if(!isValid(usbPortNumber))
						controlSocket.sendMessage(STATUS_INVALID_USB_PORT_NUMBER);

					std::condition_variable hdmiStreamStartedSignal;
					u32 clientID = GenerateID();
					std::thread hdmiStreamThread(HandleHDMIStream, 
						std::tuple<Network::Protocols::USBToHDMIStreamControlProtocol, u32, usbPortNumber, std::condition_variable>(controlSocket, clientID, usbPortNumber, std::ref(hdmiStreamStartedSignal)));
					hdmiStreamThread.detach();

					hdmiStreamStartedSignal.wait(lock);
				}
			}
		}
	}
}

#endif /* Server */

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	std::optional<Win32::Win32SourceDeviceListGuard> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if(!deviceList)
	{
		debug_log_error("Unable to get Video source device list");
		return 0;
	}
			
	std::optional<Win32::Win32SourceDevice> device = deviceList->activateDevice(0);
	if(!device)
	{
		debug_log_error("Unable to create video source device with index: %lu ", 0);
		return 0;
	}

	std::vector<std::tuple<u32, u32, u32>> preferenceList = 
	{
		{ 1920, 1080, 60 },
		{ 1920, 1080, 30 },
		{ 1366, 768, 60 },
		{ 1366, 768, 30 },
		{ 1280, 720, 60 },
		{ 1280, 720, 30 },
		{ 1024, 768, 60 },
		{ 1024, 768, 30 }, 
		{ 960, 720, 60 },
		{ 960, 720, 30 }
	};

	gHDMIStream = std::move(std::unique_ptr<VideoSourceStream>(new VideoSourceStream(device.value(), preferenceList)));

	if (!(*gHDMIStream))
		return 0;

	if(auto t = gHDMIStream->isCompressedFormat())
		debug_log_info("IsCompressedFormat: %s", (t.value() == TRUE) ? "True" : "False");					

	auto frameSize = gHDMIStream->getFrameSize();
	if(frameSize)
		debug_log_info("Frame Size: %lu, %lu", frameSize->first, frameSize->second);

	f32 frameRate = 0;
	if(auto t = gHDMIStream->getFrameRate())
	{
		frameRate = t->first / static_cast<f32>(t->second);
		debug_log_info("Frame Rate %.2f", frameRate);
	}

	if(auto t = gHDMIStream->getSampleSizeInBytes())
		debug_log_info("Sample Size in Bytes: %lu", t.value());

	if(auto t = gHDMIStream->getEncodingFormatStr())
		debug_log_info("Encoding Format: %s", t.value());

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });


	Window window(frameSize->first, frameSize->second, "Scalable KVM Over IP");

	gWin32DrawSurfaceUPtr = std::move(std::unique_ptr<Win32::Win32DrawSurface>(new Win32::Win32DrawSurface(window.getNativeHandle(), window.getSize().first, window.getSize().second, 32)));

	Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);
	Event::SubscriptionHandle windowPaintHandle = window.getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, NULL);

#ifdef BUILD_SERVER

	std::vector<std::thread> concurrentConnections;

	Network::Socket listenSocket;
	while(listenSocket.listen())
	{
		Network::Socket connectionSocket = listenSocket.accept();
		concurrentConnections.push_back(std::thread(HandleNetworkConnection, std::ref(connectionSocket)));
	}

#endif /* Server */

	// Network::Socket networkStream(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	// debug_log_info("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	// if(networkStream.connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
	// 	debug_log_info("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// std::thread networkThread(NetworkHandler, std::ref(networkStream));

	window.runGameLoop(static_cast<u32>(frameRate));

	// networkThread.join();


	window.getEvent(Window::EventType::Paint).unsubscribe(windowPaintHandle);
	window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	return 0;
}

