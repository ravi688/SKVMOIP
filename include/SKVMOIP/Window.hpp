#pragma once

#include <SKVMOIP/defines.h>

#ifdef PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>

typedef HWND Internal_WindowHandle;
typedef MSG Internal_MSG;
typedef HHOOK Internal_HookHandle;
typedef LRESULT (*Internal_HookCallback)(int code, WPARAM wParam, LPARAM lParam);

#endif /* Windows */

namespace SKVMOIP
{
	class Window
	{
	private:
		Internal_WindowHandle m_handle;
		Internal_MSG m_msg;
	public:

		typedef Internal_HookHandle HookHandle;
		typedef Internal_HookCallback HookCallback;
		enum class HookType
		{
			Keyboard,
			Mouse
		};
	
		Window(u32 width, u32 height, const char* name);
		~Window();
	
		bool shouldClose();
		void pollEvents();


		HookHandle installHook(HookType hookType, HookCallback callback);
		void uninstallHook(HookHandle hookHandle);
	};

} /* SKVMOIP */
