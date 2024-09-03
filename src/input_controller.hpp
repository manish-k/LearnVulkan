#pragma once

#include "lv_game_object.hpp"
#include "lv_window.hpp"

#include <glm/gtc/constants.hpp>

namespace lv
{
	class InputController
	{
	public:
		struct InputMappings
		{
			int keyboardMoveLeft = GLFW_KEY_A;
			int keyboardMoveRight = GLFW_KEY_D;
			int keyboardMoveForward = GLFW_KEY_W;
			int keyboardMoveBackward = GLFW_KEY_S;
			int keyboardMoveUp = GLFW_KEY_E;
			int keyboardMoveDown = GLFW_KEY_Q;
			int keyboardLookLeft = GLFW_KEY_LEFT;
			int keyboardLookRight = GLFW_KEY_RIGHT;
			int keyboardLookUp = GLFW_KEY_UP;
			int keyboardLookDown = GLFW_KEY_DOWN;
		};

		// left, right, forward, backward moves will happen
		// in XZ plane
		void updateInPlaneXZ(
			GLFWwindow* window, float dt, LvGameObject& gameObject);

		InputMappings keyMap{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
		double cursorPosX, cursorPosY;
	};
}