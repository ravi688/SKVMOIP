#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>
#include <SKVMOIP/Decoder.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/FIFOPool.hpp>

#include <thread>
#include <mutex>
#include <atomic>

namespace SKVMOIP
{
	static void FrameReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);

	class HDMIDecodeNetStream : public Network::AsyncQueueSocket
	{
	public:
		#ifdef USE_DIRECT_FRAME_DATA_COPY
			typedef DataBufferNoAlloc FrameData;
		#else
			#warning "Still using DataBuffer"
			typedef DataBuffer FrameData;
		#endif

	private:
		FIFOPool<FrameData> m_frameDataPool;
		std::condition_variable m_dataAvailableCV;
		std::mutex m_mutex;
		std::mutex m_ClientMutex;
		u32 m_inFlightRequestCount;
		std::atomic<bool> m_isStopThread;
		bool m_isDataAvailable;
		bool m_isDecodeThread;
		std::unique_ptr<std::thread> m_decodeThread;
		buffer_t m_nv12Buffer;
		buffer_t m_decodeBuffer;
		Decoder m_decoder;
		NV12ToRGBConverter m_converter;
		u32 m_width;
		u32 m_height;
		u32 m_frNum;
		u32 m_frDen;
		/* rgb bits per pixel */
		u32 m_bitsPerPixel;
		Network::AsyncQueueSocket::BinaryFormatter m_receiveFormatter;

		friend void FrameReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);
	
		void decodeThreadHandler();
	
		u32 getUncompressedFrameSize() const noexcept { return (m_width * m_height * 3) >> 1; }
		u32 getUncompressedConvertedFrameSize() const noexcept { return m_width * m_height * (m_bitsPerPixel >> 3); }
	
	public:

		HDMIDecodeNetStream(u32 width, u32 height, u32 frNum, u32 frDen, u32 bitsPerPixel);
		HDMIDecodeNetStream(HDMIDecodeNetStream&& stream) = delete;
		HDMIDecodeNetStream& operator=(HDMIDecodeNetStream&& stream) = delete;
		HDMIDecodeNetStream(HDMIDecodeNetStream& stream) = delete;
		HDMIDecodeNetStream& operator=(HDMIDecodeNetStream& stream) = delete;
	
		~HDMIDecodeNetStream();

		void stop();
		void start();
	
		#ifdef USE_DIRECT_FRAME_DATA_COPY
		void addFrameDataStorage(void* buffer);
		bool isFrameAvailable();
		#endif
		
		/* Following two functions are supposed to be callled from another client thread (main thread) */
		typename FIFOPool<FrameData>::PoolItemType borrowFrameData();
		void returnFrameData(typename FIFOPool<FrameData>::PoolItemType frameData);
	};
}
