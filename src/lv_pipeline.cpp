#include "lv_pipeline.hpp"

#include "lv_model.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lv
{
	LvPipeline::LvPipeline(
		LvDevice& device,
		const std::string& vertShaderFilepath,
		const std::string& fragShaderFilepath,
		const PipelineConfigInfo& configInfo
	) : device{device}
	{
		createGraphicPipeline(
			vertShaderFilepath, 
			fragShaderFilepath,
			configInfo
		);
	}

	LvPipeline::~LvPipeline()
	{
		vkDestroyShaderModule(device.getLogicalDevice(),
			fragShaderModule,
			nullptr);
		vkDestroyShaderModule(device.getLogicalDevice(),
			vertShaderModule,
			nullptr);
		vkDestroyPipeline(device.getLogicalDevice(), graphicsPipeline, nullptr);
	}

	std::vector<char> LvPipeline::readFile(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file " + filepath);
		}

		size_t filesize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(filesize);

		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();

		return buffer;
	}

	PipelineConfigInfo LvPipeline::defaultPipelineConfigInfo(
		uint32_t width,
		uint32_t height
	)
	{
		PipelineConfigInfo configInfo{};

		configInfo.inputAssemblyInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = 
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		configInfo.dynamicStatesInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStatesInfo.dynamicStateCount = 
			static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStatesInfo.pDynamicStates = 
			configInfo.dynamicStateEnables.data();

		configInfo.rasterizationInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		configInfo.multisampleInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = 
			VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
		configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		configInfo.depthStencilInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};  // Optional
		configInfo.depthStencilInfo.back = {};   // Optional

		return configInfo;
	}

	void LvPipeline::createGraphicPipeline(
		const std::string& vertShaderFilepath,
		const std::string& fragShaderFilepath,
		const PipelineConfigInfo& configInfo
	)
	{
		assert(
			configInfo.renderPass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no renderPass provided in configInfo");
		assert(
			configInfo.pipelineLayout != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no pipeline layout provided in configInfo");

		auto vertShader = readFile(vertShaderFilepath);
		auto fragShader = readFile(fragShaderFilepath);

		createShaderModule(vertShader, &vertShaderModule);
		createShaderModule(fragShader, &fragShaderModule);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = 
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = 
		{ 
			vertShaderStageInfo, fragShaderStageInfo 
		};

		auto bindingDescriptions = Vertex::getBindingDescriptions();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor =
			VK_BLEND_FACTOR_ONE;   // Optional
		colorBlendAttachment.dstColorBlendFactor =
			VK_BLEND_FACTOR_ZERO;  // Optional
		colorBlendAttachment.colorBlendOp =
			VK_BLEND_OP_ADD;       // Optional
		colorBlendAttachment.srcAlphaBlendFactor =
			VK_BLEND_FACTOR_ONE;   // Optional
		colorBlendAttachment.dstAlphaBlendFactor =
			VK_BLEND_FACTOR_ZERO;  // Optional
		colorBlendAttachment.alphaBlendOp =
			VK_BLEND_OP_ADD;       // Optional

		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		colorBlendInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
		colorBlendInfo.pAttachments =
			&colorBlendAttachment;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStatesInfo;

		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.layout = configInfo.pipelineLayout;

		if (vkCreateGraphicsPipelines(
			device.getLogicalDevice(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}

	void LvPipeline::createShaderModule(
		const std::vector<char>& code,
		VkShaderModule* shaderModule
	)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(
			device.getLogicalDevice(), 
			&createInfo, 
			nullptr, 
			shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

	}

	void LvPipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}
}