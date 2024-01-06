#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/defines.hpp>
#include <bufferlib/buffer.h>

static buffer_t gRawInputBuffer;
typedef std::unordered_map<SKVMOIP::Window::EventType, SKVMOIP::Event> TypedEventMap;
typedef std::unordered_map<HWND, TypedEventMap> WindowsEventRegistry;
static WindowsEventRegistry  gWindowsEventRegistry;

static TypedEventMap*getEventMap(HWND windowHandle)
{
	WindowsEventRegistry::iterator eventMapKeyValuePairIt = gWindowsEventRegistry.find(windowHandle);
	if(eventMapKeyValuePairIt != gWindowsEventRegistry.end());
		return &eventMapKeyValuePairIt->second;
	return nullptr;
}

static SKVMOIP::Event& getEvent(TypedEventMap& eventMap, SKVMOIP::Window::EventType eventType)
{
	TypedEventMap::iterator eventIt = eventMap.find(eventType);
	_assert(eventIt != eventMap.end());
	return eventIt->second;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TypedEventMap* eventMap = getEventMap(hwnd);
	if(eventMap == nullptr)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	switch (uMsg)
    {
		case WM_SIZE:
		{
			int width = LOWORD(lParam);  // Macro to get the low-order word.
			int height = HIWORD(lParam); // Macro to get the high-order word.
			break;
		}

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
					getEvent(*eventMap, SKVMOIP::Window::EventType::MouseInput).publish(reinterpret_cast<void*>(&mouseInput));
					break;
				}

				case RIM_TYPEKEYBOARD:
				{
					RAWKEYBOARD* rawKeyboard = &rawInput->data.keyboard;
					Win32::KeyboardInput keyboardInput = Win32::DecodeRawKeyboardInput(rawKeyboard);
					getEvent(*eventMap, SKVMOIP::Window::EventType::KeyboardInput).publish(reinterpret_cast<void*>(&keyboardInput));
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

		case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			if(BeginPaint(hwnd, &paintStruct) == NULL)
				Internal_ErrorExit("BeginPaint");
			Win32::WindowPaintInfo paintInfo = { paintStruct.hdc, paintStruct.rcPaint };
			getEvent(*eventMap, SKVMOIP::Window::EventType::Paint).publish(reinterpret_cast<void*>(&paintInfo));
			EndPaint(hwnd, &paintStruct);
			break;
		}
	}
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

namespace SKVMOIP
{

	Window::Window(u32 width, u32 height, const char* name)
	{
		m_handle = Win32::Win32CreateWindow(width, height, name, WindowProc);
		setSize(width, height);
		setPosition(500, 200);
		setZOrder(HWND_TOPMOST);

		gRawInputBuffer = buf_create(sizeof(u8), sizeof(RAWINPUT), 0);

		std::unordered_map<EventType, Event> eventMap;
		eventMap.reserve(EnumClassToInt(Window::EventType::MAX));

		for(typename std::underlying_type<Window::EventType>::type i = 0; i < EnumClassToInt(Window::EventType::MAX); i++)
			eventMap.insert({ IntToEnumClass<Window::EventType>(i), Event() });

		gWindowsEventRegistry.insert(std::pair<HWND, std::unordered_map<EventType, Event>> { m_handle, std::move(eventMap) });
	}

	Window::~Window()
	{
		Win32::Win32DestroyWindow(m_handle);
		gWindowsEventRegistry.erase(m_handle);
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

	void Window::setSize(u32 width, u32 height)
	{
		if(SetWindowPos(m_handle, 0, 0, 0, static_cast<int>(width), static_cast<int>(height), SWP_NOMOVE) == 0)
			Internal_ErrorExit("SetWindowPos");
	}

	void Window::setPosition(s32 x, s32 y)
	{
		if(SetWindowPos(m_handle, 0, x, y, 0, 0, SWP_NOSIZE) == 0)
			Internal_ErrorExit("SetWindowPos");
	}

	void Window::setSizeAndPosition(s32 x, s32 y, u32 width, u32 height)
	{
		if(SetWindowPos(m_handle, HWND_TOP, x, y, static_cast<int>(width), static_cast<int>(height), SWP_NOZORDER) == 0)
			Internal_ErrorExit("SetWindowPos");
	}

	void Window::setZOrder(HWND insertAfter)
	{
		if(SetWindowPos(m_handle, insertAfter, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE) == 0)
			Internal_ErrorExit("SetWindowPos");
	}

	void Window::invalidateRect(const RECT* rect, bool isEraseBackground)
	{
		Win32::Win32InvalidateRect(m_handle, rect, isEraseBackground);
	}

	void Window::update()
	{
		Win32::Win32UpdateWindow(m_handle);
	}

	void Window::redraw(RegionHandle& regionHandle, s32 x, s32 y, s32 width, s32 height)
	{
		RECT updateRect = { x, y, x + width, y + height };
		if(RedrawWindow(m_handle, &updateRect, regionHandle, 0) == 0)
			Internal_ErrorExit("RedrawWindow");
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

	Event& Window::getEvent(EventType eventType)
	{
		WindowsEventRegistry::iterator it = gWindowsEventRegistry.find(m_handle);
		if(it == gWindowsEventRegistry.end())
		{
			debug_log_fetal_error("This window is not registered in the gWindowsEventRegistry, corrupt Windoiw instance?");
			return null_reference<Event>();
		}

		TypedEventMap::iterator it2 = it->second.find(eventType);
		if(it2 == it->second.end())
		{
			debug_log_fetal_error("Unable to get event for even type: %lu, not such event exists in the registry", eventType);
			return null_reference<Event>();
		}

		return it2->second;
	}
}