#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/debug.h>

namespace Win32
{
	SKVMOIP_API HRGN GetRectRegion(s32 x, s32 y, u32 width, u32 height)
	{
		auto result = CreateRectRgn(x, y, static_cast<int>(width), static_cast<int>(height));
		if(result == NULL)
			debug_log_fetal_error("Failed to create rect region { %d, %d, %lu, %lu }", x, y, width, height);
		return result;
	}
}