#include "app.hpp"

#include <array>

namespace lv
{
	App::App()
	{
		loadModel();
		createPipelineLayout();
		createPipeline();
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

	void App::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		if (vkCreatePipelineLayout(
			lvDevice.getLogicalDevice(),
			&pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void App::createPipeline()
	{
		auto pipelineConfig = LvPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT);

		pipelineConfig.renderPass = lvSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;

		lvPipeline = std::make_unique<LvPipeline>(
			lvDevice,
			"shaders/base_vert_shader.vert.spv",
			"shaders/base_frag_shader.frag.spv",
			pipelineConfig);
	}

	void App::createCommandBuffers()
	{
		commandBuffers.resize(lvSwapChain.getImageCount());

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

		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lvSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lvSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = lvSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(
				commandBuffers[i], 
				&renderPassInfo, 
				VK_SUBPASS_CONTENTS_INLINE);

			lvPipeline->bind(commandBuffers[i]);
			lvModel->bind(commandBuffers[i]);

			lvModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void App::drawFrame()
	{
		uint32_t imageIndex;
		VkResult result = lvSwapChain.acquireNextImage(&imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = lvSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void App::loadModel()
	{
		std::vector<Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		lvModel = std::make_unique<LvModel>(lvDevice, vertices);
	}
}