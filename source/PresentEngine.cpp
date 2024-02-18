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
#include <SKVMOIP/StopWatch.hpp>

#include <chrono>

#define HDMI_CAPTURE_WIDTH 1920
#define HDMI_CAPTURE_HEIGHT 1080
#define PRESENT_ENGINE_IMAGE_COUNT 3
#define PRESENT_ENGINE_MAX_IMAGE_INFLIGHT_COUNT 3

namespace SKVMOIP
{
	static VkRenderPass CreateRenderPass(VkDevice device)
	{
		VkAttachmentDescription colorAttachment { };
		{
			#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
			colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
			#else
			colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
			#endif
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

		VkSubpassDependency dependencies[2] = { };
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = 0;
		
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_NONE_KHR;
		dependencies[1].dependencyFlags = 0;

	
		VkRenderPassCreateInfo cInfo { };
		{
			cInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			cInfo.attachmentCount = 1; 
			cInfo.pAttachments = &colorAttachment;
			cInfo.subpassCount = 1;
			cInfo.pSubpasses = &subpass;
			cInfo.dependencyCount = 2;
			cInfo.pDependencies = &dependencies[0];
		};
		VkRenderPass renderPass;
		PVK_CHECK(vkCreateRenderPass(device, &cInfo, NULL, &renderPass));
		return renderPass;
	}

	static VkSampler CreateSampler(VkDevice device, VkSamplerYcbcrConversion conversion)
	{
		VkSamplerYcbcrConversionInfo conversionInfo { };
		conversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
		conversionInfo.conversion = conversion;
		VkSamplerCreateInfo cInfo { }; 
		{
			cInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			cInfo.pNext = (conversion == VK_NULL_HANDLE) ? NULL : &conversionInfo;
			cInfo.magFilter = VK_FILTER_LINEAR;
			cInfo.minFilter = VK_FILTER_LINEAR;
			cInfo.addressModeU = (conversion == VK_NULL_HANDLE) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			cInfo.addressModeV = (conversion == VK_NULL_HANDLE) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			cInfo.addressModeW = (conversion == VK_NULL_HANDLE) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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

	static VkSamplerYcbcrConversion CreateYUVConversion(VkDevice device)
	{
		VkSamplerYcbcrConversionCreateInfo cInfo = { };
		cInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
		cInfo.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
		cInfo.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
		cInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		cInfo.chromaFilter = VK_FILTER_LINEAR;
		cInfo.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
		cInfo.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
		cInfo.forceExplicitReconstruction = VK_FALSE;
		cInfo.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		VkSamplerYcbcrConversion sampler;
		PVK_CHECK(vkCreateSamplerYcbcrConversion(device, &cInfo, NULL, &sampler));
		return sampler;
	}

	static VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, VkSampler immutableSampler)
	{
		VkDescriptorSetLayoutBinding binding = 
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = (immutableSampler	== VK_NULL_HANDLE) ? NULL :  &immutableSampler
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

	void PresentEngine::destroyWindowRelatedVkObjects()
	{
		vkDestroyPipeline(m_vkDevice, m_vkPipeline, NULL);
		pvkDestroyFramebuffers(m_vkDevice, PRESENT_ENGINE_IMAGE_COUNT, m_vkFramebuffers);
		PVK_DELETE(m_vkFramebuffers);
		pvkDestroySwapchainImageViews(m_vkDevice, m_vkSwapchain, m_vkSwapchainImageViews);
		PVK_DEBUG(m_vkSwapchainImageViews);
		vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, NULL);
	}

	void PresentEngine::createWindowRelatedVkObjects()
	{
		m_vkSwapchain = pvkCreateSwapchain(m_vkDevice, m_vkSurface, PRESENT_ENGINE_IMAGE_COUNT,
													m_window.getClientWidth(), m_window.getClientHeight(),
													#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION 
													VK_FORMAT_B8G8R8A8_UNORM, 
													#else
													VK_FORMAT_B8G8R8A8_SRGB,
													#endif
													VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, 
													VK_PRESENT_MODE_FIFO_KHR,
													2, m_queueFamilyIndices, VK_NULL_HANDLE);
		u32 imageCount;
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		m_vkSwapchainImageViews = pvkCreateSwapchainImageViews(m_vkDevice, m_vkSwapchain, VK_FORMAT_B8G8R8A8_UNORM, &imageCount);
		#else
		m_vkSwapchainImageViews = pvkCreateSwapchainImageViews(m_vkDevice, m_vkSwapchain, VK_FORMAT_B8G8R8A8_SRGB, &imageCount);
		#endif
		_assert(imageCount == PRESENT_ENGINE_IMAGE_COUNT);

		VkImageView attachments[PRESENT_ENGINE_IMAGE_COUNT];
		for(u32 i = 0; i < PRESENT_ENGINE_IMAGE_COUNT; i++)
			attachments[i] = m_vkSwapchainImageViews[i];
		m_vkFramebuffers = pvkCreateFramebuffers(m_vkDevice, m_vkRenderPass, m_window.getClientWidth(), m_window.getClientHeight(), PRESENT_ENGINE_IMAGE_COUNT, 1, attachments);
		m_vkPipeline = pvkCreateGraphicsPipelineProfile0(m_vkDevice, m_vkPipelineLayout, m_vkRenderPass, m_window.getClientWidth(), m_window.getClientHeight(), 2, (PvkShader) { m_vkVertShaderModule, PVK_SHADER_TYPE_VERTEX }, (PvkShader) { m_vkFragShaderModule, PVK_SHADER_TYPE_FRAGMENT });
	}

	PresentEngine::PresentEngine(Window& window, HDMIDecodeNetStream& decodeNetStream) : m_window(window), m_decodeNetStream(decodeNetStream), m_mapPtr(NULL)
	{
		m_vkInstance = pvkCreateVulkanInstanceWithExtensions(2, "VK_KHR_win32_surface", "VK_KHR_surface");
		m_vkSurface = pvkCreateSurface(m_vkInstance, GetModuleHandle(NULL), window.getNativeHandle());
		m_vkPhysicalDevice = pvkGetPhysicalDevice(m_vkInstance, m_vkSurface, 
														VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 
														#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
														VK_FORMAT_B8G8R8A8_UNORM,
														#else
														VK_FORMAT_B8G8R8A8_SRGB,
														#endif 
														VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, 
														VK_PRESENT_MODE_FIFO_KHR,
														PRESENT_ENGINE_IMAGE_COUNT,
														#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
														true
														#else
														false
														#endif
														);
		u32 graphicsQueueFamilyIndex = pvkFindQueueFamilyIndex(m_vkPhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
		u32 presentQueueFamilyIndex = pvkFindQueueFamilyIndexWithPresentSupport(m_vkPhysicalDevice, m_vkSurface);
		m_queueFamilyIndices[0] = graphicsQueueFamilyIndex;
		m_queueFamilyIndices[1] = presentQueueFamilyIndex;
		m_vkDevice = pvkCreateLogicalDeviceWithExtensions(m_vkInstance, 
																m_vkPhysicalDevice,
																2, m_queueFamilyIndices, 
							#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
																true, 2, VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_sampler_ycbcr_conversion"
							#else
																false, 1, VK_KHR_SWAPCHAIN_EXTENSION_NAME
							#endif
																);
		vkGetDeviceQueue(m_vkDevice, graphicsQueueFamilyIndex, 0, &m_vkGraphicsQueue);
		vkGetDeviceQueue(m_vkDevice, presentQueueFamilyIndex, 0, &m_vkPresentQueue);

		m_vkCommandPool = pvkCreateCommandPool(m_vkDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamilyIndex);
		m_vkCommandBuffers = __pvkAllocateCommandBuffers(m_vkDevice, m_vkCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, PRESENT_ENGINE_IMAGE_COUNT);

		m_pvkSemaphorePool = pvkCreateSemaphoreCircularPool(m_vkDevice, 2 * PRESENT_ENGINE_MAX_IMAGE_INFLIGHT_COUNT);
		m_vkFence = pvkCreateFence(m_vkDevice, (VkFenceCreateFlags)(0));
		m_vkRenderPass = CreateRenderPass(m_vkDevice);

		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		m_vkConversion = CreateYUVConversion(m_vkDevice);
		#endif
		m_vkSampler = CreateSampler(m_vkDevice,
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
			m_vkConversion
		#else
			VK_NULL_HANDLE
		#endif
									);
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		m_pvkBuffer = pvkCreateBuffer(m_vkPhysicalDevice, m_vkDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, (HDMI_CAPTURE_WIDTH * HDMI_CAPTURE_HEIGHT * 3) >> 1, 2, m_queueFamilyIndices);
		PVK_CHECK(vkMapMemory(m_vkDevice, m_pvkBuffer.memory, 0, (HDMI_CAPTURE_WIDTH * HDMI_CAPTURE_HEIGHT * 3) >> 1, 0, &m_mapPtr));
		#else
		m_pvkBuffer = pvkCreateBuffer(m_vkPhysicalDevice, m_vkDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, HDMI_CAPTURE_WIDTH * HDMI_CAPTURE_HEIGHT * 4, 2, m_queueFamilyIndices);
		PVK_CHECK(vkMapMemory(m_vkDevice, m_pvkBuffer.memory, 0, HDMI_CAPTURE_WIDTH * HDMI_CAPTURE_HEIGHT * 4, 0, &m_mapPtr));
		#endif
		#ifdef USE_DIRECT_FRAME_DATA_COPY
		m_decodeNetStream.addFrameDataStorage(m_mapPtr);
		#endif

		m_vkDescriptorPool = pvkCreateDescriptorPool(m_vkDevice, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		m_vkDescriptorSetLayout = CreateDescriptorSetLayout(m_vkDevice, m_vkSampler);
		#else
		m_vkDescriptorSetLayout = CreateDescriptorSetLayout(m_vkDevice, VK_NULL_HANDLE);
		#endif
		m_vkDescriptorSet = pvkAllocateDescriptorSets(m_vkDevice, m_vkDescriptorPool, 1, &m_vkDescriptorSetLayout);
	
		m_vkFragShaderModule = pvkCreateShaderModule(m_vkDevice, "shaders/sample.frag.spv");
		m_vkVertShaderModule = pvkCreateShaderModule(m_vkDevice, "shaders/sample.vert.spv");

		m_vkPipelineLayout = pvkCreatePipelineLayout(m_vkDevice, 1, &m_vkDescriptorSetLayout);

		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
		m_pvkImage = pvkCreateImage2(m_vkPhysicalDevice, m_vkDevice, 
										VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
										VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, HDMI_CAPTURE_WIDTH, HDMI_CAPTURE_HEIGHT, 
										VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
										2, m_queueFamilyIndices);
		m_vkImageView = pvkCreateImageView2(m_vkDevice, m_pvkImage.handle, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, 
													(VkImageAspectFlagBits) (VK_IMAGE_ASPECT_COLOR_BIT), 
													m_vkConversion);
		#else
		m_pvkImage = pvkCreateImage(m_vkPhysicalDevice, m_vkDevice, 
										VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
										VK_FORMAT_B8G8R8A8_SRGB, HDMI_CAPTURE_WIDTH, HDMI_CAPTURE_HEIGHT, 
										VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
										2, m_queueFamilyIndices);
		m_vkImageView = pvkCreateImageView(m_vkDevice, m_pvkImage.handle, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		#endif
		pvkWriteImageViewToDescriptor(m_vkDevice, *m_vkDescriptorSet, 0, m_vkImageView, m_vkSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		createWindowRelatedVkObjects();
		
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
				// Image Layout Transition: VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				VkImageMemoryBarrier imageMemoryBarrier = { };
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = m_queueFamilyIndices[0];
				imageMemoryBarrier.dstQueueFamilyIndex = m_queueFamilyIndices[0];
				imageMemoryBarrier.image = m_pvkImage.handle;
				#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				#else
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				#endif
				imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
				imageMemoryBarrier.subresourceRange.levelCount = 1;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				imageMemoryBarrier.subresourceRange.layerCount = 1;
				vkCmdPipelineBarrier(m_vkCommandBuffers[index],
									VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
									VK_PIPELINE_STAGE_TRANSFER_BIT, 
									VK_DEPENDENCY_BY_REGION_BIT,
									0, NULL,
									0, NULL,
									1, &imageMemoryBarrier);
				#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
				VkBufferImageCopy imageCopyInfos[2] = { };
				imageCopyInfos[0].bufferOffset = 0;
				imageCopyInfos[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
				imageCopyInfos[0].imageSubresource.layerCount = 1;
				imageCopyInfos[0].imageExtent = { HDMI_CAPTURE_WIDTH, HDMI_CAPTURE_HEIGHT, 1 };
				imageCopyInfos[1].bufferOffset = HDMI_CAPTURE_WIDTH * HDMI_CAPTURE_HEIGHT;
				imageCopyInfos[1].imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
				imageCopyInfos[1].imageSubresource.layerCount = 1;
				imageCopyInfos[1].imageExtent = { HDMI_CAPTURE_WIDTH >> 1, HDMI_CAPTURE_HEIGHT >> 1, 1 };
				vkCmdCopyBufferToImage(m_vkCommandBuffers[index], m_pvkBuffer.handle, m_pvkImage.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 2, imageCopyInfos);
				#else
				VkBufferImageCopy imageCopyInfo = { };
				imageCopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;;
				imageCopyInfo.imageSubresource.layerCount = 1;
				imageCopyInfo.imageExtent = { HDMI_CAPTURE_WIDTH, HDMI_CAPTURE_HEIGHT, 1 };
				vkCmdCopyBufferToImage(m_vkCommandBuffers[index], m_pvkBuffer.handle, m_pvkImage.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyInfo);
				#endif

				imageMemoryBarrier = { };
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = m_queueFamilyIndices[0];
				imageMemoryBarrier.dstQueueFamilyIndex = m_queueFamilyIndices[0];
				imageMemoryBarrier.image = m_pvkImage.handle;
				#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				#else
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				#endif
				imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
				imageMemoryBarrier.subresourceRange.levelCount = 1;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				imageMemoryBarrier.subresourceRange.layerCount = 1;
				vkCmdPipelineBarrier(m_vkCommandBuffers[index],
									VK_PIPELINE_STAGE_TRANSFER_BIT, 
									VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
									VK_DEPENDENCY_BY_REGION_BIT,
									0, NULL,
									0, NULL,
									1, &imageMemoryBarrier);
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
		PVK_CHECK(vkDeviceWaitIdle(m_vkDevice));
		destroyWindowRelatedVkObjects();
		vkDestroyImageView(m_vkDevice, m_vkImageView, NULL);
		pvkDestroyImage(m_vkDevice, m_pvkImage);
		vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, NULL);
		vkDestroyShaderModule(m_vkDevice, m_vkFragShaderModule, NULL);
		vkDestroyShaderModule(m_vkDevice, m_vkVertShaderModule, NULL);
		PVK_DELETE(m_vkDescriptorSet);
		vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, NULL);
		vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, NULL);
		vkUnmapMemory(m_vkDevice, m_pvkBuffer.memory);
		pvkDestroyBuffer(m_vkDevice, m_pvkBuffer);
		vkDestroySampler(m_vkDevice, m_vkSampler, NULL);
		#ifdef USE_VULKAN_FOR_COLOR_SPACE_CONVERSION	
		vkDestroySamplerYcbcrConversion(m_vkDevice, m_vkConversion, NULL);
		#endif
		vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, NULL);
		vkDestroyFence(m_vkDevice, m_vkFence, NULL);
		pvkDestroySemaphoreCircularPool(m_vkDevice, m_pvkSemaphorePool);
		PVK_DELETE(m_vkCommandBuffers);
		vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, NULL);
		vkDestroyDevice(m_vkDevice, NULL);
		vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, NULL);
		vkDestroyInstance(m_vkInstance, NULL);
	}

	void PresentEngine::recreate()
	{
		destroyWindowRelatedVkObjects();
		createWindowRelatedVkObjects();
		recordCommandBuffers();
	}

	void PresentEngine::runGameLoop(u32 frameRate)
	{
		const f64 deltaTime = 1000.0 / frameRate;
		auto startTime = std::chrono::high_resolution_clock::now();
		/* Rendering & Presentation */
		while(!m_window.shouldClose(false))
		{
			if((m_window.getClientWidth() == 0) || (m_window.getClientHeight() == 0))
			{
				m_window.pollEvents();
				continue;
			}
			auto time = std::chrono::high_resolution_clock::now();
			if(std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count() >= deltaTime)
			{
				/* Takes: 2 ms to 4 ms - same as Win32 Blit */

				startTime = time;

				auto frame = m_decodeNetStream.borrowFrameData();
				uint32_t semaphoreIndex;
				VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
				if(frame)
					imageAvailableSemaphore = pvkSemaphoreCircularPoolAcquire(m_pvkSemaphorePool, &semaphoreIndex);

				uint32_t index;
				while(!pvkAcquireNextImageKHR(m_vkDevice, m_vkSwapchain, UINT64_MAX, imageAvailableSemaphore, m_vkFence, &index))
				{
					PVK_CHECK(vkDeviceWaitIdle(m_vkDevice));
					if(imageAvailableSemaphore != VK_NULL_HANDLE)
						imageAvailableSemaphore = pvkSemaphoreCircularPoolRecreate(m_vkDevice, m_pvkSemaphorePool, semaphoreIndex);
					pvkResetFences(m_vkDevice, 1, &m_vkFence);
					recreate();
				}

				PVK_CHECK(vkWaitForFences(m_vkDevice, 1, &m_vkFence, VK_TRUE, UINT64_MAX));
				PVK_CHECK(vkResetFences(m_vkDevice, 1, &m_vkFence));

				VkSemaphore renderFinishSemaphore = VK_NULL_HANDLE;
				if(frame)
				{
					renderFinishSemaphore = pvkSemaphoreCircularPoolAcquire(m_pvkSemaphorePool, NULL);
					
					#ifndef USE_DIRECT_FRAME_DATA_COPY
					auto frameData = FIFOPool<HDMIDecodeNetStream::FrameData>::GetValue(frame);
					_assert(frameData.has_value());
					// _assert(frameData->getSize() == drawSurface->getBufferSize());
					/* Takes: 1 ms to 4 ms */
					memcpy(m_mapPtr, frameData->getPtr(), frameData->getSize());
					#endif 

					// execute commands
					pvkSubmit(m_vkCommandBuffers[index], m_vkGraphicsQueue, imageAvailableSemaphore, renderFinishSemaphore, m_vkFence);
					PVK_CHECK(vkWaitForFences(m_vkDevice, 1, &m_vkFence, VK_TRUE, UINT64_MAX));
					m_decodeNetStream.returnFrameData(frame);
					PVK_CHECK(vkResetFences(m_vkDevice, 1, &m_vkFence));
				}


				// present the output image
				if(!pvkPresent(index, m_vkSwapchain, m_vkPresentQueue, (renderFinishSemaphore == VK_NULL_HANDLE) ? 0 : 1, &renderFinishSemaphore))
				{
					PVK_CHECK(vkDeviceWaitIdle(m_vkDevice));
					recreate();
				}
			}
			m_window.pollEvents();
		}
	}
}
