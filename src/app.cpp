#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "simple_render_system.hpp"

#include <array>

namespace lv
{
	App::App()
	{
		loadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		SimpleRenderSystem simpleRenderSystem{lvDevice, lvRenderer.getSwapChainRenderPass()};

		while (!lvWindow.shouldClose())
		{
			lvWindow.pollEvents();
			if (auto commandBuffer = lvRenderer.beginFrame())
			{
				lvRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lvRenderer.endSwapChainRenderPass(commandBuffer);
				lvRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lvDevice.getLogicalDevice());
	}

	void App::loadGameObjects()
	{
		std::shared_ptr<LvModel> cubeModel = LvModel::createCubeModel(
			lvDevice, { .0f, .0f, .0f });
		auto cube = LvGameObject::createGameObject();
		cube.model = cubeModel;
		cube.transform.translation = { .0f, .0f, .5f };
		cube.transform.scale = { .5f, .5f, .5f };
		gameObjects.push_back(std::move(cube));

		std::shared_ptr<LvModel> squareModel = LvModel::createSquareModel(
			lvDevice, { .0f, .0f, .0f });
		auto square = LvGameObject::createGameObject();
		square.model = squareModel;
		square.transform.translation = { .0f, .0f, .5f };
		square.transform.scale = { .5f, .5f, .5f };
		//gameObjects.push_back(std::move(square));
	}
}