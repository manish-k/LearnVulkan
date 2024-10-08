#include "app.hpp"

#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "input_controller.hpp"

#include <array>
#include <chrono>

namespace lv
{
	App::App()
	{
		globalDescriptorPool = LvDescriptorPool::Builder(lvDevice)
			.setMaxSets(LvSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				LvSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		std::vector<std::unique_ptr<LvBuffer>> 
			uboBuffers(LvSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<LvBuffer>(
				lvDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = 
			LvDescriptorSetLayout::Builder(lvDevice)
			.addBinding(
				0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		// TODO: Do we need abstraction on VKDescriptorSet?
		std::vector<VkDescriptorSet>
			globalDescriptorSets(LvSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); ++i)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LvDescriptorWriter(*globalSetLayout, *globalDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem
		{
			lvDevice, 
			lvRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};
		PointLightSystem pointLightSystem
		{
			lvDevice,
			lvRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()
		};

		LvCamera camera{};

		auto viewerObject = LvGameObject::createGameObject();
		viewerObject.transform.translation = glm::vec3(0.f, -0.5f, -5.5f);
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
				glm::radians(50.f), aspect, 0.5f, 100.f);

			if (auto commandBuffer = lvRenderer.beginFrame())
			{
				int frameIndex = lvRenderer.getFrameIndex();

				FrameData frameData
				{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				GlobalUbo ubo{};
				ubo.prjoection = camera.getProjection();
				ubo.view = camera.getView();
				pointLightSystem.update(frameData, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				lvRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameData);
				pointLightSystem.render(frameData);
				lvRenderer.endSwapChainRenderPass(commandBuffer);
				lvRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lvDevice.getLogicalDevice());
	}

	void App::loadGameObjects()
	{
		std::shared_ptr<LvTexture> defaultTexture =
			LvTexture::createTextureFromFile(
				lvDevice,
				"textures/default.png"
			);

		std::shared_ptr<LvTexture> statueTexture =
			LvTexture::createTextureFromFile(
				lvDevice,
				"textures/statue.png"
			);

		std::shared_ptr<LvTexture> woodTexture =
			LvTexture::createTextureFromFile(
				lvDevice,
				"textures/wood.png"
			);

		/*
		std::shared_ptr<LvModel> model1 = LvModel::createModelFromFile(
			lvDevice, 
			"models/flat_vase.obj"
		);
		auto gameObj1 = LvGameObject::createGameObject();
		gameObj1.model = model1;
		gameObj1.transform.translation = { -1.f, .5f, 0.f };
		gameObj1.transform.scale = glm::vec3{3.5f};
		gameObjects.emplace(
			gameObj1.getId(), std::move(gameObj1));

		std::shared_ptr<LvModel> model2 = LvModel::createModelFromFile(
			lvDevice,
			"models/smooth_vase.obj"
		);
		auto gameObj2 = LvGameObject::createGameObject();
		gameObj2.model = model2;
		gameObj2.transform.translation = { 1.0f, .5f, 0.f };
		gameObj2.transform.scale = glm::vec3{ 3.5f };
		gameObjects.emplace(
			gameObj2.getId(), std::move(gameObj2));

		std::shared_ptr<LvModel> model3 = LvModel::createModelFromFile(
			lvDevice,
			"models/smooth_vase.obj"
		);
		auto gameObj3 = LvGameObject::createGameObject();
		gameObj3.model = model3;
		gameObj3.transform.translation = { 0.f, .5f, -1.5f };
		gameObj3.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(
			gameObj3.getId(), std::move(gameObj3));

		std::shared_ptr<LvModel> model4 = LvModel::createModelFromFile(
			lvDevice,
			"models/quad.obj"
		);
		auto floor = LvGameObject::createGameObject();
		floor.model = model4;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(
			floor.getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  //
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = LvGameObject::makePointLight(0.5f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -0.5f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
		*/

		std::shared_ptr<LvModel> roomModel = 
			LvModel::createModelFromFile(
				lvDevice,
				"models/viking_room.obj"
			);
		std::shared_ptr<LvTexture> roomTexture = 
			LvTexture::createTextureFromFile(
				lvDevice,
				"textures/viking_room.png"
			);

		auto room = LvGameObject::createGameObject();
		room.model = roomModel;
		room.texture = roomTexture;
		room.transform.translation = { 0.f, 0.f, 0.f };
		room.transform.scale = glm::vec3{ 2.f };
		room.transform.rotation = {
			glm::half_pi<float>(),
			glm::half_pi<float>(), 
			0.f };
		gameObjects.emplace(
			room.getId(), std::move(room));
	}
}