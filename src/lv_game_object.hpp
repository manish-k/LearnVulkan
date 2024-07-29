#pragma once

#include "lv_model.hpp"

namespace lv
{
	struct Transform2dComponent
	{
		glm::vec2 translation{};
		glm::vec2 scale{1.f, 1.f};
		float rotation;

		glm::mat2 mat2()
		{
			const float cos = glm::cos(rotation);
			const float sin = glm::sin(rotation);

			glm::mat2 rotMat{ {cos, sin}, {-sin, cos} };
			glm::mat2 scaleMat{ {scale.x, 0.f}, {0.f, scale.y} };

			return rotMat * scaleMat;
		}
	};

	class LvGameObject
	{
		using id_t = unsigned int;

	private:
		id_t id;
		
		LvGameObject(id_t objectId) : id{ objectId } {};

	public:
		Transform2dComponent transform{};
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
	};
}