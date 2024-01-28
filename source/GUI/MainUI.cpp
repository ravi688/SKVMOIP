#include <SKVMOIP/GUI/MainUI.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/assert.h>

#define WINDOW_MIN_SIZE_X 600
#define WINDOW_MIN_SIZE_Y 900
#define WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y

namespace SKVMOIP
{
	namespace GUI
	{

		static void OnLoginClicked(GtkWidget* button, void* userData)
		{
		  debug_log_info("Login button clicked");
		}
		
		void OnConnectClicked(GtkWidget* button, void* userData);
		
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

		MainUI::MainUI(GtkApplication* app) : m_app(app)
		{
		  // Create a new window
		  GtkWidget *window = gtk_application_window_new (app);
		  m_window = window;
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
		
		  m_bottomCont = bottomCont;
		  gtk_container_add(GTK_CONTAINER(scrolledWindow), bottomCont);
		  gtk_paned_add2(GTK_PANED(topLevelCont), scrolledWindow);
		  gtk_paned_set_position(GTK_PANED(topLevelCont), 210);
		
		  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
		  gtk_container_add(GTK_CONTAINER(window), topLevelCont);
		
		
		  gtk_widget_show_all(window);
		  gtk_window_present(GTK_WINDOW(window));
		}
		
		void MainUI::removeMachine(u32 id)
		{
		  auto it = m_machineUIs.find(id);
		  if(it == m_machineUIs.end())
		    _assert(false);
		  gtk_container_remove(GTK_CONTAINER(m_bottomCont), static_cast<GtkWidget*>(it->second));
		  m_machineUIs.erase(it);
		  m_invalidIDs.push_back(id);
		}
		
		MachineUI& MainUI::getMachine(u32 id)
		{
		  auto it = m_machineUIs.find(id);
		  if(it == m_machineUIs.end())
		    _assert(false);
		  return m_machineUIs[id];
		}

	}
}