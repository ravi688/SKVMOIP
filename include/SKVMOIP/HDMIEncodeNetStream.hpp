#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/Encoder.hpp>
#include <bufferlib/buffer.h>
#include <memory>

namespace SKVMOIP
{
	class HDMIEncodeNetStream : public Network::AsyncQueueSocket
	{
	private:
		std::unique_ptr<VideoSourceStream> m_hdmiStream;
		std::unique_ptr<Encoder> m_encoder;
		buffer_t m_nv12Buffer;
	
	public:
		HDMIEncodeNetStream(Win32::Win32SourceDevice& device, Network::Socket&& socket);
		HDMIEncodeNetStream(HDMIEncodeNetStream&& stream) = delete;
		HDMIEncodeNetStream& operator=(HDMIEncodeNetStream&& stream) = delete;
		HDMIEncodeNetStream(HDMIEncodeNetStream& stream) = delete;
		HDMIEncodeNetStream& operator=(HDMIEncodeNetStream& stream) = delete;
	
		~HDMIEncodeNetStream();
	
		void start();
	};
}