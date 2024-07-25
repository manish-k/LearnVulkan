#pragma once

#include "vulkan/vulkan.h"

#include "lv_device.hpp"

#include <string>
#include <vector>

namespace lv
{
	struct PipelineConfigInfo {
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportState;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStatesInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkRenderPass renderPass = nullptr;
		VkPipelineLayout pipelineLayout = nullptr;
		uint32_t subpass = 0;
	};

	class LvPipeline
	{
	private:
		LvDevice& device;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule, fragShaderModule;
	public:
		LvPipeline(
			LvDevice& device,
			const std::string& vertShaderFilepath,
			const std::string& fragShaderFilepath,
			const PipelineConfigInfo& configInfo
		);
		~LvPipeline();

		LvPipeline(const LvPipeline&) = delete;
		LvPipeline& operator=(const LvPipeline&) = delete;

		static void defaultPipelineConfigInfo(
			uint32_t width,
			uint32_t height,
			PipelineConfigInfo& config);

		void bind(VkCommandBuffer commandBuffer);

	private:
		static std::vector<char> readFile(const std::string& filepath);
		
		void createGraphicPipeline(
			const std::string& vertShaderFilepath,
			const std::string& fragShaderFilepath,
			const PipelineConfigInfo& configInfo
		);

		void createShaderModule(
			const std::vector<char>& code,
			VkShaderModule* shaderModule
		);
	};
}