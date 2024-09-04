#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "simple_render_system.hpp"
#include "input_controller.hpp"

#include <array>
#include <chrono>

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
		LvCamera camera{};

		auto viewerObject = LvGameObject::createGameObject();
		InputController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!lvWindow.shouldClose())
		{
			lvWindow.pollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			if (auto commandBuffer = lvRenderer.beginFrame())
			{
				float aspect = lvRenderer.getAspectRatio();
				camera.setPerspectiveProjection(
					glm::radians(50.f), aspect, 0.5f, 10.f);
				camera.setViewTarget(
					glm::vec3(-1.f, -2.f, -2.f), 
					glm::vec3(0.f, 0.f, 2.5f));

				cameraController.updateInPlaneXZ(
					lvWindow.getGLFWwindow(), frameTime, viewerObject);
				camera.setViewYXZ(
					viewerObject.transform.translation,
					viewerObject.transform.rotation);

				lvRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(
					commandBuffer, 
					gameObjects, 
					camera);
				lvRenderer.endSwapChainRenderPass(commandBuffer);
				lvRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lvDevice.getLogicalDevice());
	}

	void App::loadGameObjects()
	{
		std::shared_ptr<LvModel> cubeModel = LvModel::createCubeModel(
			lvDevice, 
			{ .0f, .0f, .0f });
		auto cube = LvGameObject::createGameObject();
		cube.model = cubeModel;
		cube.transform.translation = { .0f, .0f, 2.5f };
		cube.transform.scale = { .5f, .5f, .5f };
		gameObjects.push_back(std::move(cube));
	}
}