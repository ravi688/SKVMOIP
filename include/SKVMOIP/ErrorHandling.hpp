#pragma once

#include <SKVMOIP/defines.h>

#ifdef PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif /* Windows */


#ifdef PLATFORM_WINDOWS
#   define Internal_ErrorExit(str) __Internal_ErrorExit(TEXT(const_cast<char*>(str)), __LINE__, __FUNCTION__, __FILE__)
SKVMOIP_API void __Internal_ErrorExit(LPTSTR lpszFunction, u32 lineNumber, const char* functionName, const char* fileName);
#endif /* PLATFORM_WINDOWS */

#ifdef PLATFORM_LINUX
#   define Internal_ErrorExit(str) __Internal_ErrorExit(str, __LINE__, __FUNCTION__, __FILE__)
SKVMOIP_API void __Internal_ErrorExit(const char* erroFunctionName, u32 lineNumber, const char* functionName, const char* fileName);
#endif /* Linux */
