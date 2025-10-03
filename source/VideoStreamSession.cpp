#include <SKVMOIP/VideoStreamSession.hpp>

#include <SKVMOIP/assert.h>

#undef _ASSERT
#include <spdlog/spdlog.h>

namespace SKVMOIP
{
	VideoStreamSession::VideoStreamSession(IVideoSource& videoSource, netsocket::AsyncSocket& asyncSocket)
				: m_videoSource(videoSource), m_asyncSocket(asyncSocket)
	{
		std::pair<u32, u32> frameRatePair = m_videoSource.getInputFrameRate();
		/* For just one Kreo HDMI capture device, this assertion is not true. But Why? */
		skvmoip_assert_wrn((frameRatePair.first == 60) && (frameRatePair.second == 1));
	
		std::pair<u32, u32> frameSize = m_videoSource.getOutputFrameSize();
		skvmoip_assert((frameSize.first == 1920) && (frameSize.second == 1080));
		m_encoder = std::make_unique<Encoder>(frameSize.first, frameSize.second);
		m_nv12Buffer = buf_create(sizeof(u8), (frameSize.first * frameSize.second * 3) >> 1, 0);
	}


	VideoStreamSession::~VideoStreamSession()
	{
		if(isRunning())
			stop();
		buf_free(&m_nv12Buffer);
	}
		
	void VideoStreamSession::start()
	{
		m_thread = std::thread(&VideoStreamSession::processStream, this);
		m_isStopThread = false;
	}

	void VideoStreamSession::stop()
	{
		skvmoip_assert(isRunning() && "First call start() then stop() will work");
		if(m_thread.joinable())
		{
			m_isStopThread = true;
			m_thread.join();
		}
		auto result = m_asyncSocket.finish();
		skvmoip_assert(result == netsocket::Result::Success && "AsyncSocket coudn't finish its work");
	}

	void VideoStreamSession::processStream()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		skvmoip_assert(m_asyncSocket.isConnected() && "Socket is not connected");
	
		while(!m_isStopThread)
		{
			if(!m_videoSource.isReady())
			{
				spdlog::error("Video Source not ready, stopping video stream");
				break;
			}

			buf_clear_buffer(&m_nv12Buffer, NULL);

			// Read NV12 Frames from the Video Source
			u8* buffer = reinterpret_cast<u8*>(buf_get_ptr(&m_nv12Buffer));
			u32 bufferSize = static_cast<u32>(buf_get_capacity(&m_nv12Buffer));
			if(!m_videoSource.readNV12FrameToBuffer(buffer, bufferSize))
				continue;
	
			// Encode NV12 data
			u8* outputBuffer;
			u32 outputBufferSize;
			if(!m_encoder->encodeNV12(buffer, bufferSize, outputBuffer, outputBufferSize))
			{
				spdlog::warn("Failed to encode");
				continue;
			}
			else if(outputBuffer == NULL)
			{
				spdlog::warn("encode output buffer is NULL");
				continue;
			}

			// Send encoded data over network socket
			if(m_asyncSocket.isCanSendOrReceive())
			{
				m_asyncSocket.send(reinterpret_cast<u8*>(&outputBufferSize), sizeof(outputBufferSize));
				m_asyncSocket.send(outputBuffer, outputBufferSize);
			}
			else
			{
				spdlog::error("Can't Send or Receive, socket is not responding, stopping video stream");
				break;
			}
		}
		m_isStopThread = true;
	}
}
