#include "input_controller.hpp"

#include <limits>

namespace lv
{
	void InputController::updateInPlaneXZ(
		GLFWwindow* window, float dt, LvGameObject& gameObject)
	{
		glm::vec3 rotate{0};
		
		if (glfwGetKey(window, keyMap.keyboardLookRight))
			rotate.y += 1.f;
		if (glfwGetKey(window, keyMap.keyboardLookLeft))
			rotate.y -= 1.f;
		if (glfwGetKey(window, keyMap.keyboardLookUp))
			rotate.x += 1.f;
		if (glfwGetKey(window, keyMap.keyboardLookDown))
			rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// 1.5 randians = ~86 degrees, clamping pitch
		gameObject.transform.rotation.x = glm::clamp(
			gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(
			gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(window, keyMap.keyboardMoveForward) == GLFW_PRESS) 
			moveDir += forwardDir;
		if (glfwGetKey(window, keyMap.keyboardMoveBackward) == GLFW_PRESS) 
			moveDir -= forwardDir;
		if (glfwGetKey(window, keyMap.keyboardMoveRight) == GLFW_PRESS) 
			moveDir += rightDir;
		if (glfwGetKey(window, keyMap.keyboardMoveLeft) == GLFW_PRESS) 
			moveDir -= rightDir;
		if (glfwGetKey(window, keyMap.keyboardMoveUp) == GLFW_PRESS) 
			moveDir += upDir;
		if (glfwGetKey(window, keyMap.keyboardMoveDown) == GLFW_PRESS) 
			moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}
}