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
																m_powerCallbackHandlerInfo { id },
																m_resetCallbackHandlerInfo { id }
		
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
		  char buffer[len0 + len1 + len2 + len3 + 3];
		  memcpy(buffer, desc, len0);
		  memcpy(buffer + len0, ipAddress, len1);
		  buffer[len0 + len1] = ':';
		  memcpy(buffer + len0 + len1 + 1, portNumber, len2);
		  if(len3 > 0)
		  {
		  	buffer[len0 + len1 + len2] = ':';
		  	memcpy(buffer + len0 + len1 + len2 + 1, usbPortNumber, len3);
		  }
		  buffer[len0 + len1 + len2 + len3 + 1] = 0;
		
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
				_assert(machine->m_toggleCallbackHandlerInfo.selectHandler.first != NULL);
				machine->m_toggleCallbackHandlerInfo.selectHandler.first(machine->m_toggleCallbackHandlerInfo.id, machine->m_toggleCallbackHandlerInfo.selectHandler.second);
			}
			else
			{
				_assert(machine->m_toggleCallbackHandlerInfo.deselectHandler.first != NULL);
				machine->m_toggleCallbackHandlerInfo.deselectHandler.first(machine->m_toggleCallbackHandlerInfo.id, machine->m_toggleCallbackHandlerInfo.deselectHandler.second);
			}
		}

		static void VideoButtonClickHandler(GtkWidget* widget, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			_assert(machine->m_videoCallbackHandlerInfo.handler.first != NULL);
			machine->m_videoCallbackHandlerInfo.handler.first(machine->m_videoCallbackHandlerInfo.id, machine->m_videoCallbackHandlerInfo.handler.second);
		}

		static void PowerButtonClickHandler(GtkWidget* widget, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			_assert(machine->m_powerCallbackHandlerInfo.handler.first != NULL);
			machine->m_powerCallbackHandlerInfo.handler.first(machine->m_powerCallbackHandlerInfo.id, machine->m_powerCallbackHandlerInfo.handler.second);
		}

		static void ResetButtonClickHandler(GtkWidget* widget, void* userData)
		{
			MachineUI* machine = reinterpret_cast<MachineUI*>(userData);
			_assert(machine->m_resetCallbackHandlerInfo.handler.first != NULL);
			machine->m_resetCallbackHandlerInfo.handler.first(machine->m_resetCallbackHandlerInfo.id, machine->m_resetCallbackHandlerInfo.handler.second);
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
		void MachineUI::setPowerButtonCallback(Callback callback, void* userData)
		{ 
			m_powerCallbackHandlerInfo.handler.first = callback; m_powerCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_powerButton), "clicked", G_CALLBACK(PowerButtonClickHandler), reinterpret_cast<void*>(this));
		}
		void MachineUI::setResetButtonCallback(Callback callback, void* userData)
		{ 
			m_resetCallbackHandlerInfo.handler.first = callback; m_resetCallbackHandlerInfo.handler.second = userData;
			g_signal_connect(G_OBJECT(m_resetButton), "clicked", G_CALLBACK(ResetButtonClickHandler), reinterpret_cast<void*>(this));
		}
	}
}