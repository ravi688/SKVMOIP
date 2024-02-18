#include <SKVMOIP/Win32/Win32Window.hpp>
#include <SKVMOIP/ErrorHandling.hpp>

namespace Win32
{
	SKVMOIP_API HWND Win32CreateWindow(u32 width, u32 height, const char* name, WNDPROC callback)
	{
		/* HMODULE and HINSTANCE are the same thing. Got to know from the internet */
		HINSTANCE hInstance = GetModuleHandle(NULL);

		/* Populate Window Class structure */
		WNDCLASS wc = { };
		wc.lpfnWndProc = (callback == NULL) ? DefWindowProc : callback;;
		wc.hInstance = hInstance;
		wc.lpszClassName = "MainWClass";

		RegisterClass(&wc);

		/* Now create the window */
		HWND hHandle = CreateWindowEx(0, 
										(LPSTR)"MainWClass",
										(LPSTR)name, 
										WS_OVERLAPPEDWINDOW, 
										CW_USEDEFAULT, 
										CW_USEDEFAULT, 
										CW_USEDEFAULT, 
										CW_USEDEFAULT, 
										NULL,
										NULL,
										hInstance, 
										NULL);
		if(hHandle == NULL)
			Internal_ErrorExit("CreateWindowEx");

		return hHandle;
	}

	SKVMOIP_API void Win32ShowWindow(HWND handle)
	{
		ShowWindow(handle, SW_SHOWNORMAL);
	}

	SKVMOIP_API void Win32DestroyWindow(HWND handle)
	{
		DestroyWindow(handle);
	}

	SKVMOIP_API void Win32UpdateWindow(HWND handle)
	{
		if(UpdateWindow(handle) == 0)
			Internal_ErrorExit("UpdateWindow");
	}

	SKVMOIP_API void Win32InvalidateRect(HWND handle, const RECT* rect, bool isEraseBackground)
	{
		if(InvalidateRect(handle, rect, isEraseBackground) == 0)
			Internal_ErrorExit("InvalidateWindow");
	}
}