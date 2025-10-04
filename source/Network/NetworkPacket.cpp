#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/debug.h>

namespace SKVMOIP
{
	namespace Network
	{
		SKVMOIP_API NetworkPacket GetNetworkPacket(OptionalReference<const Win32::KMInputData> inputData, u8 modifierKeys, 
												std::optional<NetworkPacketValues::KeyStatus> powerButton,
												std::optional<NetworkPacketValues::KeyStatus> resetButton)
		{
			static_assert(sizeof(NetworkPacket) == 12);
		
			NetworkPacket packet = { };

			if(inputData)
			{
				switch(inputData->deviceType)
				{
					case Win32::RawInputDeviceType::Keyboard:
					{
						const Win32::KeyboardInput& input = inputData->keyboardInput;
						packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::Keyboard);
						packet.keyStatus = EnumClassToInt((input.keyStatus == Win32::KeyStatus::Released) ? 
															NetworkPacketValues::KeyStatus::Released :
															NetworkPacketValues::KeyStatus::Pressed);
						packet.usbHIDUsageID = GetHIDUsageFromPS2Set1MakeCode(IntToEnumClass<PS2Set1MakeCode>(input.makeCode));
						packet.modifierKeys = modifierKeys;
						break;
					}
					case Win32::RawInputDeviceType::Mouse:
					{
						const Win32::MouseInput& input = inputData->mouseInput;
						packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::Mouse);

						static u8 mouseButtons = 0;
						if(input.isAnyButton)
						{
							if(input.leftButton.isTransition)
							{
								if(input.leftButton.status == Win32::KeyStatus::Pressed)
									mouseButtons |= EnumClassToInt(NetworkPacketValues::MouseButtonBits::LeftButtonBit);
								else mouseButtons &= ~EnumClassToInt(NetworkPacketValues::MouseButtonBits::LeftButtonBit);
							}
							if(input.rightButton.isTransition)
							{
								if(input.rightButton.status == Win32::KeyStatus::Pressed)
									mouseButtons |= EnumClassToInt(NetworkPacketValues::MouseButtonBits::RightButtonBit);
								else
									mouseButtons &= ~EnumClassToInt(NetworkPacketValues::MouseButtonBits::RightButtonBit);
							}
							if(input.middleButton.isTransition)
							{
								if(input.middleButton.status == Win32::KeyStatus::Pressed)
									mouseButtons |= EnumClassToInt(NetworkPacketValues::MouseButtonBits::MiddleButtonBit);
								else
									mouseButtons &= ~EnumClassToInt(NetworkPacketValues::MouseButtonBits::MiddleButtonBit);
							}
							if(input.browseForwardButton.isTransition)
							{
								if(input.browseForwardButton.status == Win32::KeyStatus::Pressed)
									mouseButtons |= EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseForwardButtonBit);
								else
									mouseButtons &= ~EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseForwardButtonBit);
							}
							if(input.browseBackwardButton.isTransition)
							{
								if(input.browseBackwardButton.status == Win32::KeyStatus::Pressed)
									mouseButtons |= EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseBackwardButtonBit);
								else
									mouseButtons &= ~EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseBackwardButtonBit);
							}
						}
						packet.mouseButtons = mouseButtons;

						if(input.isMoveRelative)
						{
							packet.mousePointX = input.movement.x;
							packet.mousePointY = input.movement.y;
							packet.mouseWheelX = input.wheel.x;
							packet.mouseWheelY = input.wheel.y;
						}
						else if(input.isMoveAbsolute)
						{
							debug_log_warning("Absolute Mouse Pointer movement is not supported, You're probably using a PenTablet");
						}
						else if(input.isVirtualDesktop)
						{
							debug_log_warning("Virtual Desktop Pointer movement is not supported");
						}
						break;
					}
					default: { skvmoip_debug_assert(false); break; }
				}
			}
			else if(powerButton || resetButton)
			{
				skvmoip_debug_assert(!(powerButton && resetButton));
				packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::FrontPanel);
				packet.buttonStatus = ((powerButton && (powerButton.value() == NetworkPacketValues::KeyStatus::Pressed)) 
										|| (resetButton && (resetButton.value() == NetworkPacketValues::KeyStatus::Pressed))) ? 1 : 0;
				packet.isPowerButton = static_cast<bool>(powerButton) ? 1 : 0;
				packet.isResetButton = static_cast<bool>(resetButton) ? 1 : 0;
			}
			return packet;
		}

		SKVMOIP_API NetworkPacket GetPowerStatusRequestNetworkPacket() 
		{ 
			NetworkPacket packet { }; 
			packet.deviceType = EnumClassToInt(NetworkPacketValues::DeviceType::PowerStatusRequest);
			return packet;
		}

		SKVMOIP_API void DumpNetworkPacket(const NetworkPacket& packet)
		{
			switch(IntToEnumClass<NetworkPacketValues::DeviceType>(packet.deviceType))
			{
				case NetworkPacketValues::DeviceType::Keyboard:
				{
					debug_log_info("KeyboardInput { HID Usage ID: %u, Status: %u }", packet.usbHIDUsageID, packet.keyStatus);
					break;
				}
				case NetworkPacketValues::DeviceType::Mouse:
				{
					debug_log_info("MouseInput { Point (%d, %d), Wheel: (%d, %d), MB: %u, LB: %u, RB: %u, BFB: %u, BBB: %u }",
									packet.mousePointX, 
									packet.mousePointY,
									packet.mouseWheelX,
									packet.mouseWheelY,
									HAS_FLAG(packet.mouseButtons, EnumClassToInt(NetworkPacketValues::MouseButtonBits::MiddleButtonBit)), 
									HAS_FLAG(packet.mouseButtons, EnumClassToInt(NetworkPacketValues::MouseButtonBits::LeftButtonBit)), 
									HAS_FLAG(packet.mouseButtons, EnumClassToInt(NetworkPacketValues::MouseButtonBits::RightButtonBit)), 
									HAS_FLAG(packet.mouseButtons, EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseForwardButtonBit)), 
									HAS_FLAG(packet.mouseButtons, EnumClassToInt(NetworkPacketValues::MouseButtonBits::BrowseBackwardButtonBit)));
					break;
				}
				default:
				{
					skvmoip_debug_assert(false);
					break;
				}
			}
		}
	}
}