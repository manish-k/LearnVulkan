#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>

namespace lv
{
	SimpleRenderSystem::SimpleRenderSystem(
		LvDevice& device, 
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout)
		: lvDevice{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(lvDevice.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantsData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 
			static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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
		FrameData& frameData)
	{
		lvPipeline->bind(frameData.commandBuffer);

		vkCmdBindDescriptorSets(
			frameData.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameData.globalDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameData.gameObjects)
		{
			auto& object = kv.second;
			if (object.model == nullptr) continue;
			SimplePushConstantsData push{};
			push.modelMatrix = object.transform.mat4();
			push.normalMatrix = object.transform.normalMat4();

			vkCmdPushConstants(
				frameData.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantsData),
				&push);

			object.model->bind(frameData.commandBuffer);
			object.model->draw(frameData.commandBuffer);
		}
	}
}
