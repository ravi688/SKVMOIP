#ifndef BUILD_CLIENT
#error "BUILD_CLIENT is not defined, but still main.client.cpp is being built"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/Window.hpp>

#include <SKVMOIP/HDMIDecodeNetStream.hpp>
#include <SKVMOIP/KMNetStream.hpp>
#include <SKVMOIP/Win32/Win32DrawSurface.hpp>

using namespace SKVMOIP;

#define SERVER_IP_ADDRESS "192.168.1.11"
#define SERVER_PORT_NUMBER "2020"

static std::unique_ptr<Win32::Win32DrawSurface> gWin32DrawSurfaceUPtr;
/* NOTE: HDMIDecodeNetStream creates 2 threads - one for Receiving data from network and Another for Decoding/Converting 
 * Typically we would need 3 threads for HDMIDecodeNetStream 
 * 		1. One for receiving data from network 
 * 		2. Second for decoding the data
 * 		3. Third for converting the decoded data into RGB data */
static std::unique_ptr<HDMIDecodeNetStream> gHDMIDecodeNetStream;

static std::unique_ptr<KMNetStream> gKMNetStream;

static void MouseInputHandler(void* mouseInputData, void* userData)
{
	_assert(mouseInputData != NULL);
	gKMNetStream->sendMouseInput(*reinterpret_cast<Win32::MouseInput*>(mouseInputData));
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	_assert(keyboardInputData != NULL);
	gKMNetStream->sendKeyboardInput(*reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData));
}

static void WindowPaintHandler(void* paintInfo, void* userData)
{
	if(auto frame = gHDMIDecodeNetStream->borrowFrameData())
	{
		// debug_log_info("FrameData borrowed");
		auto frameData = FIFOPool<HDMIDecodeNetStream::FrameData>::GetValue(frame);
		_assert(frameData.has_value());
		_assert(frameData->getSize() == gWin32DrawSurfaceUPtr->getBufferSize());
		memcpy(gWin32DrawSurfaceUPtr->getPixels(), frameData->getPtr(), frameData->getSize());
		gHDMIDecodeNetStream->returnFrameData(frame);
		// debug_log_info("FrameData returned");
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
	gKMNetStream = std::move(std::unique_ptr<KMNetStream>(new KMNetStream()));

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });

	Window window(1920, 1080, "Scalable KVM Over IP");

	gWin32DrawSurfaceUPtr = std::move(std::unique_ptr<Win32::Win32DrawSurface>(new Win32::Win32DrawSurface(window.getNativeHandle(), window.getSize().first, window.getSize().second, 32)));

	// Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	// Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);
	Event::SubscriptionHandle windowPaintHandle = window.getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, NULL);

	DEBUG_LOG_INFO("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	/* pauses (acquires mutex from) the network thread and waiting for connecting with the server */
	if(gHDMIDecodeNetStream->connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
		DEBUG_LOG_INFO("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// Network::Socket networkStream(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	// debug_log_info("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	// if(networkStream.connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
	// 	debug_log_info("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// std::thread networkThread(NetworkHandler, std::ref(networkStream));

	window.runGameLoop(static_cast<u32>(60));

	// networkThread.join();

	window.getEvent(Window::EventType::Paint).unsubscribe(windowPaintHandle);
	// window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	// window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	Win32::DeinitializeMediaFoundationAndCOM();
	return 0;
}
