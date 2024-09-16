#include "app.hpp"

#include "simple_render_system.hpp"
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
				VK_SHADER_STAGE_VERTEX_BIT)
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

			if (auto commandBuffer = lvRenderer.beginFrame())
			{
				int frameIndex = lvRenderer.getFrameIndex();

				FrameData frameData
				{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				GlobalUbo ubo{};
				ubo.prjoectionView = 
					camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				lvRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(
					frameData, 
					gameObjects);
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