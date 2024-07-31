#pragma once

#include "lv_window.hpp"
#include "lv_device.hpp"
#include "lv_pipeline.hpp"
#include "lv_swapchain.hpp"
#include "lv_game_object.hpp"
#include "lv_renderer.hpp"

#include <memory>
#include <vector>

namespace lv
{
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