#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/Encoder.hpp>

#include <netsocket/netasyncsocket.hpp> // for netsocket::AsyncSocket
#include <bufferlib/buffer.h>

#include <memory>

namespace SKVMOIP
{
	class HDMIEncodeNetStream : public netsocket::AsyncSocket
	{
	private:
		std::unique_ptr<VideoSourceStream> m_hdmiStream;
		std::unique_ptr<Encoder> m_encoder;
		buffer_t m_nv12Buffer;
		Win32::Win32SourceDevice m_device;
		bool m_isValid;

	public:
		HDMIEncodeNetStream(Win32::Win32SourceDevice&& device, netsocket::Socket&& socket);
		HDMIEncodeNetStream(HDMIEncodeNetStream&& stream) = delete;
		HDMIEncodeNetStream& operator=(HDMIEncodeNetStream&& stream) = delete;
		HDMIEncodeNetStream(HDMIEncodeNetStream& stream) = delete;
		HDMIEncodeNetStream& operator=(HDMIEncodeNetStream& stream) = delete;
	
		~HDMIEncodeNetStream();
	
		void start();
		void stop() { close(); }
	};
}