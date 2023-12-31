
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <iostream>

#ifdef PLATFORM_WINDOWS

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	SKVMOIP::Window window(500, 500, "Scalable KVM Over IP");

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });

	while(!window.shouldClose())
	{
		window.pollEvents();
	}

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
