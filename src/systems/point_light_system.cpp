#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>

namespace lv
{
	PointLightSystem::PointLightSystem(
		LvDevice& device,
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout)
		: lvDevice{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(lvDevice.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void PointLightSystem::createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout)
	{
		//VkPushConstantRange pushConstantRange{};
		//pushConstantRange.stageFlags =
		//	VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		//pushConstantRange.offset = 0;
		//pushConstantRange.size = sizeof(SimplePushConstantsData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount =
			static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges =nullptr;
		if (vkCreatePipelineLayout(
			lvDevice.getLogicalDevice(),
			&pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		LvPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		lvPipeline = std::make_unique<LvPipeline>(
			lvDevice,
			"shaders/point_light.vert.spv",
			"shaders/point_light.frag.spv",
			pipelineConfig);
	}

	void PointLightSystem::render(FrameData& frameData)
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

		vkCmdDraw(frameData.commandBuffer, 6, 1, 0, 0);
	}
}
