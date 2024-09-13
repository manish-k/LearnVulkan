#pragma once

#include "lv_device.hpp"
#include "lv_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace lv
{
	class LvModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal &&
					uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		LvModel(LvDevice& device, const LvModel::Builder& builder);
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
		//static std::unique_ptr<LvModel> createCircleModel(
		//	LvDevice& device, 
		//	unsigned int numSides,
		//	glm::vec3 offset);
		static std::unique_ptr<LvModel> createModelFromFile(
			LvDevice& device,
			const std::string& filepath);
	private:
		LvDevice& device;

		std::unique_ptr<LvBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer{ false };
		std::unique_ptr<LvBuffer> indexBuffer;
		uint32_t indexCount;

		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
	};
}
