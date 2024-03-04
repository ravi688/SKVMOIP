#pragma once

#include <SKVMOIP/defines.hpp>

// "192.168.104.105"
#define IPV4_STR_LEN (15 + 1)
#define PORT_STR_LEN (5 + 1)
#define USB_PORT_STR_LEN (3 + 1)
#define IP_ADDRESS(ip1, ip2, ip3, ip4) BIT32_PACK8(ip1, ip2, ip3, ip4)

namespace SKVMOIP
{
    struct MachineData
    {
    private:
      u32 m_videoIPAddress;
      u32 m_keyMoIPAddress;
      u16 m_keyMoPortNumber;
      u16 m_videoPortNumber;
      u8 m_videoUSBPortNumber;
      u8 m_nameLength;
      u8 m_name[255];
    
      char m_videoIPAddressStr[IPV4_STR_LEN];
      char m_keyMoIPAddressStr[IPV4_STR_LEN];
      char m_keyMoPortNumberStr[PORT_STR_LEN];
      char m_videoPortNumberStr[PORT_STR_LEN];
      char m_videoUSBPortNumberStr[USB_PORT_STR_LEN];
    
      u32 m_id;

    public:
      MachineData() = default;
      MachineData(u32 videoIPAddress, u32 keyMoIPAddress, u16 videoPortNumber, u16 keyMoPortNumber, const char* name, u8 videoUSBPortNumber = 0) noexcept;
      ~MachineData() noexcept = default;
    
      bool operator==(const MachineData& data) const;
      MachineData& operator=(const MachineData&  data);

      u32 getVideoIPAddress() const noexcept { return m_videoIPAddress; }
      u32 getKeyMoIPAddress() const noexcept { return m_keyMoIPAddress; }
      u16 getVideoPortNumber() const noexcept { return m_videoPortNumber; }
      u16 getKeyMoPortNumber() const noexcept { return m_keyMoPortNumber; }
      u8 getVideoUSBPortNumber() const noexcept { return m_videoUSBPortNumber; }
      const char* getName() const noexcept { return reinterpret_cast<const char*>(m_name); }
    
      const char* getVideoIPAddressStr() const noexcept { return m_videoIPAddressStr; }
      const char* getKeyMoIPAddressStr() const noexcept { return m_keyMoIPAddressStr; }
      const char* getVideoPortNumberStr() const noexcept { return m_videoPortNumberStr; }
      const char* getKeyMoPortNumberStr() const noexcept { return m_keyMoPortNumberStr; }
      const char* getVideoUSBPortNumberStr() const noexcept { return m_videoUSBPortNumberStr; }

      void setID(u32 id) noexcept { m_id = id; }
      u32 getID() const noexcept { return m_id; }
    };
}
