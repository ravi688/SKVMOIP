#ifndef BUILD_GUITEST
#error "BUILD_GUITEST is not defined but still main.guidtest.cpp is being compiled"
#endif

#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>
#include <SKVMOIP/GUI/MachineDashboard.hpp>

#include <vector>

#define WINDOW_MIN_SIZE_X 600
#define WINDOW_MIN_SIZE_Y 900
#define WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y

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

static void OnLoginClicked(GtkWidget* button, void* userData)
{
  debug_log_info("Login button clicked");
}

static void OnConnectClicked(GtkWidget* button, void* userData)
{
  debug_log_info ("Connect button clicked");
}

static void OnAddClicked(GtkWidget* button, void* userData)
{
  debug_log_info ("Add button clicked");
}

static void OnEditClicked(GtkWidget* button, void* userData)
{
  debug_log_info ("Edit button clicked");
}

static void OnRemoveClicked(GtkWidget* button, void* userData)
{
  debug_log_info ("Remove button clicked");
}

static std::vector<MachineData> gMachineDataList;
static std::vector<SKVMOIP::GUI::MachineDashboard> gMachineUIs;

static void on_activate (GtkApplication *app) {
  // Create a new window
  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title(GTK_WINDOW(window), "SKVMOIP Client");
  gtk_widget_set_size_request(window, WINDOW_MIN_SIZE_X, WINDOW_MIN_SIZE_Y);
  gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_DEF_SIZE_X, WINDOW_DEF_SIZE_Y);
  gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_NORTH_WEST);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  /* Containers */
  GtkWidget* topLevelCont = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget* topCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
      GtkWidget* loginCont = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
      GtkWidget* loginButton = gtk_button_new_with_label("Login");
      GtkWidget* bannerLabel = gtk_label_new("Scalable KVM Over IP");
      GtkWidget* searchCont = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget* searchInOutCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
          GtkWidget* findMachineLabel = gtk_label_new("Find Machine");
          GtkWidget* textInputField = gtk_entry_new();
          GtkWidget* connectStatusLabel = gtk_label_new("");
        GtkWidget* buttonCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget* connectButton = gtk_button_new_with_label("Connect");
        GtkWidget* addButton = gtk_button_new_with_label("Add");
        GtkWidget* editButton = gtk_button_new_with_label("Edit");
        GtkWidget* removeButton = gtk_button_new_with_label("Remove");
    GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* bottomCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  g_signal_connect(G_OBJECT(loginButton), "clicked", G_CALLBACK(OnLoginClicked), NULL);
  g_signal_connect(G_OBJECT(connectButton), "clicked", G_CALLBACK(OnConnectClicked), NULL);
  g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(OnAddClicked), NULL);
  g_signal_connect(G_OBJECT(editButton), "clicked", G_CALLBACK(OnEditClicked), NULL);
  g_signal_connect(G_OBJECT(removeButton), "clicked", G_CALLBACK(OnRemoveClicked), NULL);


  gtk_box_set_spacing(GTK_BOX(searchInOutCont), 20);
  gtk_box_pack_start(GTK_BOX(searchInOutCont), findMachineLabel, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(searchInOutCont), textInputField, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(searchInOutCont), connectStatusLabel, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(buttonCont), connectButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(buttonCont), addButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(buttonCont), editButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(buttonCont), removeButton, FALSE, FALSE, 0);

  gtk_widget_set_margin_top(searchCont, 20);
  gtk_box_set_spacing(GTK_BOX(searchCont), 10);
  gtk_box_pack_start(GTK_BOX(searchCont), searchInOutCont, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(searchCont), buttonCont, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(loginCont), loginButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(loginCont), bannerLabel, TRUE, TRUE, 0);

  gtk_widget_set_margin_bottom(searchCont, 10);
  gtk_box_pack_start(GTK_BOX(topCont), loginCont, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(topCont), searchCont, FALSE, FALSE, 0);

  gtk_paned_add1(GTK_PANED(topLevelCont), topCont);

  gMachineDataList = std::move(GetMachineDataListFromServer());
  gMachineUIs.reserve(gMachineDataList.size());
  for(std::size_t i = 0; i < gMachineDataList.size(); i++)
  {
    gMachineUIs.emplace_back(static_cast<u32>(i), "Dummy Machine");
    auto& ui = gMachineUIs[i];
    GtkWidget* button = static_cast<GtkWidget*>(ui);
    gtk_box_pack_start(GTK_BOX(bottomCont), button, FALSE, FALSE, 5);

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
  gtk_container_add(GTK_CONTAINER(scrolledWindow), bottomCont);
  gtk_paned_add2(GTK_PANED(topLevelCont), scrolledWindow);
  gtk_paned_set_position(GTK_PANED(topLevelCont), 210);

  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_container_add(GTK_CONTAINER(window), topLevelCont);


  gtk_widget_show_all(window);
  gtk_window_present(GTK_WINDOW(window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication *app = gtk_application_new ("com.example.GtkApplication",
                                             G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  return g_application_run (G_APPLICATION (app), argc, argv);
}

