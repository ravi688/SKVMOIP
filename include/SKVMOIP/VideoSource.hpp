#pragma once

#include <SKVMOIP/defines.hpp>

namespace SKVMOIP
{
	class IVideoSource
	{
	public:
		enum class Usage
		{
			RGB32Read,
			RGB24Read,
			NV12Read
		};
		using DeviceID = u64;
	private:
		DeviceID m_deviceID;
	public:

		IVideoSource(DeviceID deviceID) : m_deviceID(deviceID) { };
		
		// Not copyable and not movable
		IVideoSource(IVideoSource&) = delete;
		IVideoSource(IVideoSource&&) = delete;

		// Virtual Destructor
		virtual ~IVideoSource() = default;

		enum class Result
		{
			Success,
			Failed
		};
		// Starts the video source device
		// If this function returns Result::Success then isOpen() should return 'true'
		virtual Result open() = 0;
		// Shuts down the video source device
		virtual void close() = 0;
		// Checks if the video source device is ready and can deliver NV12 frames
		// If something goes wrong then this returns 'false', in which case you should call close() to shutdown the device.
		virtual bool isReady() = 0;

		// Reads a single NV12 frame from the video source and writes it to the passed buffer
		// Returns 'true' if the frame has been read successfully, otherwise it returns 'false'
		// Note that it allowed to call this function next time even if it returned 'false' for the previous frame,
		// It is just taht the previous frame would be skipped. 
		virtual bool readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize) = 0;

		virtual std::pair<u32, u32> getInputFrameRate() = 0;
		virtual std::pair<u32, u32> getOutputFrameSize() = 0;

		DeviceID getDeviceID() const noexcept { return m_deviceID; }
	};
}
