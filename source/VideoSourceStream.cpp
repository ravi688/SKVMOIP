#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <strmif.h>
#include <mfapi.h>
#include <unknwn.h>
#include <wmcodecdsp.h>
#include <mftransform.h>
#include <mferror.h>

namespace SKVMOIP
{
	VideoSourceStream::VideoSourceStream(VideoSourceDeviceConnectionID deviceID)
	{

	}

	static const char* getEncodingString(const GUID& guid)
	{
		if(guid == MFVideoFormat_RGB8)
			return "MFVideoFormat_RGB8";
		else if(guid == MFVideoFormat_RGB555)
			return "MFVideoFormat_RGB555";
		else if(guid == MFVideoFormat_RGB565)
			return "MFVideoFormat_RGB565";
		else if(guid == MFVideoFormat_RGB24)
			return "MFVideoFormat_RGB24";
		else if(guid == MFVideoFormat_RGB32)
			return "MFVideoFormat_RGB32";
		else if(guid == MFVideoFormat_ARGB32)
			return "MFVideoFormat_ARGB32";
		else if(guid == MFVideoFormat_A2R10G10B10)
			return "MFVideoFormat_A2R10G10B10";
		else if(guid == MFVideoFormat_A16B16G16R16F)
			return "MFVideoFormat_A16B16G16R16F";
		else if(guid == MFVideoFormat_AI44)
			return "MFVideoFormat_AI44";
		else if(guid == MFVideoFormat_AYUV)
			return "MFVideoFormat_AYUV";
		else if(guid == MFVideoFormat_I420)
			return "MFVideoFormat_I420";
		else if(guid == MFVideoFormat_IYUV)
			return "MFVideoFormat_IYUV";
		else if(guid == MFVideoFormat_NV11)
			return "MFVideoFormat_NV11";
		else if(guid == MFVideoFormat_NV12)
			return "MFVideoFormat_NV12";
		else if(guid == MFVideoFormat_NV21)
			return "MFVideoFormat_NV21";
		else if(guid == MFVideoFormat_UYVY)
			return "MFVideoFormat_UYVY";
		else if(guid == MFVideoFormat_Y41P)
			return "MFVideoFormat_Y41P";
		else if(guid == MFVideoFormat_Y41T)
			return "MFVideoFormat_Y41T";
		else if(guid == MFVideoFormat_Y42T)
			return "MFVideoFormat_Y42T";
		else if(guid == MFVideoFormat_YUY2)
			return "MFVideoFormat_YUY2";
		else if(guid == MFVideoFormat_YVU9)
			return "MFVideoFormat_YVU9";
		else if(guid == MFVideoFormat_YV12)
			return "MFVideoFormat_YV12";
		else if(guid == MFVideoFormat_YVYU)
			return "MFVideoFormat_YVYU";
		else if(guid == MFVideoFormat_P010)
			return "MFVideoFormat_P010";
		else if(guid == MFVideoFormat_P016)
			return "MFVideoFormat_P016";
		else if(guid == MFVideoFormat_P210)
			return "MFVideoFormat_P210";
		else if(guid == MFVideoFormat_P216)
			return "MFVideoFormat_P216";
		else if(guid == MFVideoFormat_v210)
			return "MFVideoFormat_v210";
		else if(guid == MFVideoFormat_v216)
			return "MFVideoFormat_v216";
		else if(guid == MFVideoFormat_v410)
			return "MFVideoFormat_v410";
		else if(guid == MFVideoFormat_Y210)
			return "MFVideoFormat_Y210";
		else if(guid == MFVideoFormat_Y216)
			return "MFVideoFormat_Y216";
		else if(guid == MFVideoFormat_Y410)
			return "MFVideoFormat_Y410";
		else if(guid == MFVideoFormat_Y416)
			return "MFVideoFormat_Y416";
		else if(guid == MFVideoFormat_L8)
			return "MFVideoFormat_L8";
		else if(guid == MFVideoFormat_L16)
			return "MFVideoFormat_L16";
		else if(guid == MFVideoFormat_D16)
			return "MFVideoFormat_D16";
		else if(guid == MFVideoFormat_DV25)
			return "MFVideoFormat_DV25";
		else if(guid == MFVideoFormat_DV50)
			return "MFVideoFormat_DV50";
		else if(guid == MFVideoFormat_DVC)
			return "MFVideoFormat_DVC";
		else if(guid == MFVideoFormat_DVH1)
			return "MFVideoFormat_DVH1";
		else if(guid == MFVideoFormat_DVHD)
			return "MFVideoFormat_DVHD";
		else if(guid == MFVideoFormat_DVSD)
			return "MFVideoFormat_DVSD";
		else if(guid == MFVideoFormat_DVSL)
			return "MFVideoFormat_DVSL";
		// else if(guid == MFVideoFormat_H263)
			// return "MFVideoFormat_H263";
		else if(guid == MFVideoFormat_H264)
			return "MFVideoFormat_H264";
		else if(guid == MFVideoFormat_H265)
			return "MFVideoFormat_H265";
		else if(guid == MFVideoFormat_H264_ES)
			return "MFVideoFormat_H264_ES";
		else if(guid == MFVideoFormat_HEVC)
			return "MFVideoFormat_HEVC";
		else if(guid == MFVideoFormat_HEVC_ES)
			return "MFVideoFormat_HEVC_ES";
		else if(guid == MFVideoFormat_M4S2)
			return "MFVideoFormat_M4S2";
		else if(guid == MFVideoFormat_MJPG)
			return "MFVideoFormat_MJPG";
		else if(guid == MFVideoFormat_MP43)
			return "MFVideoFormat_MP43";
		else if(guid == MFVideoFormat_MP4S)
			return "MFVideoFormat_MP4S";
		else if(guid == MFVideoFormat_MP4V)
			return "MFVideoFormat_MP4V";
		else if(guid == MFVideoFormat_MPEG2)
			return "MFVideoFormat_MPEG2";
		else if(guid == MFVideoFormat_VP80)
			return "MFVideoFormat_VP80";
		else if(guid == MFVideoFormat_VP90)
			return "MFVideoFormat_VP90";
		else if(guid == MFVideoFormat_MPG1)
			return "MFVideoFormat_MPG1";
		else if(guid == MFVideoFormat_MSS1)
			return "MFVideoFormat_MSS1";
		else if(guid == MFVideoFormat_MSS2)
			return "MFVideoFormat_MSS2";
		else if(guid == MFVideoFormat_WMV1)
			return "MFVideoFormat_WMV1";
		else if(guid == MFVideoFormat_WMV2)
			return "MFVideoFormat_WMV2";
		else if(guid == MFVideoFormat_WMV3)
			return "MFVideoFormat_WMV3";
		else if(guid == MFVideoFormat_WVC1)
			return "MFVideoFormat_WVC1";
		else if(guid == MFVideoFormat_420O)
			return "MFVideoFormat_420O";
		else if(guid == MFVideoFormat_AV1)
			return "MFVideoFormat_AV1";
		return "<unknown>";
	}

	static IMFMediaType* FindMediaType(IMFSourceReader *pReader, const std::tuple<u32, u32, u32>& res)
	{
	    DWORD dwMediaTypeIndex = 0;
	    HRESULT hr = S_OK;	
	    while (SUCCEEDED(hr))
	    {
	        IMFMediaType *pType = NULL;
	        hr = pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwMediaTypeIndex, &pType);
	        if (hr != S_OK)
	        {
	            hr = S_OK;
	            break;
	        }
	        else if (SUCCEEDED(hr))
	        {
	            // Examine the media type. (Not shown.)
	            GUID m_encodingFormat;
	            if(pType->GetGUID(MF_MT_SUBTYPE, &m_encodingFormat) != S_OK)
					debug_log_error("Unable to get encoding");
				else
				{
	        		UINT32 frNumerator, frDenominator;
					if(MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &frNumerator, &frDenominator) != S_OK)
						debug_log_error("\tUnable to get the frame rate");
					UINT32 width, height;
					if(MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height) != S_OK)
						debug_log_error("\tUnable to get the frame size");
					if(((frNumerator / frDenominator) == std::get<2>(res)) && (width == std::get<0>(res)) && (height == std::get<1>(res)))
						return pType;
				}
	            pType->Release();

	        }
	        ++dwMediaTypeIndex;
	    }
	    return NULL;
	}

	static IMFMediaType* SelectMediaType(IMFSourceReader* pSourceReader, const std::vector<std::tuple<u32, u32, u32>>& resPrefList)
	{
		for(const std::tuple<u32, u32, u32>& res : resPrefList)
			if(auto mediaType = FindMediaType(pSourceReader, res))
				return mediaType;
		return NULL;
	}

	VideoSourceStream::VideoSourceStream(Win32::Win32SourceDevice& device, const std::vector<std::tuple<u32, u32, u32>>& resPrefList) : 
																				m_sourceReader(NULL), 
																				m_mediaType(NULL),
																				m_outputMediaType(NULL), 
																				m_stagingMediaBuffer(NULL),
																				m_videoColorConverter(NULL),
																				m_outputSample(NULL)
	{
		if(MFCreateSourceReaderFromMediaSource(device.getInternalHandle(), NULL, &m_sourceReader) != S_OK)
		{
			debug_log_error("Unable to create SourceReader from MediaSource");
			m_isValid = false;
			return;
		}

		m_mediaType = SelectMediaType(m_sourceReader, resPrefList);
		if(m_mediaType == NULL)
		{
			debug_log_error("Failed to match any of the available media formats on the capture device");
			goto RELEASE;
		}

		if(m_sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, m_mediaType) != S_OK)
		{
			debug_log_error("Unable to set current media type");
			goto RELEASE;
		}

		AM_MEDIA_TYPE* pInfo;
		if(m_mediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
		{
			debug_log_error("Unable to get Representation");
			goto RELEASE;
		}
		m_isFixedSizedSamples = (pInfo->bFixedSizeSamples == 1) ? true : false;
		m_isTemporalCompression = (pInfo->bFixedSizeSamples == 1) ? true : false;

		_assert(m_isFixedSizedSamples == true);
		// _assert(pInfo->bTemporalCompression = FALSE);

		m_sampleSize = pInfo->lSampleSize;

		if(m_mediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
		{
			debug_log_error("Unable to free Representation");
			goto RELEASE;
		}

		GUID majorType;
		if(m_mediaType->GetMajorType(&majorType) == S_OK)
			_assert(majorType == MFMediaType_Video);
		else
		{
			debug_log_error("Unable to get major type");
			goto RELEASE;
		}

		if(m_mediaType->GetGUID(MF_MT_SUBTYPE, &m_encodingFormat) != S_OK)
		{
			debug_log_error("Unable to get encoding");
			goto RELEASE;
		}


		IMFTransform* pVideoColorConverter;
		if(CoCreateInstance(CLSID_CColorConvertDMO, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pVideoColorConverter)) != S_OK)
		{
			debug_log_error("Failed to create CLSID_CColorConvertDMO");
			goto RELEASE;
		}

		m_videoColorConverter = pVideoColorConverter;

		if(pVideoColorConverter->SetInputType(0, m_mediaType, 0) != S_OK)
		{
			debug_log_error("Failed to set input type");
			goto RELEASE;
		}

		if(MFCreateMediaType(&m_outputMediaType) != S_OK)
		{
			debug_log_error("Failed to create output media type");
			goto RELEASE;
		}

		if(m_mediaType->CopyAllItems(m_outputMediaType) != S_OK)
		{
			debug_log_error("Failed to copy all items from input media to output media");
			goto RELEASE;
		}

		if(m_outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video) != S_OK)
		{
			debug_log_error("Failed to set major type on output media type");
			goto RELEASE;
		}

		if(m_outputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24) != S_OK)
		{
			debug_log_error("Failed to set sub type on output media type");
			goto RELEASE;
		}

		if(pVideoColorConverter->SetOutputType(0, m_outputMediaType, 0) != S_OK)
		{
			debug_log_error("Failed to set output type");
			goto RELEASE;
		}		

		if(m_outputMediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
		{
			debug_log_error("Unable to get Representation for output media type (RGB24) ");
			goto RELEASE;
		}
		
		m_outputSampleSize = pInfo->lSampleSize;

		if(m_outputMediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
		{
			debug_log_error("Unable to free Representation");
			goto RELEASE;
		}

		MFT_OUTPUT_STREAM_INFO outputStreamInfo;
		if(m_videoColorConverter->GetOutputStreamInfo(0, &outputStreamInfo) != S_OK)
		{
			debug_log_error("Unable to get output stream info");
			goto RELEASE;
		}


		if((outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) != MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)
		{
			_assert(m_outputSampleSize == outputStreamInfo.cbSize);
			debug_log_info("Output Sample Size: %lu", outputStreamInfo.cbSize);
			if(MFCreateMemoryBuffer(outputStreamInfo.cbSize, &m_stagingMediaBuffer) != S_OK)
			{
				debug_log_error("Unable to create Media Buffer");
				goto RELEASE;
			}
			if(MFCreateSample(&m_outputSample) != S_OK)
			{
				debug_log_error("Failed to create IMFSample for output stream");
				goto RELEASE;
			}
			if(m_outputSample->AddBuffer(m_stagingMediaBuffer) != S_OK)
			{
				debug_log_error("Failed to add staging buffer into the output sample");
				goto RELEASE;
			}
		}
		else
			m_outputSample = NULL;

		if(pVideoColorConverter->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0) != S_OK)
		{
			debug_log_error("Failed to Process Message MFT_MESSAGE_NOTIFY_BEGIN_STREAMING");
			goto RELEASE;
		}

		m_isValid = true;
		return;

		RELEASE:
			if(m_outputSample != NULL)
				m_outputSample->Release();
			m_outputSample = NULL;
			if(m_stagingMediaBuffer != NULL)
				m_stagingMediaBuffer->Release();
			m_stagingMediaBuffer = NULL;
			if(m_videoColorConverter != NULL)
				m_videoColorConverter->Release();
			if(m_sourceReader != NULL)
				m_sourceReader->Release();
			m_isValid = false;
			return;
	}

	VideoSourceStream::VideoSourceStream(VideoSourceStream&& stream) : m_sourceReader(stream.m_sourceReader),
																	   m_stagingMediaBuffer(stream.m_stagingMediaBuffer),
																	   m_videoColorConverter(stream.m_videoColorConverter),
																	   m_outputSample(stream.m_outputSample),
																	   m_mediaType(stream.m_mediaType),
																	   m_isValid(stream.m_isValid)
	{
		stream.m_sourceReader = NULL;
		stream.m_stagingMediaBuffer = NULL;
		stream.m_videoColorConverter = NULL;
		stream.m_outputSample = NULL;
		stream.m_mediaType = NULL;
		stream.m_isValid = false;
	}

	void VideoSourceStream::destroy()
	{
		if(m_sourceReader != NULL)
		{
			m_sourceReader->Release();
			m_sourceReader = NULL;
			m_stagingMediaBuffer->Release();
			m_stagingMediaBuffer = NULL;
			m_videoColorConverter->Release();
			m_videoColorConverter = NULL;
			if(m_outputSample != NULL)
				m_outputSample->Release();
			m_outputSample = NULL;
			m_mediaType = NULL;
			m_isValid = false;
		}
	}

	VideoSourceStream::~VideoSourceStream()
	{
		destroy();
	}

	std::optional<bool> VideoSourceStream::isCompressedFormat()
	{
		BOOL isCompressed;
		if(m_mediaType->IsCompressedFormat(&isCompressed) != S_OK)
		{
			debug_log_error("Unable to determine if the media type is compressed");
			return { };
		}
		return { isCompressed };
	}

	std::optional<u32> VideoSourceStream::getSampleSizeInBytes() const
	{
		if(!m_isValid)
			return { };
		return { m_sampleSize };
	}

	std::optional<std::pair<u32, u32>> VideoSourceStream::getFrameSize()
	{
		UINT32 width, height;
		if(MFGetAttributeSize(m_mediaType, MF_MT_FRAME_SIZE, &width, &height) != S_OK)
		{
			debug_log_error("Unable to get the frame size");
			return { };
		}
		return { { width, height } };
	}

	std::optional<std::pair<u32, u32>> VideoSourceStream::getFrameRate()
	{
		UINT32 frNumerator, frDenominator;
		if(MFGetAttributeRatio(m_mediaType, MF_MT_FRAME_RATE, &frNumerator, &frDenominator) != S_OK)
		{
			debug_log_error("Unable to get the frame rate");
			return { };
		}
		return { { frNumerator, frDenominator } };
	}

	std::optional<const char*> VideoSourceStream::getEncodingFormatStr()
	{
		if(m_isValid)
			return { getEncodingString(m_encodingFormat) };
		else
			return { };
	}

	static u32 GetTotalMaxSize(IMFSample* pSample)
	{
		DWORD bufferCount;
		if(pSample->GetBufferCount(&bufferCount) != S_OK)
		{
			debug_log_error("Failed to get buffer count");
			return 0;
		}

		DWORD totalLength = 0;
		for(DWORD i = 0; i < bufferCount; i++)
		{
			IMFMediaBuffer* pBuffer;
			if(pSample->GetBufferByIndex(i, &pBuffer) != S_OK)
			{
				debug_log_error("Failed to get buffer by index");
				pSample->Release();
				return 0;
			}
			DWORD maxLength;
			if(pBuffer->GetMaxLength(&maxLength) != S_OK)
			{
				debug_log_error("Failed to get max length");
				pSample->Release();
				return 0;
			}
			totalLength += maxLength;
		}
		pSample->Release();
		return totalLength;
	}

	bool VideoSourceStream::readRGBFrameToBuffer(u8* const rgbBuffer, u32 rgbBufferSize)
	{
		DWORD streamIndex;
		DWORD streamFlags;
		LONGLONG timeStamp;
		IMFSample* pSample;
		if(m_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &streamFlags, &timeStamp, &pSample) != S_OK)
		{
			debug_log_error("Unable to read sample");
			return false;
		}
		if(pSample == NULL)
		{
			debug_log_error("IMFSample data is NULL");
			return false;
		}

		DWORD totalLength;
		if(pSample->GetTotalLength(&totalLength) == S_OK)
		{
			_assert(totalLength <= m_sampleSize);
		}
		else
			debug_log_error("Unable to verify total sample length");

		// pSample->AddRef();
		// auto totalMaxSize = GetTotalMaxSize(pSample);
		// _assert(totalMaxSize == m_sampleSize);

PROCESS_INPUT:

		if(m_videoColorConverter->ProcessInput(0, pSample, 0) != S_OK)
		{
			debug_log_error("Failed to Process Input");
			return false;
		}
		else
		{
			// debug_log_info("Processed Input");
		}

		DWORD flags;
		HRESULT result;
		if((result = m_videoColorConverter->GetOutputStatus(&flags)) != S_OK)
		{
			if(result != E_NOTIMPL)
			{
				debug_log_error("Transform type is not set");
				goto RELEASE_FALSE;
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
		result = m_videoColorConverter->ProcessOutput(0, 1, &buffer, &status);
		if(result != S_OK)
		{
			if(result == MF_E_TRANSFORM_NEED_MORE_INPUT)
				goto PROCESS_INPUT;
			else if(result == MF_E_TRANSFORM_STREAM_CHANGE)
				debug_log_info("Output Stream has changed");
		}

		BYTE* pMappedBuffer;
		DWORD currentLength;
		if(m_stagingMediaBuffer->Lock(&pMappedBuffer, NULL, &currentLength) != S_OK)
		{
			debug_log_error("Failed to lock the staging media buffer");
			goto RELEASE_FALSE;
		}

		/* convert to RGB and the copy the data to the RGB Buffer */
		for(u32 i = 0; i < currentLength; i += 3)
		{
			rgbBuffer[(i / 3) * 4 + 0] = pMappedBuffer[i + 0];
			rgbBuffer[(i / 3) * 4 + 1] = pMappedBuffer[i + 1];
			rgbBuffer[(i / 3) * 4 + 2] = pMappedBuffer[i + 2];
			rgbBuffer[(i / 3) * 4 + 3] = 255;
		}

		if(m_stagingMediaBuffer->Unlock() != S_OK)
		{
			debug_log_error("Failed to unlock the staging media buffer");
		}

		pSample->Release();
		return true;

RELEASE_FALSE:
		pSample->Release();
		return false;
	}
}