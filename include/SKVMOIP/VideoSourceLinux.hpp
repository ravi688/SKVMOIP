#pragma once

#include <SKVMOIP/VideoSource.hpp> // for IVideoSource

namespace SKVMOIP
{
	class VideoSourceLinux : public IVideoSource
	{
	public:
		
		// Not copyable and not movable
		VideoSourceLinux(VideoSourceLinux&) = delete;
		VideoSourceLinux(VideoSourceLinux&&) = delete;

		// IVideoSource Interface Implementation
		virtual IVideoSource::Result open() override;
		virtual void close() override;
		virtual bool isReady() override;
		virtual bool readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize) override;
	};
}