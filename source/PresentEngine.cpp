#define PVK_IMPLEMENTATION
#ifdef GLOBAL_DEBUG
#	ifndef PVK_DEBUG
#		define PVK_DEBUG
#	endif
#endif
#include <PlayVk/PlayVk.h>

#include <SKVMOIP/PresentEngine.hpp>

namespace SKVMOIP
{
	PresentEngine::PresentEngine(Window& window) : m_window(window)
	{
		m_vkInstance = pvkCreateVulkanInstanceWithExtensions(2, "VK_KHR_win32_surface", "VK_KHR_surface");
	}

	PresentEngine::~PresentEngine()
	{
		vkDestroyInstance(m_vkInstance, NULL);
	}

	void PresentEngine::runGameLoop(u32 frameRate)
	{

	}
}
