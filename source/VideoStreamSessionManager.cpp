#include <SKVMOIP/VideoStreamSessionManager.hpp>
#include <SKVMOIP/assert.h>

#undef _ASSERT
#include <spdlog/spdlog.h>

namespace SKVMOIP
{
	VideoStreamSessionManager::VideoStreamSessionManager()
	{
		m_videoSourceManager = std::make_unique<ABSTRACT_VIDEO_SOURCE_MANAGER>();
	}

	com::OptionalReference<std::unique_ptr<VideoStreamSession>> VideoStreamSessionManager::createSessionForClient(ClientID clientID, IVideoSource::DeviceID deviceID, netsocket::AsyncSocket& socket)
	{
		std::optional<std::unique_ptr<ABSTRACT_VIDEO_SOURCE>>videoSource = m_videoSourceManager->acquireVideoSource(deviceID, IVideoSource::Usage::NV12Read, std::vector<std::tuple<u32, u32, u32>> {
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
																					});
		if(!videoSource)
		{
			spdlog::error("Failed to acquire video source for device id: {}", deviceID);
			return { };
		}
		std::unique_ptr<VideoStreamSession> session = std::make_unique<VideoStreamSession>(*videoSource.value(), socket);
		session->start();
		SessionContext sessionContext
		{
			.videoSource = std::move(*videoSource),
			.streamSession = std::move(session)
		};
		m_sessionContexts.insert({ clientID, std::move(sessionContext) });
		auto& sessionRef = m_sessionContexts.at(clientID).streamSession;
		return { sessionRef };
	}

	com::OptionalReference<std::unique_ptr<VideoStreamSession>> VideoStreamSessionManager::getSessionForClient(ClientID clientID)
	{
		auto it = m_sessionContexts.find(clientID);
		if(it != m_sessionContexts.end())
			return { it->second.streamSession };
		return { };
	}

	void VideoStreamSessionManager::destroySessionForClient(ClientID clientID)
	{
		auto it = m_sessionContexts.find(clientID);
		if(it != m_sessionContexts.end())
		{
			// Stop the video stream
			auto& context = it->second;
			context.streamSession->stop();
			m_videoSourceManager->releaseVideoSource(context.videoSource);
			
			// Destroy the video stream object
			m_sessionContexts.erase(it);
		}
		else
		{
			spdlog::error("Invalid call to destroySessionForClient(), No session was ever created for client {}", clientID);
		}
	}
}
