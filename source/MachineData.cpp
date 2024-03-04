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

  bool MachineData::operator==(const MachineData& data) const
  {
    bool b1 = m_videoIPAddress == data.m_videoIPAddress;
    bool b2 = m_keyMoIPAddress == data.m_keyMoIPAddress;
    bool b3 = m_keyMoPortNumber == data.m_keyMoPortNumber;
    bool b4 = m_videoPortNumber == data.m_videoPortNumber;
    bool b5 = m_videoUSBPortNumber == data.m_videoUSBPortNumber;
    bool b6 = m_nameLength == data.m_nameLength;
    bool b7 = strcmp(reinterpret_cast<const char*>(m_name), reinterpret_cast<const char*>(data.m_name)) == 0;
    bool b8 = strcmp(m_videoIPAddressStr, data.m_videoIPAddressStr) == 0;
    bool b9 = strcmp(m_keyMoIPAddressStr, data.m_keyMoIPAddressStr) == 0;
    bool b10 = strcmp(m_keyMoPortNumberStr, data.m_keyMoPortNumberStr) == 0;
    bool b11 = strcmp(m_videoPortNumberStr, data.m_videoPortNumberStr) == 0;
    bool b12 = strcmp(m_videoUSBPortNumberStr, data.m_videoUSBPortNumberStr) == 0;
    return b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10 && b11 && b12;
  }

  MachineData& MachineData::operator=(const MachineData&  data)
  {
    m_videoIPAddress == data.m_videoIPAddress;
    m_keyMoIPAddress == data.m_keyMoIPAddress;
    m_keyMoPortNumber == data.m_keyMoPortNumber;
    m_videoPortNumber == data.m_videoPortNumber;
    m_videoUSBPortNumber == data.m_videoUSBPortNumber;
    m_nameLength == data.m_nameLength;
    strcpy(reinterpret_cast<char*>(m_name), reinterpret_cast<const char*>(data.m_name));
    strcpy(m_videoIPAddressStr, data.m_videoIPAddressStr);
    strcpy(m_keyMoIPAddressStr, data.m_keyMoIPAddressStr);
    strcpy(m_keyMoPortNumberStr, data.m_keyMoPortNumberStr);
    strcpy(m_videoPortNumberStr, data.m_videoPortNumberStr);
    strcpy(m_videoUSBPortNumberStr, data.m_videoUSBPortNumberStr);
    return *this;
  }
}
