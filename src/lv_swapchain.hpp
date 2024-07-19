#pragma once

#include "vulkan/vulkan.h"

#include "lv_device.hpp"

#include <vector>

namespace lv
{
	class LvSwapChain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	private:
		LvDevice& device;
		VkExtent2D windowExtent;
		
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkRenderPass renderPass;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;

	public:
		LvSwapChain(LvDevice& device, VkExtent2D extent);
		~LvSwapChain();

		LvSwapChain(const LvSwapChain&) = delete;
		void operator=(const LvSwapChain&) = delete;

		VkRenderPass getRenderPass() { return renderPass; }
		size_t getImageCount() { return swapChainImages.size(); }
		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; };
		VkExtent2D getSwapChainExtent() { return swapChainExtent; };

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(
			const VkCommandBuffer* buffers, 
			uint32_t* imageIndex);

	private:
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(
			const VkSurfaceCapabilitiesKHR& capabilities);
	};
}