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

		ShowWindow(hHandle, SW_SHOWNORMAL);

		return hHandle;
	}

	SKVMOIP_API void Win32DestroyWindow(HWND handle)
	{
		DestroyWindow(handle);
	}
}