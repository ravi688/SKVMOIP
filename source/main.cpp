
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <iostream>

#ifdef PLATFORM_WINDOWS

static LRESULT WindowsKeyboardCallback(int code, WPARAM wParam, LPARAM lParam)
{
	std::cout << "WindowsKeyboardCallback" << std::endl;
	return CallNextHookEx(NULL, code, wParam, lParam);
}

static LRESULT WindowsMouseCallback(int code, WPARAM wParam, LPARAM lParam)
{
	std::cout << "WindowsMouseCallback" << std::endl;
	return CallNextHookEx(NULL, code, wParam, lParam);
}

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	SKVMOIP::Window window(500, 500, "Scalable KVM Over IP");

	SKVMOIP::Window::HookHandle keyboardHook = window.installHook(SKVMOIP::Window::HookType::Keyboard, WindowsKeyboardCallback);
	SKVMOIP::Window::HookHandle mouseHook = window.installHook(SKVMOIP::Window::HookType::Mouse, WindowsMouseCallback);

	while(!window.shouldClose())
	{
		window.pollEvents();
	}

	window.uninstallHook(keyboardHook);
	window.uninstallHook(mouseHook);

	return 0;
}
#endif /* Windows */

#ifdef PLATFORM_LINUX
int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Linux");
	return 0;
}
#endif /* Linux */
