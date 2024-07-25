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
	struct SimplePushConstantsData
	{
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	class App
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

	private:
		LvWindow lvWindow{ "The Vulkan", WIDTH, HEIGHT};
		LvDevice lvDevice{lvWindow};
		std::unique_ptr<LvSwapChain> lvSwapChain;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<LvPipeline> lvPipeline;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<LvModel> lvModel;


	public:
		App();
		~App();

		void run();

	private:
		void recreateSwapChain();
		void createPipeline();
		void createPipelineLayout();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateCommandBuffers();
		void recordCommandBuffers(uint32_t imageIndex);
		void drawFrame();
		void loadModel();
	};
}