#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>

namespace SKVMOIP
{
	namespace Network
	{
		struct NetworkPacketValues
		{
			enum class DeviceType : u8
			{
				Keyboard = 0,
				Mouse = 1
			};

			enum class Bool : u8
			{
				False = 0,
				True = 1
			};

			enum class KeyStatus : u8
			{
				Released = 0,
				Pressed = 1
			};
		};

		/* 10 Bytes */
		union NetworkPacket
		{
		    u8 deviceType: 1;
		    
		    /* Keyboard: 2 Bytes */
		    struct
		    {
		        u8 : 1;
		        u8 keyStatus : 1;
			    u8 usbHIDUsageID : 8;
		    };
		
			/* Mouse: 10 Bytes */
			struct
			{
			    /* 2 Bytes */
			    u8 : 1;
				u8 middleMBPressed : 1;
				u8 middleMBReleased : 1;
				u8 leftMBPressed : 1;
				u8 leftMBReleased : 1;
				u8 rightMBPressed : 1;
				u8 rightMBReleased : 1;
				u8 bfMBPressed : 1;
				u8 bfMBReleased : 1;
				u8 bbMBPressed : 1;
				u8 bbMBReleased : 1;
				
			    /* 8 Bytes */
				s16 mousePointX;
				s16 mousePointY;
				s16 mouseWheelX;
				s16 mouseWheelY;
			};
		};

		SKVMOIP_API NetworkPacket GetNetworkPacketFromKMInputData(const Win32::KMInputData& inputData);
		SKVMOIP_API void DumpNetworkPacket(const NetworkPacket& packet);
	}
}
