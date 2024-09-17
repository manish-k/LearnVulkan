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
	class PointLightSystem
	{
	private:
		LvDevice& lvDevice;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<LvPipeline> lvPipeline;

	public:
		PointLightSystem(
			LvDevice& device,
			VkRenderPass renderPass,
			VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();
		void render(FrameData& frameData);

	private:
		void createPipeline(
			VkRenderPass renderPass);
		void createPipelineLayout(
			VkDescriptorSetLayout globalSetLayout);
	};
}