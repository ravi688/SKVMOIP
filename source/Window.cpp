#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/debug.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        {
            int width = LOWORD(lParam);  // Macro to get the low-order word.
            int height = HIWORD(lParam); // Macro to get the high-order word.

            // Respond to the message:
            // OnSize(hwnd, (UINT)wParam, width, height);
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static Internal_WindowHandle SKVM_CreateWindow(const char* name)
{
#ifdef PLATFORM_WINDOWS

	/* HMODULE and HINSTANCE are the same thing. Got to know from the internet */
	HINSTANCE hInstance = GetModuleHandle(NULL);

	/* Populate Window Class structure */
	WNDCLASS wc = { };
	wc.lpfnWndProc = WindowProc;
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
#endif /* Windows */
}

static int SKVM_getWin32HookFromHookType(SKVMOIP::Window::HookType hookType)
{
	using HookType = SKVMOIP::Window::HookType;
	switch(hookType)
	{
		case HookType::Keyboard: return WH_KEYBOARD;
		case HookType::Mouse: return WH_MOUSE;
		default:
		{
			debug_log_fetal_error("Unrecognized SKVMOIP::Window::HookType: %lu", static_cast<u32>(hookType));
			return -1;
		}
	}
}

static void SKVM_DestroyWindow(Internal_WindowHandle handle)
{
#ifdef PLATFORM_WINDOWS
	DestroyWindow(handle);
#endif /* Windows */
}

namespace SKVMOIP
{

	Window::Window(u32 width, u32 height, const char* name)
	{
		m_handle = ::SKVM_CreateWindow(name);
	}

	Window::~Window()
	{
		::SKVM_DestroyWindow(m_handle);
	}

	bool Window::shouldClose()
	{
		BOOL result = GetMessage(&m_msg, m_handle, 0, 0);
		if(result == 0)
			return true;
		else if(result == -1)
		{
			/* Invoke the error handler here*/
			/* But for now let's exit */
			exit(-1);
		}
		return false;
	}

	void Window::pollEvents()
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}


	Window::HookHandle Window::installHook(HookType hookType, HookCallback callback)
	{
		HHOOK hook;
		if((hook = SetWindowsHookExA(SKVM_getWin32HookFromHookType(hookType), callback, NULL, GetCurrentThreadId())) == NULL)
			Internal_ErrorExit("SetWindowsHookExA");
		return hook;
	}

	void Window::uninstallHook(HookHandle hookHandle)
	{
		if(UnhookWindowsHookEx(static_cast<HHOOK>(hookHandle)) == 0)
		{
			Internal_ErrorExit("UnhookWindowsHookEx");
			exit(-1);
		}
	}
}