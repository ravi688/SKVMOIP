#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/HDMIDecodeNetStream.hpp>
#include <SKVMOIP/KMNetStream.hpp>
#include <SKVMOIP/Win32/Win32DrawSurface.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Event.hpp>
#include <memory>

namespace SKVMOIP
{
	static void MouseInputHandler(void* mouseInputData, void* userData);
	static void KeyboardInputHandler(void* keyboardInputData, void* userData);
	class RDPSession
	{
		friend void MouseInputHandler(void* mouseInputData, void* userData);
		friend void KeyboardInputHandler(void* keyboardInputData, void* userData);

		private:
			std::unique_ptr<HDMIDecodeNetStream> m_decodeNetStream;
			std::unique_ptr<KMNetStream> m_kmNetStream;
			std::unique_ptr<Win32::Win32DrawSurface> m_drawSurface;
			std::unique_ptr<Window> m_window;
			Event::SubscriptionHandle m_keyboardInputHandle;
			Event::SubscriptionHandle m_mouseInputHandle;
			Event::SubscriptionHandle m_windowPaintHandle;
			std::atomic<bool> m_isKMNetStreamConnected;
			std::unique_ptr<std::thread> m_kmConnectThread;

			void (*m_connectionStatusCallback)(bool isUp, void* userData);
			void* m_callbackUserData;
	
		public:
			RDPSession();
			RDPSession(RDPSession&&) = delete;
			RDPSession(RDPSession&) = delete;
			~RDPSession();
	
			bool isConnected();
			void setConnectionStatusCallback(void (*callback)(bool isUp, void* userData), void* userData);
			void connect(const char* kmipAddress, const char* kmPortNumber);
			void start(const char* ipAddress, const char* portNumber);
	
			std::unique_ptr<HDMIDecodeNetStream>& getDecodeNetStream() noexcept { return m_decodeNetStream; }
			std::unique_ptr<KMNetStream>& getKMNetStream() noexcept { return m_kmNetStream; }
			std::unique_ptr<Win32::Win32DrawSurface>& getDrawSurface() noexcept { return m_drawSurface; }
			std::unique_ptr<Window>& getWindow() noexcept { return m_window; }
	};
}
