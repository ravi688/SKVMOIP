#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/debug.h>

namespace SKVMOIP
{
	namespace Network
	{
		SKVMOIP_API NetworkPacket GetNetworkPacketFromKMInputData(const Win32::KMInputData& inputData)
		{
			static_assert(sizeof(NetworkPacket) == 6);
		
			NetworkPacket packet = { };
			switch(inputData.deviceType)
			{
				case Win32::RawInputDeviceType::Keyboard:
				{
					const Win32::KeyboardInput& input = inputData.keyboardInput;
					packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::Keyboard);
					packet.keyStatus = EnumClassToInt((input.keyStatus == Win32::KeyStatus::Released) ? 
														NetworkPacketValues::KeyStatus::Released :
														NetworkPacketValues::KeyStatus::Pressed);
					packet.usbHIDUsageID = GetHIDUsageFromPS2Set1MakeCode(IntToEnumClass<PS2Set1MakeCode>(input.makeCode));
					break;
				}
				case Win32::RawInputDeviceType::Mouse:
				{
					const Win32::MouseInput& input = inputData.mouseInput;
					packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::Mouse);

					if(input.isAnyButton)
					{
						if(input.isMiddleButton)
						{
							if(input.buttonStatus == Win32::KeyStatus::Pressed)
								packet.middleMBPressed = EnumClassToInt(NetworkPacketValues::Bool::True);
							else if(input.buttonStatus == Win32::KeyStatus::Released)
								packet.middleMBReleased = EnumClassToInt(NetworkPacketValues::Bool::True);
						}
						if(input.isLeftButton)
						{
							if(input.buttonStatus == Win32::KeyStatus::Pressed)
								packet.leftMBPressed = EnumClassToInt(NetworkPacketValues::Bool::True);
							else if(input.buttonStatus == Win32::KeyStatus::Released)
								packet.leftMBReleased = EnumClassToInt(NetworkPacketValues::Bool::True);
						}
						if(input.isRightButton)
						{
							if(input.buttonStatus == Win32::KeyStatus::Pressed)
								packet.rightMBPressed = EnumClassToInt(NetworkPacketValues::Bool::True);
							else if(input.buttonStatus == Win32::KeyStatus::Released)
								packet.rightMBReleased = EnumClassToInt(NetworkPacketValues::Bool::True);
						}
						if(input.isBrowseForwardButton)
						{
							if(input.buttonStatus == Win32::KeyStatus::Pressed)
								packet.bfMBPressed = EnumClassToInt(NetworkPacketValues::Bool::True);
							else if(input.buttonStatus == Win32::KeyStatus::Released)
								packet.bfMBReleased = EnumClassToInt(NetworkPacketValues::Bool::True);
						}
						if(input.isBrowseBackwardButton)
						{
							if(input.buttonStatus == Win32::KeyStatus::Pressed)
								packet.bbMBPressed = EnumClassToInt(NetworkPacketValues::Bool::True);
							else if(input.buttonStatus == Win32::KeyStatus::Released)
								packet.bbMBReleased = EnumClassToInt(NetworkPacketValues::Bool::True);
						}
					}
					break;
				}
				default: { _assert(false); break; }
			}
			return packet;
		}

		SKVMOIP_API void DumpNetworkPacket(const NetworkPacket& packet)
		{
			switch(IntToEnumClass<NetworkPacketValues::DeviceType>(packet.deviceType))
			{
				case NetworkPacketValues::DeviceType::Keyboard:
				{
					debug_log_info("KeyboardInput { HID Usage ID: %u }", packet.usbHIDUsageID);
					break;
				}
				case NetworkPacketValues::DeviceType::Mouse:
				{
					debug_log_info("MouseInput { Point (%d, %d), Wheel: (%d, %d), MBPr: %u, MBRl: %u, LBPr: %lu, LBRl: %u, RBPr: %u, RBRl: %u, BFBPr: %u, BFBRl: %u, BBBPr: %lu, BBBRl: %u }",
									packet.mousePointX, 
									packet.mousePointY,
									packet.mouseWheelX,
									packet.mouseWheelY,
									packet.middleMBPressed, 
									packet.middleMBReleased, 
									packet.leftMBPressed, 
									packet.leftMBReleased, 
									packet.rightMBPressed, 
									packet.rightMBReleased, 
									packet.bfMBPressed, 
									packet.bfMBReleased, 
									packet.bbMBPressed, 
									packet.bbMBReleased);
					break;
				}
				default:
				{
					_assert(false);
					break;
				}
			}
		}
	}
}