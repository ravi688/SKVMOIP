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

#define LISTEN_IP_ADDRESS "192.168.1.12"
#define LISTEN_PORT_NUMBER "2020"

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

int main(int argc, const char* argv[])
{
	Win32::InitializeMediaFundationAndCOM();
	debug_log_info("Platform is Windows");

	std::optional<Win32::Win32SourceDeviceListGuard> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if(!deviceList)
	{
		debug_log_error("Unable to get Video source device list");
		return 0;
	}

	const char* listenIPAddress = GetLocalIPAddress();
	if(listenIPAddress == NULL)
	{
		DEBUG_LOG_ERROR("Failed to determine local IP address for listening");
		return -1;
	}

	Network::Socket listenSocket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	if(listenSocket.bind(listenIPAddress, LISTEN_PORT_NUMBER) != Network::Result::Success)
	{
		debug_log_error("Failed to bind list socket to %s:%s", listenIPAddress, LISTEN_PORT_NUMBER);
		return 1;
	}

	u32 deviceIndex = 0;
	u32 numMaxConnections = 4;
	u32 numConnections = 0;
	do
	{
		DEBUG_LOG_INFO("Listening on %s:%s", listenIPAddress, LISTEN_PORT_NUMBER);
		auto result = listenSocket.listen();
		if(result != Network::Result::Success)
		{
			DEBUG_LOG_ERROR("Unable to listen");
			continue;
		}
		if(numConnections >= numMaxConnections)
		{
			DEBUG_LOG_INFO("Max number of connections (=%u) is reached", numMaxConnections);
			continue;
		}
		if(std::optional<Network::Socket> acceptedSocket = listenSocket.accept())
		{
			DEBUG_LOG_INFO("Connection accepted");
			_assert(acceptedSocket->isConnected());
			Network::Socket streamSocket = Network::Socket::CreateInvalid();
			streamSocket = std::move(*acceptedSocket);

			std::optional<Win32::Win32SourceDevice> device = deviceList->activateDevice(deviceIndex++);
			if(!device)
			{
				debug_log_error("Unable to create video source device with index: %lu ", deviceIndex);
				streamSocket.close();
				continue;
			}
			auto thread = std::thread([](Win32::Win32SourceDevice&& device, Network::Socket&& socket)
			{
				HDMIEncodeNetStream netStream(device, std::move(socket));
				netStream.start();
			}, std::move(device.value()), std::move(streamSocket));
			thread.detach();
			++numConnections;
		}
		else
		{
			DEBUG_LOG_ERROR("Unable to accept incoming connection");
			continue;
		}
	} while(true);

	Win32::DeinitializeMediaFoundationAndCOM();
	return 0;
}
