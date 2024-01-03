#pragma once

#include <SKVMOIP/defines.hpp>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
# 	include <Windows.h>
#endif

namespace Win32
{
	/*

		More information: https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input

		Key Code: Virtual Code

		Repeat Count:
		
		You can check the repeat count to determine whether a keystroke message represents more than one keystroke. 
		The system increments the count when the keyboard generates WM_KEYDOWN or WM_SYSKEYDOWN messages faster than an application can process them. 
		This often occurs when the user holds down a key long enough to start the keyboard's automatic repeat feature. 
		Instead of filling the system message queue with the resulting key-down messages, 
		the system combines the messages into a single key down message and increments the repeat count. 
		Releasing a key cannot start the automatic repeat feature, so the repeat count for WM_KEYUP and WM_SYSKEYUP messages is always set to 1.

		Scan Codes:
		
		The scan code is the value that the system generates when the user presses a key. 
		It is a value that identifies the key pressed regardless of the active keyboard layout, 
		as opposed to the character represented by the key. An application typically ignores scan codes. 
		Instead, it uses the virtual-key codes to interpret keystroke messages.

		Modern keyboards are using Human Interface Devices (HID) specification to communicate with a computer. 
		Keyboard driver converts reported HID Usage values sent from the keyboard to scan сodes and passes them on to applications.


		Extended Key Flag:

		The extended-key flag indicates whether the keystroke message originated from one of the additional keys on the Enhanced 101/102-key keyboard. 
		The extended keys consist of the ALT and CTRL keys on the right-hand side of the keyboard; 
		the INS, DEL, HOME, END, PAGE UP, PAGE DOWN, and arrow keys in the clusters to the left of the numeric keypad; 
		the NUM LOCK key; the BREAK (CTRL+PAUSE) key; the PRINT SCRN key; and the divide (/) and ENTER keys in the numeric keypad. 
		The right-hand SHIFT key is not considered an extended-key, it has a separate scan code instead.

		If specified, the scan code consists of a sequence of two bytes, where the first byte has a value of 0xE0.


	*/

	enum class KeyStatus : u8
	{
		Released = 0,
		Pressed
	};

	struct KeyboardInput
	{
		/* PS/2 set 1 make code
		 * see the table: https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf  */
		u32 makeCode;
		/* Windows virtual key code */
		u16 virtualKey;
		KeyStatus keyStatus;
		/* True if the scan code contains E0 in the first byte (from left to right) */
		bool isExtended0;
		/* True if the scan code contains E1 in the first byet (from left to right) */
		bool isExtended1;
		/* True if Alt or F10 key were pressed when this key is pressed */
		bool isAltorF10;
	};

	struct MouseInput
	{
		struct
		{
			s16 x;
			s16 y;
		} movement;
		struct
		{
			s16 x;
			s16 y;
		} wheel;

		KeyStatus buttonStatus;

		bool isWheelX;
		bool isWheelY;

		bool isAnyButton;

		struct
		{
			bool isTransition;
			KeyStatus status;;
		} leftButton;
		
		struct
		{
			bool isTransition;
			KeyStatus status;;
		}  rightButton;

		struct
		{
			bool isTransition;
			KeyStatus status;;
		} middleButton;

		struct
		{
			bool isTransition;
			KeyStatus status;;
		} browseForwardButton;

		struct
		{
			bool isTransition;
			KeyStatus status;;
		} browseBackwardButton;

		bool isMoveRelative;
		bool isMoveAbsolute;
		bool isVirtualDesktop;
	};

	SKVMOIP_API KeyboardInput DecodeRawKeyboardInput(RAWKEYBOARD* rawKeyboard);
	SKVMOIP_API void DumpKeyboardInput(const KeyboardInput* keyboardInput);

	SKVMOIP_API MouseInput DecodeRawMouseInput(RAWMOUSE* rawMouse);
	SKVMOIP_API void DumpMouseInput(const MouseInput* mouseInput);

	enum class RawInputDeviceType
	{
		Mouse,
		Keyboard
	};

	struct KMInputData
	{
		Win32::RawInputDeviceType deviceType;
		union
		{
			Win32::MouseInput mouseInput;
			Win32::KeyboardInput keyboardInput;
		};
	};


	SKVMOIP_API void DisplayRawInputDeviceList();
	SKVMOIP_API void RegisterRawInputDevices(std::vector<RawInputDeviceType> deviceTypes);
}
