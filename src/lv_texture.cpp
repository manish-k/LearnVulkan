#include "lv_texture.hpp"
#include "lv_buffer.hpp"

namespace lv
{
	void LvTexture::Builder::loadTexture(const std::string& filepath)
	{
		pixels = stbi_load(
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
		stbi_image_free(pixels);
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
	}

	LvTexture::~LvTexture()
	{
		VkDevice vkDevice = device.getLogicalDevice();
		
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
}