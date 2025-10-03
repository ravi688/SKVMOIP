#include <SKVMOIP/VideoSourceLinux.hpp>

namespace SKVMOIP
{
		virtual IVideoSource::Result VideoSourceLinux::open() override
		{
			return IVideoSource::Result::Failed;
		}
		virtual void VideoSourceLinux::close() override
		{

		}
		virtual bool VideoSourceLinux::isReady() override
		{
			return false;
		}
		virtual bool VideoSourceLinux::readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize) override
		{
			return false;
		}
}
