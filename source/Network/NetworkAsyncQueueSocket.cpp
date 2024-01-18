#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>
#include <SKVMOIP/debug.h>

#include <functional>

namespace SKVMOIP
{
	namespace Network
	{
		AsyncQueueSocket::Transxn::Transxn(const u8* data, u32 dataSize, Type type) : m_type(type), m_isValid(false)
		{
			m_buffer = buf_create(sizeof(u8), dataSize, 0);
			switch(type)
			{
				case Type::Send:
				{
					buf_pushv(&m_buffer, reinterpret_cast<void*>(const_cast<u8*>(data)), dataSize);
					break;
				}
				case Type::Receive:
				{
					buf_push_pseudo(&m_buffer, dataSize);
					break;
				}
				default:
				{
					debug_log_error("Unrecognized Transxn::Type: %lu", EnumClassToInt(type));
					return;
				}
			}
			m_isValid = true;
		}
	
		AsyncQueueSocket::Transxn::Transxn(Transxn&& transxn) : m_type(transxn.m_type), m_isValid(transxn.m_isValid)
		{
			buf_move(&transxn.m_buffer, &m_buffer);
			transxn.m_isValid = false;
		}
	
		AsyncQueueSocket::Transxn::~Transxn()
		{
			if(!m_isValid)
				return;
			buf_free(&m_buffer);
			m_isValid = false;
		}
	
		u8* AsyncQueueSocket::Transxn::getBufferPtr() { return buf_get_ptr_typeof(&m_buffer, u8); }
		u32 AsyncQueueSocket::Transxn::getBufferSize() { return static_cast<u32>(buf_get_element_size(&m_buffer)); }
	
		bool AsyncQueueSocket::Transxn::doit(Socket& socket)
		{
			switch(m_type)
			{
				case Type::Send:
				{
					return socket.send(buf_get_ptr_typeof(&m_buffer, u8), static_cast<u32>(buf_get_element_size(&m_buffer))) == Result::Success;
				}
				case Type::Receive:
				{
					return socket.receive(buf_get_ptr_typeof(&m_buffer, u8), static_cast<u32>(buf_get_element_size(&m_buffer))) == Result::Success;
				}
				default:
				{
					debug_log_error("Unrecognized Transxn::Type: %lu", EnumClassToInt(m_type));
					break;
				}
			}
			return false;
		}
	
		AsyncQueueSocket::AsyncQueueSocket(Socket&& socket) : m_socket(std::move(socket)), m_thread(std::bind(threadHandler, this)) { }
		AsyncQueueSocket::AsyncQueueSocket(AsyncQueueSocket&& asyncSocket) : m_socket(std::move(asyncSocket.m_socket)), 
																									m_transxnQueue(std::move(asyncSocket.m_transxnQueue)),
																									m_thread(std::move(asyncSocket.m_thread)) { }
		AsyncQueueSocket::~AsyncQueueSocket()
		{
			close();
		}
	
		Result AsyncQueueSocket::connect(const char* ipAddress, const char* port)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_socket.connect(ipAddress, port);
		}
	
		Result AsyncQueueSocket::close()
		{
			Result result;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				result = m_socket.close();
				if(result != Result::Success)
					return result;
			}
			m_thread.join();
			return result;
		}
	
		void AsyncQueueSocket::send(const u8* bytes, u32 size)
		{
			Transxn transxn(bytes, size, Transxn::Type::Send);
			std::lock_guard<std::mutex> lock(m_mutex);
			m_transxnQueue.push_front({ std::move(transxn), nullptr });
		}
	
		void AsyncQueueSocket::receive(ReceiveCallbackHandler receiveHandler, u32 size)
		{
			Transxn transxn(NULL, size, Transxn::Type::Send);
			std::lock_guard<std::mutex> lock(m_mutex);
			m_transxnQueue.push_front({ std::move(transxn), receiveHandler });
		}
	
		void AsyncQueueSocket::threadHandler()
		{
			bool isConnected, isQueueEmpty;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				isConnected = m_socket.isConnected();
				isQueueEmpty = m_transxnQueue.empty();
			}
	
			while(isConnected && (!isQueueEmpty))
			{
				Transxn& transxn = m_transxnQueue.back().first;
				bool result = transxn.doit(m_socket);
				if(!result)
				{
					debug_log_error("Failed to do transaction");
					if(transxn.getType() == Transxn::Type::Receive)
						m_transxnQueue.back().second(NULL, transxn.getBufferSize());
					break;
				}
				else
				{
					if(transxn.getType() == Transxn::Type::Receive)
						m_transxnQueue.back().second(transxn.getBufferPtr(), transxn.getBufferSize());
				}
				m_transxnQueue.pop_back();
				
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					isConnected = m_socket.isConnected();
					isQueueEmpty = m_transxnQueue.empty();
				}
			}
		}
	}
}
