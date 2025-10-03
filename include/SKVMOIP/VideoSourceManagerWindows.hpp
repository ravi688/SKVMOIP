#pragma once

#include <SKVMOIP/VideoSourceManager.hpp>

#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp> // for Win32::Win32SourceDeviceListGuard
#include <SKVMOIP/VideoSourceWindows.hpp>

#include <unordered_map> // for std::unordered_map<>
#include <vector> // for std::vector<>
#include <memory> // for std::unique_ptr<>
#include <optional> // for std::optional<>

namespace SKVMOIP
{
	class VideoSourceManagerWindows : public IVideoSourceManager<VideoSourceWindows>
	{
	private:
		std::vector<IVideoSource::DeviceID> m_availableDevices;
		std::unordered_map<IVideoSource::DeviceID, IVideoSource::DeviceID> m_deviceIDMap;
		std::unique_ptr<Win32::Win32SourceDeviceListGuard> m_deviceList;

	public:
		VideoSourceManagerWindows();
		~VideoSourceManagerWindows();

		// Implementation of IVideoSourceManager interface
		virtual u32 getNumVideoSources() override;
		virtual std::optional<std::unique_ptr<VideoSourceWindows>> acquireVideoSource(IVideoSource::DeviceID deviceID,
																						IVideoSource::Usage usage,
																						const std::vector<std::tuple<u32, u32, u32>>& resPrefList) override;
		virtual void releaseVideoSource(std::unique_ptr<VideoSourceWindows>& videoSource) override;
	};
}
