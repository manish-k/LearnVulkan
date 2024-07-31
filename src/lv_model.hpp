#pragma once

#include "lv_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace lv
{


	class LvModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec2 texCoord;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		LvModel(LvDevice& device, const std::vector<Vertex>& vertices);
		~LvModel();

		LvModel(const LvModel&) = delete;
		LvModel& operator=(const LvModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		static std::unique_ptr<LvModel> createCubeModel(
			LvDevice& device, 
			glm::vec3 offset);
		static std::unique_ptr<LvModel> createSquareModel(
			LvDevice& device, 
			glm::vec3 offset);
		static std::unique_ptr<LvModel> createTriangleModel(
			LvDevice& device, 
			glm::vec3 offset);
		static std::unique_ptr<LvModel> createCircleModel(
			LvDevice& device, 
			unsigned int numSides,
			glm::vec3 offset);
	private:
		LvDevice& device;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
	};
}
