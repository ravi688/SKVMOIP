
#pragma once

#include <common/defines.h>

#ifdef SKVMOIP_STATIC_LIBRARY
#	define SKVMOIP_API
#elif SKVMOIP_DYNAMIC_LIBRARY
#	define SKVMOIP_API __declspec(dllimport)
#elif BUILD_DYNAMIC_LIBRARY
#	define SKVMOIP_API __declspec(dllexport)
#else
#	define SKVMOIP_API
#endif
