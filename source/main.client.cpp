#ifndef BUILD_CLIENT
#error "BUILD_CLIENT is not defined, but still main.client.cpp is being built"
#endif

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/debug.h>
#include <SKVMOIP/Win32/Win32.hpp>

#include <SKVMOIP/GUI/MachineUI.hpp>
#include <SKVMOIP/GUI/MainUI.hpp>
#include <SKVMOIP/MachineData.hpp>

#include <SKVMOIP/RDPSession.hpp>

using namespace SKVMOIP;

static std::vector<MachineData> GetMachineDataListFromServer()
{
  std::vector<MachineData> machines = 
  {
    { IP_ADDRESS(192, 168, 1, 11), IP_ADDRESS(192, 168, 1, 18), 2020, 101, "Win11-AMD-Ryzen-5-5600G", 1  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 19), 100, 101, "Dummy Machine", 2  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 20), 100, 101, "Dummy Machine", 3  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 21), 100, 101, "Dummy Machine", 4  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 22), 100, 101, "Dummy Machine", 5  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 23), 100, 101, "Dummy Machine", 6  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 24), 100, 101, "Dummy Machine", 7  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 25), 100, 101, "Dummy Machine", 8  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 26), 100, 101, "Dummy Machine", 9  },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 27), 100, 101, "Dummy Machine", 10 },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 28), 100, 101, "Dummy Machine", 11 },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 29), 100, 101, "Dummy Machine", 12 },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 30), 100, 101, "Dummy Machine", 13 },
    { IP_ADDRESS(192, 168, 1, 17), IP_ADDRESS(192, 168, 1, 31), 100, 101, "Dummy Machine", 14 }
  };
  return machines;
}

static std::vector<MachineData> gMachineDataList;
static std::unique_ptr<SKVMOIP::GUI::MainUI> gMainUI;
static std::vector<u32> gSelectedMachines;

namespace SKVMOIP
{
	namespace GUI
	{
		void OnConnectClicked(GtkWidget* button, void* userData)
		{
			debug_log_info ("Connect button clicked");
			std::for_each(gSelectedMachines.begin(), gSelectedMachines.end(),
				[](u32& id)
				{
					std::string ipAddrString(gMachineDataList[id].getVideoIPAddressStr());
					std::string prtNumString(gMachineDataList[id].getVideoPortNumberStr());
					// std::string ipAddrString(gMainUI->getMachine(id).getOutputAddressStr());
					auto sessionThread = std::thread([](std::string ipAddress, std::string portNumber)
					{
						RDPSession session;
						session.start(ipAddress.c_str(), portNumber.c_str());
					}, ipAddrString, prtNumString);

					sessionThread.detach();
				});
		}
	}
}

static void OnMachineSelect(u32 id, void* userData)
{
	gSelectedMachines.push_back(id);
	debug_log_info("%u:%s", id, __FUNCTION__);
}

static void OnMachineDeselect(u32 id, void* userData)
{
	auto it = std::find(gSelectedMachines.begin(), gSelectedMachines.end(), id);
	assert(it != gSelectedMachines.end());
	gSelectedMachines.erase(it);
	debug_log_info("%u:%s", id, __FUNCTION__);
}

static void OnVideoClicked(u32 id, void* userData)
{
	debug_log_info("%u:%s", id, __FUNCTION__);
}

static void OnPowerClicked(u32 id, void* userData)
{
	debug_log_info("%u:%s", id, __FUNCTION__);
}

static void OnResetClicked(u32 id, void* userData)
{
	debug_log_info("%u:%s", id, __FUNCTION__);
}

static void on_activate (GtkApplication *app) {

	gMainUI = std::move(std::unique_ptr<SKVMOIP::GUI::MainUI>(new SKVMOIP::GUI::MainUI(app)));

	gMachineDataList = GetMachineDataListFromServer();
	for(std::size_t i = 0; i < gMachineDataList.size(); i++)
	{
		u32 id = gMainUI->createMachine(static_cast<u32>(i), "Dummy Machine");
		auto& ui = gMainUI->getMachine(id);
		auto& data = gMachineDataList[i];
	    
	    ui.setName(data.getName());
	    ui.setOutputAddress(data.getVideoIPAddressStr(), data.getVideoPortNumberStr());
	    ui.setInputAddress(data.getKeyMoIPAddressStr(), data.getKeyMoPortNumberStr());
	    ui.setStatus("Status: Unknown");

	    ui.setSelectDeselectCallback(OnMachineSelect, OnMachineDeselect, NULL);
	    ui.setVideoButtonCallback(OnVideoClicked, NULL);
	    ui.setPowerButtonCallback(OnPowerClicked, NULL);
	    ui.setResetButtonCallback(OnResetClicked, NULL);
	  }
}

int main (int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new ("com.example.GtkApplication", G_APPLICATION_FLAGS_NONE);
  	g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
	Win32::InitializeMediaFundationAndCOM();
	Win32::DisplayRawInputDeviceList();
	Win32::RegisterRawInputDevices({ Win32::RawInputDeviceType::Mouse, Win32::RawInputDeviceType::Keyboard });
	bool result = g_application_run (G_APPLICATION (app), argc, argv);
	Win32::DeinitializeMediaFoundationAndCOM();
	return result;
}

