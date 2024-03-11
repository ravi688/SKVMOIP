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
		strings.push_back(std::string(buffer));
	}
	stream.close();
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

			/* Get ID of the one of the available devices */
			u32 deviceIndex;
			{
				{
					std::unique_lock<std::mutex> lock(gMutex);
					if(gAvailableDevices.size() <= 0)
					{
						DEBUG_LOG_ERROR("No more devices to allocate, all are still being used by other connections");
						continue;
					}
					deviceIndex = gAvailableDevices.back();
				}
				DEBUG_LOG_INFO("Device ID allocated: %lu", deviceIndex);
			}

			std::optional<Win32::Win32SourceDevice> device = deviceList->activateDevice(deviceIndex);
			if(!device)
			{
				DEBUG_LOG_ERROR("Unable to create video source device with index: %lu, Closing connection...", deviceIndex);
				streamSocket.close();
				continue;
			}

			/* Now we are confirmed to have access to the device, therefore remove it from the list of Available Devices */
			{
				std::unique_lock<std::mutex> lock(gMutex);
				gAvailableDevices.pop_back();
			}
			++gNumConnections;

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
