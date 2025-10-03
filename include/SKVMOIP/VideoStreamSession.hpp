#pragma once

#include <SKVMOIP/VideoSource.hpp>
#include <SKVMOIP/Encoder.hpp>
#include <netsocket/netasyncsocket.hpp>
#include <bufferlib/buffer.h>

#include <memory> // for std::unique_ptr<>
#include <atomic> // for std::atomic<bool>
#include <thread> // for std::thread

namespace SKVMOIP
{
	class VideoStreamSession
	{
	private:
		IVideoSource& m_videoSource;
		std::unique_ptr<Encoder> m_encoder;
		netsocket::AsyncSocket& m_asyncSocket;
		std::thread m_thread;
		std::atomic<bool> m_isStopThread;
		buffer_t m_nv12Buffer;

		// Blocking call, runs inside a separate thread (above)
		void processStream();
	public:
		VideoStreamSession(IVideoSource& videoSource, netsocket::AsyncSocket& asyncSocket);

		// Not copyable and not movable
		VideoStreamSession(VideoStreamSession&) = delete;
		VideoStreamSession(VideoStreamSession&&) = delete;

		~VideoStreamSession();

		bool isRunning() const { return !m_isStopThread; }

		// Calling this would dispatch a worker thread which runs a read-encode-send loop
		void start();
		// Calling this would break the loop and join the worker thread and flush the socket's send buffer.
		void stop();
	};
}
