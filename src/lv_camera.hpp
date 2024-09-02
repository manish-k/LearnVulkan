#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lv
{
	class LvCamera
	{
	private:
		glm::mat4 projectionMatrix{ 1.0f };
		glm::mat4 viewMatrix{ 1.0f };

	public:
		void setOrthographicProjection(
			float left,
			float right,
			float top,
			float bottom,
			float near,
			float far
		);
		void setPerspectiveProjection(
			float fovy,
			float aspect,
			float near,
			float far
		);

		void setViewDirection(
			glm::vec3 position, 
			glm::vec3 direction, 
			glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }
		);
		void setViewTarget(
			glm::vec3 position, 
			glm::vec3 target, 
			glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }
		);

		// Tait-bryan YXZ rotation for camera
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4 getProjection() const 
		{
			return projectionMatrix;
		};

		const glm::mat4 getView() const
		{
			return viewMatrix;
		};
	};
}