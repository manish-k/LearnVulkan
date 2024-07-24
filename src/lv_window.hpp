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

		int width;
		int height;

		const std::string name;
	private:
		void initWindow();

	public:
		bool resized = false;

		LvWindow(std::string_view name, const int width, const int height);
		~LvWindow();

		LvWindow(const LvWindow&) = delete;
		LvWindow& operator=(const LvWindow&) = delete;

		static void framebufferResizeCallback(
			GLFWwindow* window, 
			int width, 
			int height);

		bool shouldClose();
		void pollEvents();
		void waitEvents();
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		VkExtent2D getExtent();
		
		bool isResized() { return resized; };
		void resetResizedState() { resized = false; };
		void setWidth(int newWidth) { width = newWidth; };
		void setHeight(int newHeight) { height = newHeight; };
	};
}