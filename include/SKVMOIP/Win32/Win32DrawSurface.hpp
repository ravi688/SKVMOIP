#pragma once

#include <SKVMOIP/defines.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <utility>

namespace Win32
{
	class SKVMOIP_API Win32DrawSurface
	{
	private:
		HANDLE m_memoryFile;
		HWND m_windowHandle;

		HBITMAP m_memoryBitmap;
		HDC m_memoryHDC;
		HDC m_windowHDC;
		u8* m_memoryView;
		const u32 m_width;
		const u32 m_height;
		const u32 m_bitsPerPixel;

		HGDIOBJ m_oldObject;

	public:
		Win32DrawSurface(HWND windowHandle, u32 width, u32 height, u32 bitsPerPixel);
		~Win32DrawSurface();

		void setPixels(const u8* bytes, u32 size);
		u8* getPixels();
		std::pair<u32, u32> getSize() const;
		u32 getBitsPerPixel() const;
		u32 getBufferSize() const;
		HDC getHDC();
	};
}
