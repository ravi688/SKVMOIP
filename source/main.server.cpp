#ifndef BUILD_SERVER
#error "BUILD_SERVER is not defined, but still main.server.cpp is being compiled"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Protocol.hpp>
#include <SKVMOIP/VideoStreamSessionManager.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

#include <netsocket/netinterface.hpp>

#include <thread>
#include <memory>
#include <atomic>

#include <conio.h>

#undef _ASSERT
#include <spdlog/spdlog.h>

#define LISTEN_PORT_NUMBER "2020"

using namespace SKVMOIP;

/* Key: Client ID (u32), Value: Video Stream associated with the client */
static std::mutex gMutexStreamSockets;
static std::unordered_map<u32, std::unique_ptr<netsocket::AsyncSocket>> gStreamSockets;

static u32 GenerateClientID() noexcept
{
	static u32 id = 0;
	return id++;
}

static com::OptionalReference<std::unique_ptr<netsocket::AsyncSocket>> GetStreamSocketForClient(ClientID clientID)
{
	auto it = gStreamSockets.find(clientID);
	if(it != gStreamSockets.end())
		return { it->second };
	else
		return { };
}

int main(int argc, const char* argv[])
{
	std::mutex sessionManagerMutex;
	SKVMOIP::VideoStreamSessionManager sessionManager;

	const std::string listenIPAddress = netsocket::GetIPv4Address("192.168.1.1");

	netsocket::Socket listenSocket(netsocket::SocketType::Stream, netsocket::IPAddressFamily::IPv4, netsocket::IPProtocol::TCP);
	if(listenSocket.bind(listenIPAddress, LISTEN_PORT_NUMBER) != netsocket::Result::Success)
	{
		spdlog::error("Failed to bind list socket to {}:{}", listenIPAddress.c_str(), LISTEN_PORT_NUMBER);
		return 1;
	}

	do
	{
		spdlog::info("Listening on {}:{}", listenIPAddress, LISTEN_PORT_NUMBER);
		auto result = listenSocket.listen();
		
		if(result != netsocket::Result::Success)
		{
			spdlog::error("Unable to listen, Retrying...");
			continue;
		}
		
		if(std::optional<netsocket::Socket> acceptedSocket = listenSocket.accept())
		{
			spdlog::info("Connection accepted");
			skvmoip_debug_assert(acceptedSocket->isConnected());
			
			netsocket::Socket socket = netsocket::Socket::CreateInvalid();
			
			socket = std::move(*acceptedSocket);
			
			spdlog::info("Waiting for the socket type");
			u8 socketType;
			if(socket.receive(&socketType, sizeof(u8)) != netsocket::Result::Success)
			{
				spdlog::error("Unable to receive socket type, closing connection");
				socket.close();
				continue;
			}

			if(socketType == EnumClassToInt(SocketType::Control))
			{
				spdlog::info("Socket Type: Control");
				u32 clientID = GenerateClientID();
				spdlog::info("Client ID Generated: {}", clientID);
				if(socket.send(reinterpret_cast<u8*>(&clientID), sizeof(u32)) != netsocket::Result::Success)
				{
					spdlog::error("Unable to send client id, closing connection");
					socket.close();
					continue;
				}
				auto thread = std::thread([&sessionManagerMutex, &sessionManager](netsocket::Socket&& socket, u32 clientID)
				{
					while(socket.isConnected())
					{
						spdlog::info("Receiving control message...");
						u8 controlMessage;
						if(socket.receive(&controlMessage, sizeof(u8)) != netsocket::Result::Success)
						{
							spdlog::error("Failed to receive control message for client with ID: {}, disposing client", clientID);
							/* close the control socket */
							spdlog::info("Closing control socket");
							socket.close();
							{
								std::lock_guard<std::mutex> lock(sessionManagerMutex);
								/* close the runner (encoder) */
								if(sessionManager.getSessionForClient(clientID))
								{
									spdlog::info("Destroying session for client id {}", clientID);
									sessionManager.destroySessionForClient(clientID);
								}
							}
							/* close the stream socket and remove it from the stream sockets list */
							auto it = gStreamSockets.find(clientID);
							if(it != gStreamSockets.end())
							{
								spdlog::info("Closing stream socket");
								netsocket::AsyncSocket& streamSocket = *it->second;
								streamSocket.close();
								gStreamSockets.erase(it);
							}
							spdlog::info("Client is desposed!");
							break;
						}

						if(controlMessage == EnumClassToInt(Message::Start))
						{
							u8 deviceID;
							if(socket.receive(&deviceID, sizeof(u8)) != netsocket::Result::Success)
							{
								spdlog::error("Failed to receive device ID for Start command, ignoring the start command");
								socket.close();
								continue;
							}
							spdlog::info("Device ID received: {}", deviceID);

							// Get stream socket for this client (the stream socket must have already be connected to the server by the client)
							auto asyncSocket = GetStreamSocketForClient(clientID);
							if(!asyncSocket)
							{
								spdlog::error("Not stream socket is found for client {}, make sure the client implements the Protocol correctly", clientID);
								continue;
							}

							{
								std::lock_guard<std::mutex> lock(sessionManagerMutex);
								// If there is already a session associated with the same client id, then destroy it first
								if(sessionManager.getSessionForClient(clientID))
									sessionManager.destroySessionForClient(clientID);

								// Create session for the client with specified device id
								if(auto session = sessionManager.createSessionForClient(clientID, deviceID, *asyncSocket.value()))
								{
									if(!(*session)->isRunning())
									{
										spdlog::error("Failed to start the stream runner, destroying session for client {}", clientID);
										sessionManager.destroySessionForClient(clientID);
									}
								}
								else
									spdlog::error("Failed to create session for client {} with device id {}", clientID, deviceID);
							}
						}
						else if(controlMessage == EnumClassToInt(Message::Stop))
						{
							std::lock_guard<std::mutex> lock(sessionManagerMutex);
							if(sessionManager.getSessionForClient(clientID))
								sessionManager.destroySessionForClient(clientID);
							else
								spdlog::error("No session is every created for client {} that you're trying to stop it", clientID);
						}
						else
							spdlog::error("Unrecognized control message: {}, ignored", controlMessage);
					}
				}, std::move(socket), clientID);
				thread.detach();
			}
			else if(socketType == EnumClassToInt(SocketType::Stream))
			{
				spdlog::info("Socket Type: Stream");
				u32 clientID;
				if(socket.receive(reinterpret_cast<u8*>(&clientID), sizeof(u32)) == netsocket::Result::Success)
				{
					spdlog::info("Client ID received: {}", clientID);
				}
				else
				{
					spdlog::error("Unable to receive client id, refusing stream socket");
					socket.close();
					continue;
				}

				{
					std::unique_lock<std::mutex> lock(gMutexStreamSockets);

					auto it = gStreamSockets.find(clientID);
					if(it != gStreamSockets.end())
					{
						spdlog::warn("Stream socket with client id {} already exists, closing the existing one", clientID);
						sessionManager.destroySessionForClient(clientID);
						it->second->close();
						gStreamSockets.erase(it);
					}

					std::pair<u32, std::unordered_map<u32, std::unique_ptr<netsocket::AsyncSocket>>*>* userData = 
					new std::pair<u32, std::unordered_map<u32, std::unique_ptr<netsocket::AsyncSocket>>*> { clientID, &gStreamSockets };
					socket.setOnDisconnect([](netsocket::Socket& socket, void* userData)
					{
						auto& data = *reinterpret_cast<std::pair<u32, std::unordered_map<u32, netsocket::AsyncSocket>*>*>(userData);
						{
							std::unique_lock<std::mutex> lock(gMutexStreamSockets);
							auto it = data.second->find(data.first);
							skvmoip_debug_assert(it != data.second->end());
							data.second->erase(it);
						}
						delete &data;
					}, reinterpret_cast<void*>(userData));
								
					u8 ackMessage = EnumClassToInt(Message::ACK);
					if(socket.send(&ackMessage, sizeof(u8)) == netsocket::Result::Success)
					{
						spdlog::info("Acknowledgement is sent");
						gStreamSockets.insert({ clientID, std::make_unique<netsocket::AsyncSocket>(std::move(socket)) });
					}
					else
					{
						spdlog::error("Unable to send ACK Message to the stream socket just accpeted, refusing stream socket");
						socket.close();
						continue;
					}
				}
			}
			else
			{
				spdlog::error("Unrecognized socket type, refused - closing");
				socket.close();
			}
		}
		else
		{
			spdlog::error("Unable to accept incoming connection");
			continue;
		}
	} while(true);

	spdlog::error("Press any key to exit...");
	getch();
	return 0;
}
