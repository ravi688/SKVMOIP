#include <SKVMOIP/MachineData.hpp>
#include <SKVMOIP/assert.h>
#include <cstring>

namespace SKVMOIP
{
	MachineData::MachineData(u32 videoIPAddress, 
							u32 keyMoIPAddress, 
							u16 videoPortNumber, 
							u16 keyMoPortNumber, 
							const char* name, 
							u8 videoUSBPortNumber) noexcept : 
							m_videoIPAddress(videoIPAddress), 
							m_keyMoIPAddress(keyMoIPAddress), 
							m_videoPortNumber(videoPortNumber), 
							m_keyMoPortNumber(keyMoPortNumber), 
							m_videoUSBPortNumber(videoUSBPortNumber),
                            m_id(0)
  {
    u32 len = strlen(name);
    _assert(len < 255);
    memcpy(m_name, name, len);
    m_name[len] = 0;
    m_nameLength = len;

    sprintf(m_videoIPAddressStr, "%u.%u.%u.%d", BIT32_UNPACK8(videoIPAddress, 3),
                                                BIT32_UNPACK8(videoIPAddress, 2),
                                                BIT32_UNPACK8(videoIPAddress, 1),
                                                BIT32_UNPACK8(videoIPAddress, 0));
    sprintf(m_keyMoIPAddressStr, "%u.%u.%u.%u", BIT32_UNPACK8(keyMoIPAddress, 3),
                                                BIT32_UNPACK8(keyMoIPAddress, 2),
                                                BIT32_UNPACK8(keyMoIPAddress, 1),
                                                BIT32_UNPACK8(keyMoIPAddress, 0));
    sprintf(m_videoPortNumberStr, "%u", videoPortNumber);
    sprintf(m_keyMoPortNumberStr, "%u", keyMoPortNumber);
    sprintf(m_videoUSBPortNumberStr, "%u", videoUSBPortNumber);
  }
}
