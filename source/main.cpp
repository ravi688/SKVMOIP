
#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/Win32/Win32DrawSurface.hpp>
#include <SKVMOIP/StopWatch.hpp>

#pragma push_macro("assert")
#pragma push_macro("_assert")
#undef assert
#undef _assert
#include <SKVMOIP/third_party/NvDecoder.hpp>
#include <SKVMOIP/third_party/AppDecUtils.hpp>
#pragma pop_macro("assert")
#pragma pop_macro("_assert")

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>
#include <memory>

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <bufferlib/buffer.h>

#include <x264/include/x264.h>
#include <x264/include/x264_config.h>

#include <NvidiaCodec/include/cuviddec.h>
#include <NvidiaCodec/include/nvcuvid.h>
#include <NvidiaCodec/include/nvEncodeAPI.h>


using namespace SKVMOIP;

#define SERVER_IP_ADDRESS "192.168.1.113"
#define SERVER_PORT_NUMBER "2000"

#define NETWORK_THREAD_BUFFER_SIZE 1024

static std::mutex gMutex;
static std::condition_variable gCv;

static std::deque<Win32::KMInputData> gInputQueue;

static std::array<Win32::KMInputData, NETWORK_THREAD_BUFFER_SIZE> gNetworkThreadBuffer;

static u8 gModifierKeys = 0;

static void NetworkHandler(Network::Socket& networkStream)
{
	while(1)
	{
		std::size_t index = 0;
		{
			std::unique_lock<std::mutex> lock(gMutex);
			gCv.wait(lock, [] { return !gInputQueue.empty(); });
			while(!gInputQueue.empty() && (index < NETWORK_THREAD_BUFFER_SIZE))
			{
				gNetworkThreadBuffer[index++] = gInputQueue.back();
				gInputQueue.pop_back();
			}
		}

		std::size_t count = index;

		if(count == 0) continue;

#ifdef GLOBAL_DEBUG
		if(count == NETWORK_THREAD_BUFFER_SIZE)
			debug_log_warning("Consider increasing the value of NETWORK_THREAD_BUFFER_SIZE, which is currently set at %lu, but should be at least %lu", 
							 NETWORK_THREAD_BUFFER_SIZE, gInputQueue.size() + NETWORK_THREAD_BUFFER_SIZE);
#endif /* Debug Mode */

		index = 0;
		while(index < count)
		{
			const Win32::KMInputData& kmInputData = gNetworkThreadBuffer[index];
			const Network::NetworkPacket netPacket = Network::GetNetworkPacketFromKMInputData(kmInputData, gModifierKeys);
			// Network::DumpNetworkPacket(netPacket);
			Network::Result result = networkStream.send(reinterpret_cast<const u8*>(&netPacket), sizeof(netPacket));
			if(result == Network::Result::SocketError)
			{
				debug_log_error("Failed to send Network Packet over Network Stream, error code: %d", WSAGetLastError());
				exit(-1);
			}
			++index;
		}
	}
}

static std::unordered_map<u32, Win32::KeyStatus> gPressedKeys;

static void MouseInputHandler(void* mouseInputData, void* userData)
{
	std::unique_lock<std::mutex> lock(gMutex);
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Mouse };
	memcpy(&kmInputData.mouseInput, mouseInputData, sizeof(Win32::MouseInput));
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

static HIDUsageIDModifierBits GetModifierBitFromMakeCode(PS2Set1MakeCode makeCode)
{
	switch(makeCode)
	{
		case PS2Set1MakeCode::LeftControl: return HIDUsageIDModifierBits::LEFTCTRL_BIT;
		case PS2Set1MakeCode::LeftShift: return HIDUsageIDModifierBits::LEFTSHIFT_BIT;
		case PS2Set1MakeCode::LeftAlt: return HIDUsageIDModifierBits::LEFTALT_BIT;
		case PS2Set1MakeCode::LeftGUI: return HIDUsageIDModifierBits::LEFTGUI_BIT; 
		case PS2Set1MakeCode::RightControl: return HIDUsageIDModifierBits::RIGHTCTRL_BIT; 
		case PS2Set1MakeCode::RightShift: return HIDUsageIDModifierBits::RIGHTSHIFT_BIT;
		case PS2Set1MakeCode::RightAlt: return HIDUsageIDModifierBits::RIGHTALT_BIT; 
		case PS2Set1MakeCode::RightGUI: return HIDUsageIDModifierBits::RIGHTGUI_BIT;
		default: return IntToEnumClass<HIDUsageIDModifierBits>(static_cast<u8>(0));
	}
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	Win32::KeyboardInput* kInput = reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData);
	if(kInput->keyStatus == Win32::KeyStatus::Pressed)
	{
		switch(kInput->makeCode)
		{
			case PS2Set1MakeCode::LeftControl:
			case PS2Set1MakeCode::LeftShift:
			case PS2Set1MakeCode::LeftAlt:
			case PS2Set1MakeCode::LeftGUI: 
			case PS2Set1MakeCode::RightControl: 
			case PS2Set1MakeCode::RightShift:
			case PS2Set1MakeCode::RightAlt: 
			case PS2Set1MakeCode::RightGUI: 
			{
				u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(kInput->makeCode));
				_assert(modifierKey != 0);
				gModifierKeys |= modifierKey;
				break;
			}
		}

		if(gPressedKeys.find(kInput->makeCode) != gPressedKeys.end())
			/* skip as the key is already pressed */
			return;
		else
			gPressedKeys.insert({kInput->makeCode, Win32::KeyStatus::Pressed});
	}
	else if(kInput->keyStatus == Win32::KeyStatus::Released)
	{
		switch(kInput->makeCode)
		{
			case PS2Set1MakeCode::LeftControl:
			case PS2Set1MakeCode::LeftShift:
			case PS2Set1MakeCode::LeftAlt:
			case PS2Set1MakeCode::LeftGUI: 
			case PS2Set1MakeCode::RightControl: 
			case PS2Set1MakeCode::RightShift:
			case PS2Set1MakeCode::RightAlt: 
			case PS2Set1MakeCode::RightGUI: 
			{
				u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(kInput->makeCode));
				_assert(modifierKey != 0);
				_assert((gModifierKeys & modifierKey) == modifierKey);
				gModifierKeys &= ~(modifierKey);
				break;
			}
		}
		_assert(gPressedKeys.erase(kInput->makeCode) == 1);
	}
	Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Keyboard };
	memcpy(&kmInputData.keyboardInput, keyboardInputData, sizeof(Win32::KeyboardInput));
	
	std::unique_lock<std::mutex> lock(gMutex);
	gInputQueue.push_front(kmInputData);
	lock.unlock();
	gCv.notify_one();
}

static std::unique_ptr<Win32::Win32DrawSurface> gWin32DrawSurfaceUPtr;
static std::unique_ptr<VideoSourceStream> gHDMIStream;
static buffer_t gNV12Buffer;

static u32 counter = 0;

class Encoder
{
private:
	x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *h;
    x264_nal_t *nal;
    int i_nal;
    u32 m_width;
    u32 m_height;
    u32 m_frameCount;
 	bool m_isValid;
public:
	Encoder(u32 width, u32 height);
	~Encoder();

	bool encodeNV12(u8* const nv12Data, u32 nv12DataSize, u8* &outputBuffer, u32& outputBufferSize);
};

Encoder::Encoder(u32 width, u32 height) : m_width(width), m_height(height), m_frameCount(0), m_isValid(false)
{
	if(x264_param_default_preset(&param, "ultrafast", "zerolatency") < 0)
	{
		debug_log_error("x264: Failed to set default preset");
		return;
	}

	/* Configure non-default params */
	param.i_threads = 6;
	param.i_bitdepth = 8;
	param.i_csp = X264_CSP_NV12;
	param.i_width  = width;
	param.i_height = height;
	param.b_vfr_input = 0;
	param.b_repeat_headers = 1;
	param.b_annexb = 1;
	param.b_vfr_input = 0;
	param.b_opencl = 1;
	param.i_fps_num = 144;
	param.i_fps_den = 1;

  	if(x264_param_apply_profile(&param, "high444") < 0)
  	{
  		debug_log_error("x264: Failed to apply profile restrictions");
  		return;
  	}

   if(x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) < 0)
   {
   	debug_log_error("x264: Failed to allocate picture");
   	return;
   }

   h = x264_encoder_open(&param);
   if(!h)
   {
   	debug_log_error("Failed to open the encoder");
   	x264_picture_clean(&pic);
   	return;
   }
   m_isValid = true;
}

Encoder::~Encoder()
{
	if(!m_isValid) return;
	 x264_encoder_close(h);
	x264_picture_clean(&pic);
}

bool Encoder::encodeNV12(u8* const nv12Data, u32 nv12DataSize, u8* &outputBuffer, u32& outputBufferSize)
{
	_assert(m_isValid);

	u32 luma_size = m_width * m_height;
	u32 chroma_size = luma_size >> 2;

	memcpy(pic.img.plane[0], nv12Data,  luma_size);
	memcpy(pic.img.plane[1], nv12Data + luma_size, chroma_size + chroma_size);

	pic.i_pts = m_frameCount;
	auto i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
	if(i_frame_size < 0)
	{
		debug_log_error("Failed to encode");
		outputBuffer = NULL;
		outputBufferSize = 0;
		return false;
	}
	else if(i_frame_size)
	{
		outputBuffer = nal->p_payload;
		outputBufferSize = i_frame_size;;
	}

    ++m_frameCount;
	return true;
}

static std::unique_ptr<Encoder> gH264Encoder;
static std::unique_ptr<NvDecoder> gNvDecoder; 
static std::unique_ptr<NV12ToRGBConverter> gCSConverter;

static void WindowPaintHandler(void* paintInfo, void* userData)
{
	u8* pixels = gWin32DrawSurfaceUPtr->getPixels();

	auto drawSurfaceSize = gWin32DrawSurfaceUPtr->getSize();

	// if(!gHDMIStream->readRGBFrameToBuffer(, gWin32DrawSurfaceUPtr->getBufferSize()))
	// {
	// 	return;
	// }

	buf_clear_buffer(&gNV12Buffer, NULL);
	u8* buffer = reinterpret_cast<u8*>(buf_get_ptr(&gNV12Buffer));
	u32 bufferSize = static_cast<u32>(buf_get_capacity(&gNV12Buffer));
	if(!gHDMIStream->readNV12FrameToBuffer(buffer, bufferSize))
		return;

	u8* outputBuffer;
	u32 outputBufferSize;
	SKVMOIP::StopWatch encodeWatch;
	if(!gH264Encoder->encodeNV12(buffer, bufferSize, outputBuffer, outputBufferSize))
	{
		encodeWatch.stop();
		debug_log_error("Failed to encode");
		return;
	}
	else if(outputBuffer == NULL)
	{
		encodeWatch.stop();
		return;
	}
	auto encodeTime = encodeWatch.stop();

	int nFrameReturned = 0;
	static int nFrame = 0;

	SKVMOIP::StopWatch decodeWatch;
	nFrameReturned = gNvDecoder->Decode(outputBuffer, outputBufferSize);
	if (nFrameReturned)
	{
		auto decodeTime = decodeWatch.stop();
		u8* frame = gNvDecoder->GetFrame();
		_assert(gNvDecoder->GetFrameSize() == bufferSize);
		u8* rgbData;
		SKVMOIP::StopWatch convertWatch;
		if((rgbData = gCSConverter->convert(frame, bufferSize)) != NULL)
		{
			auto convertTime = convertWatch.stop();
			auto rgbDataSize = gCSConverter->getRGBDataSize();
			_assert(rgbDataSize == gWin32DrawSurfaceUPtr->getBufferSize());
			memcpy(pixels, rgbData, gCSConverter->getRGBDataSize());
			debug_log_info("Time info: encode: %lu, decode: %lu, convert: %lu, encodedSize: %.2f kb", encodeTime, decodeTime, convertTime, outputBufferSize / 1024.0);
		}
		else { convertWatch.stop(); debug_log_error("Failed to convert color space"); }
		nFrame += nFrameReturned;
	}
	else { decodeWatch.stop(); debug_log_error("Failed to decode, return value %d", nFrameReturned); }

	Win32::WindowPaintInfo* winPaintInfo = reinterpret_cast<Win32::WindowPaintInfo*>(paintInfo);
	BitBlt(winPaintInfo->deviceContext, 0, 0, drawSurfaceSize.first, drawSurfaceSize.second, gWin32DrawSurfaceUPtr->getHDC(), 0, 0, SRCCOPY);
}

#ifdef BUILD_SERVER

static void HandleHDMIStream(Network::Protocols::USBToHDMIStreamControlProtocol& controlProtocol, u32 clientID, u32 usbPortNumber, std::condition_variable& sync)
{
	USBToHDMIStream hdmiStream(usbPortNumber);

	if(!hdmiStream.isValid())
		controlProtocol.sendMessage(STATUS_FAILED);
	else
		controlProtocol.sendMessage(STATUS_OK, clientID);

	Network::Socket listenSocket;
	while(listenSocket.listen())
	{
		Network::Socket streamSocket = listenSocket.accpet();
		Network::Protocols::USBToHDMIStreamProtocol streamProtocol(streamSocket);
		
		std::vector<u8> buffer;
		buffer.reserve(hdmiStream.getFrameSizeInBytes());

		while(!streamProtocol.shouldClose())
		{
			Encoding::ImageEncode(hdmiStream.getLatestFrame(), buffer);
			streamProtocol.sendFrame(STATUS_OK, buffer);
		}
	}
}

static void HandleNetworkConnection(Network::Socket& connectionSocket)
{
	// UHSCP Protocol
	Network::Protocols::USBToHDMIStreamControlProtocol controlSocket(connectionSocket);
	while(!controlSocket.shouldClose())
	{
		Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket messagePacket;
		if(controlSocket.peekMessage(messagePacket))
		{
			switch(messagePacket.message)
			{
				case Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket::MessageType::AvailableHDMIConnection:
				{
					std::optional<std::vector<u32>> usbPortNumbers = GetAvailableUSBToHDMIConnections();
					if(!usbPortNumbers)
						controlSocket.sendMessage(STATUS_FAILED);
					else
						controlSocket.sendMessage(STATUS_OK, usbPortNumbers);
					break;
				}
				case Network::Protocols::USBToHDMIStreamControlProtocol::MessagePacket::MessageType::StartHDMIStream:
				{
					u32 usbPortNumber = message.getUSBPortNumber();
					if(!isValid(usbPortNumber))
						controlSocket.sendMessage(STATUS_INVALID_USB_PORT_NUMBER);

					std::condition_variable hdmiStreamStartedSignal;
					u32 clientID = GenerateID();
					std::thread hdmiStreamThread(HandleHDMIStream, 
						std::tuple<Network::Protocols::USBToHDMIStreamControlProtocol, u32, usbPortNumber, std::condition_variable>(controlSocket, clientID, usbPortNumber, std::ref(hdmiStreamStartedSignal)));
					hdmiStreamThread.detach();

					hdmiStreamStartedSignal.wait(lock);
				}
			}
		}
	}
}

#endif /* Server */

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	std::optional<Win32::Win32SourceDeviceListGuard> deviceList = Win32::Win32GetSourceDeviceList(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if(!deviceList)
	{
		debug_log_error("Unable to get Video source device list");
		return 0;
	}
			
	std::optional<Win32::Win32SourceDevice> device = deviceList->activateDevice((argc > 1) ? (atoi(argv[1])) : 0);
	if(!device)
	{
		debug_log_error("Unable to create video source device with index: %lu ", 0);
		return 0;
	}

	std::vector<std::tuple<u32, u32, u32>> preferenceList = 
	{
		{ 1920, 1080, 60 },
		{ 1920, 1080, 30 },
		{ 1366, 768, 60 },
		{ 1366, 768, 30 },
		{ 1280, 720, 60 },
		{ 1280, 720, 30 },
		{ 1024, 768, 60 },
		{ 1024, 768, 30 }, 
		{ 960, 720, 60 },
		{ 960, 720, 30 }
	};

	// gHDMIStream = std::move(std::unique_ptr<VideoSourceStream>(new VideoSourceStream(device.value(), VideoSourceStream::Usage::RGB32Read, preferenceList)));
	gHDMIStream = std::move(std::unique_ptr<VideoSourceStream>(new VideoSourceStream(device.value(), VideoSourceStream::Usage::NV12Read, preferenceList)));

	if (!(*gHDMIStream))
		return 0;

	gHDMIStream->dump();
	// gHDMIStream->doReadyRGBReader();

	std::pair<u32, u32> frameSize = gHDMIStream->getOutputFrameSize();
	std::pair<u32, u32> frameRatePair = gHDMIStream->getInputFrameRate();
	u32 frameRate = gHDMIStream->getInputFrameRateF32();

	gNV12Buffer = buf_create(sizeof(u8), (frameSize.first * frameSize.second * 3) >> 1, 0);

	gH264Encoder = std::move(std::unique_ptr<Encoder>(new Encoder(frameSize.first, frameSize.second)));


		int iGpu = 0;
     ck(cuInit(0));
        int nGpu = 0;
        ck(cuDeviceGetCount(&nGpu));
        if (iGpu < 0 || iGpu >= nGpu)
        {
            std::ostringstream err;
            err << "GPU ordinal out of range. Should be within [" << 0 << ", " << nGpu - 1 << "]" << std::endl;
            throw std::invalid_argument(err.str());
        }

        CUcontext cuContext = NULL;
        createCudaContext(&cuContext, iGpu, 0);

	gNvDecoder = std::move(std::unique_ptr<NvDecoder>(new NvDecoder(cuContext, false, cudaVideoCodec_H264, true, false)));
	gCSConverter = std::move(std::unique_ptr<NV12ToRGBConverter>(new NV12ToRGBConverter(frameSize.first, frameSize.second, frameRatePair.first, frameRatePair.second, 32)));

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });


	Window window(frameSize.first, frameSize.second, "Scalable KVM Over IP");

	gWin32DrawSurfaceUPtr = std::move(std::unique_ptr<Win32::Win32DrawSurface>(new Win32::Win32DrawSurface(window.getNativeHandle(), window.getSize().first, window.getSize().second, 32)));

	Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);
	Event::SubscriptionHandle windowPaintHandle = window.getEvent(Window::EventType::Paint).subscribe(WindowPaintHandler, NULL);

#ifdef BUILD_SERVER

	std::vector<std::thread> concurrentConnections;

	Network::Socket listenSocket;
	while(listenSocket.listen())
	{
		Network::Socket connectionSocket = listenSocket.accept();
		concurrentConnections.push_back(std::thread(HandleNetworkConnection, std::ref(connectionSocket)));
	}

#endif /* Server */

	// Network::Socket networkStream(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	// debug_log_info("Trying to connect to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);
	// if(networkStream.connect(SERVER_IP_ADDRESS, SERVER_PORT_NUMBER) == Network::Result::Success)
	// 	debug_log_info("Connected to %s:%s", SERVER_IP_ADDRESS, SERVER_PORT_NUMBER);

	// std::thread networkThread(NetworkHandler, std::ref(networkStream));

	window.runGameLoop(static_cast<u32>(frameRate));

	// networkThread.join();


	buf_free(&gNV12Buffer);

	window.getEvent(Window::EventType::Paint).unsubscribe(windowPaintHandle);
	window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	return 0;
}

