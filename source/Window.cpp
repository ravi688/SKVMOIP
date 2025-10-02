#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/defines.hpp>
#include <bufferlib/buffer.h>

#include <chrono>

typedef std::unordered_map<SKVMOIP::Window::EventType, SKVMOIP::Event> TypedEventMap;
typedef std::unordered_map<HWND, TypedEventMap> WindowsEventRegistry;
static WindowsEventRegistry  gWindowsEventRegistry;
static std::unordered_map<HWND, SKVMOIP::Window*> gWindowsSelfReferenceRegistry;

static TypedEventMap* getEventMap(HWND windowHandle)
{
	WindowsEventRegistry::iterator eventMapKeyValuePairIt = gWindowsEventRegistry.find(windowHandle);
	if(eventMapKeyValuePairIt != gWindowsEventRegistry.end())
		return &eventMapKeyValuePairIt->second;
	return nullptr;
}

static SKVMOIP::Event& getEvent(TypedEventMap& eventMap, SKVMOIP::Window::EventType eventType)
{
	TypedEventMap::iterator eventIt = eventMap.find(eventType);
	skvmoip_debug_assert(eventIt != eventMap.end());
	return eventIt->second;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TypedEventMap* eventMap = getEventMap(hwnd);
	if(eventMap == nullptr)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	auto it = gWindowsSelfReferenceRegistry.find(hwnd);
	skvmoip_debug_assert(it != gWindowsSelfReferenceRegistry.end());
	SKVMOIP::Window* window = it->second;

	switch (uMsg)
    {
		case WM_SIZE:
		{
			int width = LOWORD(lParam);  // Macro to get the low-order word.
			int height = HIWORD(lParam); // Macro to get the high-order word.
			window->m_width = width;
			window->m_height = height;
			RECT rect { };
			BOOL result = GetClientRect(hwnd, &rect);
			if(result == 0)
				Internal_ErrorExit("AdjustWindowRect");
			window->m_clientWidth = rect.right;
			window->m_clientHeight = rect.bottom;
			if(window->isLocked())
			{
				RECT winRect;
				GetWindowRect(hwnd, &winRect);

				winRect.right = rect.right + winRect.left;
				winRect.bottom = rect.bottom + winRect.top;

				ClipCursor(&winRect); 
			}
			break;
		}

		case WM_INPUT:
		{
			UINT bufferSize;
			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER)) == ((UINT)-1))
				Internal_ErrorExit("GetRawinputData");

			if(buf_get_capacity(&window->m_rawInputBuffer) < bufferSize)
				buf_resize(&window->m_rawInputBuffer, bufferSize);

			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf_get_ptr(&window->m_rawInputBuffer), &bufferSize, sizeof(RAWINPUTHEADER)) != bufferSize)
				Internal_ErrorExit("GetRawInputData");

			RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(buf_get_ptr(&window->m_rawInputBuffer));

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
					std::vector<Win32::KeyboardInput>& curKeyComb = window->m_curKeyComb;
					auto& m_pressedKeys = window->m_pressedKeys;
					if(keyboardInput.keyStatus == Win32::KeyStatus::Pressed)
					{
						if(m_pressedKeys.find(keyboardInput.makeCode) != m_pressedKeys.end())
							/* skip as the key is already pressed */
							break;
						else
							m_pressedKeys.insert({keyboardInput.makeCode, Win32::KeyStatus::Pressed});

						curKeyComb.push_back(keyboardInput);

						bool isKeyComb = false;
						for(auto& keyCombEventPair : window->m_keyCombs)
						{
							std::vector<Win32::KeyCode>& keyComb = keyCombEventPair.first;
							if(keyComb.size() != curKeyComb.size())
								continue;
							else
							{
								bool isMatched = true;
								for(std::size_t i = 0; i < curKeyComb.size(); i++)
								{
									if(IntToEnumClass<Win32::KeyCode>(static_cast<u8>(curKeyComb[i].virtualKey)) != keyComb[i])
									{
										isMatched = false;
										break;
									}
								}
								if(!isMatched)
									continue;
								keyCombEventPair.second.publish(reinterpret_cast<void*>(&curKeyComb));
								isKeyComb = true;
								break;
							}
						}
						if(isKeyComb)
							break;
					}
					else
					{
						auto result = m_pressedKeys.erase(keyboardInput.makeCode);
						skvmoip_debug_assert_wrn(result == 1);

						skvmoip_debug_assert(keyboardInput.keyStatus == Win32::KeyStatus::Released);
						Win32::KeyCode virtualKey = IntToEnumClass<Win32::KeyCode>(static_cast<u8>(curKeyComb.back().virtualKey));
						if(virtualKey == keyboardInput.virtualKey)
							curKeyComb.pop_back();
						else
							curKeyComb.clear();
					}
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

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
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

	Window::Window(u32 width, u32 height, const char* name) : m_isMessageAvailable(false), m_width(width), m_height(height), m_isFullScreen(false), m_isLocked(false)
	{
		m_handle = Win32::Win32CreateWindow(width, height, name, WindowProc);
		setSize(width, height);
		setPosition(0, 0);
		setZOrder(HWND_TOP);

		m_rawInputBuffer = buf_create(sizeof(u8), sizeof(RAWINPUT), 0);

		std::unordered_map<EventType, Event> eventMap;
		eventMap.reserve(EnumClassToInt(Window::EventType::MAX));

		for(typename std::underlying_type<Window::EventType>::type i = 0; i < EnumClassToInt(Window::EventType::MAX); i++)
			eventMap.insert({ IntToEnumClass<Window::EventType>(i), Event() });

		gWindowsEventRegistry.insert(std::pair<HWND, std::unordered_map<EventType, Event>> { m_handle, std::move(eventMap) });
		gWindowsSelfReferenceRegistry.insert(std::pair<HWND, Window*> { m_handle, this });

		GetClipCursor(&m_saveClipRect);
	}

	Window::~Window()
	{
		lock(false);
		Win32::Win32DestroyWindow(m_handle);
		gWindowsEventRegistry.erase(m_handle);
		gWindowsSelfReferenceRegistry.erase(m_handle);
		buf_free(&m_rawInputBuffer);
	}

	void Window::runGameLoop()
	{
		while(!shouldClose(false))
		{
			invalidateRect();
			pollEvents();
		}
	}

	void Window::runGameLoop(u32 frameRate)
	{
		const f64 deltaTime = 1000.0 / frameRate;
		auto startTime = std::chrono::high_resolution_clock::now();
		while(!shouldClose(false))
		{
			auto time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count() >= deltaTime)
			{
				invalidateRect();	
				startTime = time;
			}
			
			pollEvents();
		}
	}

	void Window::runGameLoop(u32 frameRate, std::atomic<bool>& isLoop)
	{
		const f64 deltaTime = 1000.0 / frameRate;
		auto startTime = std::chrono::high_resolution_clock::now();
		while(isLoop && (!shouldClose(false)))
		{
			auto time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count() >= deltaTime)
			{
				invalidateRect();	
				startTime = time;
			}
			
			pollEvents();
		}
	}

	bool Window::shouldClose(bool isBlock)
	{
		if(isBlock)
		{
			BOOL result = GetMessage(&m_msg, NULL, 0, 0);
			if(result == 0)
			{
				DestroyWindow(m_handle);
				return true;
			}
			else if(result == -1)
			{
				/* Invoke the error handler here*/
				/* But for now let's exit */
				exit(-1);
			}
			m_isMessageAvailable = true;
		}
		else if(PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			m_isMessageAvailable = true;
			if(m_msg.message == WM_QUIT)
			{
				DestroyWindow(m_handle);
				return true;
			}
		}
		return false;
	}

	void Window::show(bool isFullScreen)
	{
		Win32::Win32ShowWindow(m_handle);
		if(isFullScreen)
		{
			setFullScreen(true);
			if(!isLocked())
				lock(true);
		}
	}

	void Window::setFullScreen(bool isFullScreen)
	{
		// Below code is taken from: https://src.chromium.org/viewvc/chrome/trunk/src/ui/views/win/fullscreen_handler.cc?revision=HEAD&view=markup

		// Save current window state if not already fullscreen.
		if (!m_isFullScreen)
		{
			// Save current window information.  We force the window into restored mode
			// before going fullscreen because Windows doesn't seem to hide the
			// taskbar if the window is in the maximized state.
			m_beforeFullScreenInfo.isZoomed = !!::IsZoomed(m_handle);
			if (m_beforeFullScreenInfo.isZoomed)
      			::SendMessage(m_handle, WM_SYSCOMMAND, SC_RESTORE, 0);
    		m_beforeFullScreenInfo.style = GetWindowLong(m_handle, GWL_STYLE);
    		m_beforeFullScreenInfo.exStyle = GetWindowLong(m_handle, GWL_EXSTYLE);
    		GetWindowRect(m_handle, &m_beforeFullScreenInfo.windowRect);
  		}

  		m_isFullScreen = isFullScreen;

  		if (m_isFullScreen)
  		{
    		// Set new window style and size.
    		SetWindowLong(m_handle, GWL_STYLE, m_beforeFullScreenInfo.style & ~(WS_CAPTION | WS_THICKFRAME));
    		SetWindowLong(m_handle, GWL_EXSTYLE, m_beforeFullScreenInfo.exStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

			// On expand, if we're given a window_rect, grow to it, otherwise do
			// not resize.
			// if (!for_metro)
			{
				MONITORINFO monitor_info;
				monitor_info.cbSize = sizeof(monitor_info);
				GetMonitorInfo(MonitorFromWindow(m_handle, MONITOR_DEFAULTTONEAREST), &monitor_info);
				RECT rect = monitor_info.rcMonitor;
				SetWindowPos(m_handle, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    		}
		}
		else
		{
			// Reset original window style and size.  The multiple window size/moves
			// here are ugly, but if SetWindowPos() doesn't redraw, the taskbar won't be
			// repainted.  Better-looking methods welcome.
			SetWindowLong(m_handle, GWL_STYLE, m_beforeFullScreenInfo.style);
			SetWindowLong(m_handle, GWL_EXSTYLE, m_beforeFullScreenInfo.exStyle);

			// if (!for_metro)
			{
				// On restore, resize to the previous saved rect size.
				RECT rect = m_beforeFullScreenInfo.windowRect;
				SetWindowPos(m_handle, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			}

			if (m_beforeFullScreenInfo.isZoomed)
				::SendMessage(m_handle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
	}

	void Window::showCursor(bool isShow)
	{
		m_isLocked = !isShow;
		if(isShow)
		{
			ClipCursor(&m_saveClipRect);
			ShowCursor(TRUE);
		}
		else
		{
			ShowCursor(FALSE);
			GetClientRect(m_handle, &m_newClipRect);
			RECT winRect;
			GetWindowRect(m_handle, &winRect);

			winRect.right = m_newClipRect.right + winRect.left;
			winRect.bottom = m_newClipRect.bottom + winRect.top;

			ClipCursor(&winRect);
		}
	}

	void Window::pollEvents()
	{
		if(!m_isMessageAvailable)
			return;
		m_isMessageAvailable = false;
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
		else
		{
			m_width = width;
			m_height = height;
			RECT rect { };
			BOOL result = GetClientRect(m_handle, &rect);
			if(result == 0)
				Internal_ErrorExit("AdjustWindowRect");
			m_clientWidth = rect.right;
			m_clientHeight = rect.bottom;
		}
	}

	std::pair<u32, u32> Window::getSize() const
	{
		return { m_width, m_height };
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

	std::pair<u32, u32> Window::getClientSize() const
	{
		return { m_clientWidth, m_clientHeight };
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

	Event& Window::createKeyCombinationEvent(const KeyComb& keyComb)
	{
		std::size_t index = m_keyCombs.size();
		m_keyCombs.push_back({ keyComb, Event() });
		return m_keyCombs[index].second;
	}
}