#pragma once

#include "lv_window.hpp"
#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_game_object.hpp"
#include "lv_renderer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>

namespace lv
{
	struct GlobalUbo
	{
		glm::mat4 prjoectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(
			glm::vec3{ 1.f, -3.f, -1.f });
	};

	class App
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

	private:
		LvWindow lvWindow{ "The Vulkan", WIDTH, HEIGHT};
		LvDevice lvDevice{lvWindow};
		LvRenderer lvRenderer{ lvWindow, lvDevice };

		std::vector<LvGameObject> gameObjects;

	public:
		App();
		~App();

		void run();

	private:
		void loadGameObjects();
	};
}