#include <SKVMOIP/RDPSession.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	static void MouseInputHandler(void* mouseInputData, void* userData);
	static void KeyboardInputHandler(void* keyboardInputData, void* userData);
	static void WindowPaintHandler(void* paintInfo, void* userData);

	void RDPSession::start(const char* ipAddress, const char* portNumber)
	{
		m_decodeNetStream = std::unique_ptr<HDMIDecodeNetStream>(new HDMIDecodeNetStream(1920, 1080, 60, 1, 32));
		m_kmNetStream = std::unique_ptr<KMNetStream>(new KMNetStream());
		DEBUG_LOG_INFO("Trying to connect to %s:%s", ipAddress, portNumber);
		/* pauses (acquires mutex from) the network thread and waiting for connecting with the server */
		if(m_decodeNetStream->connect(ipAddress, portNumber) == Network::Result::Success)
		{
			DEBUG_LOG_INFO("Connected to %s:%s", ipAddress, portNumber);
			if(!m_window)
			{
				m_window = std::move(std::unique_ptr<Window>(new Window(1920, 1080, "Scalable KVM Over IP")));
				m_keyboardInputHandle = m_window->getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, reinterpret_cast<void*>(this));
				m_mouseInputHandle = m_window->getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, reinterpret_cast<void*>(this));
				m_windowPaintHandle = m_window->getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, reinterpret_cast<void*>(this));
			}
			if(!m_drawSurface)
			{
				m_drawSurface = std::move(std::unique_ptr<Win32::Win32DrawSurface>(
							new Win32::Win32DrawSurface(m_window->getNativeHandle(), m_window->getSize().first, m_window->getSize().second, 32)));
			}
		}
	
		m_window->runGameLoop(static_cast<u32>(60));
	
		m_window->getEvent(Window::EventType::Paint).unsubscribe(m_windowPaintHandle);
		m_window->getEvent(Window::EventType::MouseInput).unsubscribe(m_mouseInputHandle);
		m_window->getEvent(Window::EventType::KeyboardInput).unsubscribe(m_keyboardInputHandle);
	}

	static void MouseInputHandler(void* mouseInputData, void* userData)
	{
		_assert(mouseInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		rdp.getKMNetStream()->sendMouseInput(*reinterpret_cast<Win32::MouseInput*>(mouseInputData));
	}

	static void KeyboardInputHandler(void* keyboardInputData, void* userData)
	{
		_assert(keyboardInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		rdp.getKMNetStream()->sendKeyboardInput(*reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData));
	}
	
	static void WindowPaintHandler(void* paintInfo, void* userData)
	{
		_assert(paintInfo != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& decodeNetStream = rdp.getDecodeNetStream();
		auto& drawSurface = rdp.getDrawSurface();
	
		if(auto frame = decodeNetStream->borrowFrameData())
		{
			// debug_log_info("FrameData borrowed");
			auto frameData = FIFOPool<HDMIDecodeNetStream::FrameData>::GetValue(frame);
			_assert(frameData.has_value());
			_assert(frameData->getSize() == drawSurface->getBufferSize());
			memcpy(drawSurface->getPixels(), frameData->getPtr(), frameData->getSize());
			decodeNetStream->returnFrameData(frame);
			// debug_log_info("FrameData returned");
		}
	
		auto drawSurfaceSize = drawSurface->getSize();
		Win32::WindowPaintInfo* winPaintInfo = reinterpret_cast<Win32::WindowPaintInfo*>(paintInfo);
		BitBlt(winPaintInfo->deviceContext, 0, 0, drawSurfaceSize.first, drawSurfaceSize.second, drawSurface->getHDC(), 0, 0, SRCCOPY);
	}
}
