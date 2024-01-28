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
	class RDPSession
	{
		private:
			std::unique_ptr<HDMIDecodeNetStream> m_decodeNetStream;
			std::unique_ptr<KMNetStream> m_kmNetStream;
			std::unique_ptr<Win32::Win32DrawSurface> m_drawSurface;
			std::unique_ptr<Window> m_window;
			Event::SubscriptionHandle m_keyboardInputHandle;
			Event::SubscriptionHandle m_mouseInputHandle;
			Event::SubscriptionHandle m_windowPaintHandle;
	
	
		public:
			RDPSession() = default;
			RDPSession(RDPSession&&) = delete;
			RDPSession(RDPSession&) = delete;
			~RDPSession() = default;
	
			void start(const char* ipAddress, const char* portNumber);
	
			std::unique_ptr<HDMIDecodeNetStream>& getDecodeNetStream() noexcept { return m_decodeNetStream; }
			std::unique_ptr<KMNetStream>& getKMNetStream() noexcept { return m_kmNetStream; }
			std::unique_ptr<Win32::Win32DrawSurface>& getDrawSurface() noexcept { return m_drawSurface; }
			std::unique_ptr<Window>& getWindow() noexcept { return m_window; }
	};
}
