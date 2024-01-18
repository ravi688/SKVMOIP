#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>

#include <bufferlib/buffer.h>

#include <thread>
#include <deque>
#include <mutex>

namespace SKVMOIP
{
	namespace Network
	{
		class AsyncQueueSocket
		{
		private:
			class Transxn
			{
			public:
				enum class Type
				{
					Send,
					Receive
				};
	
			private:
				buffer_t m_buffer;
				Type m_type;
				bool m_isValid;
	
			public:
				Transxn(const u8* data, u32 dataSize, Type type);
				Transxn(Transxn&& transxn);
				Transxn(Transxn&) = delete;
				Transxn& operator=(Transxn&) = delete;
				~Transxn();
	
				Type getType() const noexcept { return m_type; }
				u8* getBufferPtr();
				u32 getBufferSize();
	
				bool doit(Socket& socket);
			};
	
		private:
			Socket m_socket;
			typedef void (*ReceiveCallbackHandler)(const u8* bytes, u32 size);
			std::deque<std::pair<Transxn, ReceiveCallbackHandler>> m_transxnQueue;
			std::thread m_thread;
			std::mutex m_mutex;
		
		public:
			AsyncQueueSocket(Socket&& socket);
			AsyncQueueSocket(AsyncQueueSocket& asyncSocket) = delete;
			AsyncQueueSocket& operator=(AsyncQueueSocket& asyncSocket) = delete;
			AsyncQueueSocket(AsyncQueueSocket&& asyncSocket);
			~AsyncQueueSocket();
		
			Result connect(const char* ipAddress, const char* port);
			Result close();
			void send(const u8* bytes, u32 size);
			void receive(ReceiveCallbackHandler receiveHandler, u32 size);
	
	
			void threadHandler();
		};
	}
}
