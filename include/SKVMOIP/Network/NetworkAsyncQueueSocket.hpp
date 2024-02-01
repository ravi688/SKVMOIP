#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Network/NetworkSocket.hpp>

#include <bufferlib/buffer.h>

#include <vector>
#include <optional>
#include <thread>
#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>

namespace SKVMOIP
{
	namespace Network
	{
		class AsyncQueueSocket
		{
		public:
			class BinaryFormatter
			{
			public:
				enum class Type
				{
					U32,
					U64,
					LengthU32,
					LengthU64,
					Data
				};

				typedef typename std::underlying_type<Type>::type IntType;

			private:
				std::vector<Type> m_format;
				std::vector<Type>::size_type m_count;
				u64 m_lengthFeedback;

				u32 getSizeFromType(Type type)
				{
					switch(type)
					{
						case Type::U32: return 4;
						case Type::U64: return 8;
						case Type::LengthU32: return 4; 
						case Type::LengthU64: return 8;
						case Type::Data: return U32_MAX;
						default: _assert(false); return U32_MAX;
					}
				}

			public:

				BinaryFormatter() noexcept { rewind(); }
				BinaryFormatter(BinaryFormatter&) = delete;
				BinaryFormatter& operator=(BinaryFormatter&) = delete;
				BinaryFormatter(BinaryFormatter&& formatter) : 
																m_format(std::move(formatter.m_format)), 
																m_count(formatter.m_count),
																m_lengthFeedback(formatter.m_lengthFeedback)
				{

				}
				BinaryFormatter& operator=(BinaryFormatter&& formatter)
				{
					m_format = std::move(formatter.m_format);
					m_count = formatter.m_count;
					m_lengthFeedback = formatter.m_lengthFeedback;
					return *this;
				}

				void add(Type type)
				{
					_assert((m_format.size() != 0) || (type != Type::Data));

				 	if(type == Type::Data)
				 		_assert((m_format.back() == Type::LengthU32) || (m_format.back() == Type::LengthU64)); 
					m_format.push_back(type); 
				}

				void rewind() noexcept { m_count = 0; }
				
				std::optional<std::pair<u32, Type>> getNext()
				{ 
					if(m_count >= m_format.size()) 
						return { };
					Type type = m_format[m_count++];
					return { { getSizeFromType(type), type } };
				}

				typedef void* (*FormatCallbackHandler)(u32 size, void* userData);

				bool format(FormatCallbackHandler handler, void* userData)
				{
					rewind();
					auto next = getNext();
					while(next)
					{
						if(next->second == Type::Data)
							handler(m_lengthFeedback, userData);
						else
						{
							void* ptr = handler(next->first, userData);
							if(ptr == NULL)
								return false;
							switch(next->second)
							{
								case Type::LengthU32: { m_lengthFeedback = (*reinterpret_cast<u32*>(ptr)); break; } 
								case Type::LengthU64: { m_lengthFeedback = (*reinterpret_cast<u64*>(ptr)); break; }
								default: _assert(false); return false;
							}
						}
						next = getNext();
					}
					return true;
				}
			};

		private:

			class Transxn
			{
			public:
				enum class Type
				{
					Send,
					Receive
				};
				typedef void (*ReceiveCallbackHandler)(const u8* bytes, u32 size, void* userData);
	
			private:
				buffer_t m_buffer;
				ReceiveCallbackHandler m_receiveHandler;
				void* m_userData;
				Type m_type;
				OptionalReference<BinaryFormatter> m_receiveFormatter;
				bool m_isValid;
	
			public:
				Transxn(const u8* data, u32 dataSize, Type type);
				Transxn(ReceiveCallbackHandler receiveHandler, void* userData, BinaryFormatter& receiveFormatter, Type type);
				Transxn(Transxn&& transxn);
				Transxn& operator=(Transxn&& transxn) = delete;
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
			std::deque<Transxn> m_transxnQueue;
			std::condition_variable m_dataAvailableCV;
			std::unique_ptr<std::thread> m_thread;
			std::mutex m_mutex;
			bool m_isValid;
			std::atomic<bool> m_isCanSendOrReceive;
			
			void threadHandler();
		
		public:
			AsyncQueueSocket(Socket&& socket);
			AsyncQueueSocket(AsyncQueueSocket& asyncSocket) = delete;
			AsyncQueueSocket& operator=(AsyncQueueSocket& asyncSocket) = delete;
			AsyncQueueSocket(AsyncQueueSocket&& asyncSocket) = delete;
			AsyncQueueSocket& operator=(AsyncQueueSocket&& asyncSocket) = delete;
			virtual ~AsyncQueueSocket();
		
			Result connect(const char* ipAddress, const char* port);
			Result close();
			void send(const u8* bytes, u32 size);
			void receive(Transxn::ReceiveCallbackHandler receiveHandler, void* userData, BinaryFormatter& receiveFormatter);

			Socket& getSocket() { return m_socket; }
			bool isCanSendOrReceive() const { return m_isCanSendOrReceive; }
		};
	}
}
