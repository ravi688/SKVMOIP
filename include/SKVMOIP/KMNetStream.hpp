#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>
#include <SKVMOIP/Network/NetworkAsyncQueueSocket.hpp>

#include <unordered_map>

namespace SKVMOIP
{
	class KMNetStream : public Network::AsyncQueueSocket
	{
	private:
		std::unordered_map<u32, Win32::KeyStatus> m_pressedKeys;
		u8 m_modifierKeys;
	
	public:
	
		KMNetStream();
		KMNetStream(KMNetStream&&) = delete;
		KMNetStream(KMNetStream&) = delete;
		KMNetStream& operator=(KMNetStream&) = delete;
		~KMNetStream() = default;
	
		void sendInput(const Win32::KMInputData& inputData);
		void sendMouseInput(const Win32::MouseInput& mouseInput);
		void sendKeyboardInput(const Win32::KeyboardInput& keyboardInput);
	};
}
