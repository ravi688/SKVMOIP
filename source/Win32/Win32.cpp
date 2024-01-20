#include <SKVMOIP/Win32/Win32.hpp>
#include <SKVMOIP/debug.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>

namespace Win32
{
	SKVMOIP_API void InitializeMediaFundationAndCOM()
	{
		/* COM Runtime */
		HRESULT result = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (SUCCEEDED(result))
		{
			debug_log_info("COM initialized success");
			/* Media Foundation */
			result = MFStartup(MF_VERSION);
			if(FAILED(result))
				debug_log_fetal_error("Failed to initialize Media Foundation");
			debug_log_info("Media Foundation initialize success");
		}
		else debug_log_fetal_error("Failed to initialize COM Runtime");
	}

	SKVMOIP_API void DeinitializeMediaFoundationAndCOM()
	{
		MFShutdown();
		debug_log_info("Media Foundation Shutdown success");
		CoUninitialize();
		debug_log_info("COM uninitialized sucess");
	}


	SKVMOIP_API HRGN GetRectRegion(s32 x, s32 y, u32 width, u32 height)
	{
		auto result = CreateRectRgn(x, y, static_cast<int>(width), static_cast<int>(height));
		if(result == NULL)
			debug_log_fetal_error("Failed to create rect region { %d, %d, %lu, %lu }", x, y, width, height);
		return result;
	}
}