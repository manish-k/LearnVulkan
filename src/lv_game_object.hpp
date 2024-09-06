#pragma once

#include "lv_model.hpp"

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

	class LvGameObject
	{
	public:
		using id_t = unsigned int;

	private:
		id_t id;
		
		LvGameObject(id_t objectId) : id{ objectId } {};

	public:
		TransformComponent transform{};
		std::shared_ptr<LvModel> model{};
		glm::vec3 color{};

		static LvGameObject createGameObject()
		{
			static int currentId = 0;
			return LvGameObject(currentId++);
		}

		LvGameObject(const LvGameObject&) = delete;
		LvGameObject& operator=(const LvGameObject&) = delete;
		LvGameObject(LvGameObject&&) = default;
		LvGameObject& operator=(LvGameObject&&) = default;

		const id_t getId() { return id; };
	};
}