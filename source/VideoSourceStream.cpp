#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <strmif.h>
#include <mfapi.h>

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
		return "<unknown>";
	}

	VideoSourceStream::VideoSourceStream(Win32::Win32SourceDevice& device)
	{
		if(MFCreateSourceReaderFromMediaSource(device.getInternalHandle(), NULL, &m_sourceReader) != S_OK)
		{
			debug_log_error("Unable to create SourceReader from MediaSource");
			m_isValid = false;
			return;
		}

		if(m_sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, MF_SOURCE_READER_CURRENT_TYPE_INDEX, &m_mediaType) != S_OK)
		{
			debug_log_error("Unable to get NativeMediaType");
			goto RELEASE;
		}

		AM_MEDIA_TYPE* pInfo;
		if(m_mediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void**>(&pInfo)) != S_OK)
		{
			debug_log_error("Unable to get Representation");
			goto RELEASE;
		}

		_assert(pInfo->bFixedSizeSamples == TRUE);
		// _assert(pInfo->bTemporalCompression = FALSE);

		m_sampleSize = pInfo->lSampleSize;

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

		if(m_mediaType->FreeRepresentation(AM_MEDIA_TYPE_REPRESENTATION, reinterpret_cast<void*>(pInfo)) != S_OK)
		{
			debug_log_error("Unable to free Representation");
			goto RELEASE;
		}

		m_isValid = true;
		return;

		RELEASE:
			m_sourceReader->Release();
			m_isValid = false;
			return;
	}

	VideoSourceStream::VideoSourceStream(VideoSourceStream&& stream) : m_sourceReader(stream.m_sourceReader),
																	   m_mediaType(stream.m_mediaType),
																	   m_isValid(stream.m_isValid)
	{
		stream.m_sourceReader = NULL;
		stream.m_mediaType = NULL;
		stream.m_isValid = false;
	}

	void VideoSourceStream::destroy()
	{
		if(m_sourceReader != NULL)
		{
			m_sourceReader->Release();
			m_sourceReader = NULL;
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

	bool VideoSourceStream::readRGBFrameToBuffer(u8* const rgbBuffer)
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
		pSample->Release();
		return true;
	}
}