#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <bufferlib/buffer.h>

static buffer_t gRawInputBuffer;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
    {
		case WM_SIZE:
		{
			int width = LOWORD(lParam);  // Macro to get the low-order word.
			int height = HIWORD(lParam); // Macro to get the high-order word.
			break;
		}

		// /* Keyboard Input Handling */
		// case WM_SYSKEYUP:
		// case WM_SYSKEYDOWN:
		// case WM_KEYUP:
		// case WM_KEYDOWN:
		// {
		// 	Win32::KeyboardMessage message = Win32::GetKeyboardMessage(wParam, lParam);
		// 	std::cout << "Keyboard Messsage: \n";
		// 	std::cout << "\tKeyCode: " << message.keyCode << "\n";
		// 	std::cout << "\tScanCode: " << message.scanCode << "\n";
		// 	std::cout << "\tRepeatCount: " << message.repeatCount << "\n";
		// 	std::cout << "\tWasKeyDown: " << message.wasKeyDown << "\n";
		// 	std::cout << "\tIsKeyReleased: " << message.isKeyReleased << "\n";
		// 	std::cout << "\tIsAltDown: " << message.isAltDown << "\n";
		// 	std::cout << "\tIsExtendedKey: " << message.isExtendedKey << std::endl;
		// 	break;
		// }

		// /* Mouse Input Handling */
		// case WM_CAPTURECHANGED:
		// {
		// 	break;
		// }

		case WM_INPUT:
		{
			UINT bufferSize;
			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER)) == ((UINT)-1))
				Internal_ErrorExit("GetRawinputData");

			if(buf_get_capacity(&gRawInputBuffer) < bufferSize)
				buf_resize(&gRawInputBuffer, bufferSize);

			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf_get_ptr(&gRawInputBuffer), &bufferSize, sizeof(RAWINPUTHEADER)) != bufferSize)
				Internal_ErrorExit("GetRawInputData");

			RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(buf_get_ptr(&gRawInputBuffer));

			switch(rawInput->header.dwType)
			{
				case RIM_TYPEMOUSE:
				{
					RAWMOUSE* rawMouse = &rawInput->data.mouse;
					Win32::MouseInput mouseInput = Win32::DecodeRawMouseInput(rawMouse);
					Win32::DumpMouseInput(&mouseInput);
					break;
				}

				case RIM_TYPEKEYBOARD:
				{
					RAWKEYBOARD* rawKeyboard = &rawInput->data.keyboard;
					Win32::KeyboardInput keyboardInput = Win32::DecodeRawKeyboardInput(rawKeyboard);
					Win32::DumpKeyboardInput(&keyboardInput);
					break;
				}

				case RIM_TYPEHID:
				{
					debug_log_info("Unknown HID Raw Input");
					break;
				}
			}
			break;
		}
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
		case HookType::KeyboardLowLevel: return WH_KEYBOARD_LL;
		case HookType::Mouse: return WH_MOUSE;
		case HookType::MouseLowLevel: return WH_MOUSE_LL;
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
		gRawInputBuffer = buf_create(sizeof(u8), sizeof(RAWINPUT), 0);
		m_handle = ::SKVM_CreateWindow(name);
	}

	Window::~Window()
	{
		::SKVM_DestroyWindow(m_handle);
		buf_free(&gRawInputBuffer);
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

	void Window::setMouseCapture()
	{
		SetCapture(m_handle);
	}

	void Window::releaseMouseCapture()
	{
		if(ReleaseCapture() == 0)
			Internal_ErrorExit("ReleaseCapture");
	}

	Window::HookHandle Window::installLocalHook(HookType hookType, HookCallback callback)
	{
		HHOOK hook;
		if((hook = SetWindowsHookExA(SKVM_getWin32HookFromHookType(hookType), callback, NULL, GetCurrentThreadId())) == NULL)
			Internal_ErrorExit("SetWindowsHookExA");
		return hook;
	}

	void Window::uninstallLocalHook(HookHandle hookHandle)
	{
		if(UnhookWindowsHookEx(static_cast<HHOOK>(hookHandle)) == 0)
			Internal_ErrorExit("UnhookWindowsHookEx");
	}
}