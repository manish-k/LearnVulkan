#include "app.hpp"

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
		std::vector<std::unique_ptr<LvBuffer>> uboBuffers(LvSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<LvBuffer>(
				lvDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

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

			cameraController.updateInPlaneXZ(
				lvWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(
				viewerObject.transform.translation,
				viewerObject.transform.rotation);

			float aspect = lvRenderer.getAspectRatio();
			camera.setPerspectiveProjection(
				glm::radians(50.f), aspect, 0.5f, 10.f);
			//camera.setViewTarget(
			//	glm::vec3(-1.f, -2.f, -2.f),
			//	glm::vec3(0.f, 0.f, 2.5f));

			if (auto commandBuffer = lvRenderer.beginFrame())
			{
				int frameIndex = lvRenderer.getFrameIndex();

				GlobalUbo ubo{};
				ubo.prjoectionView = 
					camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

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
		std::shared_ptr<LvModel> model1 = LvModel::createModelFromFile(
			lvDevice, 
			"models/flat_vase.obj"
		);
		auto gameObj1 = LvGameObject::createGameObject();
		gameObj1.model = model1;
		gameObj1.transform.translation = { -1.f, .5f, 2.5f };
		gameObj1.transform.scale = glm::vec3{3.5f};
		gameObjects.push_back(std::move(gameObj1));

		std::shared_ptr<LvModel> model2 = LvModel::createModelFromFile(
			lvDevice,
			"models/smooth_vase.obj"
		);
		auto gameObj2 = LvGameObject::createGameObject();
		gameObj2.model = model2;
		gameObj2.transform.translation = { .0f, .5f, 2.5f };
		gameObj2.transform.scale = glm::vec3{ 3.5f };
		gameObjects.push_back(std::move(gameObj2));

		std::shared_ptr<LvModel> model3 = LvModel::createModelFromFile(
			lvDevice,
			"models/smooth_vase.obj"
		);
		auto gameObj3 = LvGameObject::createGameObject();
		gameObj3.model = model3;
		gameObj3.transform.translation = { 1.f, .5f, 2.5f };
		gameObj3.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.push_back(std::move(gameObj3));
	}
}