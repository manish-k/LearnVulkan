#pragma once

#include "lv_window.hpp"
#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_game_object.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace lv
{
	class LvRenderer
	{
	private:
		LvWindow& lvWindow;
		LvDevice& lvDevice;
		std::unique_ptr<LvSwapChain> lvSwapChain;
		std::unique_ptr<LvPipeline> lvPipeline;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		bool isFrameStarted{ false };
		int currentFrameIndex{ 0 };

	public:
		LvRenderer(LvWindow& window, LvDevice& device);
		~LvRenderer();

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkRenderPass getSwapChainRenderPass() const 
		{ 
			return lvSwapChain->getRenderPass(); 
		};
		bool isFrameInProgress() const { return isFrameStarted; };
		VkCommandBuffer getCurrentCommandBuffer() const
		{ 
			assert(isFrameStarted && 
				"Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex]; 
		};
		int getFrameIndex() const {
			assert(isFrameStarted &&
				"Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}
		float getAspectRatio() const {
			auto extent =  lvSwapChain->getSwapChainExtent();
			return static_cast<float>(extent.width) / 
				static_cast<float>(extent.height);
		}

	private:
		void recreateSwapChain();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateCommandBuffers();
	};
}