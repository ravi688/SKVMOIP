#include <SKVMOIP/Encoder.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/StopWatch.hpp>
#include <cstring>

namespace SKVMOIP
{
	Encoder::Encoder(u32 width, u32 height) : m_width(width), m_height(height), m_frameCount(0), m_isValid(false)
	{
		if(x264_param_default_preset(&param, "ultrafast", "zerolatency") < 0)
		{
			debug_log_error("x264: Failed to set default preset");
			return;
		}
	
		/* Configure non-default params */
		param.i_threads = 6;
		param.i_bitdepth = 8;
		param.i_csp = X264_CSP_NV12;
		param.i_width  = width;
		param.i_height = height;
		param.b_vfr_input = 0;
		param.b_repeat_headers = 1;
		param.b_annexb = 1;
		param.b_vfr_input = 0;
		param.b_opencl = 1;
		param.i_fps_num = 144;
		param.i_fps_den = 1;
	
	  	if(x264_param_apply_profile(&param, "high444") < 0)
	  	{
	  		debug_log_error("x264: Failed to apply profile restrictions");
	  		return;
	  	}
	
	   if(x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) < 0)
	   {
	   	debug_log_error("x264: Failed to allocate picture");
	   	return;
	   }
	
	   h = x264_encoder_open(&param);
	   if(!h)
	   {
	   	debug_log_error("Failed to open the encoder");
	   	x264_picture_clean(&pic);
	   	return;
	   }
	   m_isValid = true;
	}

	Encoder::Encoder(Encoder&& encoder) :
										param(encoder.param),
	    								pic(encoder.pic),
	    								pic_out(encoder.pic_out),
	    								h(encoder.h),
	    								nal(encoder.nal),
	    								i_nal(encoder.i_nal),
	    								m_width(encoder.m_width),
	    								m_height(encoder.m_height),
	    								m_frameCount(encoder.m_frameCount),
	 									m_isValid(encoder.m_isValid)
	{
	    encoder.h = NULL;
	    encoder.nal = NULL;
	    encoder.i_nal = 0;
	    encoder.m_width = 0;
	    encoder.m_height = 0;
	    encoder.m_frameCount = 0;
	 	encoder.m_isValid = false;
	}
	
	Encoder::~Encoder()
	{
		if(!m_isValid) return;
		 x264_encoder_close(h);
		x264_picture_clean(&pic);
	}
	
	bool Encoder::encodeNV12(u8* const nv12Data, u32 nv12DataSize, u8* &outputBuffer, u32& outputBufferSize)
	{
		_assert(m_isValid);
	
		u32 luma_size = m_width * m_height;
		u32 chroma_size = luma_size >> 2;
	
		memcpy(pic.img.plane[0], nv12Data,  luma_size);
		memcpy(pic.img.plane[1], nv12Data + luma_size, chroma_size + chroma_size);

		pic.i_pts = m_frameCount;
		/* Takes: 4 ms to 8 ms on Logitech Brio
		 *  and : 2 ms to 5 ms on Kreo 1080p60fps HDMI capture card */
		auto i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
		if(i_frame_size < 0)
		{
			debug_log_error("Failed to encode");
			outputBuffer = NULL;
			outputBufferSize = 0;
			return false;
		}
		else if(i_frame_size)
		{
			outputBuffer = nal->p_payload;
			outputBufferSize = i_frame_size;;
		}
	
	    ++m_frameCount;
		return true;
	}

}