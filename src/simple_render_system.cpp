#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>

namespace lv
{
	SimpleRenderSystem::SimpleRenderSystem(LvDevice& device, VkRenderPass renderPass)
		: lvDevice{ device }
	{
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(lvDevice.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantsData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(
			lvDevice.getLogicalDevice(),
			&pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		LvPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		lvPipeline = std::make_unique<LvPipeline>(
			lvDevice,
			"shaders/base_vert_shader.vert.spv",
			"shaders/base_frag_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(
		VkCommandBuffer commandBuffer,
		std::vector<LvGameObject>& gameObjects)
	{
		lvPipeline->bind(commandBuffer);

		for (auto& object : gameObjects)
		{
			object.model->bind(commandBuffer);

			SimplePushConstantsData push{};
			object.transform.rotation.y = 
				glm::mod(object.transform.rotation.y + 0.0001f, glm::two_pi<float>());
			object.transform.rotation.x =
				glm::mod(object.transform.rotation.x + 0.00005f, glm::two_pi<float>());

			push.color = object.color;
			push.transform = object.transform.mat4();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantsData),
				&push);

			object.model->draw(commandBuffer);
		}
	}
}
