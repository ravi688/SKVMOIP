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
    { IP_ADDRESS(192, 168, 1, 8), IP_ADDRESS(192, 168, 1, 113), 2020, 2000, "Win11-AMD-Ryzen-5-5600G", 1  },
    { IP_ADDRESS(192, 168, 1, 8), IP_ADDRESS(192, 168, 1, 114), 2020, 2000, "Ubuntu20-Intel-Core-i3-10100F", 2  },
    { IP_ADDRESS(192, 168, 1, 8), IP_ADDRESS(192, 168, 1, 115), 2020, 2000, "MacOS-M1", 3  },
    { IP_ADDRESS(192, 168, 1, 8), IP_ADDRESS(192, 168, 1, 116), 2020, 2000, "Encoder-Win10-Intel-Core-i5-6400T", 4  },
  };
  return machines;
}

static std::vector<MachineData> gMachineDataList;
static std::unique_ptr<SKVMOIP::GUI::MainUI> gMainUI;
static std::vector<u32> gSelectedMachines;
static std::unordered_map<u32, std::unique_ptr<RDPSession>>* gActiveSessions;

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
					auto it = gActiveSessions->find(id);
					if(it == gActiveSessions->end())
						gActiveSessions->insert({ id, std::unique_ptr<RDPSession>(new RDPSession()) });

					std::unique_ptr<RDPSession>& rdp = (*gActiveSessions)[id];
					if(!rdp->isConnected())
					{
						SKVMOIP::GUI::MachineUI& mui = gMainUI->getMachine(id);
						mui.setStatus("Status: Connecting...");
						rdp->setConnectionStatusCallback([](bool isUp, void* userData)
						{
							SKVMOIP::GUI::MachineUI* muiptr = reinterpret_cast<SKVMOIP::GUI::MachineUI*>(userData);
							muiptr->setStatus(isUp ? "Status: Connected" : "Status: No Response");
						}, reinterpret_cast<void*>(&mui));
						rdp->connect(gMachineDataList[id].getKeyMoIPAddressStr(), gMachineDataList[id].getKeyMoPortNumberStr());
					}
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

	auto it = gActiveSessions->find(id);
	if(it == gActiveSessions->end())
	{
		DEBUG_LOG_ERROR("Please click on \"Connect\" button first to establish connection with the host");
		return;
	}
	std::unique_ptr<RDPSession>& rdp = it->second;

	std::string ipAddrString(gMachineDataList[id].getVideoIPAddressStr());
	std::string prtNumString(gMachineDataList[id].getVideoPortNumberStr());
	auto sessionThread = std::thread([](RDPSession* rdp, std::string ipAddress, std::string portNumber)
	{
		rdp->start(ipAddress.c_str(), portNumber.c_str());
	}, rdp.get(), ipAddrString, prtNumString);

	sessionThread.detach();
}

static void OnPowerPress(u32 id, void* userData)
{
	DEBUG_LOG_INFO("%u:%s", id, __FUNCTION__);

	auto it = gActiveSessions->find(id);
	_ASSERT(it != gActiveSessions->end());
	std::unique_ptr<RDPSession>& rdp = it->second;

	rdp->getKMNetStream()->sendFrontPanelInput({ true });
}

static void OnPowerStatusReceive(bool isUp, void* userData)
{
	DEBUG_LOG_INFO("Power Status: %s", (isUp) ? "On" : "Off");
}

static void OnPowerRelease(u32 id, void* userData)
{
	DEBUG_LOG_INFO("%u:%s", id, __FUNCTION__);

	auto it = gActiveSessions->find(id);
	_ASSERT(it != gActiveSessions->end());
	std::unique_ptr<RDPSession>& rdp = it->second;

	rdp->getKMNetStream()->sendFrontPanelInput({ false });
	rdp->getKMNetStream()->receivePowerStatus(OnPowerStatusReceive, NULL);
}

static void OnResetPress(u32 id, void* userData)
{
	DEBUG_LOG_INFO("%u:%s", id, __FUNCTION__);

	auto it = gActiveSessions->find(id);
	_ASSERT(it != gActiveSessions->end());
	std::unique_ptr<RDPSession>& rdp = it->second;

	rdp->getKMNetStream()->sendFrontPanelInput({ }, { true });
}

static void OnResetRelease(u32 id, void* userData)
{
	DEBUG_LOG_INFO("%u:%s", id, __FUNCTION__);
	
	auto it = gActiveSessions->find(id);
	_ASSERT(it != gActiveSessions->end());
	std::unique_ptr<RDPSession>& rdp = it->second;

	rdp->getKMNetStream()->sendFrontPanelInput({ }, { false });
}

static void on_activate (GtkApplication *app) {

	gMainUI = std::move(std::unique_ptr<SKVMOIP::GUI::MainUI>(new SKVMOIP::GUI::MainUI(app)));
	gActiveSessions = new std::unordered_map<u32, std::unique_ptr<RDPSession>>();

	gMachineDataList = GetMachineDataListFromServer();
	for(std::size_t i = 0; i < gMachineDataList.size(); i++)
	{
		u32 id = gMainUI->createMachine("Dummy Machine");
		auto& ui = gMainUI->getMachine(id);
		auto& data = gMachineDataList[i];
	    
	    ui.setName(data.getName());
	    ui.setOutputAddress(data.getVideoIPAddressStr(), data.getVideoPortNumberStr());
	    ui.setInputAddress(data.getKeyMoIPAddressStr(), data.getKeyMoPortNumberStr());
	    ui.setStatus("Status: Unknown");

	    ui.setSelectDeselectCallback(OnMachineSelect, OnMachineDeselect, NULL);
	    ui.setVideoButtonCallback(OnVideoClicked, NULL);
	    ui.setPowerButtonPressCallback(OnPowerPress, NULL);
	    ui.setPowerButtonReleaseCallback(OnPowerRelease, NULL);
	    ui.setResetButtonPressCallback(OnResetPress, NULL);
	    ui.setResetButtonReleaseCallback(OnResetRelease, NULL);
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
	delete gActiveSessions;
	Win32::DeinitializeMediaFoundationAndCOM();
	return result;
}

