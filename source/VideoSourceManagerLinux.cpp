#include <SKVMOIP/VideoSourceManagerLinux.hpp>

#ifdef _ASSERT
#	undef _ASSERT
#endif
#include <spdlog/spdlog.h>

#include <string>
#include <vector>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>
#include <cstdint>

namespace SKVMOIP
{
	static std::vector<VideoDevice> list_video_devices()
	{
	    std::vector<VideoDevice> devices;
	    DIR* devDir = opendir("/dev");
	    if (!devDir) return devices;

	    struct dirent* entry;
	    while ((entry = readdir(devDir)) != nullptr) {
	        if (strncmp(entry->d_name, "video", 5) == 0) {
	            std::string devPath = "/dev/" + std::string(entry->d_name);

	            int fd = open(devPath.c_str(), O_RDWR | O_NONBLOCK);
	            if (fd >= 0) {
	                v4l2_capability cap{};
	                if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
	                    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
	                        VideoDevice dev;
	                        dev.path = devPath;
	                        dev.name = (char*)cap.card;
	                        dev.hasRaw = false;
	                        dev.hasCompressed = false;

	                        v4l2_fmtdesc fmtdesc{};
	                        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	                        for (int i = 0;; i++) {
	                            fmtdesc.index = i;
	                            if (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == -1)
	                                break;

	                            std::string desc = (char*)fmtdesc.description;
	                            dev.formats.push_back(desc);

	                            uint32_t fmt = fmtdesc.pixelformat;
	                            if (fmt == V4L2_PIX_FMT_YUYV ||
	                                fmt == V4L2_PIX_FMT_NV12 ||
	                                fmt == V4L2_PIX_FMT_YUV420) {
	                                dev.hasRaw = true;
	                            }
	                            if (fmt == V4L2_PIX_FMT_MJPEG ||
	                                fmt == V4L2_PIX_FMT_H264) {
	                                dev.hasCompressed = true;
	                            }
	                        }
	                        if(dev.formats.size() != 0)
	                                devices.push_back(dev);
	                    }
	                }
	                close(fd);
	            }
	        }
	    }
	    closedir(devDir);
	    return devices;
	}

	static void DumpDeviceInfos(const std::vector<VideoDevice>& devices)
	{
		spdlog::info("Available Video Devices:");
    	for(size_t i = 0; i < m_devices.size(); i++)
    	{
    	    spdlog::info("{} : {} ({}) {}", i, m_devices[i].path, m_devices[i].name, 
    	    	m_devices[i].hasRaw ? "[RAW]" : "",
    	    	m_devices[i].hasRaw ? "[COMPRESSED]" : "");

    	    spdlog::info("Formats: ");
    	    for (auto& f : m_devices[i].formats)
    	        spdlog::info("\t {}", f);
    	}
	}

	VideoSourceManagerLinux::VideoSourceManagerLinux()
	{
    	m_devices = list_video_devices();

    	if(!m_devices.empty())
    		DumpDeviceInfos(m_devices);
    	else
    		spdlog::error("No video capture devices found");

    	u32 deviceCount = m_devices.size();
    	m_availableDevices.reserve(deviceCount);
    	for(u32 i = 0; i < deviceCount; ++i)
    		m_availableDevices.push_back(i);
	}

	VideoSourceManagerLinux::~VideoSourceManagerLinux()
	{
		skvmoip_assert(m_availableDevices.size() == m_devices.size()
			&& "Not all video source devices have been released destroying VideoSourceManagerLinux");
	}

	u32 VideoSourceManagerLinux::getNumVideoSources()
	{
		return m_devices.size();
	}

	std::optional<std::unique_ptr<VideoSourceLinux>> VideoSourceManagerLinux::acquireVideoSource(IVideoSource::DeviceID deviceID, 
																										IVideoSource::Usage usage,
																										const std::vector<std::tuple<u32, u32, u32>>& resPrefList)
	{
		skvmoip_assert(usage == IVideoSource::Usage::NV12Read);
		skvmoip_assert(deviceID < m_devices.size());

		auto it = std::ranges::find(m_availableDevices, deviceID);
		if(it == m_availableDevices.end())
		{
			spdlog::error("Unable to acquire device with id: {}, it has already been acquired");
			return { };
		}

		std::string& devicePath = m_devices[deviceID].path;
		
		std::unique_ptr<VideoSourceLinux> videoSource = std::make_unique<VideoSourceLinux>(devicePath);

		if(!videoSource->isReady())
		{
			videoSource.reset();
			return { };
		}

		m_availableDevices.erase(it);

		return { videoSource };
	}

	void VideoSourceManagerLinux::releaseVideoSource(std::unique_ptr<VideoSourceLinux>& videoSource)
	{
		auto it = std::ranges::find(m_availableDevices, videoSource->getDeviceID());
		if(it == m_availableDevices.end())
		{
			// Destroy the video source, it automatically shutsdown/closes
			videoSource.reset();
			m_availableDevices.push_back(videoSource->getDeviceID());
		}
		else
			spdlog::error("Invalid call to releaseVideoSource(), No video source was ever acquired for device id {}", videoSource->getDeviceID());
	}
}
