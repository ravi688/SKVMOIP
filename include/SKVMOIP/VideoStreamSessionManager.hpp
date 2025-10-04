#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/VideoStreamSession.hpp>

#include <common/platform.h>
#include <common/defines.hpp>

#include <unordered_map>
#include <vector>
#include <memory>

#ifdef PLATFORM_WINDOWS
#	include <SKVMOIP/VideoSourceManagerWindows.hpp>
#	define ABSTRACT_VIDEO_SOURCE VideoSourceWindows
#	define ABSTRACT_VIDEO_SOURCE_MANAGER VideoSourceManagerWindows
#else // PLATFORM_LINUX
#	include <SKVMOIP/VideoSourceManagerLinux.hpp>
#	define ABSTRACT_VIDEO_SOURCE VideoSourceLinux
#	define ABSTRACT_VIDEO_SOURCE_MANAGER VideoSourceManagerLinux
#endif

namespace SKVMOIP
{
	using ClientID = u32;
	class VideoStreamSessionManager final
	{
	private:
		struct SessionContext
		{
			std::unique_ptr<ABSTRACT_VIDEO_SOURCE> videoSource;
			std::unique_ptr<VideoStreamSession> streamSession;
		};
		std::unordered_map<ClientID, SessionContext> m_sessionContexts;

		std::unique_ptr<ABSTRACT_VIDEO_SOURCE_MANAGER> m_videoSourceManager;

	public:
		VideoStreamSessionManager();
		~VideoStreamSessionManager() = default;

		com::OptionalReference<std::unique_ptr<VideoStreamSession>> createSessionForClient(ClientID clientID, IVideoSource::DeviceID deviceID, netsocket::AsyncSocket& socket);
		com::OptionalReference<std::unique_ptr<VideoStreamSession>> getSessionForClient(ClientID clientID);
		void destroySessionForClient(ClientID clientID);
	};
}
