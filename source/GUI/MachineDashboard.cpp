#include <SKVMOIP/GUI/MachineDashboard.hpp>

namespace SKVMOIP
{
	namespace GUI
	{
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
		                                                        m_topLevelBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)),
		                                                        m_topLevelButton(gtk_check_button_new())
		
		{
		
		  gtk_widget_set_margin_end(m_buttonBox, 40);
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
		
		}
		
		MachineDashboard::~MachineDashboard()
		{
		
		}
		
		void MachineDashboard::setOutputAddress(const char* ipAddress, const char* portNumber)
		{
		  const char* desc = "Video Output: ";
		  auto len0 = strlen(desc);
		  auto len1 = strlen(ipAddress);
		  auto len2 = strlen(portNumber);
		  char buffer[len0 + len1 + len2 + 2];
		  memcpy(buffer, desc, len0);
		  memcpy(buffer + len0, ipAddress, len1);
		  buffer[len0 + len1] = ':';
		  memcpy(buffer + len0 + len1 + 1, portNumber, len2);
		  buffer[len0 + len1 + len2 + 1] = 0;
		
		  gtk_label_set_text(GTK_LABEL(m_outputAddressLabel), buffer);
		}
		
		void MachineDashboard::setInputAddress(const char* ipAddress, const char* portNumber)
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
	}
}