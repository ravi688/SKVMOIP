#include <SKVMOIP/Win32/Win32DrawSurface.hpp>
#include <SKVMOIP/assert.h>

namespace Win32
{
	Win32DrawSurface::Win32DrawSurface(HWND windowHandle, u32 width, u32 height, u32 bitsPerPixel) : m_width(width), m_height(height), m_bitsPerPixel(bitsPerPixel)
	{
		u32 fileSize = width * height * (bitsPerPixel >> 3);
		m_memoryFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, fileSize, NULL);
		m_memoryView = (u8*)MapViewOfFile(m_memoryFile, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);
		memset(m_memoryView, 0xFF, fileSize);

		BITMAPINFOHEADER biheader = { sizeof(biheader), static_cast<LONG>(width), -static_cast<LONG>(height), 1, static_cast<WORD>(bitsPerPixel), BI_RGB };

		m_windowHandle = windowHandle;
		m_windowHDC = GetDC(windowHandle);
		LPVOID bits;
		m_memoryBitmap = CreateDIBSection(m_windowHDC, (BITMAPINFO*)&biheader, DIB_RGB_COLORS, &bits, m_memoryFile, 0);

		m_memoryHDC = CreateCompatibleDC(m_windowHDC);
		m_oldObject = SelectObject(m_memoryHDC, m_memoryBitmap);
	}

	Win32DrawSurface::~Win32DrawSurface()
	{
		SelectObject(m_memoryHDC, m_oldObject);
		DeleteObject(m_memoryBitmap);
		DeleteDC(m_memoryHDC);
		ReleaseDC(m_windowHandle, m_windowHDC);
		UnmapViewOfFile(m_memoryView);
		CloseHandle(m_memoryFile);
	}

	void Win32DrawSurface::setPixels(const u8* bytes, u32 size)
	{
		_assert(size == getBufferSize());
		memcpy(m_memoryView, bytes, size);
	}

	u8* Win32DrawSurface::getPixels()
	{
		return m_memoryView;
	}

	std::pair<u32, u32> Win32DrawSurface::getSize() const
	{
		return std::pair<u32, u32> { m_width, m_height };
	}

	u32 Win32DrawSurface::getBitsPerPixel() const
	{
		return m_bitsPerPixel;
	}

	u32 Win32DrawSurface::getBufferSize() const
	{
		return m_width * m_height * (m_bitsPerPixel >> 3);
	}

	HDC Win32DrawSurface::getHDC()
	{
		return m_memoryHDC;
	}
}
