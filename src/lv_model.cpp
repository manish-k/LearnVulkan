#include "lv_model.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "lv_device.hpp"

#include <cassert>

namespace lv
{
	LvModel::LvModel(LvDevice& device, const std::vector<Vertex>& vertices)
		: device{device}
	{
		createVertexBuffers(vertices);
	}

	LvModel::~LvModel()
	{
		vkDestroyBuffer(device.getLogicalDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.getLogicalDevice(), vertexBufferMemory, nullptr);
	}

	void LvModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());

		assert(vertexCount >= 3 && "vertex count should be at least 3");

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		void* data;
		vkMapMemory(
			device.getLogicalDevice(), 
			vertexBufferMemory, 
			0, 
			bufferSize, 
			0, 
			&data
		);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.getLogicalDevice(), vertexBufferMemory);
	}

	void LvModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = {vertexBuffer};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LvModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> LvModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> LvModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		
		// position attrib description
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		// color attrib description
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		
		return attributeDescriptions;
	}

	std::unique_ptr<LvModel> LvModel::createCubeModel(
		LvDevice& device, glm::vec3 offset)
	{
		std::vector<LvModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LvModel>(device, vertices);
	}

	std::unique_ptr<LvModel> LvModel::createSquareModel(
		LvDevice& device,
		glm::vec3 offset) {
		std::vector<LvModel::Vertex> vertices = {
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LvModel>(device, vertices);
	}

	std::unique_ptr<LvModel> LvModel::createTriangleModel(
		LvDevice& device,
		glm::vec3 offset) {
		std::vector<LvModel::Vertex> vertices = {
			{{.0f, -0.5f, .0f}, {1.f, .0f, .0f}},
			{{0.5f, .0f, .0f}, {.0f, 1.f, .0f}},
			{{-0.5f, .0f, .0f}, {.0f, .0f, 1.f}}
		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LvModel>(device, vertices);
	}

	std::unique_ptr<LvModel> LvModel::createCircleModel(
		LvDevice& device,
		unsigned int numSides,
		glm::vec3 offset)
	{
		std::vector<LvModel::Vertex> uniqueVertices{};
		for (unsigned int i = 1; i <= numSides; i++) {
			float angle = (i - 1) * glm::two_pi<float>() / numSides;
			uniqueVertices.push_back({ 
				{
					glm::cos(angle), 
					glm::sin(angle), 
					.0f
				}
			});
		}
		uniqueVertices.push_back({});  // adds center vertex at 0, 0

		std::vector<LvModel::Vertex> vertices{};
		for (unsigned int i = 0; i <= numSides; i++) {
			vertices.push_back(uniqueVertices[i - 1]);
			vertices.push_back(uniqueVertices[(i) % numSides]);
			vertices.push_back(uniqueVertices[numSides]);
		}

		//for (auto& v : vertices) {
		//	v.position += offset;
		//}
		return std::make_unique<LvModel>(device, vertices);
	}
}