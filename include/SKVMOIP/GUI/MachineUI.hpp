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
			static void PowerButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			static void PowerButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			static void ResetButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			static void ResetButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData);
		
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
			friend void PowerButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			friend void PowerButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			friend void ResetButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData);
			friend void ResetButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData);

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
		
		  ToggleHandlerInfo m_toggleCallbackHandlerInfo;
		  HandlerInfo m_videoCallbackHandlerInfo;
		  HandlerInfo m_powerPressCallbackHandlerInfo;
		  HandlerInfo m_powerReleaseCallbackHandlerInfo;
		  HandlerInfo m_resetPressCallbackHandlerInfo;
		  HandlerInfo m_resetReleaseCallbackHandlerInfo;
		
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
		  void setPowerButtonPressCallback(Callback callback, void* userData);
		  void setPowerButtonReleaseCallback(Callback callback, void* userData);
		  void setResetButtonPressCallback(Callback callback, void* userData);
		  void setResetButtonReleaseCallback(Callback callback, void* userData);

		  const char* getOutputAddressStr() { return gtk_label_get_text(GTK_LABEL(m_outputAddressLabel)); }
		  const char* getInputAddressStr() { return gtk_label_get_text(GTK_LABEL(m_inputAddressLabel)); }
		};
	}
}
