#pragma once

#include <SKVMOIP/defines.h>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif /* Windows include */

namespace Win32
{
	struct WindowPaintInfo
	{
		HDC deviceContext;
		RECT paintRect;
	};

	SKVMOIP_API HWND Win32CreateWindow(u32 width, u32 height, const char* name, WNDPROC callback);
	SKVMOIP_API void Win32DestroyWindow(HWND handle);
}
