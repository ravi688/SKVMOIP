#pragma once

#include <SKVMOIP/defines.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <memory> // std::unique_ptr

#include <mfapi.h>
#include <mfidl.h>

namespace Win32
{
	class Win32SourceDevice
	{
	private:
		IMFMediaSource* m_source;
		u32 m_id;

	public:
		Win32SourceDevice(IMFMediaSource* source, u32 id);
		Win32SourceDevice(Win32SourceDevice&&);
		Win32SourceDevice& operator=(Win32SourceDevice&&);
		Win32SourceDevice(Win32SourceDevice&) = delete;
		Win32SourceDevice& operator=(Win32SourceDevice&) = delete;
		~Win32SourceDevice();


		void shutdown();
		u32 getID() const noexcept { return m_id; }
		IMFMediaSource* getInternalHandle() const noexcept { return m_source; }
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
		u32 getDeviceCount() const noexcept { return m_count; }
		IMFActivateList getActivateList() noexcept { return m_activateList; }
		std::optional<std::string> getSymbolicLink(u32 index);
		std::unordered_map<std::string, u32> getSymolicLinkToDeviceIDMap();
	};

	class Win32SourceDeviceListGuard : public Win32SourceDeviceList
	{
		friend std::optional<std::unique_ptr<Win32SourceDeviceListGuard>> Win32GetSourceDeviceList(const GUID& deviceGUID);
	private:
		Win32SourceDeviceListGuard(IMFActivateList& activateList, UINT32 count);

	public:
		Win32SourceDeviceListGuard(Win32SourceDeviceListGuard&& deviceList);
		Win32SourceDeviceListGuard& operator=(Win32SourceDeviceListGuard& deviceList) = delete;
		~Win32SourceDeviceListGuard();
	};

	SKVMOIP_API std::optional<std::unique_ptr<Win32SourceDeviceListGuard>> Win32GetSourceDeviceList(const GUID& deviceGUID);
	SKVMOIP_API void Win32DumpSourceDevices(Win32SourceDeviceList& list);
}