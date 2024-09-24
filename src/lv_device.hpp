#pragma once

#include <vulkan/vulkan.h>

#include "lv_window.hpp"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>

namespace lv
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() 
				&& presentFamily.has_value();
		}
	};

	class LvDevice
	{
	private:
		VkInstance vkInstance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSurfaceKHR surface;
		VkCommandPool commandPool;

		const std::vector<const char*> deviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		const std::vector<const char*> validationLayers {
			"VK_LAYER_KHRONOS_validation"
		};

	public:
		LvDevice(LvWindow& window);
		~LvDevice();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		};

		VkDevice getLogicalDevice() { return device; };
		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; };
		VkSurfaceKHR getSurface() { return surface; };
		VkQueue getGraphicsQueue() { return graphicsQueue; };
		VkQueue getPresentQueue() { return presentQueue; };
		VkCommandPool getCommandPool() { return commandPool; };

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamily(VkPhysicalDevice device);
		void createBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer &buffer,
			VkDeviceMemory &bufferMemory
		);
		void createImage(
			uint32_t width,
			uint32_t height,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);
		void transitionImageWithLayout(
			VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout);
		VkFormat findSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		void copyBuffer(
			VkBuffer srcBuffer,
			VkBuffer dstBuffer,
			VkDeviceSize size);
		void copyBufferToImage(
			VkBuffer buffer,
			VkImage image,
			uint32_t width, 
			uint32_t height);

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	private:
		void createVulkanInstance();
		void cleanup();
		
		bool isValidationLayerEnabled();
		bool checkValidationLayerSupport();
		
		std::vector<const char*> getRequiredExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		
		void setupDebugLogger();
		void populateDebugMessengerCreateInfo(
			VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkResult createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		void destroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);
		
		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);

		void createLogicalDevice();
		void createSurface(LvWindow& window);
		void createCommandPool();

		uint32_t findMemoryType(
			uint32_t typeFilter,
			VkMemoryPropertyFlags properties);
	};
}