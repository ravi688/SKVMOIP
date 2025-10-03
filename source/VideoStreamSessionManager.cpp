#include <SKVMOIP/VideoStreamSessionManager.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Win32/Win32.hpp>
#include <common/platform.h>

#undef _ASSERT
#include <spdlog/spdlog.h>

#include <fstream>

#define PERSISTENT_DATA_FILE_PATH "./.server.data"

namespace SKVMOIP
{
#ifdef PLATFORM_WINDOWS

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
	static std::unordered_map<VideoSourceDeviceID, VideoSourceDeviceID> GetDeviceMap(Win32::Win32SourceDeviceList& list)
	{
		std::unordered_map<VideoSourceDeviceID, VideoSourceDeviceID> map;
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

	static void DumpDeviceMap(const std::unordered_map<VideoSourceDeviceID, VideoSourceDeviceID>& map)
	{
		for(const std::pair<VideoSourceDeviceID, VideoSourceDeviceID>& pair : map)
			DEBUG_LOG_INFO("Device: %lu --> %lu", pair.first, pair.second);
	}
#endif

	VideoStreamSessionManager::VideoStreamSessionManager()
	{
#ifdef PLATFORM_WINDOWS
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
#endif
	}

	VideoStreamSessionManager::~VideoStreamSessionManager()
	{
#ifdef PLATFORM_WINDOWS
		Win32::DeinitializeMediaFoundationAndCOM();		
#endif
	}

	com::OptionalReference<std::unique_ptr<VideoStreamSession>> VideoStreamSessionManager::createSessionForClient(ClientID clientID, VideoSourceDeviceID deviceID, netsocket::AsyncSocket& socket)
	{
#ifdef PLATFORM_WINDOWS
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

		std::unique_ptr<VideoSourceWindows> videoSource = std::make_unique<VideoSourceWindows>(*device, VideoSourceWindows::Usage::NV12Read, 
																					std::vector<std::tuple<u32, u32, u32>> {
																						{ 1920, 1080, 60 },
																						{ 1920, 1080, 30 },
																						{ 1366, 768, 60 },
																						{ 1366, 768, 30 },
																						{ 1280, 720, 60 },
																						{ 1280, 720, 30 },
																						{ 1024, 768, 60 },
																						{ 1024, 768, 30 }, 
																						{ 960, 720, 60 },
																						{ 960, 720, 30 }
																					});
#endif
		std::unique_ptr<VideoStreamSession> session = std::make_unique<VideoStreamSession>(*videoSource, socket);
		session->start();
		SessionContext sessionContext
		{
			.device = std::move(*device),
			.videoSource = std::move(videoSource),
			.streamSession = std::move(session)
		};
		m_sessionContexts.insert({ clientID, std::move(sessionContext) });
		auto& sessionRef = m_sessionContexts.at(clientID).streamSession;
		return { sessionRef };
	}

	com::OptionalReference<std::unique_ptr<VideoStreamSession>> VideoStreamSessionManager::getSessionForClient(ClientID clientID)
	{
		auto it = m_sessionContexts.find(clientID);
		if(it != m_sessionContexts.end())
			return { it->second.streamSession };
		return { };
	}

	void VideoStreamSessionManager::destroySessionForClient(ClientID clientID)
	{
		auto it = m_sessionContexts.find(clientID);
		if(it != m_sessionContexts.end())
		{
			// Stop the video stream
			auto& streamSession = it->second.streamSession;
			streamSession->stop();
#ifdef PLATFORM_WINDOWS
			auto& device = it->second.device;
			device.shutdown();
#endif

			// Destroy the video stream object
			m_sessionContexts.erase(it);
		}
		else
		{
			spdlog::error("Invalid call to destroySessionForClient(), No session was ever created for client {}", clientID);
		}
	}
}
