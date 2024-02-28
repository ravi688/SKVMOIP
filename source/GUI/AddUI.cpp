#include <SKVMOIP/GUI/AddUI.hpp>

#define WINDOW_MIN_SIZE_X 500
#define WINDOW_MIN_SIZE_Y 400
#define	WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y

namespace SKVMOIP
{
	namespace GUI
	{
		AddUI::AddUI(GtkWidget* parent, void (*onAddCallbackHandler)(MachineData& data, void* userData), void* userData) : m_onAddCallback(onAddCallbackHandler), m_userData(userData)
		{
			m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);


			gtk_widget_set_sensitive(parent, FALSE);
			gtk_widget_set_can_focus(parent, FALSE);
			gtk_window_set_title(GTK_WINDOW(m_window), "Add Machine");
			gtk_widget_set_size_request(m_window, WINDOW_MIN_SIZE_X, WINDOW_MIN_SIZE_Y);
		  	gtk_window_set_default_size(GTK_WINDOW(m_window), WINDOW_DEF_SIZE_X, WINDOW_DEF_SIZE_Y);
		  	gtk_window_set_transient_for(GTK_WINDOW(m_window), GTK_WINDOW(parent));
		  	// gtk_window_set_gravity(GTK_WINDOW(m_window), GDK_GRAVITY_NORTH_WEST);
		  	// gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);


		  	m_topLevelVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		  		m_entryVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		  		gtk_box_pack_start(GTK_BOX(m_topLevelVBox), m_entryVBox, TRUE, TRUE, 0);
		  			m_nameHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		  			gtk_box_pack_start(GTK_BOX(m_entryVBox), m_nameHBox, FALSE, FALSE, 0);
		  				m_nameLabel = gtk_label_new("Name");
		  				gtk_box_pack_start(GTK_BOX(m_nameHBox), m_nameLabel, FALSE, FALSE, 0);
		  				m_nameEntry = gtk_entry_new();
		  				gtk_box_pack_start(GTK_BOX(m_nameHBox), m_nameEntry, TRUE, TRUE, 0);
		  			m_VIPAddrHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		  			gtk_box_pack_start(GTK_BOX(m_entryVBox), m_VIPAddrHBox, FALSE, FALSE, 0);
		  				m_VIPAddrLabel = gtk_label_new("VIP Addr");
		  				gtk_box_pack_start(GTK_BOX(m_VIPAddrHBox), m_VIPAddrLabel, FALSE, FALSE, 0);
		  				m_VIPAddrEntry = gtk_entry_new();
		  				gtk_box_pack_start(GTK_BOX(m_VIPAddrHBox), m_VIPAddrEntry, TRUE, TRUE, 0);
		  			m_KMIPAddrHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		  			gtk_box_pack_start(GTK_BOX(m_entryVBox), m_KMIPAddrHBox, FALSE, FALSE, 0);
		  				m_KMIPAddrLabel = gtk_label_new("KMIP Addr");
		  				gtk_box_pack_start(GTK_BOX(m_KMIPAddrHBox), m_KMIPAddrLabel, FALSE, FALSE, 0);
		  				m_KMIPAddrEntry = gtk_entry_new();
		  				gtk_box_pack_start(GTK_BOX(m_KMIPAddrHBox), m_KMIPAddrEntry, TRUE, TRUE, 0);
		  		m_buttonHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		  		gtk_box_pack_start(GTK_BOX(m_topLevelVBox), m_buttonHBox, FALSE, FALSE, 0);
		  			m_okButton = gtk_button_new_with_label("Ok");
		  			gtk_box_pack_start(GTK_BOX(m_buttonHBox), m_okButton, TRUE, TRUE, 0);
		  			m_cancelButton = gtk_button_new_with_label("Cancel");
		  			gtk_box_pack_start(GTK_BOX(m_buttonHBox), m_cancelButton, TRUE, TRUE, 0);

		  	gtk_container_add(GTK_CONTAINER(m_window), m_topLevelVBox);

			gtk_widget_show_all(m_window);
			gtk_window_present(GTK_WINDOW(m_window));
			gtk_widget_set_sensitive(parent, TRUE);
			gtk_widget_set_can_focus(parent, TRUE);
		}

		AddUI::~AddUI()
		{

		}
	}
}
