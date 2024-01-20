#pragma once

#include <SKVMOIP/defines.hpp>
#include <winsock2.h>

#include <optional>

namespace SKVMOIP
{
	namespace Network
	{
		enum class IPAddressFamily : int
		{
			IPv4, /*AF_INET*/
			IPv6  /*AF_INET6*/

			/* TODO: Add more in future */
		};

		enum class SocketType : int
		{
			Stream, /* Reliable data transfer */
			Raw

			/* TODO: Add more in future */
		};

		enum class SocketRole : int
		{
			Client,
			Server
		};

		enum class IPProtocol : int
		{
			TCP,
			UDP,
			RM
		};

		enum class Result
		{
			Success = 0,
			Failed,
			SocketError
		};

		class Socket
		{
		private:
			SOCKET m_socket;
			int m_ipaFamily;
			int m_socketType;
			int m_ipProtocol;
			SocketRole m_role;
			bool m_isConnected;
			bool m_isValid;


			Socket() : m_socket(INVALID_SOCKET) { }
			static Socket CreateAcceptedSocket(SOCKET socket, int socketType, int ipAddressFamily, int ipProtocol)
			{
				Socket s;
				s.m_socket = socket;
				s.m_ipaFamily = ipAddressFamily;
				s.m_socketType = socketType;
				s.m_role = SocketRole::Client;
				s.m_isConnected = true;
				s.m_isValid = true;
				return s;
			}

		public:

			static Socket CreateInvalid()
			{
				Socket s;
				s.m_socket = INVALID_SOCKET;
				s.m_ipaFamily = 0;
				s.m_socketType = 0;
				s.m_isConnected = false;
				s.m_isValid = false;
				return s;
			}

			Socket(SocketType socketType, IPAddressFamily ipAddressFamily, IPProtocol ipProtocol, SocketRole role = SocketRole::Client);
			Socket(Socket&& socket);
			Socket& operator=(Socket&& socket);
			Socket(Socket& socket) = delete;
			Socket& operator=(Socket& socket) = delete;
			~Socket();

			bool isConnected() const noexcept { return m_isConnected && isValid(); }
			bool isValid() const noexcept { return m_isValid; }
			Result listen();
			std::optional<Socket> accept();
			Result bind(const char* ipAddress, const char* portNumber);
			Result connect(const char* ipAddress, const char* port);
			Result close();

			Result send(const u8* bytes, u32 size);
			Result receive(u8* bytes, u32 size);

		};
	}
}