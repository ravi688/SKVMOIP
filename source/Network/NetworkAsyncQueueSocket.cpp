#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>
#include <SKVMOIP/debug.h>

#include <functional>

namespace SKVMOIP
{
	namespace Network
	{
		AsyncQueueSocket::Transxn::Transxn(const u8* data, u32 dataSize, Type type) : m_type(type), m_isValid(false)
		{
			_assert(type == Type::Send);
			m_buffer = buf_create(sizeof(u8), dataSize, 0);
			buf_pushv(&m_buffer, reinterpret_cast<void*>(const_cast<u8*>(data)), dataSize);
			m_isValid = true;
		}

		AsyncQueueSocket::Transxn::Transxn(ReceiveCallbackHandler receiveHandler, void* userData, BinaryFormatter& receiveFormatter, Type type) : 
																																				  m_receiveHandler(receiveHandler),
																																				  m_userData(userData), 
																																				  m_type(type), 
																																				  m_receiveFormatter(receiveFormatter), 
																																				  m_isValid(false)
		{
			_assert(type == Type::Receive);
			m_buffer = buf_create(sizeof(u8), 0, 0);
			m_isValid = true;
		}
	
		AsyncQueueSocket::Transxn::Transxn(Transxn&& transxn) : 
															 	m_receiveHandler(transxn.m_receiveHandler),
															 	m_userData(transxn.m_userData),
																m_type(transxn.m_type),
																m_receiveFormatter(transxn.m_receiveFormatter),
																m_isValid(transxn.m_isValid)
		{
			buf_move(&transxn.m_buffer, &m_buffer);
			transxn.m_isValid = false;
		}

/*		AsyncQueueSocket::Transxn& AsyncQueueSocket::Transxn::operator=(Transxn& transxn)
		{
			m_receiveHandler = transxn.m_receiveHandler;
			m_userData = transxn.m_userData;
			m_type = transxn.m_type;
			m_receiveFormatter = transxn.m_receiveFormatter;
			m_isValid = transxn.m_isValid;
			buf_move(&transxn.m_buffer, &m_buffer);
			transxn.m_isValid = false;
		}*/
	
		AsyncQueueSocket::Transxn::~Transxn()
		{
			if(!m_isValid)
				return;
			buf_free(&m_buffer);
			m_isValid = false;
		}
	
		u8* AsyncQueueSocket::Transxn::getBufferPtr() { return buf_get_ptr_typeof(&m_buffer, u8); }
		u32 AsyncQueueSocket::Transxn::getBufferSize() { return static_cast<u32>(buf_get_element_count(&m_buffer)); }
	
		bool AsyncQueueSocket::Transxn::doit(Socket& socket)
		{
			switch(m_type)
			{
				case Type::Send:
				{
					debug_log_info("Sending Data...");
					return socket.send(buf_get_ptr_typeof(&m_buffer, u8), static_cast<u32>(buf_get_element_size(&m_buffer))) == Result::Success;
				}
				case Type::Receive:
				{
					debug_log_info("Receiving Data...");
					std::pair<Socket*, decltype(this)> socketTransxnPair { &socket, this };

					bool result = m_receiveFormatter->format([](u32 size, void* userData) -> void* 
															{
																auto* pair = reinterpret_cast<std::pair<Socket*, Transxn*>*>(userData);
																Socket* socket = pair->first;
																Transxn* transxn = pair->second;
																auto index = buf_get_element_count(&transxn->m_buffer);
																buf_push_pseudo(&transxn->m_buffer, size);
																void* ptr = buf_get_ptr_at(&transxn->m_buffer, index);
																_assert(ptr != NULL);
																bool result = socket->receive(reinterpret_cast<u8*>(ptr), size) == Network::Result::Success;
																if(!result)
																{
																	debug_log_error("Failed to receive from socket");
																	return NULL;
																}
																return ptr;
															}, reinterpret_cast<void*>(&socketTransxnPair));

					_assert(m_receiveHandler != NULL);
					if(result)
						m_receiveHandler(getBufferPtr(), getBufferSize(), m_userData);
					else
						m_receiveHandler(NULL, getBufferSize(), m_userData);
					return result;
				}
				default:
				{
					debug_log_error("Unrecognized Transxn::Type: %lu", EnumClassToInt(m_type));
					break;
				}
			}
			return false;
		}
	
		AsyncQueueSocket::AsyncQueueSocket(Socket&& socket) : m_socket(std::move(socket)), m_isValid(false)
		{
			m_isValid = true; 
		}
/*		AsyncQueueSocket::AsyncQueueSocket(AsyncQueueSocket&& asyncSocket) : m_socket(std::move(asyncSocket.m_socket)), m_thread(std::move(asyncSocket.m_thread)), m_isValid(asyncSocket.m_isValid)
		{
			std::lock_guard<std::mutex> lock(asyncSocket.m_mutex);
			m_transxnQueue = std::move(asyncSocket.m_transxnQueue);
			asyncSocket.m_isValid = false;
		}
		AsyncQueueSocket& AsyncQueueSocket::operator=(AsyncQueueSocket&& asyncSocket)
		{
			m_thread = std::move(asyncSocket.m_thread);
			m_isValid = asyncSocket.m_isValid;
			std::lock_guard<std::mutex> lock(asyncSocket.m_mutex);
			m_socket = std::move(asyncSocket.m_socket);
			m_transxnQueue = std::move(asyncSocket.m_transxnQueue);
			asyncSocket.m_isValid = false;
			return *this;
		}*/

		AsyncQueueSocket::~AsyncQueueSocket()
		{
			if(m_isValid)
				close();
		}
	
		Result AsyncQueueSocket::connect(const char* ipAddress, const char* port)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto result = m_socket.connect(ipAddress, port);
			if(result == Result::Success)
				m_thread = std::move(std::thread(threadHandler, this));
			return result;
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
			std::unique_lock<std::mutex> lock(m_mutex);
			m_transxnQueue.push_front(std::move(transxn));
			lock.unlock();
			m_dataAvailableCV.notify_one();
		}
	
		void AsyncQueueSocket::receive(Transxn::ReceiveCallbackHandler receiveHandler, void* userData, BinaryFormatter& receiveFormatter)
		{
			Transxn transxn(receiveHandler, userData, receiveFormatter, Transxn::Type::Receive);
			std::unique_lock<std::mutex> lock(m_mutex);
			m_transxnQueue.push_front(std::move(transxn));
			lock.unlock();
			m_dataAvailableCV.notify_one();
		}
	
		void AsyncQueueSocket::threadHandler()
		{
			while(true)
			{
				/* lock and copy/move the Transxn object into the local storage of this thread */
				std::unique_lock<std::mutex> lock(m_mutex);
				while(m_transxnQueue.empty())
					m_dataAvailableCV.wait(lock);
				debug_log_info("Request received for Data Receive");
				Transxn transxn(std::move(m_transxnQueue.back()));
				/* now we are done moving/copying, and let the client thread add more send and receive transactions into the queue */
				lock.unlock();

				/* Transxn::doit takes a long time to process due to network latency */
				bool result = transxn.doit(m_socket);
				if(!result)
				{
					debug_log_error("Failed to do transaction");
					
					lock.lock();
					m_transxnQueue.pop_back();
					lock.unlock();

					/* socket might have been closed, so terminate this thread */
					std::terminate();
				}

				lock.lock();
				m_transxnQueue.pop_back();
				lock.unlock();
			}
		}
	}
}
