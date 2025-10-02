#ifndef BUILD_SERVER
#error "BUILD_SERVER is not defined, but still main.server.cpp is being compiled"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>

#include <SKVMOIP/HDMIEncodeNetStream.hpp>
#include <SKVMOIP/Protocol.hpp>

#include <netsocket/netinterface.hpp>

#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <fstream>
#include <algorithm>

#include <conio.h>
#include <ctype.h> // isdigit

#undef _ASSERT
#include <spdlog/spdlog.h>

#define LISTEN_PORT_NUMBER "2020"
#define MAX_CONNECTIONS 4
#define DEVICE_RELEASE_COOL_DOWN_TIME 4000 /* 4 seconds */
#define PERSISTENT_DATA_FILE_PATH "./.server.data"

using namespace SKVMOIP;


static std::atomic<u32> gNumConnections = 0;

static std::mutex gMutex;
static std::vector<u32> gAvailableDevices;
/* Key: Client ID (u32), Value: Video Stream associated with the client */
static std::mutex gMutexStreamSockets;
static std::unordered_map<u32, netsocket::Socket> gStreamSockets;

static std::unordered_map<u32, u32> gDeviceIDMap;
static std::unique_ptr<Win32::Win32SourceDeviceListGuard> gDeviceList;


class Runner
{
private:
	std::unique_ptr<std::thread> m_thread;
	std::unique_ptr<HDMIEncodeNetStream> m_netStream;
	std::optional<Win32::Win32SourceDevice> m_device;
	u32 m_deviceID;
	bool m_isRunning;

public:
	Runner(u32 deviceID, u32 clientID);
	~Runner();

	void stop();
	bool isRunning() const noexcept { return m_isRunning; }
};

Runner::Runner(u32 deviceID, u32 clientID) : m_deviceID(deviceID), m_isRunning(true)
{
	{
		std::unique_lock<std::mutex> lock(gMutexStreamSockets);
		auto it = gStreamSockets.find(clientID);
		if(it == gStreamSockets.end())
		{
			DEBUG_LOG_ERROR("Unable to find stream socket for client id: %lu", clientID);
			m_isRunning	 = false;
			return;
		}

		netsocket::Socket& streamSocket = it->second;
		if(!streamSocket.isConnected())
		{
			DEBUG_LOG_ERROR("Stream socket is not connected for client id: %lu", clientID);
			m_isRunning = false;
			return;
		}

		{
			std::unique_lock<std::mutex> lock(gMutex);
			auto it = std::find(gAvailableDevices.begin(), gAvailableDevices.end(), m_deviceID);
			if(it == gAvailableDevices.end())
			{
				DEBUG_LOG_ERROR("Unable to acquire device with id: %lu, it may be still in use by another Runner", m_deviceID);
				m_isRunning = false;
				return;
			}
			m_device = gDeviceList->activateDevice(gDeviceIDMap[deviceID]);
			if(!m_device)
			{
				DEBUG_LOG_ERROR("Unable to create video source device with index: %lu, Closing connection...", deviceID);
				m_isRunning = false;
				return;
			}
			skvmoip_debug_assert(m_device.has_value());
			gAvailableDevices.erase(it);
			++gNumConnections;
		}

		m_netStream = std::move(std::unique_ptr<HDMIEncodeNetStream>(new HDMIEncodeNetStream(std::move(*m_device), std::move(streamSocket))));
	}

	m_thread = std::move(std::unique_ptr<std::thread>(new std::thread([](HDMIEncodeNetStream& netStream, u32 deviceID, bool& isRunning, std::mutex& mtx)
	{
		netStream.start();
		std::this_thread::sleep_for(std::chrono::milliseconds(DEVICE_RELEASE_COOL_DOWN_TIME));
		
		std::unique_lock<std::mutex> lock(mtx);
		gAvailableDevices.push_back(deviceID);
		--gNumConnections;
		
		isRunning = false;
	}, std::ref(*m_netStream), m_deviceID, std::ref(m_isRunning), std::ref(gMutex))));
}

Runner::~Runner()
{
	stop();
}

void Runner::stop()
{
	if(m_netStream)
		m_netStream->stop();

	if(m_thread)
		m_thread->join();
}

static u32 GenerateClientID() noexcept
{
	static u32 id = 0;
	return id++;
}

/* Algorithm:
	
	stream_runner:
		while socket.connected() && device.connected() && !semaphore:
			frame = device.get_frame()
			socket.send(frame)

	start_stream:
		device = activate_device(device id)
		socket = streams.find(client id)
		semaphore = create semaphore
		thread = create thread (stream_runner, device, socket, semaphore);
		return stream_handle(thread, semaphore);
	
	stream_handle.close:
		semaphore = stop
		thread.join()
		delete semaphore

	client_thread_function:
		handle = create empty handle
		while socket is not closed:
			cmd = socket.receive(u8)
			if error:
				break;
			if cmd == Start:
				if handle has value:
					handle.stop()
				d_id = socket.receive(u8)
				handle = call start_stream(device id : d_id, client id : c_id)
			else if cmd == Stop:
				if handle has value:
					handle.stop()
		socket.close()
		handle.stop()

	while listen:
		socket = accept
		header = socket.receive(byte)
		if header == Control:
			id = GenerateClientID()
			socket.send(id)
			thread = create thread (client_thread_function, socket)
			thread.detach()
		else if header == Stream:
			id = socket.receive(byte)
			streams.insert(id, move(socket))
*/

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

	std::optional<std::unique_ptr<Win32::Win32SourceDeviceListGuard>> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if(!deviceList)
	{
		debug_log_error("Unable to get Video source device list");
		return 0;
	}

	gDeviceList = std::move(*deviceList);
 
 	DEBUG_LOG_INFO("Devices Found: %u", gDeviceList->getDeviceCount());
	Win32::Win32DumpSourceDevices(*gDeviceList);

 	/* Populate the std::vector with the available device ids - in this (initially) case, all devices would be available */
	gAvailableDevices.reserve(gDeviceList->getDeviceCount());
	for(s32 i = gDeviceList->getDeviceCount() - 1; i >= 0; --i)
		gAvailableDevices.push_back(static_cast<u32>(i));

	gDeviceIDMap = GetDeviceMap(*gDeviceList);
	DumpDeviceMap(gDeviceIDMap);

	const std::string listenIPAddress = netsocket::GetIPv4Address("192.168.1.1");

	netsocket::Socket listenSocket(netsocket::SocketType::Stream, netsocket::IPAddressFamily::IPv4, netsocket::IPProtocol::TCP);
	if(listenSocket.bind(listenIPAddress, cmdOptions->portNumberStr) != netsocket::Result::Success)
	{
		debug_log_error("Failed to bind list socket to %s:%s", listenIPAddress.c_str(), cmdOptions->portNumberStr);
		return 1;
	}

	do
	{
		DEBUG_LOG_INFO("Listening on %s:%s", listenIPAddress.c_str(), cmdOptions->portNumberStr);
		auto result = listenSocket.listen();
		
		if(result != netsocket::Result::Success)
		{
			DEBUG_LOG_ERROR("Unable to listen, Retrying...");
			continue;
		}
		
		if(gNumConnections >= MAX_CONNECTIONS)
		{
			DEBUG_LOG_INFO("Max number of connections (=%u) is reached, Refused to connect!", MAX_CONNECTIONS);
			continue;
		}
		
		if(std::optional<netsocket::Socket> acceptedSocket = listenSocket.accept())
		{
			DEBUG_LOG_INFO("Connection accepted");
			skvmoip_debug_assert(acceptedSocket->isConnected());
			
			netsocket::Socket socket = netsocket::Socket::CreateInvalid();
			
			socket = std::move(*acceptedSocket);
			
			DEBUG_LOG_INFO("Waiting for the socket type");
			u8 socketType;
			if(socket.receive(&socketType, sizeof(u8)) != netsocket::Result::Success)
			{
				DEBUG_LOG_ERROR("Unable to receive socket type, closing connection");
				socket.close();
				continue;
			}

			if(socketType == EnumClassToInt(SocketType::Control))
			{
				DEBUG_LOG_INFO("Socket Type: Control");
				u32 clientID = GenerateClientID();
				DEBUG_LOG_INFO("Client ID Generated: %lu", clientID);
				if(socket.send(reinterpret_cast<u8*>(&clientID), sizeof(u32)) != netsocket::Result::Success)
				{
					DEBUG_LOG_ERROR("Unable to send client id, closing connection");
					socket.close();
					continue;
				}
				auto thread = std::thread([](netsocket::Socket&& socket, u32 clientID)
				{
					std::unique_ptr<Runner> runner;
					while(socket.isConnected())
					{
						DEBUG_LOG_INFO("Receiving control message...");
						u8 controlMessage;
						if(socket.receive(&controlMessage, sizeof(u8)) != netsocket::Result::Success)
						{
							DEBUG_LOG_ERROR("Failed to receive control message for client with ID: %lu, disposing client", clientID);
							/* close the control socket */
							debug_log_info("Closing control socket");
							socket.close();
							/* close the runner (encoder) */
							if(runner)
							{
								debug_log_info("Destroying Runner");
								runner.reset();
							}
							/* close the stream socket and remove it from the stream sockets list */
							auto it = gStreamSockets.find(clientID);
							if(it != gStreamSockets.end())
							{
								debug_log_info("Closing stream socket");
								netsocket::Socket& streamSocket = it->second;
								streamSocket.close();
								gStreamSockets.erase(it);
							}
							debug_log_info("Client is desposed!");
							break;
						}

						if(controlMessage == EnumClassToInt(Message::Start))
						{
							u8 deviceID;
							if(socket.receive(&deviceID, sizeof(u8)) != netsocket::Result::Success)
							{
								DEBUG_LOG_ERROR("Failed to receive device ID for Start command, ignoring the start command");
								socket.close();
								continue;
							}
							DEBUG_LOG_INFO("Device ID received: %u", deviceID);
							if(runner)
								runner.reset();
							runner = std::move(std::unique_ptr<Runner>(new Runner(deviceID, clientID)));
							if(!runner->isRunning())
							{
								DEBUG_LOG_ERROR("Failed to start the stream runner");
								runner.reset();
							}
						}
						else if(controlMessage == EnumClassToInt(Message::Stop))
						{
							if(runner)
								runner.reset();
						}
						else
							DEBUG_LOG_ERROR("Unrecognized control message: %u, ignored", controlMessage);
					}
				}, std::move(socket), clientID);
				thread.detach();
			}
			else if(socketType == EnumClassToInt(SocketType::Stream))
			{
				DEBUG_LOG_INFO("Socket Type: Control");
				u32 clientID;
				if(socket.receive(reinterpret_cast<u8*>(&clientID), sizeof(u32)) != netsocket::Result::Success)
				{
					DEBUG_LOG_ERROR("Unable to receive client id, refusing stream socket");
					socket.close();
					continue;
				}
				else DEBUG_LOG_INFO("Client ID received: %lu", clientID);

				{
					std::unique_lock<std::mutex> lock(gMutexStreamSockets);

					auto it = gStreamSockets.find(clientID);
					if(it != gStreamSockets.end())
					{
						DEBUG_LOG_WARNING("Stream socket with client id %lu already exists, closing the existing one", clientID);
						it->second.close();
						gStreamSockets.erase(it);
					}

					std::pair<u32, std::unordered_map<u32, netsocket::Socket>*>* userData = new std::pair<u32, std::unordered_map<u32, netsocket::Socket>*> { clientID, &gStreamSockets };
					socket.setOnDisconnect([](netsocket::Socket& socket, void* userData)
					{
						auto& data = *reinterpret_cast<std::pair<u32, std::unordered_map<u32, netsocket::Socket>*>*>(userData);
						{
							std::unique_lock<std::mutex> lock(gMutexStreamSockets);
							auto it = data.second->find(data.first);
							skvmoip_debug_assert(it != data.second->end());
							data.second->erase(it);
						}
						delete &data;
					}, reinterpret_cast<void*>(userData));
				
					gStreamSockets.insert({ clientID, std::move(socket) });
					netsocket::Socket& socketRef = gStreamSockets.at(clientID);
				
					u8 ackMessage = EnumClassToInt(Message::ACK);
					if(socketRef.send(&ackMessage, sizeof(u8)) != netsocket::Result::Success)
					{
						DEBUG_LOG_ERROR("Unable to send ACK Message to the stream socket just accpeted, refusing stream socket");
						socket.close();
						gStreamSockets.erase(gStreamSockets.find(clientID));
						continue;
					}
					else DEBUG_LOG_INFO("Acknowledgement is sent");
				}
			}
			else
			{
				DEBUG_LOG_ERROR("Unrecognized socket type, refused - closing");
				socket.close();
			}
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
