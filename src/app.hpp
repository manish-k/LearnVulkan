#pragma once

#include "lv_window.hpp"
#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_model.hpp"

#include <memory>
#include <vector>

namespace lv
{
	class App
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

	private:
		LvWindow lvWindow{ "The Vulkan", WIDTH, HEIGHT};
		LvDevice lvDevice{lvWindow};
		LvSwapChain lvSwapChain{ lvDevice, lvWindow.getExtent() };
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<LvPipeline> lvPipeline;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<LvModel> lvModel;


	public:
		App();
		~App();

		void run();

	private:
		void createPipeline();
		void createPipelineLayout();
		void createCommandBuffers();
		void drawFrame();
		void loadModel();
	};
}