#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>

#include <optional>

namespace SKVMOIP
{
	namespace Network
	{
		struct NetworkPacketValues
		{
			enum class DeviceType : u8
			{
				Keyboard = 0,
				Mouse = 1,
				FrontPanel = 2,
				PowerStatusRequest = 3
			};

			enum class Bool : u8
			{
				False = 0,
				True = 1
			};

			/* https://wiki.osdev.org/USB_Human_Interface_Devices */
			enum class MouseButtonBits : u8
			{
				LeftButtonBit = BIT8(0),
				RightButtonBit = BIT8(1),
				MiddleButtonBit = BIT8(2),

				/* HID device specific */
				BrowseForwardButtonBit = BIT8(3),
				BrowseBackwardButtonBit = BIT8(4)
			};

			enum class KeyStatus : u8
			{
				Released = 0,
				Pressed = 1
			};
		};

		/* 10 Bytes */
		struct NetworkPacket
		{
			/* 1 Byte */
		    u8 deviceType;
		    
		    union
		    {
		    	/* Keyboard: 3 Bytes */
		    	struct
		    	{
		    	    u8 keyStatus;
			  	  	u8 usbHIDUsageID;
			  	  	u8 modifierKeys;
		    	};
		
				/* Mouse: 9 Bytes */
				struct
				{
				  /* BIT8(0) = left button is being pressed 
				   * BIT8(1) = right button is being pressed 
				   * BIT8(2) = middle button is being pressed */
				  u8 mouseButtons;
				  /* 8 Bytes */
				  s16 mousePointX;
				  s16 mousePointY;
				  s16 mouseWheelX;
				  s16 mouseWheelY;
				};

				/* Front Panel: 1 Byte */
				struct
				{
					u8 buttonStatus : 1;
					u8 isPowerButton : 1;
					u8 isResetButton : 1;
				};
			};
		};

		SKVMOIP_API NetworkPacket GetNetworkPacket(OptionalReference<const Win32::KMInputData> inputData, u8 modifierKeys = 0, 
			std::optional<NetworkPacketValues::KeyStatus> powerButton = { },
			std::optional<NetworkPacketValues::KeyStatus> resetButton = { });
		SKVMOIP_API NetworkPacket GetPowerStatusRequestNetworkPacket();
		SKVMOIP_API void DumpNetworkPacket(const NetworkPacket& packet);
	}
}
