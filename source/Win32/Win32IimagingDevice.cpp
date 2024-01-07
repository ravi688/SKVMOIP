#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <SKVMOIP/debug.h>
#include <mfidl.h>
#include <mferror.h>

namespace Win32
{
	CONSTRUCTOR_FUNCTION void InitializeMediaFundationAndCOM()
	{
		/* COM Runtime */
		HRESULT result = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (SUCCEEDED(result))
		{
			/* Media Foundation */
			result = MFStartup(MF_VERSION);
			if(FAILED(result))
				debug_log_fetal_error("Failed to initialize Media Foundation");
		}
		else debug_log_fetal_error("Failed to initialize COM Runtime");
	}

	DESTRUCTOR_FUNCTION void DeinitializeMediaFoundationAndCOM()
	{
		MFShutdown();
		CoUninitialize();
	}

	Win32SourceDevice::Win32SourceDevice(IMFMediaSource* source) : m_source(source)
	{

	}

	Win32SourceDeviceList::Win32SourceDeviceList(IMFActivateList& activateList, UINT32 count) : m_activateList(activateList), m_count(count) { }

	void Win32SourceDeviceList::destroy()
	{
		if(m_activateList != NULL)
		{
			for(DWORD i = 0; i < m_count; i++)
				m_activateList[i]->Release();
			CoTaskMemFree(m_activateList);
			m_activateList = NULL;
			m_count = 0;
		}
	}

	std::optional<Win32SourceDevice> Win32SourceDeviceList::activateDevice(u32 index)
    {
    	if((m_activateList == NULL) || (m_count == 0))
    		return { };

   		IMFMediaSource *pSource;
       	HRESULT result = m_activateList[index]->ActivateObject(IID_PPV_ARGS(&pSource));
       	if(result != S_OK)
       		return { };

    	return { Win32SourceDevice(pSource) };
    }

	
	Win32SourceDeviceListGuard::Win32SourceDeviceListGuard(IMFActivateList& activateList, UINT32 count) : Win32SourceDeviceList(activateList, count) { }

	Win32SourceDeviceListGuard::Win32SourceDeviceListGuard(Win32SourceDeviceListGuard&& deviceList) : Win32SourceDeviceList(deviceList.m_activateList, deviceList.m_count)
	{
		deviceList.m_activateList = NULL;
		deviceList.m_count = 0;
	}

	Win32SourceDeviceListGuard::~Win32SourceDeviceListGuard() { destroy(); }

	SKVMOIP_API std::optional<Win32SourceDeviceListGuard> Win32GetSourceDeviceList(const GUID& deviceGUID)
	{
		IMFAttributes *pConfig = NULL;

		// Create an attribute store to hold the search criteria.
    	HRESULT result = MFCreateAttributes(&pConfig, 1);

		// Request video capture devices.
		if (SUCCEEDED(result))
			result = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, deviceGUID);

    	// Enumerate the devices,
    	if (SUCCEEDED(result))
    	{
    		UINT32 count = 0;
    		IMFActivate **ppDevices = NULL;
    		result = MFEnumDeviceSources(pConfig, &ppDevices, &count);
    		return std::optional<Win32SourceDeviceListGuard> { Win32SourceDeviceListGuard(ppDevices, count) };
    	}

    	return std::optional<Win32SourceDeviceListGuard> { };
    }
}
