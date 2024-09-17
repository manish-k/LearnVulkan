#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <array>

namespace lv
{
	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

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
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags =
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

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
		pipelineLayoutInfo.pPushConstantRanges =&pushConstantRange;
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

	void PointLightSystem::update(FrameData& frameData, GlobalUbo& ubo)
	{
		auto rotateLight = glm::rotate(
			glm::mat4(1.f), 
			0.5f * frameData.frameTime, // angle
			{ 0.f, -1.f, 0.f });        // axis
		int lightIndex = 0;
		for (auto& kv : frameData.gameObjects)
		{
			auto& gameObject = kv.second;
			if (gameObject.pointLight == nullptr) continue;

			assert(lightIndex < MAX_POINT_LIGHTS &&
				"point lights exceede limits");

			//animation
			gameObject.transform.translation = glm::vec3(
					rotateLight * 
					glm::vec4(gameObject.transform.translation, 1.f));

			ubo.pointLights[lightIndex].position = 
				glm::vec4(gameObject.transform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(
				gameObject.color, gameObject.pointLight->lightIntensity);

			lightIndex += 1;
		}

		ubo.numLights = lightIndex;
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

		for (auto& kv : frameData.gameObjects)
		{
			auto& gameObject = kv.second;
			if (gameObject.pointLight == nullptr) continue;

			PointLightPushConstants push{};
			push.position = glm::vec4(gameObject.transform.translation, 1.f);
			push.color = glm::vec4(
				gameObject.color, 
				gameObject.pointLight->lightIntensity);
			push.radius = gameObject.transform.scale.x;

			vkCmdPushConstants(
				frameData.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);
			vkCmdDraw(frameData.commandBuffer, 6, 1, 0, 0);
		}
	}
}
