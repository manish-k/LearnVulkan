#include "simple_render_system.hpp"
#include "lv_descriptor.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>

namespace lv
{
	SimpleRenderSystem::SimpleRenderSystem(
		LvDevice& device, 
		VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout)
		: lvDevice{ device }
	{
		localDescriptorPool = LvDescriptorPool::Builder(device)
			.setMaxSets(LvSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				LvSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		localDescriptorSetLayout = LvDescriptorSetLayout::Builder(device)
			.addBinding(
				0,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		localDescriptorSets.resize(LvSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < localDescriptorSets.size(); ++i)
		{
			// TODO check status of build function
			LvDescriptorWriter(
				*localDescriptorSetLayout, 
				*localDescriptorPool
			).build(localDescriptorSets[i]);
		}

		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(
			lvDevice.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(
		VkDescriptorSetLayout globalSetLayout)
	{
		assert(localDescriptorSetLayout != VK_NULL_HANDLE 
			&& "local descriptor set layout is not present");

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantsData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout,
			localDescriptorSetLayout->getDescriptorSetLayout()
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

			if (object.texture != nullptr)
			{
				auto imageInfo = object.texture->descriptorInfo();
				LvDescriptorWriter(
					*localDescriptorSetLayout,
					*localDescriptorPool)
					.writeImage(
						0, 
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
						&imageInfo)
					.overwrite(localDescriptorSets[frameData.frameIndex]);

				vkCmdBindDescriptorSets(
					frameData.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					1,
					1,
					&localDescriptorSets[frameData.frameIndex],
					0,
					nullptr);
			}

			object.model->bind(frameData.commandBuffer);
			object.model->draw(frameData.commandBuffer);
		}
	}
}
