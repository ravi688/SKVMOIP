#include <SKVMOIP/HDMIDecodeNetStream.hpp>
#include <SKVMOIP/StopWatch.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

#include <functional>

namespace SKVMOIP
{

	HDMIDecodeNetStream::HDMIDecodeNetStream(u32 width, u32 height, u32 frNum, u32 frDen, u32 bitsPerPixel) : 
												AsyncQueueSocket(std::move(Network::Socket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP))),
												m_frameDataPool([] (DataBuffer& db) { db.destroy(); }),
												m_inFlightRequestCount(0),
												m_decodeThread(decodeThreadHandler, this),
												m_converter(width, height, frNum, frDen, bitsPerPixel),
												m_width(width), 
												m_height(height),
												m_frNum(frNum),
												m_frDen(frDen),
												m_bitsPerPixel(bitsPerPixel),
												m_isDataAvailable(false)
	{
		/* This buffer stores the decoded data - which is in NV12 format */
		m_nv12Buffer = buf_create_byte_buffer(getUncompressedFrameSize());
		buf_clear(&m_nv12Buffer, NULL);

		/* This buffer stores the encoded data which is just received from Network Thread to be decoded */
		m_decodeBuffer = buf_create_byte_buffer(30 * 1024);
		buf_clear(&m_decodeBuffer, NULL);

		/* Encoded (to be Received from Network) Packet structure 
		 * first: is the length of the encoded frame 
		 * second: is the data of size 'length' (above) */
		m_receiveFormatter.add(BinaryFormatter::Type::LengthU32);
		m_receiveFormatter.add(BinaryFormatter::Type::Data);
	}
/*	
	HDMIDecodeNetStream::HDMIDecodeNetStream(HDMIDecodeNetStream&& stream) :  
																			m_decodeThread(std::move(stream.m_decodeThread)),
																			m_decoder(std::move(stream.m_decoder)),
																			m_converter(std::move(stream.m_converter)),
																			AsyncQueueSocket(std::move(reinterpret_cast<AsyncQueueSocket&&>(stream)))
	{
		m_isDataAvailable.store(stream.m_isDataAvailable.load());
		std::lock_guard<std::mutex> lock(stream.m_mutex);
		m_frameDataPool = std::move(stream.m_frameDataPool);
		buf_move(&stream.m_nv12Buffer, &m_nv12Buffer);
		buf_move(&stream.m_decodeBuffer, &m_decodeBuffer);
	}

	HDMIDecodeNetStream& HDMIDecodeNetStream::operator=(HDMIDecodeNetStream&& stream)
	{
		m_decodeThread = std::move(stream.m_decodeThread);
		m_isDataAvailable.store(stream.m_isDataAvailable.load());
		AsyncQueueSocket::operator=(std::move(reinterpret_cast<AsyncQueueSocket&&>(stream)));

		return *this;
	}*/
	
	HDMIDecodeNetStream::~HDMIDecodeNetStream()
	{
		m_decodeThread.join();
		Network::Result result = AsyncQueueSocket::close();
		if(result != Network::Result::Success)
			debug_log_error("Failed to close network socket");
		buf_free(&m_nv12Buffer);
	}

	typename FIFOPool<HDMIDecodeNetStream::FrameData>::PoolItemType HDMIDecodeNetStream::borrowFrameData()
	{
		std::lock_guard<std::mutex> lock_guard(m_ClientMutex);
		return m_frameDataPool.getActive();
	}
	
	void HDMIDecodeNetStream::returnFrameData(typename FIFOPool<HDMIDecodeNetStream::FrameData>::PoolItemType frameData)
	{
		std::lock_guard<std::mutex> lock_guard(m_ClientMutex);
		m_frameDataPool.returnActive(frameData);
	}
	
	static void FrameReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData)
	{
		HDMIDecodeNetStream* decodeStream = reinterpret_cast<HDMIDecodeNetStream*>(userData);
		
		std::unique_lock<std::mutex> lock(decodeStream->m_mutex);
		// debug_log_info	("Data Received");
		/* wait until decode thread has ran out of data */
		while(decodeStream->m_isDataAvailable)
		{
			debug_log_info("...waiting decode thread to finish previous decode");
			decodeStream->m_dataAvailableCV.wait(lock);
		}
		
		/* copy the data from the network stream to the decode thread */
		buf_set_element_count(&decodeStream->m_decodeBuffer, 0);
		_assert(dataSize >= sizeof(u32));
		
		/* if the payload (after length data) is of zero size then we don't we to process the frame */
		if(dataSize > sizeof(u32))
		{
			dataSize = dataSize - sizeof(u32);
			buf_ensure_capacity(&decodeStream->m_decodeBuffer, dataSize);
			memcpy(buf_get_ptr(&decodeStream->m_decodeBuffer), reinterpret_cast<void*>(const_cast<u8*>(data) + sizeof(u32)), dataSize);
			buf_set_element_count(&decodeStream->m_decodeBuffer, dataSize);
			decodeStream->m_isDataAvailable = true;
			lock.unlock();
			decodeStream->m_dataAvailableCV.notify_one();
		}
	}

	#define MAX_IN_FLIGHT_REQUEST_COUNT 5
	
	void HDMIDecodeNetStream::decodeThreadHandler()
	{
		while(true)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while(!m_isDataAvailable)
			{
				if(m_inFlightRequestCount < MAX_IN_FLIGHT_REQUEST_COUNT)
				{
					receive(FrameReceiveCallbackHandler, reinterpret_cast<void*>(this), m_receiveFormatter);
					++m_inFlightRequestCount;
				}
				else
					DEBUG_LOG_INFO("MAX_IN_FLIGHT_REQUEST_COUNT(=%d) is reached", MAX_IN_FLIGHT_REQUEST_COUNT);
				m_dataAvailableCV.wait(lock);
			}

			if(m_isDataAvailable)
			{
				_assert(m_inFlightRequestCount > 0);
				--m_inFlightRequestCount;
				_assert(m_inFlightRequestCount < MAX_IN_FLIGHT_REQUEST_COUNT);
				auto decodeBufferSize = buf_get_element_count(&m_decodeBuffer);
				auto decodeBufferPtr = buf_get_ptr_typeof(&m_decodeBuffer, u8);
				_assert(decodeBufferSize != 0);
				u32 nFrameReturned = 0;
				SKVMOIP::StopWatch decodeWatch;
				if(m_decoder.decode(decodeBufferPtr, decodeBufferSize, nFrameReturned))
				{
					/* needs more data for the next frame decode - so notify the network thread to start receiving */
					m_isDataAvailable = false;
					lock.unlock();
					m_dataAvailableCV.notify_one();

					_assert(nFrameReturned == 1);
					auto decodeTime = decodeWatch.stop();
					u8* frame = m_decoder.getFrame();
					_assert(m_decoder.getFrameSize() == getUncompressedFrameSize());
					u8* rgbData;
					SKVMOIP::StopWatch convertWatch;
					if((rgbData = m_converter.convert(frame, m_decoder.getFrameSize())) != NULL)
					{
						auto convertTime = convertWatch.stop();
						
						std::lock_guard<std::mutex> lock(m_ClientMutex);
						if (!m_frameDataPool.hasInactive())
						{
							DEBUG_LOG_INFO("Allocating new FrameData object");
							m_frameDataPool.createInactive(getUncompressedConvertedFrameSize());
						}

						if(auto result = m_frameDataPool.getInactive())
						{
							auto rgbDataSize = m_converter.getRGBDataSize();
							_assert(rgbDataSize == FIFOPool<FrameData>::GetValue(result)->getSize());
							_assert(rgbDataSize == getUncompressedConvertedFrameSize());
							memcpy(FIFOPool<FrameData>::GetValue(result)->getPtr(), rgbData, rgbDataSize);
							m_frameDataPool.returnInactive(result);
						}
						// debug_log_info("Time info: decode: %lu, convert: %lu, encodedSize: %.2f kb", decodeTime, convertTime, buf_get_element_count(&m_decodeBuffer) / 1024.0);
					}
					else 
					{ 
						convertWatch.stop(); 
						debug_log_error("Failed to convert color space"); 
					}
				}

				else 
				{ 
					decodeWatch.stop();
					debug_log_error("Failed to decode, return value %d", nFrameReturned); 
					/* needs more data for the next frame decode - so notify the network thread to start receiving */
					m_isDataAvailable = false;
					lock.unlock();
					m_dataAvailableCV.notify_one();
				}
			}
		}
	}
}