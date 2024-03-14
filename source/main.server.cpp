#ifndef BUILD_SERVER
#error "BUILD_SERVER is not defined, but still main.server.cpp is being compiled"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>

#include <SKVMOIP/HDMIEncodeNetStream.hpp>

#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <fstream>

#include <conio.h>
#include <ctype.h> // isdigit

#define LISTEN_PORT_NUMBER "2020"
#define MAX_CONNECTIONS 4
#define DEVICE_RELEASE_COOL_DOWN_TIME 4000 /* 4 seconds */
#define PERSISTENT_DATA_FILE_PATH "./.data"

static std::atomic<u32> gNumConnections = 0;

static std::mutex gMutex;
static std::vector<u32> gAvailableDevices;
static std::unordered_map<u32, u32> gDeviceIDMap;

/* Key: Client ID (u32), Value: Video Stream associated with the client */
static std::unordered_map<u32, Network::Socket> gStreamSockets;

enum class MessageCode : u8
{
	StreamStart = 0,
	StreamStop,
	Client,
	Stream
};

static u32 GenerateClientID() noexcept
{
	static u32 id = 0;
	return id++;
}

/* Algorithm:

	while listen:

		socket = accept
		header = socket.receive(byte)
		if header == Client:
			id = GenerateClientID()
			socket.send(id)
			clients.insert(id, socket)
		else if header == Stream:
			id = socket.receive(byte)
			streams.insert(id, socket)
*/

using namespace SKVMOIP;

const char* GetLocalIPAddress()
{
   char host[256];
   char *IP;
   struct hostent *host_entry;
   int hostname;
   hostname = gethostname(host, sizeof(host)); //find the host name
   if(hostname == -1)
   		return NULL;
   host_entry = gethostbyname(host); //find host information
   if(host_entry == NULL)
   		return NULL;
   IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
   if(IP == NULL)
   		return NULL;
   return IP;
}

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

ReadSample: 5 ms to 11 ms
Encode: 2 ms to 5 ms
Network: 0 ms
Decode: 5 ms to 10 ms

Convert: 5 ms to 10 ms
Frame Copy: 1 ms to 3 ms
Draw Surface Copy: 1 ms to 4 ms
Blit: 1 ms to 5 ms


Total: { 20 ms, 48 ms }

Convert to Blit takes: 22 ms in worst case and 8 ms in best case.

Convert, Frame Copy, Draw Surface Copy, and Blit - can be merged into one using a GPU program 
																	and latency can be reduced to 2 ms to 3 ms (expected)

Expected Total (after GPU program): { 14 ms, 29 ms }

*/

static void PrintHelp()
{
	DEBUG_LOG_INFO("Correct usage: ./server <port number>\n"
						"\tExample usage: ./server 2000\n");
}

struct CmdOptions
{
	const char* portNumberStr;
	u16 portNumber;
};

static bool IsPositiveInteger(const char* str)
{
	u32 len = strlen(str);
	for(u32 i = 0; i < len; i++)
		if(!isdigit(str[i]))
			return false;
	return true;
}

static std::optional<CmdOptions>	ParseCmdOptions(int argc, const char* argv[])
{
	CmdOptions options = { LISTEN_PORT_NUMBER, static_cast<u16>(atoi(LISTEN_PORT_NUMBER)) };
	if(argc <= 1)
		return { options };

	const char* arg = argv[1];
	if(!IsPositiveInteger(arg))
	{
		DEBUG_LOG_ERROR("Invalid portNumber");
		PrintHelp();
		return { };
	}
	options.portNumberStr = arg;
	options.portNumber = atoi(arg);
	return { options };
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
static std::unordered_map<u32, u32> GetDeviceMap(Win32::Win32SourceDeviceList& list)
{
	std::unordered_map<u32, u32> map;
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

static void DumpDeviceMap(const std::unordered_map<u32, u32>& map)
{
	for(const std::pair<u32, u32>& pair : map)
		DEBUG_LOG_INFO("Device: %lu --> %lu", pair.first, pair.second);
}


int main(int argc, const char* argv[])
{
	std::optional<CmdOptions> cmdOptions = ParseCmdOptions(argc, argv);
	if(!cmdOptions.has_value())
		return -1;

	Win32::InitializeMediaFundationAndCOM();
	debug_log_info("Platform is Windows");

	std::optional<Win32::Win32SourceDeviceListGuard> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if(!deviceList)
	{
		debug_log_error("Unable to get Video source device list");
		return 0;
	}
 
 	DEBUG_LOG_INFO("Devices Found: %u", deviceList->getDeviceCount());
	Win32::Win32DumpSourceDevices(*deviceList);

 	/* Populate the std::vector with the available device ids - in this (initially) case, all devices would be available */
	gAvailableDevices.reserve(deviceList->getDeviceCount());
	for(s32 i = deviceList->getDeviceCount() - 1; i >= 0; --i)
		gAvailableDevices.push_back(static_cast<u32>(i));

	gDeviceIDMap = GetDeviceMap(*deviceList);
	DumpDeviceMap(gDeviceIDMap);

	const char* listenIPAddress = GetLocalIPAddress();
	if(listenIPAddress == NULL)
	{
		DEBUG_LOG_ERROR("Failed to determine local IP address for listening");
		return -1;
	}

	Network::Socket listenSocket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	if(listenSocket.bind(listenIPAddress, cmdOptions->portNumberStr) != Network::Result::Success)
	{
		debug_log_error("Failed to bind list socket to %s:%s", listenIPAddress, cmdOptions->portNumberStr);
		return 1;
	}

	do
	{
		DEBUG_LOG_INFO("Listening on %s:%s", listenIPAddress, cmdOptions->portNumberStr);
		auto result = listenSocket.listen();
		if(result != Network::Result::Success)
		{
			DEBUG_LOG_ERROR("Unable to listen, Retrying...");
			continue;
		}
		if(gNumConnections >= MAX_CONNECTIONS)
		{
			DEBUG_LOG_INFO("Max number of connections (=%u) is reached, Refused to connect!", MAX_CONNECTIONS);
			continue;
		}
		if(std::optional<Network::Socket> acceptedSocket = listenSocket.accept())
		{
			DEBUG_LOG_INFO("Connection accepted");
			_assert(acceptedSocket->isConnected());
			Network::Socket streamSocket = Network::Socket::CreateInvalid();
			streamSocket = std::move(*acceptedSocket);

			std::optional<Win32::Win32SourceDevice> device { };
			{
				std::unique_lock<std::mutex> lock(gMutex);
				/* Get ID of the one of the available devices */
				u32 deviceIndex;
				{
					if(gAvailableDevices.size() <= 0)
					{
						DEBUG_LOG_ERROR("No more devices to allocate, all are still being used by other connections");
						continue;
					}
					deviceIndex = gAvailableDevices.back();
					DEBUG_LOG_INFO("Device ID allocated: %lu", deviceIndex);
				}

				device = deviceList->activateDevice(gDeviceIDMap[deviceIndex]);
				if(!device)
				{
					DEBUG_LOG_ERROR("Unable to create video source device with index: %lu, Closing connection...", deviceIndex);
					streamSocket.close();
					continue;
				}

				/* Now we are confirmed to have access to the device, therefore remove it from the list of Available Devices */
				gAvailableDevices.pop_back();
				++gNumConnections;
			}

			_assert(device.has_value());

			auto thread = std::thread([](Win32::Win32SourceDevice&& device, Network::Socket&& socket, std::mutex& mtx)
			{
				u32 id = device.getID();
				{
					HDMIEncodeNetStream netStream(std::move(device), std::move(socket));
					netStream.start();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(DEVICE_RELEASE_COOL_DOWN_TIME));

				/* Put this device ID back into the Available Devices list */
				{
						std::unique_lock<std::mutex> lock(mtx);
						gAvailableDevices.push_back(id);
						--gNumConnections;
				}
			}, std::move(device.value()), std::move(streamSocket), std::ref(gMutex));
			thread.detach();
		}
		else
		{
			DEBUG_LOG_ERROR("Unable to accept incoming connection");
			continue;
		}
	} while(true);

	Win32::DeinitializeMediaFoundationAndCOM();

	DEBUG_LOG_INFO("Press any key to exit...");
	getch();
	return 0;
}
