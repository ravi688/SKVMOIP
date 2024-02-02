#include <SKVMOIP/Win32/Win32ImagingDevice.hpp>
#include <SKVMOIP/debug.h>

namespace Win32
{
	Win32SourceDevice::Win32SourceDevice(IMFMediaSource* source, u32 id) : m_source(source), m_id(id)
	{

	}

	Win32SourceDevice::~Win32SourceDevice()
	{
	}

	Win32SourceDevice::Win32SourceDevice(Win32SourceDevice&& device) : m_source(device.m_source), m_id(device.m_id)
	{
		device.m_source = NULL;
		device.m_id = U32_MAX;
	}

	Win32SourceDevice& Win32SourceDevice::operator=(Win32SourceDevice&& device)
	{
		m_source = device.m_source;
		m_id = device.m_id;
		device.m_source = NULL;
		device.m_id = U32_MAX;
		return *this;
	}

	void Win32SourceDevice::shutdown()
	{		
		if(m_source == NULL) return;

		if(m_source->Shutdown() != S_OK)
			DEBUG_LOG_ERROR("Failed to shutdown device, ID = %lu", m_id);
		DEBUG_LOG_INFO("Win32SourceDevice shutdown success");
		m_source = NULL;
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

    	return { Win32SourceDevice(pSource, index) };
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
