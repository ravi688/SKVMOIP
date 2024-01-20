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
	void ReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);

	class HDMIDecodeNetStream : public Network::AsyncQueueSocket
	{
	public:
		class FrameData
		{
		private:
			buffer_t m_buffer;
			bool m_isValid;
	
		public:
			FrameData(u32 capacity);
			FrameData(FrameData&& data);
			FrameData& operator=(FrameData&& data);
			FrameData(FrameData& data) = delete;
			FrameData& operator=(FrameData& data) = delete;
			~FrameData();
	
			const u8* getPtr() const;
			u8* getPtr();
			u32 getSize() const;
		};

	private:
		FIFOPool<FrameData> m_frameDataPool;
		std::condition_variable m_dataAvailableCV;
		std::thread m_decodeThread;
		std::mutex m_mutex;
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
		bool m_isDataAvailable;

		friend void ReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);
	
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
	
		/* Following two functions are supposed to be callled from another client thread (main thread) */
		std::optional<FIFOPool<FrameData>::ItemType> borrowFrameData();
		void returnFrameData(FIFOPool<FrameData>::ItemType frameData);
	};
}
