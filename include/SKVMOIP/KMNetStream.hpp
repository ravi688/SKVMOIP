#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>
#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>

#include <unordered_map>
#include <chrono>
#include <optional>

namespace SKVMOIP
{
	static void PowerStatusReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);

	class KMNetStream : public Network::AsyncQueueSocket
	{
		friend void PowerStatusReceiveCallbackHandler(const u8* data, u32 dataSize, void* userData);
	private:
		std::unordered_map<u32, Win32::KeyStatus> m_pressedKeys;
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
		KMNetStream(KMNetStream&&) = delete;
		KMNetStream(KMNetStream&) = delete;
		KMNetStream& operator=(KMNetStream&) = delete;
		~KMNetStream() = default;
	
		void sendInput(const Win32::KMInputData& inputData);
		void sendMouseInput(const Win32::MouseInput& mouseInput);
		void sendKeyboardInput(const Win32::KeyboardInput& keyboardInput);
		void sendFrontPanelInput(std::optional<bool> powerButton, std::optional<bool> resetButton = { });
		void receivePowerStatus(void (*callback)(bool isOn, void* userData), void* userData);
	};
}
