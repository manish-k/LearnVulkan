#pragma once

#include "lv_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace lv
{
	struct Vertex
	{
		glm::vec2 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	class LvModel
	{
	public:
		LvModel(LvDevice& device, const std::vector<Vertex>& vertices);
		~LvModel();

		LvModel(const LvModel&) = delete;
		void operator=(const LvModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		LvDevice& device;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
	};
}
