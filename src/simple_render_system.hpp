#pragma once

#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_game_object.hpp"

#include <memory>
#include <vector>

namespace lv
{
	struct SimplePushConstantsData
	{
		glm::mat4 transform{ 1.f };
		alignas(16) glm::vec3 color;
	};

	class SimpleRenderSystem
	{
	private:
		LvDevice& lvDevice;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<LvPipeline> lvPipeline;

	public:
		SimpleRenderSystem(LvDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();
		void renderGameObjects(
			VkCommandBuffer commandBuffer,
			std::vector<LvGameObject>& gameObjects);

	private:
		void createPipeline(VkRenderPass renderPass);
		void createPipelineLayout();
	};
}