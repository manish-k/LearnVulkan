#pragma once

#include "lv_device.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace lv
{
	class LvTexture
	{
	public:
		struct Builder
		{
			int texWidth, texHeight, texChannels;
			void* pixels = nullptr;
			void loadTexture(const std::string& filepath);
			void unloadTexture();
		};

	private:
		LvDevice& device;

		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
		VkSampler textureSampler = VK_NULL_HANDLE;

		int width;
		int height;

		void createTextureSampler();

	public:
		LvTexture(LvDevice& device, const Builder& builder);
		~LvTexture();

		LvTexture(const LvTexture&) = delete;
		LvTexture operator=(const LvTexture&) = delete;

		static std::unique_ptr<LvTexture> createTextureFromFile(
			LvDevice& device,
			const std::string& filepath);

		VkImage getImage() const { return textureImage; }
		VkDescriptorImageInfo descriptorInfo();
		void* getMappedMemory() const { return textureImageMemory; }
	};
}