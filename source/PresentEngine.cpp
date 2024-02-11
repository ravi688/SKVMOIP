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

#include <chrono>

namespace SKVMOIP
{
	static VkRenderPass CreateRenderPass(VkDevice device)
	{
		VkAttachmentDescription colorAttachment { };
		{
			colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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

	static VkSampler CreateSampler(VkDevice device)
	{
		VkSamplerCreateInfo cInfo { }; 
		{
			cInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			cInfo.magFilter = VK_FILTER_LINEAR;
			cInfo.minFilter = VK_FILTER_LINEAR;
			cInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			cInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			cInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			cInfo.anisotropyEnable = VK_FALSE;
			cInfo.maxAnisotropy = 1.0f; // Optional
			cInfo.compareEnable = VK_FALSE;
			cInfo.compareOp = VK_COMPARE_OP_ALWAYS;	// Optional
			cInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			cInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			cInfo.mipLodBias = 0.0f;
			cInfo.minLod = 0.0f;
			cInfo.maxLod = 1.0f;
		};

		VkSampler sampler;
		PVK_CHECK(vkCreateSampler(device, &cInfo, NULL, &sampler));
		return sampler;
	}

	static VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device)
	{
		VkDescriptorSetLayoutBinding binding = 
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		};
	
		VkDescriptorSetLayoutCreateInfo cInfo = 
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings = &binding
		};
	
		VkDescriptorSetLayout setLayout;
		PVK_CHECK(vkCreateDescriptorSetLayout(device, &cInfo, NULL, &setLayout));
		return setLayout;
	}

	PresentEngine::PresentEngine(Window& window) : m_window(window), m_callback(NULL), m_userData(NULL)
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
										VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
										2, queueFamilyIndices);
		m_vkImageView = pvkCreateImageView(m_vkDevice, m_pvkImage.handle, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		VkImageView attachments[PRESENT_ENGINE_IMAGE_COUNT];
		for(u32 i = 0; i < PRESENT_ENGINE_IMAGE_COUNT; i++)
			attachments[i] = m_vkSwapchainImageViews[i];
		m_vkFramebuffers = pvkCreateFramebuffers(m_vkDevice, m_vkRenderPass, window.getClientWidth(), window.getClientHeight(), PRESENT_ENGINE_IMAGE_COUNT, 1, attachments);

		m_vkSampler = CreateSampler(m_vkDevice);

		m_vkDescriptorPool = pvkCreateDescriptorPool(m_vkDevice, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
		m_vkDescriptorSetLayout = CreateDescriptorSetLayout(m_vkDevice);
		m_vkDescriptorSet = pvkAllocateDescriptorSets(m_vkDevice, m_vkDescriptorPool, 1, &m_vkDescriptorSetLayout);
		pvkWriteImageViewToDescriptor(m_vkDevice, *m_vkDescriptorSet, 0, m_vkImageView, m_vkSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	
		m_vkFragShaderModule = pvkCreateShaderModule(m_vkDevice, "shaders/sample.frag.spv");
		m_vkVertShaderModule = pvkCreateShaderModule(m_vkDevice, "shaders/sample.vert.spv");

		m_vkPipelineLayout = pvkCreatePipelineLayout(m_vkDevice, 1, &m_vkDescriptorSetLayout);
		m_vkPipeline = pvkCreateGraphicsPipelineProfile0(m_vkDevice, m_vkPipelineLayout, m_vkRenderPass, window.getClientWidth(), window.getClientHeight(), 2, (PvkShader) { m_vkVertShaderModule, PVK_SHADER_TYPE_VERTEX }, (PvkShader) { m_vkFragShaderModule, PVK_SHADER_TYPE_FRAGMENT });
		
		recordCommandBuffers();
	}

	void PresentEngine::recordCommandBuffers()
	{
		VkClearValue clearValue { };
		clearValue.color.float32[0] = 0.1f;
		clearValue.color.float32[1] = 0.3f;
		clearValue.color.float32[2] = 0;
		clearValue.color.float32[3] = 1;

		for(int index = 0; index < PRESENT_ENGINE_IMAGE_COUNT; index++)
		{
			pvkBeginCommandBuffer(m_vkCommandBuffers[index], (VkCommandBufferUsageFlagBits)0);
				pvkBeginRenderPass(m_vkCommandBuffers[index], m_vkRenderPass, m_vkFramebuffers[index], m_window.getClientWidth(), m_window.getClientHeight(), 1, &clearValue);
					vkCmdBindPipeline(m_vkCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipeline);
					vkCmdBindDescriptorSets(m_vkCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkPipelineLayout, 0, 1, m_vkDescriptorSet, 0, NULL);
					vkCmdDraw(m_vkCommandBuffers[index], 6, 1, 0, 0);
				pvkEndRenderPass(m_vkCommandBuffers[index]);
			pvkEndCommandBuffer(m_vkCommandBuffers[index]);
		}
	}

	PresentEngine::~PresentEngine()
	{
		vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, NULL);
		vkDestroyPipeline(m_vkDevice, m_vkPipeline, NULL);
		vkDestroyShaderModule(m_vkDevice, m_vkFragShaderModule, NULL);
		vkDestroyShaderModule(m_vkDevice, m_vkVertShaderModule, NULL);
		PVK_DELETE(m_vkDescriptorSet);
		vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, NULL);
		vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, NULL);
		vkDestroySampler(m_vkDevice, m_vkSampler, NULL);
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
		const f64 deltaTime = 1000.0 / frameRate;
		auto startTime = std::chrono::high_resolution_clock::now();
		/* Rendering & Presentation */
		while(!m_window.shouldClose(false))
		{
			auto time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count() >= deltaTime)
			{
				startTime = time;

				if(m_callback != NULL)
					m_callback(NULL, m_userData);

				VkFence fence;
				if(!pvkFencePoolAcquire(m_vkDevice, m_pvkFencePool, &fence))
				{
					m_window.pollEvents();
					continue;
				}
				uint32_t semaphoreIndex;
				VkSemaphore imageAvailableSemaphore = pvkSemaphoreCircularPoolAcquire(m_pvkSemaphorePool, &semaphoreIndex);
				uint32_t index;
				while(!pvkAcquireNextImageKHR(m_vkDevice, m_vkSwapchain, UINT64_MAX, imageAvailableSemaphore, fence, &index))
				{
					PVK_CHECK(vkDeviceWaitIdle(m_vkDevice));
					imageAvailableSemaphore = pvkSemaphoreCircularPoolRecreate(m_vkDevice, m_pvkSemaphorePool, semaphoreIndex);
					pvkResetFences(m_vkDevice, 1, &fence);
					// recreate();
				}

				VkSemaphore renderFinishSemaphore = pvkSemaphoreCircularPoolAcquire(m_pvkSemaphorePool, NULL);
				// execute commands
				pvkSubmit(m_vkCommandBuffers[index], m_vkGraphicsQueue, imageAvailableSemaphore, renderFinishSemaphore, VK_NULL_HANDLE);

				// present the output image
				if(!pvkPresent(index, m_vkSwapchain, m_vkPresentQueue, renderFinishSemaphore))
				{
					PVK_CHECK(vkDeviceWaitIdle(m_vkDevice));
					// recreate();
				}
			}
			m_window.pollEvents();
		}
	}
}
