#include <SKVMOIP/VideoSourceLinux.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include <cstdint>

namespace SKVMOIP
{
	static std::optional<VideoSourceLinux::Device> OpenDevice(const std::string_view devicePath)
	{
    	int fd = ::open(devicePath.data(), O_RDWR);
    	if(fd < 0)
    	{
    	    spdlog::error("Failed to open device at {}", devicePath);
    	    return { };
    	}

		// ----------------------------
		// Set format: NV12, 1920x1080
		// ----------------------------
		v4l2_format fmt{};
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = 1920;
		fmt.fmt.pix.height = 1080;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
		fmt.fmt.pix.field = V4L2_FIELD_NONE;

		if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
		{
			spdlog::error("Failed to set format for device at {}", devicePath);
		    return { };
		}

    	std::cout << "Set format: " << fmt.fmt.pix.width << "x"
    	          << fmt.fmt.pix.height << " "
    	          << std::hex << fmt.fmt.pix.pixelformat << std::dec << "\n";

    	// ----------------------------
    	// Set frame rate: 60 fps
    	// ----------------------------
    	v4l2_streamparm parm{};
    	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	parm.parm.capture.timeperframe.numerator = 1;
    	parm.parm.capture.timeperframe.denominator = 60;

    	if (ioctl(fd, VIDIOC_S_PARM, &parm) < 0)
    	{
    		spdlog::error("Failed to set frame rate for device at {}", devicePath);
    	    return { };
    	}

    	std::cout << "Set framerate: "
              << parm.parm.capture.timeperframe.denominator << " fps\n";

    	// ----------------------------
    	// Request buffers (MMAP)
    	// ----------------------------
    	v4l2_requestbuffers req{};
    	req.count = 4;
    	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	req.memory = V4L2_MEMORY_MMAP;
	
    	if(ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
    	{
    		spdlog::error("Failed to request buffers for device at {}", devicePath);
    	    return { };
    	}

    	std::vector<VideoSourceLinux::Buffer> buffers(req.count);

    	for(int i = 0; i < req.count; i++)
    	{
    	    v4l2_buffer buf{};
    	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	    buf.memory = V4L2_MEMORY_MMAP;
    	    buf.index = i;
	
    	    if(ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0)
    	    {
    	        spdlog::error("Failed to query buffers for device at {}", devicePath);
    	        for(int k = 0; k < i; ++k)
    	        {
    	        	auto b& = buffers[k];
        			munmap(b.start, b.length);
    	        }
    	        return { };
    	    }
	
    	    buffers[i].length = buf.length;
    	    buffers[i].start = mmap(NULL, buf.length,
    	                            PROT_READ | PROT_WRITE,
    	                            MAP_SHARED,
    	                            fd, buf.m.offset);
    	    if(buffers[i].start == MAP_FAILED)
    	    {
    	        spdlog::error("Failed to memory map for device at {}", devicePath);
    	        return { };
    	    }
    	}

    	// ----------------------------
    	// Queue all buffers
    	// ----------------------------
    	for(int i = 0; i < req.count; i++)
    	{
    	    v4l2_buffer buf{};
    	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	    buf.memory = V4L2_MEMORY_MMAP;
    	    buf.index = i;
    	    if(ioctl(fd, VIDIOC_QBUF, &buf) < 0)
    	    {
    	        spdlog::error("Failed to queue buffers for device at {}", devicePath);
    	        for(int k = 0; k < buffers.size(); ++k)
    	        {
    	        	auto b& = buffers[k];
        			munmap(b.start, b.length);
    	        }
    	        return { };
    	    }
    	}

    	VideoSourceLinux::Device device;
    	device.fd = fd;
    	device.buffers = std::move(buffers);
    	device.devicePath = devicePath;

    	return device;
	}

	VideoSourceLinux::VideoSourceLinux(const std::string_view devicePath)
	{
		m_device = OpenDevice(devicePath);
	}

	VideoSourceLinux::~VideoSourceLinux()
	{
		if(m_device)
			close();
	}

	IVideoSource::Result VideoSourceLinux::open()
	{
    	v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	if(ioctl(m_device.fd, VIDIOC_STREAMON, &type) < 0)
    	{
    	    spdlog::info("Failed to set VIDIOC_STREAMON for device at {}", m_device.devicePath);
    	    return IVideoSource::Result::Failed;
    	}
		return IVideoSource::Result::Success;
	}
	void VideoSourceLinux::close()
	{
		skvmoip_assert(m_device.has_value());

    	if (ioctl(m_device.fd, VIDIOC_STREAMOFF, &type) < 0)
    	    spdlog::errro("Failed to set VIDIOC_STREAMOFF for device at {}", m_device.devicePath);

    	for (auto& b : m_device.buffers) {
    	    munmap(b.start, b.length);
    	}

    	::close(fd);

    	m_device = { };
	}
	bool VideoSourceLinux::isReady()
	{
		return m_device.has_value();
	}
	bool VideoSourceLinux::readNV12FrameToBuffer(u8* const nv12Buffer, u32 nv12BufferSize)
	{
		v4l2_buffer buf{};
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
	
		if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
		{
			spdog::error("Failed to deque buffer");
		    return false;
		}
	
		// Access NV12 data here:
		uint8_t* data = (uint8_t*)buffers[buf.index].start;
		size_t data_size = buf.bytesused;

		skvmoip_debug_assert(data_size == nv12BufferSize);

		// Process NV12 frame (Y plane + UV interleaved)
		std::memcpy(nv12Buffer, data, data_size);
	
		// Re-queue buffer
		if(ioctl(fd, VIDIOC_QBUF, &buf) < 0)
		{
		   	spdlog::error("Failed to enqueue buffer");
		    return false;
		}

		return true;
	}

	std::pair<u32, u32> VideoSourceLinux::getInputFrameRate()
	{
		return { 60, 1 };
	}

	std::pair<u32, u32> VideoSourceLinux::getOutputFrameSize()
	{
		return { 1920, 1080 };
	}
}

