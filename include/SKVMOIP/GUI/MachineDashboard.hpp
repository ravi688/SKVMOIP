#pragma once

#include <SKVMOIP/defines.hpp>
#include <gtk/gtk.h>
#include <functional>

namespace SKVMOIP
{
	namespace GUI
	{
		
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
		  GtkWidget* m_topLevelButton;
		
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
	}
}
