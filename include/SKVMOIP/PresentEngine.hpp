#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Window.hpp>
#include <SKVMOIP/HDMIDecodeNetStream.hpp>

#ifdef GLOBAL_DEBUG
#	define PVK_DEBUG
#endif
#include <PlayVk/PlayVk.h>

#include <atomic>

namespace SKVMOIP
{
	class PresentEngine
	{
	private:
		VkInstance m_vkInstance;
		VkSurfaceKHR m_vkSurface;
		VkPhysicalDevice m_vkPhysicalDevice;
		VkDevice m_vkDevice;
		uint32_t m_queueFamilyIndices[2];
		VkQueue m_vkGraphicsQueue;
		VkQueue m_vkPresentQueue;
		VkSwapchainKHR m_vkSwapchain;
		VkImageView* m_vkSwapchainImageViews;
		VkCommandPool m_vkCommandPool;
		VkCommandBuffer* m_vkCommandBuffers;
		PvkSemaphoreCircularPool* m_pvkSemaphorePool;
		VkFence m_vkFence;
		VkRenderPass m_vkRenderPass;
		PvkImage m_pvkImage;
		VkImageView m_vkImageView;
		VkFramebuffer* m_vkFramebuffers;
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		VkSamplerYcbcrConversion m_vkConversion;
		#endif
		VkSampler m_vkSampler;
		PvkBuffer m_pvkBuffer;
		void* m_mapPtr;
		VkDescriptorPool m_vkDescriptorPool;
		VkDescriptorSetLayout m_vkDescriptorSetLayout;
		VkDescriptorSet* m_vkDescriptorSet;
		VkShaderModule m_vkVertShaderModule;
		VkShaderModule m_vkFragShaderModule;
		VkPipelineLayout m_vkPipelineLayout;
		VkPipeline m_vkPipeline;

		Window& m_window;
		HDMIDecodeNetStream& m_decodeNetStream;

		void destroyWindowRelatedVkObjects();
		void createWindowRelatedVkObjects();
		void recordCommandBuffers();
		void recreate();

	public:
		PresentEngine(Window& window, HDMIDecodeNetStream& decodeNetStream);
		PresentEngine(PresentEngine&) = delete;
		PresentEngine& operator=(PresentEngine&) = delete;
		PresentEngine(PresentEngine&&) = delete;
		PresentEngine& operator=(PresentEngine&&) = delete;
		~PresentEngine();

		void* getBufferPtr() const noexcept { return m_mapPtr; }
		void runGameLoop(u32 frameRate);
		void runGameLoop(u32 frameRate, std::atomic<bool>& isLoop);
	};
}