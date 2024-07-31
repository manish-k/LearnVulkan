#pragma once

#include "vulkan/vulkan.h"

#include "lv_device.hpp"
#include "lv_window.hpp"

#include <vector>
#include <memory>

namespace lv
{
	class LvSwapChain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	private:
		LvDevice& device;
		LvWindow& window;
		
		VkSwapchainKHR swapChain;
		std::shared_ptr<LvSwapChain> oldSwapChain;
		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
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
		LvSwapChain(LvDevice& device, LvWindow& window);
		LvSwapChain(
			LvDevice& device, 
			LvWindow& window, 
			std::shared_ptr<LvSwapChain> previous);
		~LvSwapChain();

		LvSwapChain(const LvSwapChain&) = delete;
		LvSwapChain& operator=(const LvSwapChain&) = delete;

		VkSwapchainKHR getVkSwapChain() { return swapChain; };

		VkRenderPass getRenderPass() { return renderPass; }
		size_t getImageCount() { return swapChainImages.size(); }
		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; };
		VkExtent2D getSwapChainExtent() { return swapChainExtent; };

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(
			const VkCommandBuffer* buffers, 
			uint32_t* imageIndex);

		bool compareSwapFormats(const LvSwapChain& swapChain) const {
			return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
				swapChain.swapChainImageFormat == swapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void cleanupSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();
		void createDepthResources();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
			const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(
			const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat findDepthFormat();
	};
}