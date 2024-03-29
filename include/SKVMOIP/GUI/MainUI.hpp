#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/GUI/MachineUI.hpp>
#include <SKVMOIP/GUI/AddUI.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <gtk/gtk.h>

namespace SKVMOIP
{
	namespace GUI
	{
		class MainUI
		{
		private:
		  GtkApplication* m_app;
		  GtkWidget* m_window;
		  GtkWidget* m_bottomCont;
		  GtkWidget* m_connectButton;
		  GtkWidget* m_editButton;
		  GtkWidget* m_removeButton;
		  std::unique_ptr<AddUI> m_addUI;
		  std::unordered_map<u32, MachineUI*> m_machineUIs;
		  std::vector<u32> m_invalidIDs;
		  u32 m_idGenerator;
		
		public:
		  MainUI(GtkApplication* app);
		  MainUI(MainUI&&) = delete;
		  MainUI& operator=(MainUI&&) = delete;
		  MainUI(MainUI&) = delete;
		  MainUI& operator=(MainUI&) = delete;
		  ~MainUI() = default;

		  operator GtkWidget*() noexcept { return m_window; }

		  template<typename... Args>
		  u32 createMachine(Args... args)
		  {
		    u32 id; 
		    if(m_invalidIDs.size() > 0)
		    {
		      id = m_invalidIDs.back();
		      m_invalidIDs.pop_back();
		    }
		    else
		      id = m_idGenerator++;
		    m_machineUIs.insert({ id, new MachineUI(id, std::forward<Args>(args)...) });
		    auto& machine = getMachine(id);
		    gtk_box_pack_start(GTK_BOX(m_bottomCont), static_cast<GtkWidget*>(machine), FALSE, FALSE, 5);
		
		    return id;
		  }
		  void removeMachine(u32 id);
		  MachineUI& getMachine(u32 id);

		  AddUI& showAddUI(void (*onAddCallbackHandler)(MachineData& data, void* userData), void (*onCancelClickhandler)(GtkWidget* button, void* userData), void* userData);
		  void hideAddUI();

	  	  void setConnectButtonActive(bool isActive);
		  void setEditButtonActive(bool isActive);
		  void setRemoveButtonActive(bool isActive);
		};
	}
}

