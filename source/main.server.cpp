#ifndef BUILD_SERVER
#error "BUILD_SERVER is not defined, but still main.server.cpp is being compiled"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/Network/NetworkSocket.hpp>
#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <SKVMOIP/VideoSourceStream.hpp>
#include <SKVMOIP/StopWatch.hpp>
#include <SKVMOIP/Encoder.hpp>

#include <deque>
#include <array>
#include <thread>
#include <condition_variable>
#include <memory>

#include <bufferlib/buffer.h>

#define LISTEN_IP_ADDRESS "localhost"
#define LISTEN_PORT_NUMBER "2020"

using namespace SKVMOIP;

#if 0

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

		while(!controlProtocol.shouldClose())
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

#endif /* 0 */

int main(int argc, const char* argv[])
{
	Win32::InitializeMediaFundationAndCOM();
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

	std::unique_ptr<VideoSourceStream> hdmiStream(new VideoSourceStream(device.value(), VideoSourceStream::Usage::NV12Read, preferenceList));

	if (!(*hdmiStream))
		return 0;

	hdmiStream->dump();

	std::pair<u32, u32> frameSize = hdmiStream->getOutputFrameSize();
	std::pair<u32, u32> frameRatePair = hdmiStream->getInputFrameRate();
	_assert((frameSize.first == 1920) && (frameSize.second == 1080));
	_assert((frameRatePair.first == 60) && (frameRatePair.second == 1));
	u32 frameRate = hdmiStream->getInputFrameRateF32();

	buffer_t nv12Buffer = buf_create(sizeof(u8), (frameSize.first * frameSize.second * 3) >> 1, 0);

	std::unique_ptr<Encoder> encoder(new Encoder(frameSize.first, frameSize.second));

	Network::Socket listenSocket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP);
	if(listenSocket.bind(LISTEN_IP_ADDRESS, LISTEN_PORT_NUMBER) != Network::Result::Success)
	{
		debug_log_error("Failed to bind list socket to %s:%s", LISTEN_IP_ADDRESS, LISTEN_PORT_NUMBER);
		return 1;
	}
	Network::Socket streamSocket = Network::Socket::CreateInvalid();

	u32 numMaxConnections = 1;
	u32 numConnections = 0;
	DEBUG_LOG_INFO("Listening on %s:%s", LISTEN_IP_ADDRESS, LISTEN_PORT_NUMBER);
	while((numConnections < numMaxConnections) && (listenSocket.listen() == Network::Result::Success))
	{
		if(std::optional<Network::Socket> acceptedSocket = listenSocket.accept())
		{
			DEBUG_LOG_INFO("Connection accepted");
			_assert(acceptedSocket->isConnected());
			streamSocket = std::move(*acceptedSocket);
		}
		else
		{
			DEBUG_LOG_ERROR("Unable to accept incoming connection");
			continue;
		}
		++numConnections;
	}

	_assert(streamSocket.isConnected());

	while(true)
	{
		buf_clear_buffer(&nv12Buffer, NULL);
		u8* buffer = reinterpret_cast<u8*>(buf_get_ptr(&nv12Buffer));
		u32 bufferSize = static_cast<u32>(buf_get_capacity(&nv12Buffer));
		if(!hdmiStream->readNV12FrameToBuffer(buffer, bufferSize))
			continue;

		u8* outputBuffer;
		u32 outputBufferSize;
		SKVMOIP::StopWatch encodeWatch;
		if(!encoder->encodeNV12(buffer, bufferSize, outputBuffer, outputBufferSize))
		{
			encodeWatch.stop();
			DEBUG_LOG_INFO("Failed to encode");
			continue;
		}
		else if(outputBuffer == NULL)
		{
			encodeWatch.stop();
			continue;
		}
		auto encodeTime = encodeWatch.stop();

		SKVMOIP::StopWatch netWatch;
		if(streamSocket.isValid() && streamSocket.isConnected())
		{
			bool isLengthSuccess = false;
			if(streamSocket.send(reinterpret_cast<u8*>(&outputBufferSize), sizeof(outputBufferSize)) != Network::Result::Success)
				DEBUG_LOG_ERROR("Failed to send encoded data length");
			else
				isLengthSuccess = true;
			if(isLengthSuccess && (streamSocket.send(outputBuffer, outputBufferSize) != Network::Result::Success))
				DEBUG_LOG_ERROR("Failed to send encoded data");
		}
		auto netTime = netWatch.stop();
		debug_log_info("Encode time: %lu ms, SendTime: %lu, Encode size: %.2f", encodeTime, netTime, outputBufferSize / 1024.0);
	}

	buf_free(&nv12Buffer);

	Win32::DeinitializeMediaFoundationAndCOM();
	return 0;
}
