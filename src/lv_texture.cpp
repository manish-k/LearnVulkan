#include "lv_texture.hpp"
#include "lv_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace lv
{
	void LvTexture::Builder::loadTexture(const std::string& filepath)
	{
		pixels = (void*)stbi_load(
			filepath.c_str(),
			&texWidth, 
			&texHeight, 
			&texChannels, 
			STBI_rgb_alpha);

		if (!pixels)
			throw std::runtime_error("unable to load texture " + filepath);
	}

	void LvTexture::Builder::unloadTexture()
	{
		stbi_image_free((stbi_uc*)pixels);
	}

	LvTexture::LvTexture(LvDevice& device, const Builder& builder)
		: device{device}
	{
		width = builder.texWidth;
		height = builder.texHeight;

		uint32_t imageSize = width * height * 4;

		LvBuffer stagingBuffer(
			device,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)builder.pixels);

		device.createImage(
			width,
			height,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage,
			textureImageMemory);

		device.transitionImageWithLayout(
			textureImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		
		device.copyBufferToImage(
			stagingBuffer.getBuffer(),
			textureImage,
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height));

		device.transitionImageWithLayout(
			textureImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		textureImageView = 
			device.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);

		createTextureSampler();
	}

	LvTexture::~LvTexture()
	{
		VkDevice vkDevice = device.getLogicalDevice();
		
		vkDestroySampler(vkDevice, textureSampler, nullptr);
		vkDestroyImageView(vkDevice, textureImageView, nullptr);
		vkDestroyImage(vkDevice, textureImage, nullptr);
		vkFreeMemory(vkDevice, textureImageMemory, nullptr);
	}

	std::unique_ptr<LvTexture> LvTexture::createTextureFromFile(
		LvDevice& device,
		const std::string& filepath)
	{
		Builder textureBuilder{};
		textureBuilder.loadTexture(filepath);

		return std::make_unique<LvTexture>(device, textureBuilder);
	}

	void LvTexture::createTextureSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(
			device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(
			device.getLogicalDevice(),
			&samplerInfo, 
			nullptr, 
			&textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler");
		}
	}

	VkDescriptorImageInfo LvTexture::descriptorInfo()
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
		return imageInfo;
	}
}