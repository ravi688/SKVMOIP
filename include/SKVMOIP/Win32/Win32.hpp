#pragma once

#include <SKVMOIP/Win32/Win32KeyCodes.hpp>
#include <SKVMOIP/Win32/Win32RawInput.hpp>
#include <SKVMOIP/Win32/Win32Window.hpp>


namespace Win32
{
	SKVMOIP_API void InitializeMediaFundationAndCOM();
	SKVMOIP_API void DeinitializeMediaFoundationAndCOM();
	SKVMOIP_API HRGN GetRectRegion(s32 x, s32 y, u32 width, u32 height);
}
