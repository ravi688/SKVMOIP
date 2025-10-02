#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>

#include <netsocket/netasyncsocket.hpp> // for netsocket::AsyncSocket

#include <chrono>
#include <optional>

namespace SKVMOIP
{
	static void PowerStatusReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);

	class KMNetStream : public netsocket::AsyncSocket
	{
		friend void PowerStatusReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);
	private:
		u8 m_modifierKeys;
		std::chrono::time_point<std::chrono::steady_clock> m_startTime;
		u32 m_mouseMinDelay;
		s32 m_mouseCurDispX;
		s32 m_mouseCurDispY;
		void (*m_powerStatusReceiveCallback)(bool, void*);
		void* m_callbackUserData;
		BinaryFormatter m_powerStatusFormatter;

	public:
	
		KMNetStream();

		// Not copyable and not movable
		KMNetStream(KMNetStream&&) = delete;
		KMNetStream(KMNetStream&) = delete;
	
		void sendInput(const Win32::KMInputData& inputData);
		void sendMouseInput(const Win32::MouseInput& mouseInput);
		void sendKeyboardInput(const Win32::KeyboardInput& keyboardInput);
		void sendFrontPanelInput(std::optional<bool> powerButton, std::optional<bool> resetButton = { });
		void receivePowerStatus(void (*callback)(bool isOn, void* userData), void* userData);
	};
}
