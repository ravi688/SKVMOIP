#include <SKVMOIP/RDPSession.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/StopWatch.hpp>

namespace SKVMOIP
{
	static void FullScreenHandler(void* keyComb, void* userData);
	static void LockHandler(void* keyComb, void* userData);
	static void MouseInputHandler(void* mouseInputData, void* userData);
	static void KeyboardInputHandler(void* keyboardInputData, void* userData);
	#ifndef USE_VULKAN_PRESENTATION
	static void WindowPaintHandler(void* paintInfo, void* userData);
	#endif
	#ifdef USE_VULKAN_PRESENTATION
	static bool PresentHandler(void* paintInfo, void* userData);
	#endif

	RDPSession::RDPSession() : m_keyboardInputHandle(Event::GetInvalidSubscriptionHandle()), 
							   m_mouseInputHandle(Event::GetInvalidSubscriptionHandle()),
							   m_windowPaintHandle(Event::GetInvalidSubscriptionHandle()),
							   m_isKMNetStreamConnected(false),
							   m_connectionStatusCallback(NULL),
							   m_callbackUserData(NULL)
	{

	}

	bool RDPSession::isConnected() 
	{
		return m_isKMNetStreamConnected && m_kmNetStream->isCanSendOrReceive();
	}

	void RDPSession::setConnectionStatusCallback(void (*callback)(bool isUp, void* userData), void* userData)
	{
		m_connectionStatusCallback = callback;
		m_callbackUserData = userData;
	}

	void RDPSession::connect(const char* kmIPAddress, const char* kmPortNumber)
	{
		m_kmNetStream = std::unique_ptr<KMNetStream>(new KMNetStream());
		m_kmConnectThread = std::unique_ptr<std::thread>(new std::thread([this](std::string kmIPAddress, std::string kmPortNumber)
			{
				DEBUG_LOG_INFO("KeyMo: Trying to connect to %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
				if(m_kmNetStream->connect(kmIPAddress.c_str(), kmPortNumber.c_str()) == Network::Result::Success)
				{
					m_isKMNetStreamConnected = true;
					DEBUG_LOG_INFO("KeyMo Connected to %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
					if(m_connectionStatusCallback != NULL)
						m_connectionStatusCallback(true, m_callbackUserData);
				}
				else
				{
					DEBUG_LOG_ERROR("Failed to connect to KeyMo Server at %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
					if(m_connectionStatusCallback != NULL)
						m_connectionStatusCallback(false, m_callbackUserData);
				}
			}, std::string(kmIPAddress), std::string(kmPortNumber)));
	}

	void RDPSession::start(const char* ipAddress, const char* portNumber)
	{
		/* if previous video session was connected, then destroy already constructed objects */
		#ifdef USE_VULKAN_PRESENTATION
		m_presentEngine.reset();
		#else
		m_drawSurface.reset();
		#endif
		m_window.reset();
		m_decodeNetStream.reset();
		
		m_decodeNetStream = std::unique_ptr<HDMIDecodeNetStream>(new HDMIDecodeNetStream(1920, 1080, 60, 1, 32));
		DEBUG_LOG_INFO("Trying to connect to %s:%s", ipAddress, portNumber);
		/* pauses (acquires mutex from) the network thread and waiting for connecting with the server */
		if(m_decodeNetStream->connect(ipAddress, portNumber) == Network::Result::Success)
		{
			DEBUG_LOG_INFO("Video Connected to %s:%s", ipAddress, portNumber);
			m_window = std::move(std::unique_ptr<Window>(new Window(1920, 1080, "Scalable KVM Over IP")));
			#ifdef USE_VULKAN_PRESENTATION
			m_presentEngine = std::move(std::unique_ptr<PresentEngine>(new PresentEngine(*m_window, *m_decodeNetStream)));
			#else
			m_drawSurface = std::move(std::unique_ptr<Win32::Win32DrawSurface>(new Win32::Win32DrawSurface(m_window->getNativeHandle(), 1920, 1080, 32)));
			#endif
			m_window->show();
			#if defined(USE_DIRECT_FRAME_DATA_COPY) && defined(USE_VULKAN_PRESENTATION)
			m_decodeNetStream->addFrameDataStorage(m_presentEngine->getBufferPtr());
			#endif
			
			if(m_kmNetStream)
			{
				m_keyboardInputHandle = m_window->getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, reinterpret_cast<void*>(this));
				m_mouseInputHandle = m_window->getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, reinterpret_cast<void*>(this));
			}
			
			m_window->createKeyCombinationEvent({ Win32::KeyCode::KEYCODE_CTRL, Win32::KeyCode::KEYCODE_ALT, Win32::KEYCODE_RETURN_ENTER }).subscribe(FullScreenHandler, reinterpret_cast<void*>(this));
			m_window->createKeyCombinationEvent({ Win32::KeyCode::KEYCODE_CTRL, Win32::KeyCode::KEYCODE_ALT, Win32::KeyCode::KEYCODE_K }).subscribe(LockHandler, reinterpret_cast<void*>(this));
			#ifndef USE_VULKAN_PRESENTATION
			m_windowPaintHandle = m_window->getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, reinterpret_cast<void*>(this));
			m_window->runGameLoop(static_cast<u32>(60));
			m_window->getEvent(Window::EventType::Paint).unsubscribe(m_windowPaintHandle);
			#else
			m_presentEngine->runGameLoop(static_cast<u32>(60));
			#endif
			
			if(m_kmNetStream)
			{
				m_window->getEvent(Window::EventType::MouseInput).unsubscribe(m_mouseInputHandle);
				m_window->getEvent(Window::EventType::KeyboardInput).unsubscribe(m_keyboardInputHandle);
			}

			#ifdef USE_VULKAN_PRESENTATION
			m_presentEngine.reset();
			#else
			m_drawSurface.reset();
			#endif
			m_window.reset();
			m_decodeNetStream.reset();
		}
		else
			DEBUG_LOG_ERROR("Failed to connect to Video Server at %s:%s", ipAddress, portNumber);
	}

	RDPSession::~RDPSession()
	{
		if(m_window)
		{
			if(m_kmConnectThread && m_kmConnectThread->joinable())
				m_kmConnectThread->join();
		}
	}

	static void FullScreenHandler(void* keyCombPtr, void* userData)
	{
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& window = rdp.getWindow();
		if(!window->isFullScreen())
		{
			window->setFullScreen(true);
			window->showCursor(false);
		}
		else
		{
			window->setFullScreen(false);
			window->showCursor(true);
		}
		
		std::vector<Win32::KeyboardInput>& keyComb = *reinterpret_cast<std::vector<Win32::KeyboardInput>*>(keyCombPtr);
		_assert(keyComb.size() >= 2);
		auto& kmNetStream = rdp.getKMNetStream();
		for(std::size_t i = 0; i < (keyComb.size() - 1); i++)
		{
			Win32::KeyboardInput keyboardInput = keyComb[i];
			keyboardInput.keyStatus	= Win32::KeyStatus::Released;
			kmNetStream->sendKeyboardInput(keyboardInput);
		}
		debug_log_info("Full Screen Toggled");
	}

	static void LockHandler(void* keyComb, void* userData)
	{
		debug_log_info("Lock toggled");
	}

	static void MouseInputHandler(void* mouseInputData, void* userData)
	{
		static bool isInvoked = false;
		_assert(mouseInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& kmNetStream = rdp.getKMNetStream();
		if(rdp.isConnected())
		{
			rdp.getKMNetStream()->sendMouseInput(*reinterpret_cast<Win32::MouseInput*>(mouseInputData));
			isInvoked = false;
		}
		else if(!isInvoked && (rdp.m_connectionStatusCallback != NULL))
		{
			rdp.m_connectionStatusCallback(false, rdp.m_callbackUserData);
			isInvoked = true;
		}
	}

	static void KeyboardInputHandler(void* keyboardInputData, void* userData)
	{
		static bool isInvoked = false;
		_assert(keyboardInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& kmNetStream = rdp.getKMNetStream();
		if(rdp.isConnected())
		{
			rdp.getKMNetStream()->sendKeyboardInput(*reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData));
			isInvoked = false;
		}
		else if(!isInvoked && (rdp.m_connectionStatusCallback != NULL))
		{
			rdp.m_connectionStatusCallback(false, rdp.m_callbackUserData);
			isInvoked = true;
		}
	}

	
	#ifndef USE_VULKAN_PRESENTATION
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
			/* Takes: 1 ms to 4 ms */
			memcpy(drawSurface->getPixels(), frameData->getPtr(), frameData->getSize());
			decodeNetStream->returnFrameData(frame);
			// debug_log_info("FrameData returned");
		}
	
		auto drawSurfaceSize = drawSurface->getSize();
		Win32::WindowPaintInfo* winPaintInfo = reinterpret_cast<Win32::WindowPaintInfo*>(paintInfo);
		SKVMOIP::StopWatch stopwatch;
		/* Takes: 1 ms to 5 ms*/
		BitBlt(winPaintInfo->deviceContext, 0, 0, drawSurfaceSize.first, drawSurfaceSize.second, drawSurface->getHDC(), 0, 0, SRCCOPY);
		auto t = stopwatch.stop();
	}
	#endif
}
