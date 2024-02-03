#include <SKVMOIP/KMNetStream.hpp>
#include <SKVMOIP/HID/HIDUsageID.hpp>
#include <SKVMOIP/Network/NetworkPacket.hpp>
#include <cstring>


namespace SKVMOIP
{
	KMNetStream::KMNetStream() : AsyncQueueSocket(std::move(Network::Socket(Network::SocketType::Stream, Network::IPAddressFamily::IPv4, Network::IPProtocol::TCP))),
								m_modifierKeys(0),
								m_startTime(std::chrono::steady_clock::now()),
								m_mouseMaxReponseTime(5),
								m_mouseMaxDispPerDeltaX(5),
								m_mouseMaxDispPerDeltaY(5),
								m_mouseCurDispX(0),
								m_mouseCurDispY(0)
	{
	
	}
	
	void KMNetStream::sendInput(const Win32::KMInputData& inputData)
	{
		const Network::NetworkPacket netPacket = Network::GetNetworkPacketFromKMInputData(inputData, m_modifierKeys);
		AsyncQueueSocket::send(reinterpret_cast<const u8*>(&netPacket), sizeof(netPacket));
	}
	
	void KMNetStream::sendMouseInput(const Win32::MouseInput& mouseInput)
	{
		m_mouseCurDispX += mouseInput.movement.x;
		m_mouseCurDispY += mouseInput.movement.y;
		if((((std::abs(m_mouseCurDispX) >= m_mouseMaxDispPerDeltaX) || (std::abs(m_mouseCurDispY) >= m_mouseMaxDispPerDeltaY))
					&& (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_startTime).count() >= m_mouseMaxReponseTime))
				|| mouseInput.isAnyButton)
		{
			Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Mouse };
			memcpy(&kmInputData.mouseInput, reinterpret_cast<const void*>(&mouseInput), sizeof(Win32::MouseInput));
			kmInputData.mouseInput.movement.x = m_mouseCurDispX;
			kmInputData.mouseInput.movement.y = m_mouseCurDispY;
			sendInput(kmInputData);
			m_mouseCurDispX = 0;
			m_mouseCurDispY = 0;
			m_startTime = std::chrono::steady_clock::now();
		}
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
	
	void KMNetStream::sendKeyboardInput(const Win32::KeyboardInput& keyboardInput)
	{
		if(keyboardInput.keyStatus == Win32::KeyStatus::Pressed)
		{
			switch(keyboardInput.makeCode)
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
					u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(keyboardInput.makeCode));
					_assert(modifierKey != 0);
					m_modifierKeys |= modifierKey;
					break;
				}
			}
	
			if(m_pressedKeys.find(keyboardInput.makeCode) != m_pressedKeys.end())
				/* skip as the key is already pressed */
				return;
			else
				m_pressedKeys.insert({keyboardInput.makeCode, Win32::KeyStatus::Pressed});
		}
		else if(keyboardInput.keyStatus == Win32::KeyStatus::Released)
		{
			switch(keyboardInput.makeCode)
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
					u8 modifierKey = GetModifierBitFromMakeCode(IntToEnumClass<PS2Set1MakeCode>(keyboardInput.makeCode));
					_assert(modifierKey != 0);
					_assert((m_modifierKeys & modifierKey) == modifierKey);
					m_modifierKeys &= ~(modifierKey);
					break;
				}
			}
			auto result = m_pressedKeys.erase(keyboardInput.makeCode);
			_assert(result == 1);
		}
	
		Win32::KMInputData kmInputData = { Win32::RawInputDeviceType::Keyboard };
		memcpy(&kmInputData.keyboardInput, reinterpret_cast<const void*>(&keyboardInput), sizeof(Win32::KeyboardInput));
		sendInput(kmInputData);
	}
}
