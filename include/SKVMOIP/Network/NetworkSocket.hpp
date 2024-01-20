#pragma once

#include <SKVMOIP/defines.hpp>
#include <winsock2.h>

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

		public:

			Socket(SocketType socketType, IPAddressFamily ipAddressFamily, IPProtocol ipProtocol, SocketRole role = SocketRole::Client);
			Socket(Socket&& socket);
			Socket& operator=(Socket&& socket);
			Socket(Socket& socket) = delete;
			Socket& operator=(Socket& socket) = delete;
			~Socket();

			bool isConnected() const noexcept { return m_isConnected && isValid(); }
			bool isValid() const noexcept { return m_isValid; }
			Result connect(const char* ipAddress, const char* port);
			Result close();

			Result send(const u8* bytes, u32 size);
			Result receive(u8* bytes, u32 size);

		};
	}
}