#pragma once

#include <SKVMOIP/defines.hpp>
#include <gtk/gtk.h>
#include <functional>

namespace SKVMOIP
{
	namespace GUI
	{

			static void ButtonToggledHandler(GtkToggleButton* toggleButton, void* userData);
			static void VideoButtonClickHandler(GtkWidget* widget, void* userData);
			static void PowerButtonClickHandler(GtkWidget* widget, void* userData);
			static void ResetButtonClickHandler(GtkWidget* widget, void* userData);
		
		/*
		  _______________________________________
		 |              Machine Name             |
		 | S: Connected               | Video |  |
		 | O: 192.168.1.16:192        | Power |  |
		 | I: 192.168.1.17:101        | Reset |  |
		 |_______________________________________|
		*/
		class MachineUI
		{

			friend void ButtonToggledHandler(GtkToggleButton* toggleButton, void* userData);
			friend void VideoButtonClickHandler(GtkWidget* widget, void* userData);
			friend void PowerButtonClickHandler(GtkWidget* widget, void* userData);
			friend void ResetButtonClickHandler(GtkWidget* widget, void* userData);

		public:
		  // typedef std::function<void(u32, void*)> Callback;
		  typedef void (*Callback)(u32, void*);

		  struct HandlerInfo { u32 id; std::pair<Callback, void*> handler; };
		  struct ToggleHandlerInfo
		  { 
		  	u32 id; 
		  	std::pair<Callback, void*> selectHandler;
		  	std::pair<Callback, void*> deselectHandler;
		  };
		
		private:
		  u32 m_id;
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
		
		  ToggleHandlerInfo m_toggleCallbackHandlerInfo;;
		  HandlerInfo m_videoCallbackHandlerInfo;
		  HandlerInfo m_powerCallbackHandlerInfo;
		  HandlerInfo m_resetCallbackHandlerInfo;
		
		public:
		
		  MachineUI() = default;
		  MachineUI(u32 id, const char* name = "Untitled");
		  MachineUI(MachineUI&) = default;
		  MachineUI& operator=(MachineUI&) = default;
		  MachineUI(MachineUI&&) = default;
		  MachineUI& operator=(MachineUI&&) = default;
		  ~MachineUI();
		
		  operator GtkWidget*() { return m_topLevelBox; }
		
		  u32 getID() const noexcept { return m_id; }
		  void setName(const char* name) { gtk_label_set_text(GTK_LABEL(m_nameLabel), name); }
		  void setStatus(const char* status) { gtk_label_set_text(GTK_LABEL(m_statusLabel), status); }
		  void setOutputAddress(const char* ipAddress, const char* portNumber, const char* usbPortNumber = "");
		  void setInputAddress(const char* ipAddress, const char* portNumber);
		  void setSelectDeselectCallback(Callback selectCallback, Callback deslectCallback, void* userData);
		  void setVideoButtonCallback(Callback callback, void* userData);
		  void setPowerButtonCallback(Callback callback, void* userData);
		  void setResetButtonCallback(Callback callback, void* userData);
		};
	}
}
