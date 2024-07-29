#include "app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>

namespace lv
{
	App::App()
	{
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	App::~App()
	{
		vkDestroyPipelineLayout(lvDevice.getLogicalDevice(), pipelineLayout, nullptr);
	}

	void App::run()
	{
		while (!lvWindow.shouldClose())
		{
			lvWindow.pollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(lvDevice.getLogicalDevice());
	}

	void App::recreateSwapChain()
	{
		auto extent = lvWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = lvWindow.getExtent();
			lvWindow.waitEvents();
		}

		vkDeviceWaitIdle(lvDevice.getLogicalDevice());

		if (lvSwapChain == nullptr) {
			lvSwapChain = std::make_unique<LvSwapChain>(lvDevice, lvWindow);
		}
		else {
			lvSwapChain = 
				std::make_unique<LvSwapChain>(
					lvDevice, 
					lvWindow,
					std::move(lvSwapChain)
				);
			if (lvSwapChain->getImageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();
	}


	void App::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = 
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantsData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(
			lvDevice.getLogicalDevice(),
			&pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void App::createPipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		LvPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT, pipelineConfig);

		pipelineConfig.renderPass = lvSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		lvPipeline = std::make_unique<LvPipeline>(
			lvDevice,
			"shaders/base_vert_shader.vert.spv",
			"shaders/base_frag_shader.frag.spv",
			pipelineConfig);
	}

	void App::createCommandBuffers()
	{
		commandBuffers.resize(lvSwapChain->getImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lvDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lvDevice.getLogicalDevice(),
			&allocInfo,
			commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void App::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			lvDevice.getLogicalDevice(),
			lvDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void App::recreateCommandBuffers()
	{
		freeCommandBuffers();
		createCommandBuffers();
	}

	void App::recordCommandBuffers(uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lvSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lvSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lvSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(
			commandBuffers[imageIndex],
			&renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		lvPipeline->bind(commandBuffers[imageIndex]);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lvSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lvSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = lvSwapChain->getSwapChainExtent();
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void App::drawFrame()
	{
		uint32_t imageIndex;
		VkResult result = lvSwapChain->acquireNextImage(&imageIndex);
		
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			recreateCommandBuffers();
			return;
		}
		
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		recordCommandBuffers(imageIndex);

		result = lvSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR ||
			result == VK_SUBOPTIMAL_KHR ||
			lvWindow.isResized())
		{
			lvWindow.resetResizedState(); 
			recreateSwapChain();
			recreateCommandBuffers();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void App::loadGameObjects()
	{
		std::vector<LvModel::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto baseModel = std::make_shared<LvModel>(lvDevice, vertices);
		auto triangle = LvGameObject::createGameObject();
		triangle.color = {1.f, 0.f, 0.f};
		triangle.transform.scale = { 1.5f, 1.4f };
		triangle.transform.translation = {0.5f, 0.f};
		triangle.transform.rotation = glm::two_pi<float>() * 0.25f;
		triangle.model = baseModel;

		gameObjects.push_back(std::move(triangle));
	}

	void App::renderGameObjects(VkCommandBuffer commandBuffer)
	{
		for (auto& object : gameObjects)
		{
			object.model->bind(commandBuffer);

			SimplePushConstantsData push{};
			push.offset = object.transform.translation;
			push.color = object.color;
			push.transform = object.transform.mat2();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantsData),
				&push);

			object.model->draw(commandBuffer);
		}
	}
}