#include <SKVMOIP/RDPSession.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/StopWatch.hpp>
#include <SKVMOIP/Protocol.hpp>
#include <chrono>

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
							   m_callbackUserData(NULL),
							   m_isValid(true)
	{
	}

	bool RDPSession::isConnected() 
	{
		assert(DESCRIPTION(m_isValid), "you're trying to use invalid RDPSession object, perhaps you have destroyed it in some another thread?");
		return m_isKMNetStreamConnected && m_kmNetStream->isCanSendOrReceive();
	}

	void RDPSession::setConnectionStatusCallback(void (*callback)(bool isUp, void* userData), void* userData)
	{
		assert(DESCRIPTION(m_isValid), "you're trying to use invalid RDPSession object, perhaps you have destroyed it in some another thread?");
		m_connectionStatusCallback = callback;
		m_callbackUserData = userData;
	}

	void RDPSession::connect(const char* kmIPAddress, const char* kmPortNumber, const char* vipAddress, const char* vPortNumber)
	{
		assert(DESCRIPTION(m_isValid), "you're trying to use invalid RDPSession object, perhaps you have destroyed it in some another thread?");
		m_kmNetStream = std::unique_ptr<KMNetStream>(new KMNetStream());
		m_kmConnectThread = std::unique_ptr<std::thread>(new std::thread([this](std::string&& kmIPAddress, std::string&& kmPortNumber, std::string&& vIPAddress, std::string&& vPortNumber)
			{
				// DEBUG_LOG_INFO("KeyMo: Trying to connect to %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
				// if(m_kmNetStream->connect(kmIPAddress.c_str(), kmPortNumber.c_str()) == netsocket::Result::Success)
				// 	DEBUG_LOG_INFO("KeyMo Connected to %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
				// else
				// {
				// 	DEBUG_LOG_ERROR("Failed to connect to KeyMo Server at %s:%s", kmIPAddress.c_str(), kmPortNumber.c_str());
				// 	m_kmNetStream->close();
				// 	m_kmNetStream.reset();
				// }

				m_controlSocket = std::unique_ptr<netsocket::Socket>(new netsocket::Socket(netsocket::SocketType::Stream, netsocket::IPAddressFamily::IPv4, netsocket::IPProtocol::TCP));
				DEBUG_LOG_INFO("Trying to connect to %s:%s", vIPAddress.c_str(), vPortNumber.c_str());
				if(m_controlSocket->connect(vIPAddress.c_str(), vPortNumber.c_str()) == netsocket::Result::Success)
				{
					DEBUG_LOG_INFO("Video Control Connected to %s:%s", vIPAddress.c_str(), vPortNumber.c_str());
					u8 socketType = EnumClassToInt(SocketType::Control);
					DEBUG_LOG_INFO("Sending Socket Type: Control");
					if(m_controlSocket->send(&socketType, sizeof(u8)) == netsocket::Result::Success)
					{
						DEBUG_LOG_INFO("Requesting client ID");
						if(m_controlSocket->receive(reinterpret_cast<u8*>(&m_clientID), sizeof(u32)) == netsocket::Result::Success)
						{
							DEBUG_LOG_INFO("ClientID Received: %lu", m_clientID);
							m_isKMNetStreamConnected = true;
							if(m_connectionStatusCallback != NULL)
								m_connectionStatusCallback(true, m_callbackUserData);
							return;
						}
						else DEBUG_LOG_ERROR("Failed to receive client id from video server");
					}
					else DEBUG_LOG_ERROR("Failed to send socket type: control");
				}
				else DEBUG_LOG_INFO("Failed to connect to Video Server at %s:%s", vIPAddress.c_str(), vPortNumber.c_str());
				if(m_connectionStatusCallback != NULL)
					m_connectionStatusCallback(false, m_callbackUserData);
			}, std::move(std::string(kmIPAddress)), std::move(std::string(kmPortNumber)), std::move(std::string(vipAddress)), std::move(std::string(vPortNumber))));
	}

	void RDPSession::start(const char* ipAddress, const char* portNumber, u8 deviceID)
	{
		assert(DESCRIPTION(m_isValid), "you're trying to use invalid RDPSession object, perhaps you have destroyed it in some another thread?");
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
		if(m_decodeNetStream->connect(ipAddress, portNumber) == netsocket::Result::Success)
		{
			DEBUG_LOG_INFO("Video Connected to %s:%s", ipAddress, portNumber);
			u8 socketType = EnumClassToInt(SocketType::Stream);
			DEBUG_LOG_INFO("Sending socket type: Stream: %u", socketType);
			bool isFailed = false;
			netsocket::Socket& socket = m_decodeNetStream->getSocket();
			if(socket.send(&socketType, sizeof(u8)) == netsocket::Result::Success)
			{
				DEBUG_LOG_INFO("Sending client id: %lu", m_clientID);
				if(socket.send(reinterpret_cast<u8*>(&m_clientID), sizeof(u32)) == netsocket::Result::Success)
				{
					DEBUG_LOG_INFO("Waiting for acknowledgement...");
					if((socket.receive(&socketType, sizeof(u8)) == netsocket::Result::Success) && (socketType == EnumClassToInt(Message::ACK)))
						DEBUG_LOG_INFO("Video Stream is acknowledged by the video server");
					else
					{
						DEBUG_LOG_ERROR("Failed to receive acknolwedgement");
						isFailed = true;
					}
				}
				else
				{
					DEBUG_LOG_ERROR("Failed to send client id");
					isFailed = true;
				}
			}
			else
			{
				DEBUG_LOG_INFO("Failed to send socket type: Stream");
				isFailed = true;
			};

			if(isFailed)
			{
				DEBUG_LOG_ERROR("Closing decodeNetStream");
				m_decodeNetStream->close();
				m_decodeNetStream.reset();
				return;
			}

			u8 message = EnumClassToInt(Message::Start);
			DEBUG_LOG_INFO("Requesting Stream Start...");
			if(m_controlSocket->send(&message, sizeof(u8)) == netsocket::Result::Success)
			{
				DEBUG_LOG_INFO("Sending device ID: %lu", deviceID);
				if(m_controlSocket->send(&deviceID, sizeof(u8)) == netsocket::Result::Success)
					DEBUG_LOG_INFO("Stream Device Connected!");
				else DEBUG_LOG_ERROR("Failed to send device ID: %lu", deviceID);
			}
			else DEBUG_LOG_ERROR("Failed to send stream start request");

			m_decodeNetStream->start();

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
			m_window->runGameLoop(static_cast<u32>(60), m_isValid);
			m_window->getEvent(Window::EventType::Paint).unsubscribe(m_windowPaintHandle);
			#else
			m_presentEngine->runGameLoop(static_cast<u32>(60), m_isValid);
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
		m_isValid = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if(m_window)
		{
			if(m_controlSocket)
				m_controlSocket->close();
			if(m_kmNetStream)
			{
				m_window->getEvent(Window::EventType::MouseInput).unsubscribe(m_mouseInputHandle);
				m_window->getEvent(Window::EventType::KeyboardInput).unsubscribe(m_keyboardInputHandle);
			}
		}
		if(m_kmConnectThread && m_kmConnectThread->joinable())
			m_kmConnectThread->join();
	}

	static void FullScreenHandler(void* keyCombPtr, void* userData)
	{
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& window = rdp.getWindow();
		static bool wasLocked = window->isLocked();
		if(!window->isFullScreen())
		{
			window->setFullScreen(true);
			wasLocked = window->isLocked();
			if(!wasLocked)
				window->lock(true);
		}
		else
		{
			window->setFullScreen(false);
			if(!wasLocked)
				window->lock(false);
		}
		
		if(rdp.isConnected())
		{
			std::vector<Win32::KeyboardInput>& keyComb = *reinterpret_cast<std::vector<Win32::KeyboardInput>*>(keyCombPtr);
			skvmoip_debug_assert(keyComb.size() >= 2);
			auto& kmNetStream = rdp.getKMNetStream();
			for(std::size_t i = 0; i < (keyComb.size() - 1); i++)
			{
				Win32::KeyboardInput keyboardInput = keyComb[i];
				keyboardInput.keyStatus	= Win32::KeyStatus::Released;
				kmNetStream->sendKeyboardInput(keyboardInput);
			}
		}
		debug_log_info("Full Screen Toggled");
	}

	static void LockHandler(void* keyCombPtr, void* userData)
	{
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& window = rdp.getWindow();
		window->lock(!window->isLocked());

		if(rdp.isConnected())
		{
			std::vector<Win32::KeyboardInput>& keyComb = *reinterpret_cast<std::vector<Win32::KeyboardInput>*>(keyCombPtr);
			skvmoip_debug_assert(keyComb.size() >= 2);
			auto& kmNetStream = rdp.getKMNetStream();
			for(std::size_t i = 0; i < (keyComb.size() - 1); i++)
			{
				Win32::KeyboardInput keyboardInput = keyComb[i];
				keyboardInput.keyStatus	= Win32::KeyStatus::Released;
				kmNetStream->sendKeyboardInput(keyboardInput);
			}
		}
		debug_log_info("Lock toggled");
	}

	static void MouseInputHandler(void* mouseInputData, void* userData)
	{
		static bool isInvoked = false;
		skvmoip_debug_assert(mouseInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		if(!rdp.getWindow()->isLocked())
			return;
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
		skvmoip_debug_assert(keyboardInputData != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		if(!rdp.getWindow()->isLocked())
			return;
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
		skvmoip_debug_assert(paintInfo != NULL);
		RDPSession& rdp = *reinterpret_cast<RDPSession*>(userData);
		auto& decodeNetStream = rdp.getDecodeNetStream();
		auto& drawSurface = rdp.getDrawSurface();

		if(auto frame = decodeNetStream->borrowFrameData())
		{
			// debug_log_info("FrameData borrowed");
			auto frameData = FIFOPool<HDMIDecodeNetStream::FrameData>::GetValue(frame);
			skvmoip_debug_assert(frameData.has_value());
			skvmoip_debug_assert(frameData->getSize() == drawSurface->getBufferSize());
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
