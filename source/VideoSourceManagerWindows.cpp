#include <SKVMOIP/VideoSourceManagerWindows.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Win32/Win32.hpp>

#undef _ASSERT
#include <spdlog/spdlog.h>

#include <fstream>

#define PERSISTENT_DATA_FILE_PATH "./.server.data"

namespace SKVMOIP
{
	static std::optional<std::vector<std::string>> ReadStringsFromFile(const char* file)
	{
		std::fstream stream;
		stream.open(file, std::ios::in);
	
		if(!stream.is_open())
			return { };
	
		std::vector<std::string> strings;
	
		char buffer[1024] = { };
		while(!stream.eof())
		{
			stream.getline(buffer, 1024);
			if(strlen(buffer) > 0)
				strings.push_back(std::string(buffer));
		}
		stream.close();
		if(strings.size() == 0)
			return { };
		return { strings };
	}
	
	static void WriteStringsToFile(const std::vector<std::string>& strings, const char* file)
	{
		std::fstream stream;
		stream.open(file, std::ios::out);
	
		if(!stream.is_open())
		{
			DEBUG_LOG_ERROR("Failed to write strings, Reason: Failed to open the file %s", file);
			return;
		}
		
		for(const std::string& str : strings)
		{
			stream.write(str.c_str(), str.size());
			stream << '\n';
		}
		stream.close();
	}

	/*
	
	  // prev ordering, from file
	  0 --> abcd0
	  1 --> abcd1
	  2 --> abcd2
	  3 --> abcd3
	  4 --> abcd4
	
	  // new ordering
	  0 --> abcd1
	  1 --> abcd0
	  2 --> abcd2
	  3 --> abcd4
	  4 --> abcd3
	
	  // Final Mapping
	  0 --> 1
	  1 --> 0
	  2 --> 2
	  3 --> 4
	  4 --> 3
	
	*/
	static std::unordered_map<IVideoSource::DeviceID, IVideoSource::DeviceID> GetDeviceMap(Win32::Win32SourceDeviceList& list)
	{
		std::unordered_map<IVideoSource::DeviceID, IVideoSource::DeviceID> map;
		std::optional<std::vector<std::string>> result = ReadStringsFromFile(PERSISTENT_DATA_FILE_PATH);
		if(result.has_value())
		{
			std::unordered_map<std::string, u32> newMap = list.getSymolicLinkToDeviceIDMap();
			const std::vector<std::string>& strings = result.value();
			if(strings.size() >= newMap.size())
			{
				bool isMapped = true;
				for(u32 i = 0; i < strings.size(); i++)
				{
					const std::string& str = strings[i];
					auto it = newMap.find(str);
					if(it != newMap.end())
						map.insert({ i, it->second });
					else
					{
						DEBUG_LOG_WARNING("A device is not present with symbolic id: %s", str.c_str());
						map.clear();
						isMapped = false;
						break;
					}
				}
				if(isMapped)
					return map;
			}
		}
		DEBUG_LOG_INFO("Using default device mapping");
		std::vector<std::string> strings;
		strings.reserve(list.getDeviceCount());
		for(u32 i = 0; i < list.getDeviceCount(); i++)
		{
			std::optional<std::string> str = list.getSymbolicLink(i);
			if(str.has_value())
				strings.push_back(*str);
			map.insert({ i, i });
		}
		WriteStringsToFile(strings, PERSISTENT_DATA_FILE_PATH);
		return map;
	}

	static void DumpDeviceMap(const std::unordered_map<IVideoSource::DeviceID, IVideoSource::DeviceID>& map)
	{
		for(const std::pair<IVideoSource::DeviceID, IVideoSource::DeviceID>& pair : map)
			DEBUG_LOG_INFO("Device: %lu --> %lu", pair.first, pair.second);
	}

	VideoSourceManagerWindows::VideoSourceManagerWindows()
	{
		Win32::InitializeMediaFundationAndCOM();
		spdlog::info("Platform is Windows");
		std::optional<std::unique_ptr<Win32::Win32SourceDeviceListGuard>> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		skvmoip_assert(deviceList && "Unable to get Video source device list");

		m_deviceList = std::move(*deviceList);
 
 		spdlog::info("Devices Found: {}", m_deviceList->getDeviceCount());
		Win32::Win32DumpSourceDevices(*m_deviceList);

 		/* Populate the std::vector with the available device ids - in this (initially) case, all devices would be available */
		m_availableDevices.reserve(m_deviceList->getDeviceCount());
		for(s32 i = m_deviceList->getDeviceCount() - 1; i >= 0; --i)
			m_availableDevices.push_back(static_cast<u32>(i));

		m_deviceIDMap = GetDeviceMap(*m_deviceList);
		DumpDeviceMap(m_deviceIDMap);
	}

	VideoSourceManagerWindows::~VideoSourceManagerWindows()
	{
		skvmoip_assert(m_availableDevices.size() == m_deviceIDMap.size()
			&& "Not all video source devices have been released before destroying VideoSourceManagerWindows");
		Win32::DeinitializeMediaFoundationAndCOM();		
	}

	std::optional<std::unique_ptr<VideoSourceWindows>> VideoSourceManagerWindows::acquireVideoSource(IVideoSource::DeviceID deviceID, 
																										IVideoSource::Usage usage,
																										const std::vector<std::tuple<u32, u32, u32>>& resPrefList)
	{
		auto it = std::find(m_availableDevices.begin(), m_availableDevices.end(), deviceID);
		if(it == m_availableDevices.end())
		{
			spdlog::error("Unable to acquire device with id: {}, it may be still in use by another Runner", deviceID);
			return { };
		}
		auto device = m_deviceList->activateDevice(m_deviceIDMap[deviceID]);
		if(!device)
		{
			spdlog::error("Unable to create video source device with index: {}, Closing connection...", deviceID);
			return { };
		}
		m_availableDevices.erase(it);

		std::unique_ptr<VideoSourceWindows> videoSource = std::make_unique<VideoSourceWindows>(deviceID, *device, usage, resPrefList);
		if(auto result = videoSource->open(); result != IVideoSource::Result::Success)
		{
			spdlog::error("Failed to open the video source deivce with id: {}", deviceID);
			return { };
		}
		if(!videoSource->isReady())
		{
			spdlog::error("Video source device is not ready, device id: {}", deviceID);
			return { };
		}

		return { std::move(videoSource) };
	}

	void VideoSourceManagerWindows::releaseVideoSource(std::unique_ptr<VideoSourceWindows>& videoSource)
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
