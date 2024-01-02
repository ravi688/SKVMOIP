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
		}

		DESTRUCTOR_FUNCTION void DeinitializeWSA()
		{
			WSACleanup();
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

		Socket::Socket(SocketType socketType, IPAddressFamily ipAddressFamily, IPProtocol ipProtocol) : 
																							m_ipaFamily(GetWin32IPAddressFamily(ipAddressFamily)), 
																							m_socketType(GetWin32SocketType(socketType)), 
																							m_ipProtocol(GetWin32IPProtocol(ipProtocol))
		{
			m_socket = socket(m_ipaFamily, m_socketType, m_ipProtocol);

			if(m_socket == INVALID_SOCKET)
				debug_log_fetal_error("Unable to create socket, error code: %d", WSAGetLastError());
		}

		Socket::~Socket()
		{
			close();
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
				return Result::Failed;

			_assert((addressInfo->ai_family == m_ipaFamily) && (addressInfo->ai_socktype == m_socketType) && (addressInfo->ai_protocol == m_ipProtocol));

			result = ::connect(m_socket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);

			if(result == SOCKET_ERROR)
			{
				freeaddrinfo(addressInfo);
				return Result::SocketError;
			}
			return Result::Success;
		}

		Result Socket::close()
		{
			if(closesocket(m_socket) == SOCKET_ERROR)
				return Result::SocketError;
			return Result::Success;
		}

		Result Socket::send(const u8* bytes, u32 size)
		{
			u32 numSentBytes = 0;
			while(numSentBytes < size)
			{
				int result = ::send(m_socket, reinterpret_cast<const char*>(bytes), size, 0);
				if(result == SOCKET_ERROR)
					return Result::SocketError;
				numSentBytes += static_cast<u32>(result);
			}
			return Result::Success;
		}

		Result Socket::receive(const u8* bytes, u32 size)
		{
			return Result::Success;
		}
	}
}
