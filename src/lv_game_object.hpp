#pragma once

#include "lv_model.hpp"
#include "lv_texture.hpp"

#include <unordered_map>

namespace lv
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation;

		glm::mat4 mat4();
		glm::mat4 normalMat4();
	};

	struct PointLightComponent
	{
		float lightIntensity = 1.0f;
	};

	class LvGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, LvGameObject>;

	private:
		id_t id;
		
		LvGameObject(id_t objectId) : id{ objectId } {};

	public:
		TransformComponent transform{};
		std::shared_ptr<LvModel> model{};
		std::shared_ptr<LvTexture> texture{};
		glm::vec3 color{};

		std::unique_ptr<PointLightComponent> pointLight = nullptr;

		static LvGameObject createGameObject()
		{
			static int currentId = 0;
			return LvGameObject(currentId++);
		}

		static LvGameObject makePointLight(
			float intensity = 10.f, 
			float radius = 0.1f, 
			glm::vec3 color = glm::vec3(1.f));

		LvGameObject(const LvGameObject&) = delete;
		LvGameObject& operator=(const LvGameObject&) = delete;
		LvGameObject(LvGameObject&&) = default;
		LvGameObject& operator=(LvGameObject&&) = default;

		const id_t getId() { return id; };

	};
}