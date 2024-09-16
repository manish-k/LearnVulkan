#pragma once

#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_game_object.hpp"
#include "lv_camera.hpp"
#include "lv_frame_data.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace lv
{
	struct SimplePushConstantsData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	class SimpleRenderSystem
	{
	private:
		LvDevice& lvDevice;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<LvPipeline> lvPipeline;

	public:
		SimpleRenderSystem(
			LvDevice& device, 
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();
		void renderGameObjects(
			FrameData& frameData,
			std::vector<LvGameObject>& gameObjects);

	private:
		void createPipeline(
			VkRenderPass renderPass);
		void createPipelineLayout(
			VkDescriptorSetLayout globalSetLayout);
	};
}