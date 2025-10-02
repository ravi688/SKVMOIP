#include <SKVMOIP/HDMIEncodeNetStream.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/StopWatch.hpp>

namespace SKVMOIP
{
	HDMIEncodeNetStream::HDMIEncodeNetStream(Win32::Win32SourceDevice&& device, netsocket::Socket&& socket) : netsocket::AsyncSocket(std::move(socket)), m_device(std::move(device)), m_isValid(false)
	{
		m_hdmiStream = std::unique_ptr<VideoSourceStream>(new VideoSourceStream(m_device, VideoSourceStream::Usage::NV12Read, 
																					{
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
																					}));
		if(!(*m_hdmiStream))
		{
			debug_log_info("Failed to create VideoSourceStream");
			return;
		}
	
		m_hdmiStream->dump();
	
		std::pair<u32, u32> frameSize = m_hdmiStream->getOutputFrameSize();
		std::pair<u32, u32> frameRatePair = m_hdmiStream->getInputFrameRate();
		skvmoip_debug_assert((frameSize.first == 1920) && (frameSize.second == 1080));
		/* For just one Kreo HDMI capture device, this assertion is not true. But Why? */
		skvmoip_assert_wrn((frameRatePair.first == 60) && (frameRatePair.second == 1));
		u32 frameRate = m_hdmiStream->getInputFrameRateF32();
	
		m_nv12Buffer = buf_create(sizeof(u8), (frameSize.first * frameSize.second * 3) >> 1, 0);
		m_encoder = std::unique_ptr<Encoder>(new Encoder(frameSize.first, frameSize.second));
		m_isValid = true;
	}
	
	HDMIEncodeNetStream::~HDMIEncodeNetStream()
	{
		if(m_isValid)
		{
			m_isValid = false;
			buf_free(&m_nv12Buffer);
		}
	}
	
	void HDMIEncodeNetStream::start()
	{
		if(!m_isValid)
		{
			m_device.shutdown();
			return;
		}
		auto& socket = getSocket();
		if(!socket.isConnected())
		{
			DEBUG_LOG_INFO("Socket is not connected");
			return;
		}
	
		while(true)
		{
			buf_clear_buffer(&m_nv12Buffer, NULL);
			u8* buffer = reinterpret_cast<u8*>(buf_get_ptr(&m_nv12Buffer));
			u32 bufferSize = static_cast<u32>(buf_get_capacity(&m_nv12Buffer));
			/* Takes: 5 ms to 50 ms on Logitech Brio 4k webcam 
			 * 	 and: 5 ms to 11 ms on Kreo 1080p60fps HDMI capture card */
			if(!m_hdmiStream->readNV12FrameToBuffer(buffer, bufferSize))
				continue;
	
			u8* outputBuffer;
			u32 outputBufferSize;
			if(!m_encoder->encodeNV12(buffer, bufferSize, outputBuffer, outputBufferSize))
			{
				DEBUG_LOG_INFO("Failed to encode");
				continue;
			}
			else if(outputBuffer == NULL)
				continue;
	
			if(!isCanSendOrReceive())
			{
				DEBUG_LOG_INFO("Can't Send or Receive, Client socket is not responding, closing HDMI stream");
				break;
			}
			send(reinterpret_cast<u8*>(&outputBufferSize), sizeof(outputBufferSize));
			if(!isCanSendOrReceive())
			{
				DEBUG_LOG_INFO("Can't Send or Receive, Client socket is not responding, closing HDMI stream");
				break;
			}
			send(outputBuffer, outputBufferSize);
		}
		m_device.shutdown();
	}
}