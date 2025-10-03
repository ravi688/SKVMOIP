#include <SKVMOIP/VideoSourceLinux.hpp>

namespace SKVMOIP
{
	IVideoSource::Result VideoSourceLinux::open()
	{
		return IVideoSource::Result::Failed;
	}
	void VideoSourceLinux::close()
	{

	}
	bool VideoSourceLinux::isReady()
	{
		return false;
	}
	bool VideoSourceLinux::readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize)
	{
		return false;
	}

	std::pair<u32, u32> VideoSourceLinux::getInputFrameRate()
	{
		return { };
	}

	std::pair<u32, u32> VideoSourceLinux::getOutputFrameSize()
	{
		return { };
	}
}

