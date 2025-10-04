#include <SKVMOIP/NV12ToRGBConverter.hpp>
#include <SKVMOIP/assert.h>
#include <common/debug.hpp>
#include <mferror.h> 
#include <wmcodecdsp.h>
#include <mfapi.h>

namespace SKVMOIP
{
	struct MediaTypeCreateInfo
	{
		std::pair<u32, u32> frameSize;
		std::pair<u32, u32> frameRate;
		GUID encodingFormat;
	};

	static IMFMediaType* CreateMediaType(const MediaTypeCreateInfo& createInfo)
	{
		IMFMediaType* mediaType;
		if(MFCreateMediaType(&mediaType) != S_OK)
		{
			debug_log_error("Failed to create");
			return NULL;
		}

		if(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video) != S_OK)
		{
			debug_log_error("Failed to set major type");
			return NULL;
		}

		if(mediaType->SetGUID(MF_MT_SUBTYPE, createInfo.encodingFormat) != S_OK)
		{
			debug_log_error("Failed to set sub type");
			return NULL;
		}

		if(mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive) != S_OK)
		{
			debug_log_error("Failed to set MF_MT_INTERLACE_MODE");
			return NULL;
		}

		if(MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, createInfo.frameSize.first, createInfo.frameSize.second) != S_OK)
		{
			debug_log_error("Failed to set frame size");
			return NULL;
		}

		if(mediaType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(0)) != S_OK)
		{
			debug_log_error("Failed to set MF_MT_DEFAULT_STRIDE");
			return NULL;
		}

		UINT imageSize;
		if(MFCalculateImageSize(createInfo.encodingFormat, createInfo.frameSize.first, createInfo.frameSize.second, &imageSize) != S_OK)
		{
			debug_log_error("Failed to calculate Image size");
			return NULL;
		}

		if(mediaType->SetUINT32(MF_MT_SAMPLE_SIZE, imageSize) != S_OK)
		{
			debug_log_error("Failed to set MF_MT_SAMPLE_SIZE");
			return NULL;
		}


		if(mediaType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE) != S_OK)
		{
			debug_log_error("Failed to set fixed size samples");
			return NULL;
		}

		if(mediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE) != S_OK)
		{
			debug_log_error("Failed to set MF_MT_ALL_SAMPLES_INDEPENDENT");
			return NULL;
		}

		if(MFSetAttributeRatio(mediaType, MF_MT_FRAME_RATE, createInfo.frameRate.first, createInfo.frameRate.second) != S_OK)
		{
			debug_log_error("Failed to set frame rate");
			return NULL;
		}

		if(MFSetAttributeRatio(mediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1) != S_OK)
		{
			debug_log_error("Failed to set MF_MT_PIXEL_ASPECT_RATIO");
			return NULL;
		}

		return mediaType;
	}

	NV12ToRGBConverter::NV12ToRGBConverter(u32 width, u32 height, u32 frameRateNum, u32 frameRateDen, u32 bitsPerPixel) : 
																					m_inputMediaType(NULL),
																					m_outputMediaType(NULL),
																					m_inputMediaBuffer(NULL),
																					m_outputMediaBuffer(NULL),
																					m_inputSample(NULL),
																					m_outputSample(NULL),
																					m_colorConverter(NULL),
																					m_width(width),
																					m_height(height),
																					m_frameRateNum(frameRateNum),
																					m_frameRateDen(frameRateDen),
																					m_bitsPerPixel(bitsPerPixel),
																					m_inputSampleSize(0),
						 															m_outputSampleSize(0),
																					m_isOutputMediaBufferLocked(false),
																					m_isValid(false)
	{
		
		MediaTypeCreateInfo nv12MediaTypeInfo  = { };
		nv12MediaTypeInfo.frameSize = { m_width, m_height };
		nv12MediaTypeInfo.frameRate = { m_frameRateNum, m_frameRateDen };
		nv12MediaTypeInfo.encodingFormat = MFVideoFormat_NV12;

		MediaTypeCreateInfo rgbMediaTypeInfo  = { };
		rgbMediaTypeInfo.frameSize = { m_width, m_height };
		rgbMediaTypeInfo.frameRate = { m_frameRateNum, m_frameRateDen };
		switch(bitsPerPixel)
		{
			case 24:
			{
				rgbMediaTypeInfo.encodingFormat = MFVideoFormat_RGB24;
				break;
			}
			case 32:
			{
				rgbMediaTypeInfo.encodingFormat = MFVideoFormat_RGB32;
				break;
			} 
			default:
			{
				skvmoip_debug_assert(false);
				break;
			}
		}

		if((m_inputMediaType = CreateMediaType(nv12MediaTypeInfo)) == NULL)
			goto RELEASE_RES;
		if((m_outputMediaType = CreateMediaType(rgbMediaTypeInfo)) == NULL)
			goto RELEASE_RES;

		/* Create Video Stream Color Converter */

		if(CoCreateInstance(CLSID_CColorConvertDMO, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&m_colorConverter)) != S_OK)
		{
			debug_log_error("Failed to create CLSID_CColorConvertDMO");
			goto RELEASE_RES;
		}

		if(m_colorConverter->SetInputType(0, m_inputMediaType, 0) != S_OK)
		{
			debug_log_error("Failed to set input type");
			goto RELEASE_RES;
		}

		if(m_colorConverter->SetOutputType(0, m_outputMediaType, 0) != S_OK)
		{
			debug_log_error("Failed to set output type");
			goto RELEASE_RES;
		}

		/* Create Input Samples and Input Media Buffers */

		MFT_INPUT_STREAM_INFO inputStreamInfo;
		if(m_colorConverter->GetInputStreamInfo(0, &inputStreamInfo) != S_OK)
		{
			debug_log_error("Unable to get input stream info");
			goto RELEASE_RES;
		}

		// skvmoip_debug_assert((inputStreamInfo.dwFlags & MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE) == MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE);
		m_inputSampleSize = inputStreamInfo.cbSize;
		skvmoip_debug_assert(((m_width * m_height * 3) >> 1) == m_inputSampleSize);

		if(MFCreateMemoryBuffer(inputStreamInfo.cbSize, &m_inputMediaBuffer) != S_OK)
		{
			debug_log_error("Unable to create input Media Buffer");
			goto RELEASE_RES;
		}
		if(MFCreateSample(&m_inputSample) != S_OK)
		{
			debug_log_error("Failed to create IMFSample for input stream");
			goto RELEASE_RES;
		}
		if(m_inputSample->AddBuffer(m_inputMediaBuffer) != S_OK)
		{
			debug_log_error("Failed to add staging buffer into the input sample");
			goto RELEASE_RES;
		}		

		/* Create Output Samples and Output Media Buffers */

		MFT_OUTPUT_STREAM_INFO outputStreamInfo;
		if(m_colorConverter->GetOutputStreamInfo(0, &outputStreamInfo) != S_OK)
		{
			debug_log_error("Unable to get output stream info");
			goto RELEASE_RES;
		}

		if((outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) != MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)
		{
			m_outputSampleSize = outputStreamInfo.cbSize;
			skvmoip_debug_assert((m_width * m_height * (bitsPerPixel >> 3)) == m_outputSampleSize);
			if(MFCreateMemoryBuffer(outputStreamInfo.cbSize, &m_outputMediaBuffer) != S_OK)
			{
				debug_log_error("Unable to create output Media Buffer");
				goto RELEASE_RES;
			}
			if(MFCreateSample(&m_outputSample) != S_OK)
			{
				debug_log_error("Failed to create IMFSample for output stream");
				goto RELEASE_RES;
			}
			if(m_outputSample->AddBuffer(m_outputMediaBuffer) != S_OK)
			{
				debug_log_error("Failed to add staging buffer into the output sample");
				goto RELEASE_RES;
			}
		}

		m_isValid = true;
		return;

RELEASE_RES:
		if(m_outputSample != NULL)
		{
			m_outputSample->Release();
			m_outputSample = NULL;
		}
		if(m_inputSample != NULL)
		{
			m_inputSample->Release();
			m_inputSample = NULL;
		}
		if(m_outputMediaBuffer != NULL)
		{
			m_outputMediaBuffer->Release();
			m_outputMediaBuffer = NULL;
		}
		if(m_inputMediaBuffer != NULL)
		{
			m_inputMediaBuffer->Release();
			m_inputMediaBuffer = NULL;
		}
		if(m_outputMediaType != NULL)
		{
			m_outputMediaType->Release();
			m_outputMediaType = NULL;
		}
		if(m_inputMediaType != NULL)
		{
			m_inputMediaType->Release();
			m_inputMediaType = NULL;
		}
		if(m_colorConverter != NULL)
			m_colorConverter->Release();
	}

/*	NV12ToRGBConverter::NV12ToRGBConverter(NV12ToRGBConverter&& converter) :
																			m_inputMediaType(converter.m_inputMediaType),
																			m_outputMediaType(converter.m_outputMediaType),
																			m_inputMediaBuffer(converter.m_inputMediaBuffer),
																			m_outputMediaBuffer(converter.m_outputMediaBuffer),
																			m_inputSample(converter.m_inputSample),
																			m_outputSample(converter.m_outputSample),
																			m_colorConverter(converter.m_colorConverter),
																			m_width(converter.m_width),
																			m_height(converter.m_height),
																			m_bitsPerPixel(converter.m_bitsPerPixel),
																			m_inputSampleSize(converter.m_inputSampleSize),
																			m_outputSampleSize(converter.m_outputSampleSize),
																			m_isOutputMediaBufferLocked(converter.m_isOutputMediaBufferLocked),
																			m_isValid(converter.m_isValid)
	{
		converter.m_inputMediaType = NULL;
		converter.m_outputMediaType = NULL;
		converter.m_inputMediaBuffer = NULL;
		converter.m_outputMediaBuffer = NULL;
		converter.m_inputSample = NULL;
		converter.m_outputSample = NULL;
		converter.m_colorConverter = NULL;
		converter.m_width = 0;
		converter.m_height = 0;
		converter.m_bitsPerPixel = 0;
		converter.m_inputSampleSize = 0;
		converter.m_outputSampleSize = 0;
		converter.m_isOutputMediaBufferLocked = false;
		converter.m_isValid = false;
	}

	NV12ToRGBConverter& NV12ToRGBConverter::operator=(NV12ToRGBConverter&& converter)
	{
		m_inputMediaType = converter.m_inputMediaType;
		m_outputMediaType = converter.m_outputMediaType;
		m_inputMediaBuffer = converter.m_inputMediaBuffer;
		m_outputMediaBuffer = converter.m_outputMediaBuffer;
		m_inputSample = converter.m_inputSample;
		m_outputSample = converter.m_outputSample;
		m_colorConverter = converter.m_colorConverter;
		m_width = converter.m_width;
		m_height = converter.m_height;
		m_bitsPerPixel = converter.m_bitsPerPixel;
		m_inputSampleSize = converter.m_inputSampleSize;
		m_outputSampleSize = converter.m_outputSampleSize;
		m_isOutputMediaBufferLocked = converter.m_isOutputMediaBufferLocked;
		m_isValid = converter.m_isValid;

		converter.m_inputMediaType = NULL;
		converter.m_outputMediaType = NULL;
		converter.m_inputMediaBuffer = NULL;
		converter.m_outputMediaBuffer = NULL;
		converter.m_inputSample = NULL;
		converter.m_outputSample = NULL;
		converter.m_colorConverter = NULL;
		converter.m_width = 0;
		converter.m_height = 0;
		converter.m_bitsPerPixel = 0;
		converter.m_inputSampleSize = 0;
		converter.m_outputSampleSize = 0;
		converter.m_isOutputMediaBufferLocked = false;
		converter.m_isValid = false;

		return *this;
	}*/

	NV12ToRGBConverter::~NV12ToRGBConverter()
	{
		if(m_outputSample != NULL)
		{
			m_outputSample->Release();
			m_outputSample = NULL;
		}
		if(m_inputSample != NULL)
		{
			m_inputSample->Release();
			m_inputSample = NULL;
		}
		if(m_outputMediaBuffer != NULL)
		{
			m_outputMediaBuffer->Release();
			m_outputMediaBuffer = NULL;
		}
		if(m_inputMediaBuffer != NULL)
		{
			m_inputMediaBuffer->Release();
			m_inputMediaBuffer = NULL;
		}
		if(m_colorConverter != NULL)
			m_colorConverter->Release();
	}

	u8* NV12ToRGBConverter::convert(u8* nv12Buffer, u32 nv12BufferSize)
	{
		skvmoip_debug_assert(nv12BufferSize == m_inputSampleSize);

		if(m_isOutputMediaBufferLocked)
		{
			if(m_outputMediaBuffer->Unlock() != S_OK)
			{
				debug_log_error("Failed to unlock the output media buffer");
				return NULL;
			}
			m_isOutputMediaBufferLocked = false;
		}

		/* populate the input sample media buffer */
		{
			BYTE* pMappedBuffer;
			DWORD currentLength;
			if(m_inputMediaBuffer->Lock(&pMappedBuffer, NULL, &currentLength) != S_OK)
			{
				debug_log_error("Failed to lock the input media buffer");
				return NULL;
			}
			memcpy(pMappedBuffer, nv12Buffer, m_inputSampleSize);
			if(m_inputMediaBuffer->SetCurrentLength(m_inputSampleSize) != S_OK)
			{
				debug_log_error("Failed to set current length for input media buffer");
				return NULL;
			}
			if(m_inputMediaBuffer->Unlock() != S_OK)
			{
				debug_log_error("Failed to unlock the input media buffer");
				return NULL;
			}
		}

PROCESS_INPUT:

		if(m_colorConverter->ProcessInput(0, m_inputSample, 0) != S_OK)
		{
			debug_log_error("Failed to Process Input");
			return NULL;
		}

		DWORD flags;
		HRESULT result;
		if((result = m_colorConverter->GetOutputStatus(&flags)) != S_OK)
		{
			if(result != E_NOTIMPL)
			{
				debug_log_error("Transform type is not set");
				return NULL;
			}
		}
		else if(flags != MFT_OUTPUT_STATUS_SAMPLE_READY)
			debug_log_error("Not ready to produce output, still proceeding to ProcessOutput");
		else if(flags == 0)
			goto PROCESS_INPUT;

		MFT_OUTPUT_DATA_BUFFER buffer;
		memset(&buffer, 0, sizeof(buffer));
		buffer.pSample = m_outputSample;

		DWORD status;
		result = m_colorConverter->ProcessOutput(0, 1, &buffer, &status);
		if(result != S_OK)
		{
			if(result == MF_E_TRANSFORM_NEED_MORE_INPUT)
				goto PROCESS_INPUT;
			else if(result == MF_E_TRANSFORM_STREAM_CHANGE)
				debug_log_info("Output Stream has changed");
		}

		BYTE* pMappedBuffer;
		DWORD currentLength;
		if(m_outputMediaBuffer->Lock(&pMappedBuffer, NULL, &currentLength) != S_OK)
		{
			debug_log_error("Failed to lock the staging media buffer");
			return NULL;
		}

		m_isOutputMediaBufferLocked = true;

		return pMappedBuffer;
	}
}
