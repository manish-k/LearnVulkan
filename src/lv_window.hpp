#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace lv
{
	class LvWindow
	{
	private: 
		GLFWwindow* window;

		const int width;
		const int height;

		const std::string name;

	private:
		void initWindow();

	public:
		LvWindow(std::string_view name, const int width, const int height);
		~LvWindow();

		LvWindow(const LvWindow&) = delete;
		LvWindow& operator=(const LvWindow&) = delete;

		bool shouldClose();
		void pollEvents();
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		VkExtent2D getExtent();
	};
}