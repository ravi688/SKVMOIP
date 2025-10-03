#pragma once

#include <SKVMOIP/VideoSource.hpp> // for IVideoSource

#include <vector> // for std::vector<>
#include <string> // for std::string
#include <optional> // for std::optional<>

namespace SKVMOIP
{
	class VideoSourceLinux : public IVideoSource
	{
	private:
		struct Buffer
		{
    		void* start;
    		size_t length;
		};

		struct Device
		{
			int fd;
			std::vector<Buffer> buffers;
			std::string devicePath;
		};

		std::optional<Device> m_device;

	public:

		VideoSourceLinux(const std::string_view devicePath);
		~VideoSourceLinux();

		// Not copyable and not movable
		VideoSourceLinux(VideoSourceLinux&) = delete;
		VideoSourceLinux(VideoSourceLinux&&) = delete;

		// IVideoSource Interface Implementation
		virtual IVideoSource::Result open() override;
		virtual void close() override;
		virtual bool isReady() override;
		virtual bool readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize) override;
		virtual std::pair<u32, u32> getInputFrameRate() override;
		virtual std::pair<u32, u32> getOutputFrameSize() override;
	};
}
