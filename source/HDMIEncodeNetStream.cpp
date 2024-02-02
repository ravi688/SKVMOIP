#include <SKVMOIP/HDMIEncodeNetStream.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	HDMIEncodeNetStream::HDMIEncodeNetStream(Win32::Win32SourceDevice&& device, Network::Socket&& socket) : AsyncQueueSocket(std::move(socket)), m_device(std::move(device))
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
		_assert((frameSize.first == 1920) && (frameSize.second == 1080));
		_assert((frameRatePair.first == 60) && (frameRatePair.second == 1));
		u32 frameRate = m_hdmiStream->getInputFrameRateF32();
	
		m_nv12Buffer = buf_create(sizeof(u8), (frameSize.first * frameSize.second * 3) >> 1, 0);
		m_encoder = std::unique_ptr<Encoder>(new Encoder(frameSize.first, frameSize.second));
	}
	
	HDMIEncodeNetStream::~HDMIEncodeNetStream()
	{
		buf_free(&m_nv12Buffer);
	}
	
	void HDMIEncodeNetStream::start()
	{
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