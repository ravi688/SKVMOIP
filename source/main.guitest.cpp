#ifndef BUILD_GUITEST
#error "BUILD_GUITEST is not defined but still main.guidtest.cpp is being compiled"
#endif

#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/GUI/MachineUI.hpp>
#include <SKVMOIP/GUI/MainUI.hpp>

#include <vector>
#include <memory>

// "192.168.104.105"
#define IPV4_STR_LEN (15 + 1)
#define PORT_STR_LEN (5 + 1)

struct MachineData
{
private:
  u32 m_videoIPAddress;
  u32 m_keyMoIPAddress;
  u16 m_keyMoPortNumber;
  u16 m_videoPortNumber;
  u8 m_nameLength;;
  u8 m_name[255];

  char m_videoIPAddressStr[IPV4_STR_LEN];
  char m_keyMoIPAddressStr[IPV4_STR_LEN];
  char m_keyMoPortNumberStr[PORT_STR_LEN];
  char m_videoPortNumberStr[PORT_STR_LEN];

public:
  MachineData(u32 videoIPAddress, u32 keyMoIPAddress, u16 videoPortNumber, u16 keyMoPortNumber, const char* name) noexcept : 
  m_videoIPAddress(videoIPAddress), m_keyMoIPAddress(keyMoIPAddress), m_videoPortNumber(videoPortNumber), m_keyMoPortNumber(keyMoPortNumber)
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
  }
  ~MachineData() noexcept = default;

  u32 getVideoIPAddress() const noexcept { return m_videoIPAddress; }
  u32 getKeyMoIPAddress() const noexcept { return m_keyMoIPAddress; }
  u16 getVideoPortNumber() const noexcept { return m_videoPortNumber; }
  u16 getKeyMoPortNumber() const noexcept { return m_keyMoPortNumber; }
  const char* getName() const noexcept { return reinterpret_cast<const char*>(m_name); }

  const char* getVideoIPAddressStr() const noexcept { return m_videoIPAddressStr; }
  const char* getKeyMoIPAddressStr() const noexcept { return m_keyMoIPAddressStr; }
  const char* getVideoPortNumberStr() const noexcept { return m_videoPortNumberStr; }
  const char* getKeyMoPortNumberStr() const noexcept { return m_keyMoPortNumberStr; }
};

#define IP_ADDRESS(ip1, ip2, ip3, ip4) BIT32_PACK8(ip1, ip2, ip3, ip4)

static std::vector<MachineData> GetMachineDataListFromServer()
{
  std::vector<MachineData> machines = 
  {
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 18), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 19), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 20), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 21), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 22), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 23), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 24), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 25), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 26), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 27), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 28), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 29), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 30), 100, 101, "Dummy Machine" },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 31), 100, 101, "Dummy Machine" }
  };
  return machines;
}

    static void OnMachineSelect(u32 id, void* userData)
    {
      debug_log_info("%u:%s", id, __FUNCTION__);
    }
    
    static void OnMachineDeselect(u32 id, void* userData)
    {
    debug_log_info("%u:%s", id, __FUNCTION__);
    }
    
    static void OnVideoClicked(u32 id, void* userData)
    {
    debug_log_info("%u:%s", id, __FUNCTION__);
    }
    
    static void OnPowerClicked(u32 id, void* userData)
    {
    debug_log_info("%u:%s", id, __FUNCTION__);
    }
    
    static void OnResetClicked(u32 id, void* userData)
    {
    debug_log_info("%u:%s", id, __FUNCTION__);
    }

static std::vector<MachineData> gMachineDataList;

static std::unique_ptr<SKVMOIP::GUI::MainUI> gMainUI;

static void on_activate (GtkApplication *app) {

  gMainUI = std::move(std::unique_ptr<SKVMOIP::GUI::MainUI>(new SKVMOIP::GUI::MainUI(app)));

  gMachineDataList = std::move(GetMachineDataListFromServer());
  for(std::size_t i = 0; i < gMachineDataList.size(); i++)
  {
    u32 id = gMainUI->createMachine(static_cast<u32>(i), "Dummy Machine");
    auto& ui = gMainUI->getMachine(id);
    auto& data = gMachineDataList[i];
    
    ui.setName(data.getName());
    ui.setOutputAddress(data.getVideoIPAddressStr(), data.getVideoPortNumberStr());
    ui.setInputAddress(data.getKeyMoIPAddressStr(), data.getKeyMoPortNumberStr());
    ui.setStatus("Status: Unknown");

    ui.setSelectDeselectCallback(OnMachineSelect, OnMachineDeselect, NULL);
    ui.setVideoButtonCallback(OnVideoClicked, NULL);
    ui.setPowerButtonCallback(OnPowerClicked, NULL);
    ui.setResetButtonCallback(OnResetClicked, NULL);
  }
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication *app = gtk_application_new ("com.example.GtkApplication",
                                             G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  return g_application_run (G_APPLICATION (app), argc, argv);
}

