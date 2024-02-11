#pragma once

#include <SKVMOIP/defines.hpp>
#include <SKVMOIP/Window.hpp>

#ifdef GLOBAL_DEBUG
#	define PVK_DEBUG
#endif
#include <PlayVk/PlayVk.h>

#define PRESENT_ENGINE_IMAGE_COUNT 3
#define PRESENT_ENGINE_MAX_IMAGE_INFLIGHT_COUNT 3

namespace SKVMOIP
{
	class PresentEngine
	{
	private:
		VkInstance m_vkInstance;
		VkSurfaceKHR m_vkSurfaceKHR;
		VkPhysicalDevice m_vkPhysicalDevice;
		VkDevice m_vkDevice;
		VkQueue m_vkGraphicsQueue;
		VkQueue m_vkPresentQueue;
		VkSwapchainKHR m_vkSwapchainKHR;
		VkCommandPool m_vkCommandPool;
		VkCommandBuffer* m_vkCommandBuffers;
		PvkSemaphoreCircularPool* m_pvkSemaphorePool;
		PvkFencePool* m_pvkFencePool;
		VkRenderPass m_vkRenderPass;
		PvkImage m_pvkImage;
		VkImageView m_vkImageView;
		VkFramebuffer m_vkFramebuffer;
		VkSampler m_vkSampler;
		VkDescriptorPool m_vkDescriptorPool;
		VkDescriptorSetLayout m_vkDescriptorSetLayout;
		VkDescriptorSet* m_vkDescriptorSet;
		VkShaderModule m_vkVertShaderModule;
		VkShaderModule m_vkFragShaderModule;
		VkPipelineLayout m_vkPipelineLayout;
		VkPipeline m_vkPipeline;

		Window& m_window;

		void (*m_callback)(void*, void*);
		void* m_userData;

	public:
		PresentEngine(Window& window);
		PresentEngine(PresentEngine&) = delete;
		PresentEngine& operator=(PresentEngine&) = delete;
		PresentEngine(PresentEngine&&) = delete;
		PresentEngine& operator=(PresentEngine&&) = delete;
		~PresentEngine();

		void setPresentCallback(void (*callback)(void* buffer, void* userData), void* userData) noexcept
		{
			m_callback = callback;
			m_userData = userData;
		}
		void runGameLoop(u32 frameRate);
	};
}