#pragma once

#include "lv_camera.hpp"
#include "lv_game_object.hpp"

#include <vulkan/vulkan.h>

namespace lv
{
#define MAX_POINT_LIGHTS 10

	struct PointLight
	{
		glm::vec4 position{};
		glm::vec4 color{}; // w is for intensity
	};

	struct GlobalUbo
	{
		glm::mat4 prjoection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, 0.2f }; // w is intensity
		PointLight pointLights[MAX_POINT_LIGHTS];
		int numLights;
	};

	struct FrameData
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LvCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		LvGameObject::Map& gameObjects;
	};
}