#include "lv_model.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "lv_device.hpp"
#include "lv_utils.hpp"

#include <cassert>
#include <unordered_map>

// injecting hash function for Vertex into std namespace
namespace std {
	template <>
	struct hash<lv::LvModel::Vertex> {
		size_t operator()(lv::LvModel::Vertex const& vertex) const {
			size_t seed = 0;
			lv::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace lv
{
	LvModel::LvModel(LvDevice& device, const LvModel::Builder& builder)
		: device{device}
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	LvModel::~LvModel()
	{

	}

	std::unique_ptr<LvModel> LvModel::createModelFromFile(
		LvDevice& device,
		const std::string& filepath)
	{
		Builder modelBuilder{};
		modelBuilder.loadModel(filepath);

		return std::make_unique<LvModel>(device, modelBuilder);
	}

	void LvModel::Builder::loadModel(const std::string& filepath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(
			&attrib, 
			&shapes, 
			&materials, 
			&warn, 
			&err, 
			filepath.c_str())) 
		{
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2
						]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2
						]
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void LvModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());

		assert(vertexCount >= 3 && "vertex count should be at least 3");

		uint32_t vertexSize = sizeof(vertices[0]);
		VkDeviceSize bufferSize = vertexSize * vertexCount;

		LvBuffer stagingBuffer(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());
		
		vertexBuffer = std::make_unique<LvBuffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		device.copyBuffer(
			stagingBuffer.getBuffer(), 
			vertexBuffer->getBuffer(),
			bufferSize);
	}

	void LvModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		uint32_t indexSize = sizeof(indices[0]);
		VkDeviceSize bufferSize = indexSize * indexCount;

		LvBuffer stagingBuffer(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*) indices.data());

		indexBuffer = std::make_unique<LvBuffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


		device.copyBuffer(
			stagingBuffer.getBuffer(), 
			indexBuffer->getBuffer(), 
			bufferSize);

	}

	void LvModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = {vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(
				commandBuffer, 
				indexBuffer->getBuffer(), 
				0, 
				VK_INDEX_TYPE_UINT32);
		}
	}

	void LvModel::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		
		attributeDescriptions.push_back(
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back(
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back(
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back(
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
		
		return attributeDescriptions;
	}

	std::unique_ptr<LvModel> LvModel::createCubeModel(
		LvDevice& device,
		glm::vec3 offset)
	{
		LvModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
		};
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<LvModel>(device, modelBuilder);
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

		LvModel::Builder modelBuilder{};
		modelBuilder.vertices = vertices;
		
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}
		return std::make_unique<LvModel>(device, modelBuilder);
	}

	std::unique_ptr<LvModel> LvModel::createTriangleModel(
		LvDevice& device,
		glm::vec3 offset) {
		std::vector<LvModel::Vertex> vertices = {
			{{.0f, -0.5f, .0f}, {1.f, .0f, .0f}},
			{{0.5f, .0f, .0f}, {.0f, 1.f, .0f}},
			{{-0.5f, .0f, .0f}, {.0f, .0f, 1.f}}
		};
		LvModel::Builder modelBuilder{};
		modelBuilder.vertices = vertices;

		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}
		return std::make_unique<LvModel>(device, modelBuilder);
	}

	//std::unique_ptr<LvModel> LvModel::createCircleModel(
	//	LvDevice& device,
	//	unsigned int numSides,
	//	glm::vec3 offset)
	//{
	//	std::vector<LvModel::Vertex> uniqueVertices{};
	//	for (unsigned int i = 1; i <= numSides; i++) {
	//		float angle = (i - 1) * glm::two_pi<float>() / numSides;
	//		uniqueVertices.push_back({ 
	//			{
	//				glm::cos(angle), 
	//				glm::sin(angle), 
	//				.0f
	//			}
	//		});
	//	}
	//	uniqueVertices.push_back({});  // adds center vertex at 0, 0

	//	std::vector<LvModel::Vertex> vertices{};
	//	for (unsigned int i = 0; i <= numSides; i++) {
	//		vertices.push_back(uniqueVertices[i - 1]);
	//		vertices.push_back(uniqueVertices[(i) % numSides]);
	//		vertices.push_back(uniqueVertices[numSides]);
	//	}

	//	//for (auto& v : vertices) {
	//	//	v.position += offset;
	//	//}
	//	return std::make_unique<LvModel>(device, vertices);
	//}
}