#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/debug.h>

namespace SKVMOIP
{
	VideoSourceStream::VideoSourceStream(VideoSourceDeviceConnectionID deviceID)
	{

	}

	VideoSourceStream::VideoSourceStream(Win32::Win32SourceDevice& device)
	{
		HRESULT result = MFCreateSourceReaderFromMediaSource(device.getInternalHandle(), NULL, &m_sourceReader);
		if(result != S_OK)
		{
			debug_log_error("Unable to create SourceReader from MediaSource");
			m_isValid = false;
			return;
		}
		result = m_sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, MF_SOURCE_READER_CURRENT_TYPE_INDEX, &m_mediaType);
		if(result != S_OK)
		{
			debug_log_error("Unable to get NativeMediaType");
			m_sourceReader->Release();
			m_isValid = false;
			return;
		}
		m_isValid = true;
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

	std::optional<VideoSourceStream::Frame> VideoSourceStream::readFrame()
	{
		DWORD streamIndex;
		DWORD streamFlags;
		LONGLONG timeStamp;
		IMFSample* pSample;
		HRESULT result = m_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &streamFlags, &timeStamp, &pSample);
		if(result != S_OK)
		{
			debug_log_error("Unable to read sample");
			return { };
		}
		return { Frame { } };
	}
}