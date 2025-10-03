#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/VideoStreamSession.hpp>

#include <common/platform.h>
#include <common/defines.hpp>

#ifdef PLATFORM_WINDOWS
#	include <SKVMOIP/Win32/Win32.hpp>
#	include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#	include <SKVMOIP/VideoSourceWindows.hpp>
#else // PLATFORM_LINUX
#	include <SKVMOIP/VideoSourceLinux.hpp>
#endif

#include <unordered_map>
#include <vector>
#include <memory>

namespace SKVMOIP
{
	using ClientID = u32;
	using VideoSourceDeviceID = u64;
	class VideoStreamSessionManager
	{
	private:
		struct SessionContext
		{
			#ifdef PLATFORM_WINDOWS
			Win32::Win32SourceDevice device;
			std::unique_ptr<VideoSourceWindows> videoSource;
			#else // PLATFORM_LINUX
			std::unique_ptr<VideoSourceLinux> videoSource;
			#endif
			std::unique_ptr<VideoStreamSession> streamSession;
		};
		std::unordered_map<ClientID, SessionContext> m_sessionContexts;

		std::vector<VideoSourceDeviceID> m_availableDevices;
		std::unordered_map<VideoSourceDeviceID, VideoSourceDeviceID> m_deviceIDMap;

#ifdef PLATFORM_WINDOWS
		std::unique_ptr<Win32::Win32SourceDeviceListGuard> m_deviceList;
#endif

	public:
		VideoStreamSessionManager();
		~VideoStreamSessionManager();

		com::OptionalReference<std::unique_ptr<VideoStreamSession>> createSessionForClient(ClientID clientID, VideoSourceDeviceID deviceID, netsocket::AsyncSocket& socket);
		com::OptionalReference<std::unique_ptr<VideoStreamSession>> getSessionForClient(ClientID clientID);
		void destroySessionForClient(ClientID clientID);
	};
}
