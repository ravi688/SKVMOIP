#include <SKVMOIP/GUI/AddUI.hpp>
#include <optional>
#include <SKVMOIP/debug.h>

#define WINDOW_MIN_SIZE_X 500
#define WINDOW_MIN_SIZE_Y 400
#define	WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y


static u32 constexpr IPV4_MAX_STRLEN = strlen("255.255.255.255:65535");
static u32 constexpr IPV4_MIN_STRLEN = strlen("0.0.0.0:0");


namespace SKVMOIP
{
	namespace GUI
	{
		static std::optional<std::pair<u32, u16>> parseIPAndPort(const char* str)
		{
			const char* ptr = str;
			u8 ip[4];
			for(u32 i = 0; i < 4; i++)
			{
				ip[i] = atoi(ptr);
				ptr = strchr(ptr, '.');
				if(ptr == NULL)
				{
					if(i == 3)
						break;
					else
						/* invalid IPv4 address */
						return { };
				}
				else ++ptr;
			}
				
			ptr = strchr(str, ':');
			if(ptr == NULL)
				/* invalid port number */
				return { };
			u16 port = atoi(ptr);
			return { { BIT32_PACK8(ip[0], ip[1], ip[2], ip[3]), port } };
		}

		static void OkButtonClickCallback(GtkButton* button, gpointer userData)
		{
			auto& ui = *reinterpret_cast<AddUI*>(userData);
			const char* voipAddrStr = gtk_entry_get_text(GTK_ENTRY(ui.m_VIPAddrEntry));
			const char* kmoipAddrStr = gtk_entry_get_text(GTK_ENTRY(ui.m_KMIPAddrEntry));
			u32 voipLen = strlen(voipAddrStr);
			u32 kmoipLen = strlen(kmoipAddrStr);
			if((voipLen >= IPV4_MIN_STRLEN) && (voipLen <= IPV4_MAX_STRLEN) || (kmoipLen >= IPV4_MIN_STRLEN) && (kmoipLen <= IPV4_MAX_STRLEN))
			{
				std::optional<std::pair<u32, u16>> voipAddr = parseIPAndPort(voipAddrStr);
				if(!voipAddr.has_value())
				{
					/* parse error, either invalid IPV4 address or the port number */
					debug_log_info("Parse error: Invalid voip IPV4 address or Port Number");
					return;
				}
				std::optional<std::pair<u32, u16>> kmoipAddr = parseIPAndPort(kmoipAddrStr);
				if(!kmoipAddr.has_value())
				{
					/* parse error, either invalid IPV4 address or the port number */
					debug_log_info("Parse error: Invalid kmoip IPV4 address or Port Number");
					return;
				}
				const char* name = gtk_entry_get_text(GTK_ENTRY(ui.m_nameEntry));
				MachineData data(voipAddr->first, kmoipAddr->first, voipAddr->second, kmoipAddr->second, name);
				ui.m_onAddCallback(data, ui.m_userData);
			}
			else 
			{
				/* TODO: show error window */
				debug_log_info("Parse error: Invalid voip IPV4 address:port or kmoip IPV4 address:port");
				return;			
			}
		}

		AddUI::AddUI(GtkWidget* parent, void (*onAddCallbackHandler)(MachineData& data, void* userData), void (*onAddUICancelClicked)(GtkWidget* button, void* userData), void* userData) : m_parent(parent), m_onAddCallback(onAddCallbackHandler), m_userData(userData)
		{
			m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

			gtk_window_set_title(GTK_WINDOW(m_window), "Add Machine");
			gtk_widget_set_size_request(m_window, WINDOW_MIN_SIZE_X, WINDOW_MIN_SIZE_Y);
		  	gtk_window_set_default_size(GTK_WINDOW(m_window), WINDOW_DEF_SIZE_X, WINDOW_DEF_SIZE_Y);
		  	gtk_window_set_transient_for(GTK_WINDOW(m_window), GTK_WINDOW(parent));

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
		  			g_signal_connect(G_OBJECT(m_okButton), "clicked", G_CALLBACK(OkButtonClickCallback), reinterpret_cast<void*>(this));
		  			gtk_box_pack_start(GTK_BOX(m_buttonHBox), m_okButton, TRUE, TRUE, 0);
		  			m_cancelButton = gtk_button_new_with_label("Cancel");
		  			g_signal_connect(G_OBJECT(m_cancelButton), "clicked", G_CALLBACK(onAddUICancelClicked), userData);
		  			gtk_box_pack_start(GTK_BOX(m_buttonHBox), m_cancelButton, TRUE, TRUE, 0);

		  	gtk_container_add(GTK_CONTAINER(m_window), m_topLevelVBox);

		  	gtk_window_set_modal(GTK_WINDOW(m_window), TRUE);
			gtk_widget_show_all(m_window);
			gtk_window_present(GTK_WINDOW(m_window));
		}

		AddUI::~AddUI()
		{
			gtk_widget_destroy(m_window);
		}
	}
}
