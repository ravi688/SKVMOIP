#ifndef BUILD_GUITEST
#error "BUILD_GUITEST is not defined but still main.guidtest.cpp is being compiled"
#endif

#include <SKVMOIP/debug.h>
#include <functional>

#define WINDOW_MIN_SIZE_X 600
#define WINDOW_MIN_SIZE_Y 900
#define WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y

// Include gtk
#include <gtk/gtk.h>

/*
  _______________________________________
 |              Machine Name             |
 | S: Connected               | Video |  |
 | O: 192.168.1.16:192        | Power |  |
 | I: 192.168.1.17:101        | Reset |  |
 |_______________________________________|
*/
class MachineDashboard
{
public:
  typedef std::function<void(u32, void*)> Callback;

private:
  GtkWidget* m_nameLabel;           /* Machine Name */
  GtkWidget* m_statusLabel;         /* Connection Status */
  GtkWidget* m_outputAddressLabel;  /* IP address and Port number of the video server - Video Output */
  GtkWidget* m_inputAddressLabel;   /* IP address and Port number of the keyboard and mouse server - KM Input */
  GtkWidget* m_videoButton;
  GtkWidget* m_powerButton;
  GtkWidget* m_resetButton;
  GtkWidget* m_buttonBox;
  GtkWidget* m_labelBox;
  GtkWidget* m_DashboardBox;
  GtkWidget* m_topLevelBox;

  std::pair<Callback, void*> m_videoCallback;
  std::pair<Callback, void*> m_powerCallback;
  std::pair<Callback, void*> m_resetCallback;

public:

  MachineDashboard() = delete;
  MachineDashboard(const char* name);
  ~MachineDashboard();

  operator GtkWidget*() { return m_topLevelBox; }

  void setName(const char* name) { gtk_label_set_text(GTK_LABEL(m_nameLabel), name); }
  void setStatus(const char* status) { gtk_label_set_text(GTK_LABEL(m_statusLabel), status); }
  void setOutputAddress(const char* ipAddress, const char* portNumber);
  void setInputAddress(const char* ipAddress, const char* portNumber);
  void setVideoButtonCallback(Callback callback, void* userData) { m_videoCallback.first = callback; m_videoCallback.second = userData;  }
  void setPowerButtonCallback(Callback callback, void* userData) { m_powerCallback.first = callback; m_powerCallback.second = userData; }
  void setResetButtonCallback(Callback callback, void* userData) { m_resetCallback.first = callback; m_resetCallback.second = userData; }
};


MachineDashboard::MachineDashboard(const char* name) : 
                                                        m_nameLabel(gtk_label_new(name)),
                                                        m_statusLabel(gtk_label_new("S: <unkown>")),
                                                        m_outputAddressLabel(gtk_label_new("O: <unknown>")),
                                                        m_inputAddressLabel(gtk_label_new("I: <unknown>")),
                                                        m_videoButton(gtk_button_new_with_label("Video")),
                                                        m_powerButton(gtk_button_new_with_label("Power")),
                                                        m_resetButton(gtk_button_new_with_label("Reset")),
                                                        m_buttonBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)),
                                                        m_labelBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)),
                                                        m_DashboardBox(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)),
                                                        m_topLevelBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0))

{
  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_videoButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_powerButton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_resetButton, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(m_labelBox), m_statusLabel, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_labelBox), m_outputAddressLabel, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_labelBox), m_inputAddressLabel, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(m_DashboardBox), m_labelBox, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(m_DashboardBox), m_buttonBox, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(m_topLevelBox), m_nameLabel, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(m_topLevelBox), m_DashboardBox, TRUE, TRUE, 0);
}

MachineDashboard::~MachineDashboard()
{

}

void MachineDashboard::setOutputAddress(const char* ipAddress, const char* portNumber)
{
  auto len1 = strlen(ipAddress);
  auto len2 = strlen(portNumber);
  char buffer[len1 + len2 + 2];
  memcpy(buffer, ipAddress, len1);
  buffer[len1] = ':';
  memcpy(buffer + len1 + 1, portNumber, len2);
  buffer[len1 + len2 + 1] = 0;

  // gtk_label_set_text(GTK_LABEL(m_outputAddressLabel), buffer);
  gtk_label_set_markup(GTK_LABEL(m_outputAddressLabel), "<span background=\"red\">Address</span>");
}

void MachineDashboard::setInputAddress(const char* ipAddress, const char* portNumber)
{
  auto len1 = strlen(ipAddress);
  auto len2 = strlen(portNumber);
  char buffer[len1 + len2 + 2];
  memcpy(buffer, ipAddress, len1);
  buffer[len1] = ':';
  memcpy(buffer + len1 + 1, portNumber, len2);
  buffer[len1 + len2 + 1] = 0;

  gtk_label_set_text(GTK_LABEL(m_nameLabel), buffer);
}


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
      GtkWidget* bannerLabel = gtk_label_new("Scalable KVM Over IP");
      GtkWidget* searchCont = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkWidget* searchInOutCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
          GtkWidget* findMachineLabel = gtk_label_new("Find Machine");
          GtkWidget* textInputField = gtk_entry_new();
          GtkWidget* connectStatusLabel = gtk_label_new("");
        GtkWidget* connectButton = gtk_button_new_with_label("Connect");
    GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* bottomCont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);


  gtk_box_pack_start(GTK_BOX(searchInOutCont), findMachineLabel, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(searchInOutCont), textInputField, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(searchInOutCont), connectStatusLabel, TRUE, TRUE, 0);

  gtk_box_set_spacing(GTK_BOX(searchCont), 20);
  gtk_box_pack_start(GTK_BOX(searchCont), searchInOutCont, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(searchCont), connectButton, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(topCont), bannerLabel, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(topCont), searchCont, TRUE, TRUE, 0);

  gtk_paned_add1(GTK_PANED(topLevelCont), topCont);
  std::vector<MachineDashboard> machines;
  machines.reserve(40);
  for(u32 i = 0; i < 40; i++)
  {
    machines.emplace_back("Dummy Machine");
    GtkWidget* button = machines[i];
    machines[i].setOutputAddress("dfd", "323");
    gtk_box_pack_start(GTK_BOX(bottomCont), button, FALSE, FALSE, 5);
  }
  gtk_container_add(GTK_CONTAINER(scrolledWindow), bottomCont);
  gtk_paned_add2(GTK_PANED(topLevelCont), scrolledWindow);
  gtk_paned_set_position(GTK_PANED(topLevelCont), 200);

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

