#pragma once

#include <SKVMOIP/defines.hpp>

#include <optional>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>


namespace Win32
{
	class Win32SourceDevice
	{
	private:
		IMFMediaSource* m_source;
	
	public:
		Win32SourceDevice(IMFMediaSource* source);
		IMFMediaSource* getInternalHandle() const { return m_source; }
	};

	typedef IMFActivate** IMFActivateList;

	class Win32SourceDeviceList
	{
	protected:
		IMFActivateList m_activateList;
		UINT32 m_count;

	protected:
		Win32SourceDeviceList(IMFActivateList& activateList, UINT32 count);
	
	public:
		~Win32SourceDeviceList() = default;

		Win32SourceDeviceList(Win32SourceDeviceList& list) = delete;
		Win32SourceDeviceList& operator=(Win32SourceDeviceList&) = delete;
		void destroy();
		std::optional<Win32SourceDevice> activateDevice(u32 index);
	};

	class Win32SourceDeviceListGuard : public Win32SourceDeviceList
	{
		friend std::optional<Win32SourceDeviceListGuard> Win32GetSourceDeviceList(const GUID& deviceGUID);
	private:
		Win32SourceDeviceListGuard(IMFActivateList& activateList, UINT32 count);

	public:
		Win32SourceDeviceListGuard(Win32SourceDeviceListGuard&& deviceList);
		Win32SourceDeviceListGuard& operator=(Win32SourceDeviceListGuard& deviceList) = delete;
		~Win32SourceDeviceListGuard();
	};

	SKVMOIP_API std::optional<Win32SourceDeviceListGuard> Win32GetSourceDeviceList(const GUID& deviceGUID);
}