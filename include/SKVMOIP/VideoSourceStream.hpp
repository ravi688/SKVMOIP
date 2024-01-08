#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <utility>

namespace SKVMOIP
{
	typedef u64 VideoSourceDeviceConnectionID;

	static inline VideoSourceDeviceConnectionID CreateVideoSourceDeviceConnectionID(u16 vid, u16 pid, u16 usbPortNumber)
	{
		return BIT64_PACK32(usbPortNumber, BIT32_PACK16(vid, pid));
	}

	class VideoSourceStream
	{
	public:

		struct Frame
		{
			u16* pixels;
			u32 pixelCount;
		};

	private:
		IMFSourceReader* m_sourceReader;
		IMFMediaType* m_mediaType;
		IMFMediaBuffer* m_stagingBuffer;
		u32 m_sampleSize;
		GUID m_encodingFormat;
		bool m_isValid;

	private:
		void destroy();

	public:
		VideoSourceStream(VideoSourceStream& stream) = delete;
		VideoSourceStream& operator=(VideoSourceStream& stream) = delete;

		VideoSourceStream(VideoSourceDeviceConnectionID deviceID);
		VideoSourceStream(Win32::Win32SourceDevice& device);
		VideoSourceStream(VideoSourceStream&& stream);
		~VideoSourceStream();

		bool isValid() const { return m_isValid; }
		operator bool() const { return isValid();  }

		std::optional<bool> isCompressedFormat();
		std::optional<u32> getSampleSizeInBytes() const;
		std::optional<std::pair<u32, u32>> getFrameSize();
		std::optional<std::pair<u32, u32>> getFrameRate();
		std::optional<const char*> getEncodingFormatStr();

		bool readRGBFrameToBuffer(u8* const rgbBuffer);
	};
}
