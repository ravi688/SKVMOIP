#include <SKVMOIP/Network/NetworkSocket.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

#include <ws2tcpip.h>

namespace SKVMOIP
{
	namespace Network
	{

		CONSTRUCTOR_FUNCTION void InitializeWSA()
		{
			static WSADATA gWSAData;
			if(WSAStartup(MAKEWORD(2, 2), &gWSAData) != 0)
				debug_log_fetal_error("WSAStartup failed");
			debug_log_info("Initialized Windows Socket : Success");
		}

		DESTRUCTOR_FUNCTION void DeinitializeWSA()
		{
			WSACleanup();
			debug_log_info("Windows Socket uninitialized");
		}

		static int GetWin32IPAddressFamily(IPAddressFamily ipaFamily)
		{
			switch(ipaFamily)
			{
				case IPAddressFamily::IPv4: return AF_INET;
				case IPAddressFamily::IPv6: return AF_INET6;
				default:
					debug_log_error("IPAddressFamily %lu is not supported as of now", ipaFamily);
			}
			return 0;
		}

		static int GetWin32SocketType(SocketType socketType)
		{
			switch(socketType)
			{
				case SocketType::Stream: return SOCK_STREAM;
				case SocketType::Raw: return SOCK_RAW;
				default:
					debug_log_error("SocketType %lu is not supported as of now", socketType);
			}
			return 0;
		}

		static int GetWin32IPProtocol(IPProtocol ipProtocol)
		{
			switch(ipProtocol)
			{
				case IPProtocol::TCP: return IPPROTO_TCP;
				case IPProtocol::UDP: return IPPROTO_UDP;
				// case IPProtocol::RM: return IPPROTO_RM;
				default:
					debug_log_error("IPProtocol %lu is not supported as of now", ipProtocol);
			}
			return 0;
		}

		Socket::Socket(SocketType socketType, IPAddressFamily ipAddressFamily, IPProtocol ipProtocol, SocketRole role) : 
																							m_ipaFamily(GetWin32IPAddressFamily(ipAddressFamily)), 
																							m_socketType(GetWin32SocketType(socketType)), 
																							m_ipProtocol(GetWin32IPProtocol(ipProtocol)),
																							m_role(role),
																							m_isConnected(false),
																							m_isValid(false),
																							m_onDisconnect(NULL),
																							m_userData(NULL)
		{
			m_socket = socket(m_ipaFamily, m_socketType, m_ipProtocol);

			if(m_socket == INVALID_SOCKET)
			{
				debug_log_error("Unable to create socket, error code: %d", WSAGetLastError());
				return;
			}

			m_isValid = true;
		}

		Socket::Socket(Socket&& socket) :
										m_socket(socket.m_socket),
										m_ipaFamily(socket.m_ipaFamily),
										m_socketType(socket.m_socketType),
										m_ipProtocol(socket.m_ipProtocol),
										m_role(socket.m_role),
										m_isConnected(socket.m_isConnected),
										m_isValid(socket.m_isValid),
										m_onDisconnect(socket.m_onDisconnect),
										m_userData(socket.m_userData)
		{
			socket.m_socket = INVALID_SOCKET;
			socket.m_ipaFamily = 0;
			socket.m_socketType = 0;
			socket.m_ipProtocol = 0;
			socket.m_isConnected = false;
			socket.m_isValid = false;
			socket.m_onDisconnect = NULL;
			socket.m_userData = NULL;
		}

		Socket& Socket::operator=(Socket&& socket)
		{
			m_socket = socket.m_socket;
			m_ipaFamily = socket.m_ipaFamily;
			m_socketType = socket.m_socketType;
			m_ipProtocol = socket.m_ipProtocol;
			m_role = socket.m_role;
			m_isConnected = socket.m_isConnected;
			m_isValid = socket.m_isValid;

			socket.m_socket = INVALID_SOCKET;
			socket.m_ipaFamily = 0;
			socket.m_socketType = 0;
			socket.m_ipProtocol = 0;
			socket.m_isConnected = false;
			socket.m_isValid = false;
			socket.m_onDisconnect = NULL;
			socket.m_userData = NULL;

			return *this;
		}

		Socket::~Socket()
		{
			if(m_isValid)
				close();
		}

		Result Socket::listen()
		{
			if(::listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
				return Result::SocketError;
			return Result::Success;
		}

		std::optional<Socket> Socket::accept()
		{
			SOCKET acceptedSocket = INVALID_SOCKET;
			acceptedSocket = ::accept(m_socket, NULL, NULL);
			if(acceptedSocket == INVALID_SOCKET)
				return { };
			return { Socket::CreateAcceptedSocket(acceptedSocket, m_socketType, m_ipaFamily, m_ipProtocol) };
		}

		Result Socket::bind(const char* ipAddress, const char* portNumber)
		{
			struct addrinfo hints;

			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = m_ipaFamily;
			hints.ai_socktype = m_socketType;
			hints.ai_protocol = m_ipProtocol;

			struct addrinfo* addressInfo = NULL;
			INT result = getaddrinfo(ipAddress, portNumber, &hints, &addressInfo);
			if(result != 0)
			{
				m_isValid = false;
				return Result::Failed;
			}

			_assert((addressInfo->ai_family == m_ipaFamily) && (addressInfo->ai_socktype == m_socketType) && (addressInfo->ai_protocol == m_ipProtocol));

			result = ::bind(m_socket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);

			if(result == SOCKET_ERROR)
			{
				freeaddrinfo(addressInfo);
				m_isValid = false;
				return Result::SocketError;
			}

			return Result::Success;			
		}

		Result Socket::connect(const char* ipAddress, const char* portNumber)
		{
			struct addrinfo hints;

			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = m_ipaFamily;
			hints.ai_socktype = m_socketType;
			hints.ai_protocol = m_ipProtocol;

			struct addrinfo* addressInfo = NULL;
			INT result = getaddrinfo(ipAddress, portNumber, &hints, &addressInfo);
			if(result != 0)
			{
				m_isValid = false;
				return Result::Failed;
			}

			_assert((addressInfo->ai_family == m_ipaFamily) && (addressInfo->ai_socktype == m_socketType) && (addressInfo->ai_protocol == m_ipProtocol));

			result = ::connect(m_socket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);

			if(result == SOCKET_ERROR)
			{
				freeaddrinfo(addressInfo);
				m_isValid = false;
				return Result::SocketError;
			}

			m_isConnected = true;
			return Result::Success;
		}

		Result Socket::close()
		{
			if(closesocket(m_socket) == SOCKET_ERROR)
			{
				m_isValid = false;
				m_isConnected = false;
				callOnDisconnect();
				return Result::SocketError;
			}
			m_isValid = false;
			m_isConnected = false;
			callOnDisconnect();
			return Result::Success;
		}

		Result Socket::send(const u8* bytes, u32 size)
		{
			u32 numSentBytes = 0;
			while(numSentBytes < size)
			{
				int result = ::send(m_socket, reinterpret_cast<const char*>(bytes + numSentBytes), size - numSentBytes, 0);
				if(result == SOCKET_ERROR)
				{
					m_isValid = false;
					m_isConnected = false;
					callOnDisconnect();
					return Result::SocketError;
				}
				numSentBytes += static_cast<u32>(result);
			}
			// debug_log_info("Sent: %lu bytes", numSentBytes);
			return Result::Success;
		}

		Result Socket::receive(u8* bytes, u32 size)
		{
			if(!m_isConnected)
				return Result::SocketError;

			u32 numReceivedBytes = 0;
			while(numReceivedBytes < size)
			{
				int result = ::recv(m_socket, reinterpret_cast<char*>(bytes + numReceivedBytes), size - numReceivedBytes, 0);
				if(result == SOCKET_ERROR)
				{
					m_isValid = false;
					m_isConnected = false;
					callOnDisconnect();
					return Result::SocketError;
				}
				else if(result == 0)
				{
					m_isConnected = false;
					callOnDisconnect();
					return Result::SocketError;
				}
				numReceivedBytes += static_cast<u32>(result);
			}
			return Result::Success;
		}

		void Socket::callOnDisconnect()
		{
			if(m_onDisconnect == NULL)
				return;
			m_onDisconnect(*this, m_userData);
		}

		void Socket::setOnDisconnect(void (*onDisconnect)(Socket& socket, void* userData), void* userData)
		{
			m_onDisconnect = onDisconnect;
			m_userData = userData;
		}
	}
}
