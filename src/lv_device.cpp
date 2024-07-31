#include "lv_device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>

namespace lv
{
	LvDevice::LvDevice(LvWindow& window)
	{
		createVulkanInstance();
		setupDebugLogger();
		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	LvDevice::~LvDevice()
	{
		cleanup();
	}

	void LvDevice::createVulkanInstance()
	{
		if (isValidationLayerEnabled() && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layer not supported");
		}
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Learn Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// extensions
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// validation layer
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (isValidationLayerEnabled())
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(
				validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vulkan instance " + result);
		}
	}

	void LvDevice::cleanup()
	{
		if (isValidationLayerEnabled())
		{
			destroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
		}
		
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(vkInstance, surface, nullptr);
		vkDestroyInstance(vkInstance, nullptr);
	}

	bool LvDevice::isValidationLayerEnabled()
	{
#ifdef NDEBUG
		return false;
#endif // NDEBUG
		return true;
	}

	bool LvDevice::checkValidationLayerSupport()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> supportedLayers(layerCount);
		vkEnumerateInstanceLayerProperties(
			&layerCount,
			supportedLayers.data()
		);

		for (const char* layer : validationLayers)
		{
			bool layerSupported = false;
			for (const auto& supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					layerSupported = true;
					break;
				}
			}

			if (!layerSupported)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> LvDevice::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (isValidationLayerEnabled())
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	bool LvDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(
			device, 
			nullptr, 
			&extensionCount,
			nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device, 
			nullptr, 
			&extensionCount, 
			availableExtensions.data());

		std::set<std::string> requiredExtensions(
			deviceExtensions.begin(), 
			deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void LvDevice::setupDebugLogger()
	{
		if (!isValidationLayerEnabled())
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			throw std::runtime_error("failed to setup debug messenger");
	}

	void LvDevice::populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	VkResult LvDevice::createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void LvDevice::destroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, 
			"vkDestroyDebugUtilsMessengerEXT");
		
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	bool LvDevice::isDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		bool indicesComplte = findQueueFamily(device).isComplete();

		bool allDeviceExtensionSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (allDeviceExtensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() 
				&& !swapChainSupport.presentModes.empty();
		}
		
		return indicesComplte
			&& allDeviceExtensionSupported
			&& swapChainAdequate;
	}

	void LvDevice::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
			throw std::runtime_error("No GPU available");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

		for (const auto device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Unable to find suitable GPU");
	}

	QueueFamilyIndices LvDevice::findQueueFamily(VkPhysicalDevice device)
	{
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
			device, 
			&familyCount, 
			nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			device, 
			&familyCount, 
			queueFamilies.data());

		QueueFamilyIndices indices;
		int index = 0;
		for (const auto& family : queueFamilies)
		{
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = index;
			}

			VkBool32 hasPresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				device, 
				index, 
				surface, 
				&hasPresentSupport);
			if (hasPresentSupport)
				indices.presentFamily = index;

			++index;
		}

		return indices;
	}

	void LvDevice::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamily(physicalDevice);
		
		std::set<uint32_t> uniqueFamilyIndices{
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};
		std::vector<VkDeviceQueueCreateInfo> queuesCreateInfo;

		float queuePriority = 1.0;
		for (const uint32_t index : uniqueFamilyIndices)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = index;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queuesCreateInfo.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 
			static_cast<uint32_t>(queuesCreateInfo.size());
		deviceCreateInfo.pQueueCreateInfos = queuesCreateInfo.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(
			deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (isValidationLayerEnabled())
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(
				validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(
			physicalDevice,
			&deviceCreateInfo,
			nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device");

		// fetch handle for graphics queue
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

		//fetch handle for presentation queue
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void LvDevice::createSurface(LvWindow& window)
	{
		window.createWindowSurface(vkInstance, &surface);
	}

	SwapChainSupportDetails LvDevice::querySwapChainSupport(
		VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			device, 
			surface, 
			&details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, 
			surface, 
			&formatCount, 
			nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				device, 
				surface, 
				&formatCount, 
				details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void LvDevice::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamily(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void LvDevice::createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(
			device,
			&bufferInfo,
			nullptr,
			&buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(
			memRequirements.memoryTypeBits, 
			properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for vertex buffer");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t LvDevice::findMemoryType(
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkFormat LvDevice::findSupportedFormat(
		const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, 
		VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && 
				(props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL && 
				(props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}


	void LvDevice::createImageWithInfo(
		const VkImageCreateInfo& imageInfo,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory) {
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device, image, imageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory!");
		}
	}
}