#include "lv_renderer.hpp"


#include <array>

namespace lv
{
	LvRenderer::LvRenderer(LvWindow& window, LvDevice& device)
		: lvWindow{window}, lvDevice{device}
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	LvRenderer::~LvRenderer()
	{
		freeCommandBuffers();
	}


	void LvRenderer::recreateSwapChain()
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
			std::shared_ptr<LvSwapChain> oldSwapChain = std::move(lvSwapChain);
			lvSwapChain =
				std::make_unique<LvSwapChain>(
					lvDevice,
					lvWindow,
					oldSwapChain
				);

			if (!oldSwapChain->compareSwapFormats(*lvSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void LvRenderer::createCommandBuffers()
	{
		commandBuffers.resize(LvSwapChain::MAX_FRAMES_IN_FLIGHT);

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

	void LvRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			lvDevice.getLogicalDevice(),
			lvDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void LvRenderer::recreateCommandBuffers()
	{
		freeCommandBuffers();
		createCommandBuffers();
	}

	VkCommandBuffer LvRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Can't call begin frame when already in progress");
		VkResult result = lvSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			recreateCommandBuffers();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void LvRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = lvSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

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

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % 
			LvSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LvRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't begin render pass while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lvSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lvSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lvSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(
			commandBuffer,
			&renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lvSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lvSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = lvSwapChain->getSwapChainExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void LvRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call end swap chain while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}