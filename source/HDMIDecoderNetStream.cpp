#include <SKVMOIP/HDMIDecodeNetStream.hpp>
#include <SKVMOIP/StopWatch.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

#include <functional>

namespace SKVMOIP
{
	HDMIDecodeNetStream::FrameData::FrameData(u32 capacity)
	{
		m_buffer = buf_create(sizeof(u8), capacity, 0);
		buf_clear(&m_buffer, NULL);
	}
	HDMIDecodeNetStream::FrameData::FrameData(FrameData&& data) : m_isValid(data.m_isValid)
	{
		buf_move(&data.m_buffer, &m_buffer);
		data.m_isValid = false;
	}
	HDMIDecodeNetStream::FrameData& HDMIDecodeNetStream::FrameData::operator=(FrameData&& data)
	{
		m_isValid = data.m_isValid;
		buf_move(&data.m_buffer, &m_buffer);
		data.m_isValid = false;
		return *this;
	}
	HDMIDecodeNetStream::FrameData::~FrameData()
	{
		if(!m_isValid) return;
		buf_free(&m_buffer);
	}
	
	const u8* HDMIDecodeNetStream::FrameData::getPtr() const
	{
		return buf_get_ptr_typeof(const_cast<buffer_t*>(&m_buffer), u8);
	}
	
	u8* HDMIDecodeNetStream::FrameData::getPtr()
	{
		return buf_get_ptr_typeof(&m_buffer, u8);
	}
	
	u32 HDMIDecodeNetStream::FrameData::getSize() const
	{
		return static_cast<u32>(buf_get_capacity(const_cast<buffer_t*>(&m_buffer)));
	}
	
	
	HDMIDecodeNetStream::HDMIDecodeNetStream(u32 width, u32 height, u32 frNum, u32 frDen, u32 bitsPerPixel) : 
												m_decodeThread(decodeThreadHandler, this),
												m_decoder(),
												m_converter(width, height, frNum, frDen, bitsPerPixel),
												m_width(width), 
												m_height(height),
												m_frNum(frNum),
												m_frDen(frDen),
												m_bitsPerPixel(bitsPerPixel),
												m_isDataAvailable(false),
												AsyncQueueSocket(std::move(Network::Socket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP)))
	{
		m_nv12Buffer = buf_create_byte_buffer(getUncompressedFrameSize());
		buf_clear(&m_nv12Buffer, NULL);
		m_decodeBuffer = buf_create_byte_buffer(4096);
		buf_clear(&m_decodeBuffer, NULL);
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
	
	std::optional<FIFOPool<HDMIDecodeNetStream::FrameData>::ItemType> HDMIDecodeNetStream::borrowFrameData()
	{
		std::lock_guard<std::mutex> lock_guard(m_mutex);
		return m_frameDataPool.getActive();
	}
	
	void HDMIDecodeNetStream::returnFrameData(FIFOPool<FrameData>::ItemType frameData)
	{
		std::lock_guard<std::mutex> lock_guard(m_mutex);
		m_frameDataPool.returnActive(frameData);
	}
	
	void ReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData)
	{
		HDMIDecodeNetStream* decodeStream = reinterpret_cast<HDMIDecodeNetStream*>(userData);
		
		std::unique_lock<std::mutex> lock(decodeStream->m_mutex);
		/* wait until decode thread has ran out of data */
		while(decodeStream->m_isDataAvailable)
		{
			debug_log_info("Data is ready to be received, waiting decode thread to finish previous decode");
			decodeStream->m_dataAvailableCV.wait(lock);
		}
		
		/* copy the data from the network stream to the decode thread */
		buf_set_element_count(&decodeStream->m_decodeBuffer, 0);
		buf_pushv(&decodeStream->m_decodeBuffer, reinterpret_cast<void*>(const_cast<u8*>(data)), dataSize);
		decodeStream->m_isDataAvailable = true;
		lock.unlock();
		decodeStream->m_dataAvailableCV.notify_one();
	}
	
	void HDMIDecodeNetStream::decodeThreadHandler()
	{
		while(true)
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while(!m_isDataAvailable)
			{
				debug_log_info("Data not available, making request");
				receive(ReceiveCallbackHandler, reinterpret_cast<void*>(this), m_receiveFormatter);
				m_dataAvailableCV.wait(lock);
			}

			if(m_isDataAvailable)
			{
				debug_log_info("Data available, Decoding...");
				u32 nFrameReturned = 0;
				SKVMOIP::StopWatch decodeWatch;
				if(m_decoder.decode(buf_get_ptr_typeof(&m_decodeBuffer, u8), buf_get_element_count(&m_decodeBuffer), nFrameReturned))
				{
					m_isDataAvailable = false;

					std::pair<FrameData&, FIFOPool<FrameData>::ItemIdType>* frameIdPair;
					
					_assert(nFrameReturned == 1);
					auto decodeTime = decodeWatch.stop();
					u8* frame = m_decoder.getFrame();
					_assert(m_decoder.getFrameSize() == getUncompressedFrameSize());
					u8* rgbData;
					SKVMOIP::StopWatch convertWatch;
					if((rgbData = m_converter.convert(frame, m_decoder.getFrameSize())) != NULL)
					{
						auto convertTime = convertWatch.stop();
						
						auto result = m_frameDataPool.getInactive();
						if (!result)
						{
							debug_log_info("Allocating new FrameData object");
							m_frameDataPool.createInactive(getUncompressedConvertedFrameSize());
							auto result = m_frameDataPool.getInactive();
							_assert(result.has_value());
							frameIdPair = &result.value();
						}
						else
						{
							debug_log_info("Using existing FrameData object");
							frameIdPair = &result.value();
						}
						
						auto rgbDataSize = m_converter.getRGBDataSize();
						_assert(rgbDataSize == frameIdPair->first.getSize());
						_assert(rgbDataSize == getUncompressedConvertedFrameSize());
						memcpy(frameIdPair->first.getPtr(), rgbData, m_converter.getRGBDataSize());
						m_frameDataPool.returnInactive(*frameIdPair);
						debug_log_info("Time info: decode: %lu, convert: %lu, encodedSize: %.2f kb", decodeTime, convertTime, buf_get_element_count(&m_decodeBuffer) / 1024.0);
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
				}
			}
		}
	}
}