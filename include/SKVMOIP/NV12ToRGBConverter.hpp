#include <SKVMOIP/defines.hpp>

#include <mfidl.h>
#include <mfreadwrite.h>

namespace SKVMOIP
{
	class NV12ToRGBConverter
	{
	private:
		IMFMediaType* m_inputMediaType;
		IMFMediaType* m_outputMediaType;
		IMFMediaBuffer* m_inputMediaBuffer;
		IMFMediaBuffer* m_outputMediaBuffer;
		IMFSample* m_inputSample;
		IMFSample* m_outputSample;
		IMFTransform* m_colorConverter;
		u32 m_width;
		u32 m_height;
		u32 m_frameRateNum;
		u32 m_frameRateDen;
		u32 m_bitsPerPixel;
		u32 m_inputSampleSize;
		u32 m_outputSampleSize;
		bool m_isOutputMediaBufferLocked;
		bool m_isValid;
	public:
		NV12ToRGBConverter(u32 width, u32 height, u32 frameRateNum, u32 frameRateDen, u32 bitsPerPixel);
		NV12ToRGBConverter(NV12ToRGBConverter&& converter) = delete;
		NV12ToRGBConverter& operator=(NV12ToRGBConverter&& converter) = delete;
		NV12ToRGBConverter(NV12ToRGBConverter& converter) = delete;
		NV12ToRGBConverter& operator =(NV12ToRGBConverter& converter) = delete;
		~NV12ToRGBConverter();
	
		u8* convert(u8* nv12Buffer, u32 nv12BufferSize);
		u32 getRGBDataSize() const noexcept { return m_width * m_height * (m_bitsPerPixel >> 3); }
	};
}
