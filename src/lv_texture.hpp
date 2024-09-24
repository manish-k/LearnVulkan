#pragma once

#include "lv_device.hpp"

#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <memory>

namespace lv
{
	class LvTexture
	{
	public:
		struct Builder
		{
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = nullptr;
			void loadTexture(const std::string& filepath);
			void unloadTexture();
		};

	private:
		LvDevice& device;

		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;

		int width;
		int height;

	public:
		LvTexture(LvDevice& device, const Builder& builder);
		~LvTexture();

		LvTexture(const LvTexture&) = delete;
		LvTexture operator=(const LvTexture&) = delete;

		static std::unique_ptr<LvTexture> createTextureFromFile(
			LvDevice& device,
			const std::string& filepath);

		VkImage getImage() const { return textureImage; }
		void* getMappedMemory() const { return textureImageMemory; }
	};
}