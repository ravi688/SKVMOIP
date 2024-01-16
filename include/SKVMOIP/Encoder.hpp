#pragma once

#include <SKVMOIP/defines.hpp>

#include <x264/include/x264.h>
#include <x264/include/x264_config.h>

namespace SKVMOIP
{
	class Encoder
	{
	private:
		x264_param_t param;
	    x264_picture_t pic;
	    x264_picture_t pic_out;
	    x264_t *h;
	    x264_nal_t *nal;
	    int i_nal;
	    u32 m_width;
	    u32 m_height;
	    u32 m_frameCount;
	 	bool m_isValid;
	public:
		Encoder(u32 width, u32 height);
		Encoder(Encoder&& encoder);
		Encoder(Encoder& encoder) = delete;
		Encoder& operator=(Encoder& encoder) = delete;
		~Encoder();
	
		bool encodeNV12(u8* const nv12Data, u32 nv12DataSize, u8* &outputBuffer, u32& outputBufferSize);
	};
}
