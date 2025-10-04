#include <SKVMOIP/MachineData.hpp>
#include <SKVMOIP/assert.h>
#include <common/debug.hpp>
#include <cstring>

static u32 constexpr IPV4_MAX_STRLEN = strlen("255.255.255.255:65535");
static u32 constexpr IPV4_MIN_STRLEN = strlen("0.0.0.0:0");

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
    skvmoip_debug_assert(len < 255);
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
    m_videoIPAddress = data.m_videoIPAddress;
    m_keyMoIPAddress = data.m_keyMoIPAddress;
    m_keyMoPortNumber = data.m_keyMoPortNumber;
    m_videoPortNumber = data.m_videoPortNumber;
    m_videoUSBPortNumber = data.m_videoUSBPortNumber;
    m_nameLength = data.m_nameLength;
    strcpy(reinterpret_cast<char*>(m_name), reinterpret_cast<const char*>(data.m_name));
    strcpy(m_videoIPAddressStr, data.m_videoIPAddressStr);
    strcpy(m_keyMoIPAddressStr, data.m_keyMoIPAddressStr);
    strcpy(m_keyMoPortNumberStr, data.m_keyMoPortNumberStr);
    strcpy(m_videoPortNumberStr, data.m_videoPortNumberStr);
    strcpy(m_videoUSBPortNumberStr, data.m_videoUSBPortNumberStr);
    return *this;
  }

  void MachineData::serialize(std::ostream& stream) const
  {
    stream << "{\n";
    stream << "\tUINT32 m_videoIPAddress: " << m_videoIPAddress << "\n";
    stream << "\tUINT32 m_keyMoIPAddress: " << m_keyMoIPAddress << "\n";
    stream << "\tUINT16 m_keyMoPortNumber: " << m_keyMoPortNumber << "\n";
    stream << "\tUINT16 m_videoPortNumber: " << m_videoPortNumber << "\n";
    stream << "\tUINT8 m_videoUSBPortNumber: " << static_cast<u16>(m_videoUSBPortNumber) << "\n";
    stream << "\tUINT8[] m_name: " << m_name << "\n";
    stream << "\tCHAR[] m_videoIPAddressStr: " << m_videoIPAddressStr << "\n";
    stream << "\tCHAR[] m_keyMoIPAddressStr: " << m_keyMoIPAddressStr << "\n";
    stream << "\tCHAR[] m_keyMoPortNumberStr: " << m_keyMoPortNumberStr << "\n";
    stream << "\tCHAR[] m_videoPortNumberStr: " << m_videoPortNumberStr << "\n";
    stream << "\tCHAR[] m_videoUSBPortNumberStr: " << m_videoUSBPortNumberStr << "\n";
    stream << "}\n";
    stream.flush();
  }

  static bool strnequal(const char* buffer, const char* const literal, u32* const outLen)
  {
    *outLen = strlen(literal);
    return strncmp(buffer, literal, *outLen) == 0;
  }
  
  bool MachineData::deserialize(std::istream& stream)
  {
    char buffer[512];
    u32 len;

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "{", &len))
      return false;

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT32 m_videoIPAddress: ", &len))
      return false;
    m_videoIPAddress = atoi(buffer + len);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT32 m_keyMoIPAddress: ", &len))
      return false;
    m_keyMoIPAddress = atoi(buffer + len);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT16 m_keyMoPortNumber: ", &len))
      return false;
    m_keyMoPortNumber = atoi(buffer + len);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT16 m_videoPortNumber: ", &len))
      return false;
    m_videoPortNumber = atoi(buffer + len);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT8 m_videoUSBPortNumber: ", &len))
      return false;
    m_videoUSBPortNumber = atoi(buffer + len);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tUINT8[] m_name: ", &len))
      return false;
    strncpy(reinterpret_cast<char*>(m_name), buffer + len, NAME_MAX_LEN);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tCHAR[] m_videoIPAddressStr: ", &len))
      return false;
    strncpy(m_videoIPAddressStr, buffer + len, IPV4_STR_LEN);
    
    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tCHAR[] m_keyMoIPAddressStr: ", &len))
      return false;
    strncpy(m_keyMoIPAddressStr, buffer + len, IPV4_STR_LEN);
    
    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tCHAR[] m_keyMoPortNumberStr: ", &len))
      return false;
    strncpy(m_keyMoPortNumberStr, buffer + len, PORT_STR_LEN);
    
    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tCHAR[] m_videoPortNumberStr: ", &len))
      return false;
    strncpy(m_videoPortNumberStr, buffer + len, PORT_STR_LEN);
    
    stream.getline(buffer, 512);
    if(!strnequal(buffer, "\tCHAR[] m_videoUSBPortNumberStr: ", &len))
      return false;
    strncpy(m_videoUSBPortNumberStr, buffer + len, USB_PORT_STR_LEN);

    stream.getline(buffer, 512);
    if(!strnequal(buffer, "}", &len))
      return false;

    return true;
  }

  static std::optional<std::pair<std::pair<u32, u16>, std::optional<u8>>> ParseIPAndPort(const char* str)
  {
    const char* ptr = str;
    u8 ip[4];
    for(u32 i = 0; i < 4; i++)
    {
      ip[i] = atoi(ptr);
      ptr = strchr(ptr, '.');
      if(ptr == NULL)
      {
        if(i == 3)
          break;
        else
          /* invalid IPv4 address */
          return { };
      }
      else ++ptr;
    }
      
    ptr = strchr(str, ':');
    if(ptr == NULL)
      /* invalid port number */
      return { };
    u16 port = atoi(++ptr);

    ptr = strchr(ptr, ':');
    if(ptr != NULL)
    {
      u8 usbPort = atoi(++ptr);
      return { { { BIT32_PACK8(ip[0], ip[1], ip[2], ip[3]), port }, { usbPort } } };
    }
    else { };

    return { { { BIT32_PACK8(ip[0], ip[1], ip[2], ip[3]), port }, { } } };
  }

  std::optional<MachineData> MachineData::CreateFromStr(const char* voipAddrStr, const char* kmoipAddrStr, const char* name)
  {
    u32 voipLen = strlen(voipAddrStr);
    u32 kmoipLen = strlen(kmoipAddrStr);
    if((voipLen >= IPV4_MIN_STRLEN) && (voipLen <= IPV4_MAX_STRLEN) || (kmoipLen >= IPV4_MIN_STRLEN) && (kmoipLen <= IPV4_MAX_STRLEN))
    {
      std::optional<std::pair<std::pair<u32, u16>, std::optional<u8>>> voipAddr = ParseIPAndPort(voipAddrStr);
      if(!voipAddr.has_value())
      {
        /* parse error, either invalid IPV4 address or the port number */
        debug_log_info("Parse error: Invalid voip IPV4 address or Port Number");
        return { };
      }
      std::optional<std::pair<std::pair<u32, u16>, std::optional<u8>>> kmoipAddr = ParseIPAndPort(kmoipAddrStr);
      if(!kmoipAddr.has_value())
      {
        /* parse error, either invalid IPV4 address or the port number */
        debug_log_info("Parse error: Invalid kmoip IPV4 address or Port Number");
        return { };
      }
      MachineData data(voipAddr->first.first, kmoipAddr->first.first, voipAddr->first.second, kmoipAddr->first.second, name, voipAddr->second.value());
      return  { data };
    }
    return { };
  }

}
