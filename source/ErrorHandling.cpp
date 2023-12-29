#include <SKVMOIP/ErrorHandling.hpp>
#include <SKVMOIP/debug.h>

#ifdef PLATFORM_WINDOWS
#	include <strsafe.h>
#endif /* Windows */


#ifdef PLATFORM_WINDOWS
SKVMOIP_API void __Internal_ErrorExit(LPTSTR lpszFunction, u32 lineNumber, const char* functionName, const char* fileName) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s, at %lu, %s, %s"),
        lpszFunction, dw, lpMsgBuf, lineNumber, functionName, fileName); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}
#endif /* Windows */

#ifdef PLATFORM_LINUX
SKVMOIP_API void __Internal_ErrorExit(const char* errorFunctionName, u32 lineNumber, const char* functionName, const char* fileName)
{
    debug_log("Error: ", lineNumber, lineNumber, functioName, fileName, "error occured in the function: %s", errorFunctionName);
    exit(-1);
}
#endif /* Linux */
