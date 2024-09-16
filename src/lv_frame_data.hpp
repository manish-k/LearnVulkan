#pragma once

#include "lv_camera.hpp"

#include <vulkan/vulkan.h>

namespace lv
{
	struct FrameData
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LvCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};
}