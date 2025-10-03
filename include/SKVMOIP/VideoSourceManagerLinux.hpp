#pragma once

#include <SKVMOIP/VideoSourceManager.hpp>
#include <SKVMOIP/VideoSourceLinux.hpp>

#include <string> // for std::string
#include <vector> // for std::vector

namespace SKVMOIP
{
	struct VideoDevice
	{
	    std::string path;
	    std::string name;
	    bool hasRaw;
	    bool hasCompressed;
	    std::vector<std::string> formats;
	};

	class VideoSourceManagerLinux : public IVideoSourceManager<VideoSourceLinux>
	{
	private:
		std::vector<VideoDevice> m_devices;
	public:
		VideoSourceManagerLinux();
		~VideoSourceManagerLinux();

		// Implementation of IVideoSourceManager interface
		virtual u32 getNumVideoSources() override;
		virtual std::optional<std::unique_ptr<VideoSourceLinux>> acquireVideoSource(IVideoSource::DeviceID deviceID,
																						IVideoSource::Usage usage,
																						const std::vector<std::tuple<u32, u32, u32>>& resPrefList) override;
		virtual void releaseVideoSource(std::unique_ptr<VideoSourceLinux>& videoSource) override;
	};
}
