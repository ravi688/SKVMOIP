#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Event.hpp>

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

		enum class EventType
		{
			KeyboardInput = 0,
			MouseInput,
			Resize,
			Paint,

			MAX
		};

		typedef Internal_HookHandle HookHandle;
		typedef Internal_HookCallback HookCallback;
		enum class HookType
		{
			Keyboard,
			KeyboardLowLevel,
			Mouse,
			MouseLowLevel
		};

		typedef HRGN RegionHandle;
	
		Window(u32 width, u32 height, const char* name);
		~Window();
	
		bool shouldClose();
		void pollEvents();
		void setMouseCapture();
		void releaseMouseCapture();
		void setSize(u32 width, u32 height);
		void setPosition(s32 x, s32 y);
		void setSizeAndPosition(s32 x, s32 y, u32 width, u32 height);
		void setZOrder(HWND insertAfter);
		void invalidateRect(const RECT* rect = NULL, bool isEraseBackground = false);
		void update();
		void redraw(RegionHandle& regionHandle, s32 x, s32 y, s32 width, s32 height);

		HookHandle installLocalHook(HookType hookType, HookCallback callback);
		void uninstallLocalHook(HookHandle hookHandle);

		Event& getEvent(EventType evenType);
	};

} /* SKVMOIP */
