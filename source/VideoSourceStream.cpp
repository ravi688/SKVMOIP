#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/StopWatch.hpp>
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

	SKVMOIP_API const char* getEncodingString(const GUID& guid)
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

	enum class NativeMediaTypePreference
	{
		Any = 0,
		NV12
	};

	static GUID getEncodingFormatFromNativeMediaTypePref(NativeMediaTypePreference pref)
	{
		switch(pref)
		{
			case NativeMediaTypePreference::Any: { _assert(false); return MFVideoFormat_NV12; }
			case NativeMediaTypePreference::NV12: { return MFVideoFormat_NV12; }
			default: { _assert(false); return MFVideoFormat_NV12; }
		}
	}

	static IMFMediaType* FindMediaType(IMFSourceReader *pReader, const std::tuple<u32, u32, u32>& res, NativeMediaTypePreference mediaTypePref)
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
					if(((frNumerator / frDenominator) == std::get<2>(res)) 
						&& (width == std::get<0>(res)) 
						&& (height == std::get<1>(res)) 
						&& ((mediaTypePref == NativeMediaTypePreference	::Any)
						|| (m_encodingFormat == getEncodingFormatFromNativeMediaTypePref(mediaTypePref))))
						return pType;
				}
	            pType->Release();

	        }
	        ++dwMediaTypeIndex;
	    }
	    return NULL;
	}

	static IMFMediaType* SelectMediaType(IMFSourceReader* pSourceReader, const std::vector<std::tuple<u32, u32, u32>>& resPrefList, NativeMediaTypePreference mediaTypePref)
	{
		for(const std::tuple<u32, u32, u32>& res : resPrefList)
			if(auto mediaType = FindMediaType(pSourceReader, res, mediaTypePref))
				return mediaType;
		return NULL;
	}

	static void dumpMediaType(IMFMediaType* mediaType)
	{
		debug_log_info("MediaType Dump: ");

		AM_MEDIA_TYPE* pInfo;
		if(mediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
		{
			debug_log_error("Unable to get Representation");
			return;
		}
		else
		{
			debug_log_info("\tIsFixedSizedSamples: %lu", pInfo->bFixedSizeSamples);
			debug_log_info("\tIsTemporalCompression: %lu", pInfo->bTemporalCompression);
			debug_log_info("\tSampleSize: %lu", pInfo->lSampleSize);
		}

		if(mediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
		{
			debug_log_error("Unable to free Representation");
			return;
		}

		BOOL isCompressed;
		if(mediaType->IsCompressedFormat(&isCompressed) != S_OK)
		{
			debug_log_error("Unable to determine if the media type is compressed");
			return;
		}
		else
		{
			debug_log_info("\tIsCompressed: %lu", isCompressed);
		}

		UINT32 width, height;
		if(MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height) != S_OK)
		{
			debug_log_error("Unable to get the input frame size");
			return;
		}
		else
		{
			debug_log_info("\twidth=%lu", width);
			debug_log_info("\theight=%lu", height);
		}

		UINT32 frNumerator, frDenominator;
		if(MFGetAttributeSize(mediaType, MF_MT_FRAME_RATE, &frNumerator, &frDenominator) != S_OK)
		{
			debug_log_error("Unable to get the input frame rate");
			return;
		}
		else
		{
			 debug_log_info("\tFrameRateNumerator: %lu", frNumerator);
			 debug_log_info("\tFrameRateDenominator: %lu", frDenominator);
		}

		GUID majorType;
		if(mediaType->GetMajorType(&majorType) == S_OK)
			_assert(majorType == MFMediaType_Video);
		else
		{
			debug_log_error("Unable to get major type");
			return;
		}

		GUID encoding;
		if(mediaType->GetGUID(MF_MT_SUBTYPE, &encoding) != S_OK)
		{
			debug_log_error("Unable to get encoding");
			return;
		}
		else
		{
			debug_log_info("\tEncoding: %s", getEncodingString(encoding));
		}

		// UINT32 stride;
		// if(mediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride) != S_OK)
		// {
		// 	debug_log_error("Unable to get default stride");
		// 	return;
		// }
		// else
		// {
		// 	debug_log_info("\tDefault Stride: %lu", stride);
		// }
	}


	static const char* HRESULTToString(HRESULT result)
	{
		switch(result)
		{
			case S_OK: return "S_OK";
			case MF_E_INVALIDMEDIATYPE: return "MF_E_INVALIDMEDIATYPE";
			case MF_E_INVALIDSTREAMNUMBER: return "MF_E_INVALIDSTREAMNUMBER";
			case MF_E_INVALIDTYPE: return "MF_E_INVALIDTYPE";
			case MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING: return "MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING";
			case MF_E_TRANSFORM_TYPE_NOT_SET: return "MF_E_TRANSFORM_TYPE_NOT_SET";
			case MF_E_UNSUPPORTED_D3D_TYPE: return "MF_E_UNSUPPORTED_D3D_TYPE";
			default: return "failed to convert to string";
				break;
		}
		return "";
	}

	static IMFMediaType* FindMediaTypeEncoder(IMFTransform *pReader, const std::tuple<u32, u32, u32, GUID>& res)
	{
	    DWORD dwMediaTypeIndex = 0;
	    HRESULT hr = S_OK;	
	    while (SUCCEEDED(hr))
	    {
	        IMFMediaType *pType = NULL;
	        hr = pReader->GetInputAvailableType(0, dwMediaTypeIndex, &pType);
	        if (hr != S_OK)
	        {
	            hr = S_OK;
	            break;
	        }
	        else if (SUCCEEDED(hr))
	        {
	        	dumpMediaType(pType);
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
					if(((frNumerator / frDenominator) == std::get<2>(res)) && (width == std::get<0>(res)) && (height == std::get<1>(res)) && (m_encodingFormat == std::get<3>(res)))
						return pType;
				}
	            pType->Release();

	        }
	        ++dwMediaTypeIndex;
	    }
	    return NULL;
	}

	static GUID getEncodingFormatFromUsage(VideoSourceStream::Usage usage)
	{
		switch(usage)
		{
			case VideoSourceStream::Usage::RGB24Read: return MFVideoFormat_RGB24;
			case VideoSourceStream::Usage::RGB32Read: return MFVideoFormat_RGB32;
			default:
				assert(false, "Undefined");
				return MFVideoFormat_RGB32;
		}
	}

	VideoSourceStream::VideoSourceStream(Win32::Win32SourceDevice& device, Usage usage, const std::vector<std::tuple<u32, u32, u32>>& resPrefList) : 
																				m_usage(usage),
																				m_sourceReader(NULL), 
																				m_inputMediaType(NULL),
																				m_outputMediaType(NULL), 
																				m_stagingMediaBuffer(NULL),
																				m_videoColorConverter(NULL),
																				m_outputSample(NULL),
																				m_inputSampleSize(0),
																				m_outputSampleSize(0),
																				m_inputFrameWidth(0),
																				m_inputFrameHeight(0),
																				m_outputFrameWidth(0),
																				m_outputFrameHeight(0),
																				m_inputFrameRateNumer(0),
																				m_inputFrameRateDenom(0),
																				m_outputFrameRateNumer(0),
																				m_outputFrameRateDenom(0),
																				m_isInputFixedSizedSamples(false),
																				m_isOutputFixedSizedSamples(false),
																				m_isInputTemporalCompression(false),
																				m_isOutputTemporalCompression(false),
																				m_isInputCompressedFormat(false),
																				m_isOutputCompressedFormat(false)
	{
		if(MFCreateSourceReaderFromMediaSource(device.getInternalHandle(), NULL, &m_sourceReader) != S_OK)
		{
			debug_log_error("Unable to create SourceReader from MediaSource");
			m_isValid = false;
			return;
		}

		switch(m_usage)
		{
			case Usage::RGB24Read:
			case Usage::RGB32Read:
			{
				m_inputMediaType = SelectMediaType(m_sourceReader, resPrefList, NativeMediaTypePreference::Any);
				break;
			}
			case Usage::NV12Read:
			{
				m_inputMediaType = SelectMediaType(m_sourceReader, resPrefList, NativeMediaTypePreference::NV12);
				break;
			}
			default:
			{
				m_inputMediaType = SelectMediaType(m_sourceReader, resPrefList, NativeMediaTypePreference::Any);
				break;
			}
		}
		if(m_inputMediaType == NULL)
		{
			debug_log_error("Failed to match any of the available media formats on the capture device");
			goto RELEASE_RES;
		}

		if(m_sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, m_inputMediaType) != S_OK)
		{
			debug_log_error("Unable to set current media type");
			goto RELEASE_RES;
		}

		AM_MEDIA_TYPE* pInfo;
		if(m_inputMediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
		{
			debug_log_error("Unable to get Representation");
			goto RELEASE_RES;
		}
		else
		{
			m_isInputFixedSizedSamples = pInfo->bFixedSizeSamples;
			m_isInputTemporalCompression = pInfo->bTemporalCompression;
			m_inputSampleSize = pInfo->lSampleSize;
		}

		_assert(m_isInputFixedSizedSamples == true);

		if(m_inputMediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
		{
			debug_log_error("Unable to free Representation");
			goto RELEASE_RES;
		}

		BOOL isCompressed;
		if(m_inputMediaType->IsCompressedFormat(&isCompressed) != S_OK)
		{
			debug_log_error("Unable to determine if the media type is compressed");
			goto RELEASE_RES;
		}
		else m_isInputCompressedFormat = isCompressed;

		UINT32 width, height;
		if(MFGetAttributeSize(m_inputMediaType, MF_MT_FRAME_SIZE, &width, &height) != S_OK)
		{
			debug_log_error("Unable to get the input frame size");
			goto RELEASE_RES;
		}
		else
		{
			m_inputFrameWidth = width;
			m_inputFrameHeight = height;
		}

		UINT32 frNumerator, frDenominator;
		if(MFGetAttributeSize(m_inputMediaType, MF_MT_FRAME_RATE, &frNumerator, &frDenominator) != S_OK)
		{
			debug_log_error("Unable to get the input frame rate");
			goto RELEASE_RES;
		}
		else
		{
			m_inputFrameRateNumer = frNumerator;
			m_inputFrameRateDenom = frDenominator;
		}

		GUID majorType;
		if(m_inputMediaType->GetMajorType(&majorType) == S_OK)
			_assert(majorType == MFMediaType_Video);
		else
		{
			debug_log_error("Unable to get major type");
			goto RELEASE_RES;
		}

		if(m_inputMediaType->GetGUID(MF_MT_SUBTYPE, &m_inputEncodingFormat) != S_OK)
		{
			debug_log_error("Unable to get encoding");
			goto RELEASE_RES;
		}
		else
		{
			/* For now we want to stick with NV12, as it is available on most devices */
			_assert(m_inputEncodingFormat == MFVideoFormat_NV12);
		}

		if((m_usage == Usage::RGB32Read) || (m_usage == Usage::RGB24Read))
		{
			/*  Create Output Media Type */

			if(MFCreateMediaType(&m_outputMediaType) != S_OK)
			{
				debug_log_error("Failed to create output media type");
				goto RELEASE_RES;
			}

			if(m_inputMediaType->CopyAllItems(m_outputMediaType) != S_OK)
			{
				debug_log_error("Failed to copy all items from input media to output media");
				goto RELEASE_RES;
			}

			if(m_outputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video) != S_OK)
			{
				debug_log_error("Failed to set major type on output media type");
				goto RELEASE_RES;
			}

			if(m_outputMediaType->SetGUID(MF_MT_SUBTYPE, getEncodingFormatFromUsage(m_usage)) != S_OK)
			{
				debug_log_error("Failed to set sub type on output media type");
				goto RELEASE_RES;
			}
			else m_outputEncodingFormat = getEncodingFormatFromUsage(m_usage);

			if(m_outputMediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
			{
				debug_log_error("Unable to get Representation");
				goto RELEASE_RES;
			}
			else
			{
				m_isOutputFixedSizedSamples = pInfo->bFixedSizeSamples;
				m_isOutputTemporalCompression = pInfo->bTemporalCompression;
				m_outputSampleSize = pInfo->lSampleSize;
			}

			_assert(m_isOutputFixedSizedSamples == true);

			if(m_outputMediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
			{
				debug_log_error("Unable to free Representation");
				goto RELEASE_RES;
			}

			if(m_outputMediaType->IsCompressedFormat(&isCompressed) != S_OK)
			{
				debug_log_error("Unable to determine if the media type is compressed");
				goto RELEASE_RES;
			}
			else m_isOutputCompressedFormat = isCompressed;

			if(MFGetAttributeSize(m_outputMediaType, MF_MT_FRAME_SIZE, &width, &height) != S_OK)
			{
				debug_log_error("Unable to get the output frame size");
				goto RELEASE_RES;
			}
			else
			{
				m_outputFrameWidth = width;
				m_outputFrameHeight = height;
			}

			if(MFGetAttributeSize(m_outputMediaType, MF_MT_FRAME_RATE, &frNumerator, &frDenominator) != S_OK)
			{
				debug_log_error("Unable to get the output frame rate");
				goto RELEASE_RES;
			}
			else
			{
				m_outputFrameRateNumer = frNumerator;
				m_outputFrameRateDenom = frDenominator;
			}

			/* Create Video Stream Color Converter */

			IMFTransform* pVideoColorConverter;
			if(CoCreateInstance(CLSID_CColorConvertDMO, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pVideoColorConverter)) != S_OK)
			{
				debug_log_error("Failed to create CLSID_CColorConvertDMO");
				goto RELEASE_RES;
			}
			else m_videoColorConverter = pVideoColorConverter;

			if(pVideoColorConverter->SetInputType(0, m_inputMediaType, 0) != S_OK)
			{
				debug_log_error("Failed to set input type");
				goto RELEASE_RES;
			}

			if(pVideoColorConverter->SetOutputType(0, m_outputMediaType, 0) != S_OK)
			{
				debug_log_error("Failed to set output type");
				goto RELEASE_RES;
			}		

			if(m_outputMediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
			{
				debug_log_error("Unable to get Representation for output media type (RGB24) ");
				goto RELEASE_RES;
			}
		
			m_outputSampleSize = pInfo->lSampleSize;

			if(m_outputMediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
			{
				debug_log_error("Unable to free Representation");
				goto RELEASE_RES;
			}

			MFT_OUTPUT_STREAM_INFO outputStreamInfo;
			if(m_videoColorConverter->GetOutputStreamInfo(0, &outputStreamInfo) != S_OK)
			{
				debug_log_error("Unable to get output stream info");
				goto RELEASE_RES;
			}


			if((outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) != MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)
			{
				_assert(m_outputSampleSize == outputStreamInfo.cbSize);
				if(MFCreateMemoryBuffer(outputStreamInfo.cbSize, &m_stagingMediaBuffer) != S_OK)
				{
					debug_log_error("Unable to create Media Buffer");
					goto RELEASE_RES;
				}
				if(MFCreateSample(&m_outputSample) != S_OK)
				{
					debug_log_error("Failed to create IMFSample for output stream");
					goto RELEASE_RES;
				}
				if(m_outputSample->AddBuffer(m_stagingMediaBuffer) != S_OK)
				{
					debug_log_error("Failed to add staging buffer into the output sample");
					goto RELEASE_RES;
				}
			}
		}
		else
		{
			m_outputSampleSize = m_inputSampleSize;
			m_outputFrameWidth = m_inputFrameWidth;
			m_outputFrameHeight = m_inputFrameHeight;
			m_outputFrameRateNumer = m_inputFrameRateNumer;
			m_outputFrameRateDenom = m_inputFrameRateDenom;
			m_outputEncodingFormat = m_inputEncodingFormat;
			m_isOutputFixedSizedSamples = m_isInputFixedSizedSamples;
			m_isOutputTemporalCompression = m_isInputTemporalCompression;
			m_isOutputCompressedFormat = m_isInputCompressedFormat;

			_assert(m_inputSampleSize == ((m_inputFrameWidth * m_inputFrameHeight * 3) >> 1));

			if(MFCreateMemoryBuffer(m_inputSampleSize, &m_stagingMediaBuffer) != S_OK)
			{
					debug_log_error("Unable to create Media Buffer");
					goto RELEASE_RES;
			}
		}

		m_isValid = true;
		return;

RELEASE_RES:
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

	VideoSourceStream::VideoSourceStream(VideoSourceStream&& stream) :
																	   m_usage(stream.m_usage),
																	   m_sourceReader(stream.m_sourceReader), 
																	   m_inputMediaType(stream.m_inputMediaType),
																	   m_outputMediaType(stream.m_outputMediaType), 
																	   m_stagingMediaBuffer(stream.m_stagingMediaBuffer),
																	   m_videoColorConverter(stream.m_videoColorConverter),
																	   m_outputSample(stream.m_outputSample),
																	   m_inputSampleSize(stream.m_inputSampleSize),
																	   m_outputSampleSize(stream.m_outputSampleSize),
																	   m_inputFrameWidth(stream.m_inputFrameWidth),
																	   m_inputFrameHeight(stream.m_inputFrameHeight),
																	   m_outputFrameWidth(stream.m_outputFrameWidth),
																	   m_outputFrameHeight(stream.m_outputFrameHeight),
																	   m_inputFrameRateNumer(stream.m_inputFrameRateNumer),
																	   m_inputFrameRateDenom(stream.m_inputFrameRateDenom),
																	   m_outputFrameRateNumer(stream.m_outputFrameRateNumer),
																	   m_outputFrameRateDenom(stream.m_outputFrameRateDenom),
																	   m_isInputFixedSizedSamples(stream.m_isInputFixedSizedSamples),
																	   m_isOutputFixedSizedSamples(stream.m_isOutputFixedSizedSamples),
																	   m_isInputTemporalCompression(stream.m_isInputTemporalCompression),
																	   m_isOutputTemporalCompression(stream.m_isOutputTemporalCompression),
																	   m_isInputCompressedFormat(stream.m_isInputCompressedFormat),
																	   m_isOutputCompressedFormat(stream.m_isOutputCompressedFormat)
	{
		stream.m_sourceReader = NULL;
		stream.m_inputMediaType = NULL;
		stream.m_outputMediaType = NULL;
		stream.m_stagingMediaBuffer = NULL;
		stream.m_videoColorConverter = NULL;
		stream.m_outputSample = NULL;
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
			if(m_videoColorConverter != NULL)
			{
				m_videoColorConverter->Release();
				m_videoColorConverter = NULL;
			}
			if(m_outputSample != NULL)
				m_outputSample->Release();
			m_outputSample = NULL;
			m_inputMediaType = NULL;
			m_isValid = false;
		}
	}

	VideoSourceStream::~VideoSourceStream()
	{
		destroy();
	}

	static inline const char* BoolToString(bool value)
	{
		return value ? "true" : "false";
	}

	static const char* getUsageString(VideoSourceStream::Usage usage)
	{
		switch(usage)
		{
			case VideoSourceStream::Usage::RGB32Read: return "RGB32Read";
			case VideoSourceStream::Usage::RGB24Read: return "RGB24Read";
			case VideoSourceStream::Usage::NV12Read: return "NV12Read";
			default: return "Undefined";
		}
	}

	void VideoSourceStream::dump() const
	{
		const char* formatStr = 
		"VideoSourceStream:\n"
		"\tusage: %s\n"
		"\tInput Sample Size: %lu\n"
		"\tOutput Sample Size: %lu\n"
		"\tInput Encoding: %s\n"
		"\tOutput Encoding: %s\n"
		"\tisInputCompressedFormat: %s\n"
		"\tisOutputCompressedFormat: %s\n"
		"\tisInputTemporalCompression: %s\n"
		"\tisOuputTemporalCompression: %s\n"
		"\tisInputFixedSizedSamples: %s\n"
		"\tisOutputFixedSizedSamples: %s\n"
		"\tinput frame size: %lu x %lu\n"
		"\toutput frame size: %lu x %lu\n"
		"\tinput frame rate: %.2f\n"
		"\tonput frame rate: %.2f";

		auto inputFrameSize = getInputFrameSize();
		auto outputFrameSize = getOutputFrameSize();

		debug_log_info(formatStr,
								getUsageString(m_usage),
								getInputSampleSize(),
								getOuputSampleSize(),
								getInputEncodingFormatStr(),
								getOutputEncodingFormatStr(),
								BoolToString(isInputCompressedFormat()),
								BoolToString(isOutputCompressedFormat()),
								BoolToString(isInputTemporalCompression()),
								BoolToString(isOuputTemporalCompression()),
								BoolToString(isInputFixedSizedSamples()),
								BoolToString(isOutputFixedSizedSamples()),
								inputFrameSize.first, inputFrameSize.second,
								outputFrameSize.first, outputFrameSize.second,
								getInputFrameRateF32(),
								getOutputFrameRateF32());
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

	bool VideoSourceStream::doReadyRGBReader()
	{
		if(m_videoColorConverter == NULL)
			return false;

		_assert((m_usage == Usage::RGB32Read) || (m_usage == Usage::RGB24Read));

		if(m_videoColorConverter->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0) != S_OK)
		{
			debug_log_error("Failed to Process Message MFT_MESSAGE_NOTIFY_BEGIN_STREAMING");
			return false;
		}
		return true;
	}

	bool VideoSourceStream::readRGBFrameToBuffer(u8* const rgbBuffer, u32 rgbBufferSize)
	{
		_assert((m_usage == Usage::RGB32Read) || (m_usage == Usage::RGB24Read));
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
			_assert(totalLength <= m_inputSampleSize);
		}
		else
			debug_log_error("Unable to verify total sample length");

		// pSample->AddRef();
		// auto totalMaxSize = GetTotalMaxSize(pSample);
		// _assert(totalMaxSize == m_inputSampleSize);

PROCESS_INPUT:

		if(m_videoColorConverter->ProcessInput(0, pSample, 0) != S_OK)
		{
			debug_log_error("Failed to Process Input");
			return false;
		}
		else
		{
			debug_log_info("Processed Input");
		}

		DWORD flags;
		HRESULT result;
		if((result = m_videoColorConverter->GetOutputStatus(&flags)) != S_OK)
		{
			if(result != E_NOTIMPL)
			{
				debug_log_error("Transform type is not set");
				goto RELEASE_RES_FALSE;
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
			goto RELEASE_RES_FALSE;
		}

		/* convert to RGB and the copy the data to the RGB Buffer */
		switch(m_usage)
		{
			case Usage::RGB24Read:
			{
				for(u32 i = 0; i < currentLength; i += 3)
				{
					rgbBuffer[(i / 3) * 4 + 0] = pMappedBuffer[i + 0];
					rgbBuffer[(i / 3) * 4 + 1] = pMappedBuffer[i + 1];
					rgbBuffer[(i / 3) * 4 + 2] = pMappedBuffer[i + 2];
					rgbBuffer[(i / 3) * 4 + 3] = 255;
				}
				break;
			}
			case Usage::RGB32Read:
			{
				for(u32 i = 0; i < currentLength; i += 4)
				{
					rgbBuffer[i + 0] = pMappedBuffer[i + 0];
					rgbBuffer[i + 1] = pMappedBuffer[i + 1];
					rgbBuffer[i + 2] = pMappedBuffer[i + 2];
					rgbBuffer[i + 3] = pMappedBuffer[i + 3];
				}
				break;
			}
		}

		if(m_stagingMediaBuffer->Unlock() != S_OK)
		{
			debug_log_error("Failed to unlock the staging media buffer");
		}

		pSample->Release();
		return true;

RELEASE_RES_FALSE:
		pSample->Release();
		return false;
	}

	bool VideoSourceStream::readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize)
	{
		_assert(m_usage == Usage::NV12Read);

		DWORD streamIndex;
		DWORD streamFlags;
		LONGLONG timeStamp;
		IMFSample* pSample;
		// StopWatch stopwatch;
		if(m_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &streamFlags, &timeStamp, &pSample) != S_OK)
		{
			debug_log_error("Unable to read sample");
			return false;
		}
		// stopwatch.stop();
		if(pSample == NULL)
		{
			debug_log_error("IMFSample data is NULL");
			return false;
		}

		DWORD totalLength;
		if(pSample->GetTotalLength(&totalLength) != S_OK)
		{
			debug_log_error("Unable to get total length of the IMFSample");
			pSample->Release();
			return false;
		}
		else _assert(totalLength == m_inputSampleSize);

		/* copy the non-contiguous data to the staging media buffer */
		if(pSample->CopyToBuffer(m_stagingMediaBuffer) != S_OK)
		{
			debug_log_error("Failed to copy to the staging media buffer");
			pSample->Release();
			return false;
		}

		BYTE* pMappedBuffer;
		DWORD currentLength;
		if(m_stagingMediaBuffer->Lock(&pMappedBuffer, NULL, &currentLength) != S_OK)
		{
			debug_log_error("Failed to lock the staging media buffer");
			pSample->Release();
			return false;
		}

		/* convert to RGB and the copy the data to the RGB Buffer */
		_assert(nv12BufferSize == currentLength);
		memcpy(nv12Buffer, pMappedBuffer, nv12BufferSize);

		if(m_stagingMediaBuffer->Unlock() != S_OK)
		{
			debug_log_error("Failed to unlock the staging media buffer");
		}

		pSample->Release();
		return true;
	}

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
				_assert(false);
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

		// _assert((inputStreamInfo.dwFlags & MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE) == MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE);
		m_inputSampleSize = inputStreamInfo.cbSize;
		_assert(((m_width * m_height * 3) >> 1) == m_inputSampleSize);

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
			_assert((m_width * m_height * (bitsPerPixel >> 3)) == m_outputSampleSize);
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

	NV12ToRGBConverter::NV12ToRGBConverter(NV12ToRGBConverter&& converter) :
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
		_assert(nv12BufferSize == m_inputSampleSize);

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