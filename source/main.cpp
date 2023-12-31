
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/Win32/Win32.hpp>
#include <iostream>

using namespace SKVMOIP;

static void MouseInputHandler(void* mouseInputData, void* userData)
{
	Win32::DumpMouseInput(reinterpret_cast<Win32::MouseInput*>(mouseInputData));
}

static void KeyboardInputHandler(void* keyboardInputData, void* userData)
{
	Win32::DumpKeyboardInput(reinterpret_cast<Win32::KeyboardInput*>(keyboardInputData));
}

int main(int argc, const char* argv[])
{
	debug_log_info("Platform is Windows");

	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });


	Window window(500, 500, "Scalable KVM Over IP");

	Event::SubscriptionHandle mouseInputHandle = window.getEvent(Window::EventType::MouseInput).subscribe(MouseInputHandler, NULL);
	Event::SubscriptionHandle keyboardInputHandle = window.getEvent(Window::EventType::KeyboardInput).subscribe(KeyboardInputHandler, NULL);

	while(!window.shouldClose())
	{
		window.pollEvents();
	}

	window.getEvent(Window::EventType::MouseInput).unsubscribe(mouseInputHandle);
	window.getEvent(Window::EventType::KeyboardInput).unsubscribe(keyboardInputHandle);

	return 0;
}

