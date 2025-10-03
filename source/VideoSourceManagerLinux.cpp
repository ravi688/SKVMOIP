#include <SKVMOIP/VideoSourceManagerLinux.hpp>

namespace SKVMOIP
{
	VideoSourceManagerLinux::VideoSourceManagerLinux()
	{
	}

	VideoSourceManagerLinux::~VideoSourceManagerLinux()
	{
	}

	std::optional<std::unique_ptr<VideoSourceLinux>> VideoSourceManagerLinux::acquireVideoSource(IVideoSource::DeviceID deviceID, 
																										IVideoSource::Usage usage,
																										const std::vector<std::tuple<u32, u32, u32>>& resPrefList)
	{
		return { };
	}

	void VideoSourceManagerLinux::releaseVideoSource(std::unique_ptr<VideoSourceLinux>& videoSource)
	{
	}
}
