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

		public:

			Socket(SocketType socketType, IPAddressFamily ipAddressFamily, IPProtocol ipProtocol);
			~Socket();

			Result connect(const char* ipAddress, const char* port);
			Result close();

			Result send(const u8* bytes, u32 size);
			Result receive(const u8* bytes, u32 size);

		};
	}
}