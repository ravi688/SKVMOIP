#include <SKVMOIP/GUI/MachineUI.hpp>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	namespace GUI
	{
		MachineUI::MachineUI(u32 id, const char* name) :  m_id(id),
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
		                                                        m_topLevelBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)),
		                                                        m_topLevelButton(gtk_toggle_button_new()),
		                                                        m_toggleCallbackHandlerInfo { id },
																m_videoCallbackHandlerInfo { id },
																m_powerPressCallbackHandlerInfo { id },
																m_powerReleaseCallbackHandlerInfo { id },
																m_resetPressCallbackHandlerInfo { id },
																m_resetReleaseCallbackHandlerInfo { id }
		
		{
		
		  gtk_widget_set_margin_end(m_buttonBox, 20);
		  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_videoButton, FALSE, FALSE, 0);
		  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_powerButton, FALSE, FALSE, 0);
		  gtk_box_pack_start(GTK_BOX(m_buttonBox), m_resetButton, FALSE, FALSE, 0);
		
		  gtk_box_pack_start(GTK_BOX(m_labelBox), m_statusLabel, FALSE, FALSE, 0);
		  gtk_box_pack_start(GTK_BOX(m_labelBox), m_outputAddressLabel, FALSE, FALSE, 0);
		  gtk_box_pack_start(GTK_BOX(m_labelBox), m_inputAddressLabel, FALSE, FALSE, 0);
		
		  gtk_container_add(GTK_CONTAINER(m_topLevelButton), m_labelBox);
		  gtk_box_pack_start(GTK_BOX(m_DashboardBox), m_topLevelButton, TRUE, TRUE, 0);
		  gtk_box_pack_start(GTK_BOX(m_DashboardBox), m_buttonBox, FALSE, FALSE, 0);
		
		  gtk_box_pack_start(GTK_BOX(m_topLevelBox), m_nameLabel, FALSE, FALSE, 0);
		  gtk_box_pack_start(GTK_BOX(m_topLevelBox), m_DashboardBox, TRUE, TRUE, 0);

		  gtk_widget_show_all(m_topLevelBox);
		}
		
		MachineUI::~MachineUI()
		{
		
		}
		
		void MachineUI::setOutputAddress(const char* ipAddress, const char* portNumber, const char* usbPortNumber)
		{
		  const char* desc = "Video Output: ";
		  auto len0 = strlen(desc);
		  auto len1 = strlen(ipAddress);
		  auto len2 = strlen(portNumber);
		  auto len3 = strlen(usbPortNumber);
		  char buffer[len0 + len1 + len2 + len3 + 3] = { };
		  memcpy(buffer, desc, len0);
		  memcpy(buffer + len0, ipAddress, len1);
		  buffer[len0 + len1] = ':';
		  memcpy(buffer + len0 + len1 + 1, portNumber, len2);
		  if(len3 > 0)
		  {
		  	buffer[len0 + len1 + len2 + 1] = ':';
		  	memcpy(buffer + len0 + len1 + len2 + 2, usbPortNumber, len3);
		  }
		  buffer[len0 + len1 + len2 + len3 + 2] = 0;
		
		  gtk_label_set_text(GTK_LABEL(m_outputAddressLabel), buffer);
		}
		
		void MachineUI::setInputAddress(const char* ipAddress, const char* portNumber)
		{
		  const char* desc = "KM Input: ";
		  auto len0 = strlen(desc);
		  auto len1 = strlen(ipAddress);
		  auto len2 = strlen(portNumber);
		  char buffer[len0 + len1 + len2 + 2];
		  memcpy(buffer, desc, len0);
		  memcpy(buffer + len0, ipAddress, len1);
		  buffer[len0 + len1] = ':';
		  memcpy(buffer + len0 + len1 + 1, portNumber, len2);
		  buffer[len0 + len1 + len2 + 1] = 0;
		
		  gtk_label_set_text(GTK_LABEL(m_inputAddressLabel), buffer);
		}

		static void ButtonToggledHandler(GtkToggleButton* toggleButton, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			if(gtk_toggle_button_get_active(toggleButton))
			{
				skvmoip_debug_assert(machine->m_toggleCallbackHandlerInfo.selectHandler.first != NULL);
				machine->m_toggleCallbackHandlerInfo.selectHandler.first(machine->m_toggleCallbackHandlerInfo.id, machine->m_toggleCallbackHandlerInfo.selectHandler.second);
			}
			else
			{
				skvmoip_debug_assert(machine->m_toggleCallbackHandlerInfo.deselectHandler.first != NULL);
				machine->m_toggleCallbackHandlerInfo.deselectHandler.first(machine->m_toggleCallbackHandlerInfo.id, machine->m_toggleCallbackHandlerInfo.deselectHandler.second);
			}
		}

		static void VideoButtonClickHandler(GtkWidget* widget, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			skvmoip_debug_assert(machine->m_videoCallbackHandlerInfo.handler.first != NULL);
			machine->m_videoCallbackHandlerInfo.handler.first(machine->m_videoCallbackHandlerInfo.id, machine->m_videoCallbackHandlerInfo.handler.second);
		}

		static void PowerButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			skvmoip_debug_assert(machine->m_powerPressCallbackHandlerInfo.handler.first != NULL);
			machine->m_powerPressCallbackHandlerInfo.handler.first(machine->m_powerPressCallbackHandlerInfo.id, machine->m_powerPressCallbackHandlerInfo.handler.second);
		}

		static void PowerButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			skvmoip_debug_assert(machine->m_powerReleaseCallbackHandlerInfo.handler.first != NULL);
			machine->m_powerReleaseCallbackHandlerInfo.handler.first(machine->m_powerReleaseCallbackHandlerInfo.id, machine->m_powerReleaseCallbackHandlerInfo.handler.second);
		}

		static void ResetButtonPressHandler(GtkWidget* widget, GdkEventButton event, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			skvmoip_debug_assert(machine->m_resetPressCallbackHandlerInfo.handler.first != NULL);
			machine->m_resetPressCallbackHandlerInfo.handler.first(machine->m_resetPressCallbackHandlerInfo.id, machine->m_resetPressCallbackHandlerInfo.handler.second);
		}

		static void ResetButtonReleaseHandler(GtkWidget* widget, GdkEventButton event, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			skvmoip_debug_assert(machine->m_resetReleaseCallbackHandlerInfo.handler.first != NULL);
			machine->m_resetReleaseCallbackHandlerInfo.handler.first(machine->m_resetReleaseCallbackHandlerInfo.id, machine->m_resetReleaseCallbackHandlerInfo.handler.second);
		}

		void MachineUI::setSelectDeselectCallback(Callback selectCallback, Callback deslectCallback, void* userData)
		{ 
			m_toggleCallbackHandlerInfo.selectHandler.first = selectCallback; m_toggleCallbackHandlerInfo.selectHandler.second = userData; 
			m_toggleCallbackHandlerInfo.deselectHandler.first = deslectCallback; m_toggleCallbackHandlerInfo.deselectHandler.second = userData; 

			g_signal_connect(G_OBJECT(m_topLevelButton), "toggled", G_CALLBACK(ButtonToggledHandler), reinterpret_cast<void*>(this));
		}

		void MachineUI::setVideoButtonCallback(Callback callback, void* userData)
		{ 
			m_videoCallbackHandlerInfo.handler.first = callback; m_videoCallbackHandlerInfo.handler.second = userData; 
			g_signal_connect(G_OBJECT(m_videoButton), "clicked", G_CALLBACK(VideoButtonClickHandler), reinterpret_cast<void*>(this));
		}
		void MachineUI::setPowerButtonPressCallback(Callback callback, void* userData)
		{ 
			m_powerPressCallbackHandlerInfo.handler.first = callback; m_powerPressCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_powerButton), "button-press-event", G_CALLBACK(PowerButtonPressHandler), reinterpret_cast<void*>(this));
		}
		void MachineUI::setPowerButtonReleaseCallback(Callback callback, void* userData)
		{ 
			m_powerReleaseCallbackHandlerInfo.handler.first = callback; m_powerReleaseCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_powerButton), "button-release-event", G_CALLBACK(PowerButtonReleaseHandler), reinterpret_cast<void*>(this));
		}
		void MachineUI::setResetButtonPressCallback(Callback callback, void* userData)
		{ 
			m_resetPressCallbackHandlerInfo.handler.first = callback; m_resetPressCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_resetButton), "button-press-event", G_CALLBACK(ResetButtonPressHandler), reinterpret_cast<void*>(this));
		}
		void MachineUI::setResetButtonReleaseCallback(Callback callback, void* userData)
		{ 
			m_resetReleaseCallbackHandlerInfo.handler.first = callback; m_resetReleaseCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_resetButton), "button-release-event", G_CALLBACK(ResetButtonReleaseHandler), reinterpret_cast<void*>(this));
		}
	}
}