#pragma once

#include <SKVMOIP/VideoSourceManager.hpp>
#include <SKVMOIP/VideoSourceLinux.hpp>

namespace SKVMOIP
{
	class VideoSourceManagerLinux : public IVideoSourceManager<VideoSourceLinux>
	{
	public:
		VideoSourceManagerLinux();
		~VideoSourceManagerLinux();

		// Implementation of IVideoSourceManager interface
		virtual std::optional<std::unique_ptr<VideoSourceLinux>> acquireVideoSource(IVideoSource::DeviceID deviceID,
																						IVideoSource::Usage usage,
																						const std::vector<std::tuple<u32, u32, u32>>& resPrefList) override;
		virtual void releaseVideoSource(std::unique_ptr<VideoSourceLinux>& videoSource) override;
	};
}
