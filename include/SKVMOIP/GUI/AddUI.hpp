#pragma once

#include <SKVMOIP/defines.hpp>
#include <gtk/gtk.h>

#include <SKVMOIP/MachineData.hpp>

namespace SKVMOIP
{
	namespace GUI
	{
		class AddUI
		{
		private:
			GtkWidget* m_window;
			GtkWidget* m_topLevelVBox;
			GtkWidget* m_entryVBox;
			GtkWidget* m_nameHBox;
			GtkWidget* m_nameLabel;
			GtkWidget* m_nameEntry;
			GtkWidget* m_VIPAddrHBox;
			GtkWidget* m_VIPAddrLabel;
			GtkWidget* m_VIPAddrEntry;
			GtkWidget* m_KMIPAddrHBox;
			GtkWidget* m_KMIPAddrLabel;
			GtkWidget* m_KMIPAddrEntry;
			GtkWidget* m_buttonHBox;
			GtkWidget* m_okButton;
			GtkWidget* m_cancelButton;
			void (*m_onAddCallback)(MachineData&, void*);
			void* m_userData;

		public:
			AddUI(GtkWidget* window, void (*onAddCallbackHandler)(MachineData& data, void* userData), void* userData);
			~AddUI();
		};
	}
}