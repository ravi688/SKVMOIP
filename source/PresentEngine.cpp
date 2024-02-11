#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif /* Windows include */

#define COMMON_HINT_OUT_IN_ALREADY_DEFINED

#define PVK_IMPLEMENTATION
#define PVK_USE_WIN32_SURFACE
#ifdef GLOBAL_DEBUG
#	ifndef PVK_DEBUG
#		define PVK_DEBUG
#	endif
#endif
#include <PlayVk/PlayVk.h>

#include <SKVMOIP/PresentEngine.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/assert.h>

namespace SKVMOIP
{
	static VkRenderPass CreateRenderPass(VkDevice device)
	{
		VkAttachmentDescription colorAttachment { };
		{
			colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		};
	
		VkAttachmentReference attachmentReference { };
		{
			attachmentReference.attachment = 0;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		};
	
		VkSubpassDescription subpass { };
		{
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &attachmentReference;
		};
	
		VkRenderPassCreateInfo cInfo { };
		{
			cInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			cInfo.attachmentCount = 1; 
			cInfo.pAttachments = &colorAttachment;
			cInfo.subpassCount = 1;
			cInfo.pSubpasses = &subpass;
		};
		VkRenderPass renderPass;
		PVK_CHECK(vkCreateRenderPass(device, &cInfo, NULL, &renderPass));
		return renderPass;
	}

	PresentEngine::PresentEngine(Window& window) : m_window(window)
	{
		m_vkInstance = pvkCreateVulkanInstanceWithExtensions(2, "VK_KHR_win32_surface", "VK_KHR_surface");
		m_vkSurface = pvkCreateSurface(m_vkInstance, GetModuleHandle(NULL), window.getNativeHandle());
		m_vkPhysicalDevice = pvkGetPhysicalDevice(m_vkInstance, m_vkSurface, 
														VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 
														VK_FORMAT_B8G8R8A8_SRGB, 
														VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, 
														VK_PRESENT_MODE_FIFO_KHR,
														PRESENT_ENGINE_IMAGE_COUNT);
		u32 graphicsQueueFamilyIndex = pvkFindQueueFamilyIndex(m_vkPhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
		u32 presentQueueFamilyIndex = pvkFindQueueFamilyIndexWithPresentSupport(m_vkPhysicalDevice, m_vkSurface);
		u32 queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
		m_vkDevice = pvkCreateLogicalDeviceWithExtensions(m_vkInstance, 
																m_vkPhysicalDevice,
																2, queueFamilyIndices,
																1, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		vkGetDeviceQueue(m_vkDevice, graphicsQueueFamilyIndex, 0, &m_vkGraphicsQueue);
		vkGetDeviceQueue(m_vkDevice, presentQueueFamilyIndex, 0, &m_vkPresentQueue);

		m_vkSwapchain = pvkCreateSwapchain(m_vkDevice, m_vkSurface, PRESENT_ENGINE_IMAGE_COUNT,
													window.getClientWidth(), window.getClientHeight(), 
													VK_FORMAT_B8G8R8A8_SRGB, 
													VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, 
													VK_PRESENT_MODE_FIFO_KHR,
													2, queueFamilyIndices, VK_NULL_HANDLE);
		u32 imageCount;
		m_vkSwapchainImageViews = pvkCreateSwapchainImageViews(m_vkDevice, m_vkSwapchain, VK_FORMAT_B8G8R8A8_SRGB, &imageCount);
		_assert(imageCount == PRESENT_ENGINE_IMAGE_COUNT);

		m_vkCommandPool = pvkCreateCommandPool(m_vkDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamilyIndex);
		m_vkCommandBuffers = __pvkAllocateCommandBuffers(m_vkDevice, m_vkCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, PRESENT_ENGINE_IMAGE_COUNT);

		m_pvkSemaphorePool = pvkCreateSemaphoreCircularPool(m_vkDevice, 2 * PRESENT_ENGINE_MAX_IMAGE_INFLIGHT_COUNT);
		m_pvkFencePool = pvkCreateFencePool(m_vkDevice, PRESENT_ENGINE_IMAGE_COUNT);
		m_vkRenderPass = CreateRenderPass(m_vkDevice);

		m_pvkImage = pvkCreateImage(m_vkPhysicalDevice, m_vkDevice, 
										VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
										VK_FORMAT_B8G8R8A8_SRGB, window.getClientWidth(), window.getClientHeight(), 
										VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, 
										2, queueFamilyIndices);
		m_vkImageView = pvkCreateImageView(m_vkDevice, m_pvkImage.handle, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		VkImageView attachments[PRESENT_ENGINE_IMAGE_COUNT];
		for(u32 i = 0; i < PRESENT_ENGINE_IMAGE_COUNT; i++)
			attachments[i] = m_vkSwapchainImageViews[i];
		m_vkFramebuffers = pvkCreateFramebuffers(m_vkDevice, m_vkRenderPass, window.getClientWidth(), window.getClientHeight(), PRESENT_ENGINE_IMAGE_COUNT, 1, attachments);
	}

	PresentEngine::~PresentEngine()
	{
		pvkDestroyFramebuffers(m_vkDevice, PRESENT_ENGINE_IMAGE_COUNT, m_vkFramebuffers);
		PVK_DELETE(m_vkFramebuffers);
		vkDestroyImageView(m_vkDevice, m_vkImageView, NULL);
		pvkDestroyImage(m_vkDevice, m_pvkImage);
		vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, NULL);
		pvkDestroyFencePool(m_vkDevice, m_pvkFencePool);
		pvkDestroySemaphoreCircularPool(m_vkDevice, m_pvkSemaphorePool);
		PVK_DELETE(m_vkCommandBuffers);
		vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, NULL);
		pvkDestroySwapchainImageViews(m_vkDevice, m_vkSwapchain, m_vkSwapchainImageViews);
		PVK_DEBUG(m_vkSwapchainImageViews);
		vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, NULL);
		vkDestroyDevice(m_vkDevice, NULL);
		vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, NULL);
		vkDestroyInstance(m_vkInstance, NULL);
	}

	void PresentEngine::runGameLoop(u32 frameRate)
	{

	}
}
