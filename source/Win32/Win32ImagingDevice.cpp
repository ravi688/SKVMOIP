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
		else
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
       	if(m_activateList[index]->DetachObject() != S_OK)
       		DEBUG_LOG_ERROR("Failed to detach IMFMediaSource from IMFActivate");

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

    SKVMOIP_API void Win32DumpSourceDevices(Win32SourceDeviceList& list)
    {
    	IMFActivateList activateList = list.getActivateList();
    	wchar_t* str = new wchar_t[1024];
    	for(u32 i = 0; i < list.getDeviceCount(); i++)
    	{
    		IMFActivate* activate = activateList[i];
    		UINT32 len;
    		if(activate->GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, str, 1024, &len) == S_OK)
    		{
    			/* convert the wchar string to char string */
    			char str2[len + 1] = { };
    			for(u32 j = 0; j < len; j++)
    				str2[j] = str[j];

    			DEBUG_LOG_INFO("[%lu] Friendly Name: %s", i, str2);
    		}
    		else
    			DEBUG_LOG_ERROR("[%lu] Friendly Name: Failed to determine", i);

    		if(activate->GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, str, 1024, &len) == S_OK)
    		{
    			/* convert the wchar string to char string */
    			char str2[len + 1] = { };
    			for(u32 j = 0; j < len; j++)
    				str2[j] = str[j];

    			DEBUG_LOG_INFO("[%lu] Symbolic Link: %s", i, str2);
    		}
    		else
    			DEBUG_LOG_ERROR("[%lu] Symbolic Link: Failed to determine", i);

    		UINT32 isHardware = 0;
    		if(activate->GetUINT32(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_HW_SOURCE, &isHardware) == S_OK)
    			DEBUG_LOG_INFO("[%lu] Hardware: %s", i, isHardware ? "True" : "False");
    		else
    			DEBUG_LOG_INFO("[%lu] Hardware: Failed to determine", i);
    	}
    	delete[] str;
    }
}
