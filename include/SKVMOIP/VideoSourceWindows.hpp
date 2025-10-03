#pragma once

#include <SKVMOIP/VideoSource.hpp> // for IVideoSource
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <utility>
#include <vector>
#include <mfreadwrite.h>

namespace SKVMOIP
{
	SKVMOIP_API const char* getEncodingString(const GUID& guid);

	class VideoSourceWindows : public IVideoSource
	{
	private:
		Win32::Win32SourceDevice& m_device;
		IVideoSource::Usage m_usage;
		IMFSourceReader* m_sourceReader;
		IMFMediaType* m_inputMediaType;
		IMFMediaType* m_outputMediaType;
		IMFMediaBuffer* m_stagingMediaBuffer;
		IMFTransform* m_videoColorConverter;
		IMFSample* m_outputSample;
		u32 m_inputSampleSize;
		u32 m_outputSampleSize;
		u32 m_inputFrameWidth;
		u32 m_inputFrameHeight;
		u32 m_outputFrameWidth;
		u32 m_outputFrameHeight;
		u32 m_inputFrameRateNumer;
		u32 m_inputFrameRateDenom;
		u32 m_outputFrameRateNumer;
		u32 m_outputFrameRateDenom;
		GUID m_inputEncodingFormat;
		GUID m_outputEncodingFormat;
		bool m_isInputFixedSizedSamples;
		bool m_isOutputFixedSizedSamples;
		bool m_isInputTemporalCompression;
		bool m_isOutputTemporalCompression;
		bool m_isInputCompressedFormat;
		bool m_isOutputCompressedFormat;
		bool m_isValid;

	private:
		void destroy();

	public:

		VideoSourceWindows(IVideoSource::DeviceID deviceID,
							Win32::Win32SourceDevice& device,
							IVideoSource::Usage usage,
							const std::vector<std::tuple<u32, u32, u32>>& resPrefList);

		// Not copyable and not movable
		VideoSourceWindows(VideoSourceWindows& stream) = delete;
		VideoSourceWindows(VideoSourceWindows&& stream) = delete;

		~VideoSourceWindows();

		// IVideoSource Interface Implementation
		virtual IVideoSource::Result open() override;
		virtual void close() override;
		virtual bool isReady() override;
		virtual bool readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize) override;

		bool isValid() const { return m_isValid; }
		operator bool() const { return isValid();  }

		void dump() const;

		u32 getInputSampleSize() const { return m_inputSampleSize; }
		u32 getOuputSampleSize() const { return m_outputSampleSize; }
		GUID getInputEncodingFormat() const { return m_inputEncodingFormat; }
		GUID getOutputEncodingFormat() const { return m_outputEncodingFormat; }
		const char* getInputEncodingFormatStr() const { return getEncodingString(getInputEncodingFormat()); }
		const char* getOutputEncodingFormatStr() const { return getEncodingString(getOutputEncodingFormat()); }
		bool isInputCompressedFormat() const { return m_isInputCompressedFormat; }
		bool isOutputCompressedFormat() const { return m_isOutputCompressedFormat; }
		bool isInputTemporalCompression() const { return m_isInputTemporalCompression; }
		bool isOuputTemporalCompression() const { return m_isOutputTemporalCompression; }
		bool isInputFixedSizedSamples() const { return m_isInputFixedSizedSamples;}
		bool isOutputFixedSizedSamples() const { return m_isOutputFixedSizedSamples; }
		std::pair<u32, u32> getInputFrameSize() const { return { m_inputFrameWidth, m_inputFrameHeight }; }
		virtual std::pair<u32, u32> getOutputFrameSize() override { return { m_outputFrameWidth, m_outputFrameHeight }; }
		virtual std::pair<u32, u32> getInputFrameRate() override { return { m_inputFrameRateNumer, m_inputFrameRateDenom }; }
		std::pair<u32, u32> getOuputFrameRate() const { return { m_outputFrameRateNumer, m_outputFrameRateDenom }; }
		f32 getInputFrameRateF32() const { return static_cast<f32>(m_inputFrameRateNumer) / m_inputFrameRateDenom; }
		f32 getOutputFrameRateF32() const { return static_cast<f32>(m_outputFrameRateNumer) / m_outputFrameRateDenom; }
	};
}
