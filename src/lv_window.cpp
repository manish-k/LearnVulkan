#include "lv_window.hpp"

#include <stdexcept>
#include <iostream>

namespace lv
{
	LvWindow::LvWindow(std::string_view name, const int w, const int h):
		name{name}, width{w}, height{h}
	{
		initWindow();
	}

	LvWindow::~LvWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void LvWindow::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(
			width, 
			height, 
			name.c_str(), 
			nullptr, 
			nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		int ewidth, eheight;
		glfwGetFramebufferSize(window, &ewidth, &eheight);

		std::cout << "extent " << ewidth << " " << eheight << std::endl;
	}

	bool LvWindow::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	void LvWindow::pollEvents()
	{
		glfwPollEvents();
	}

	void LvWindow::waitEvents()
	{
		glfwWaitEvents();
	}

	void LvWindow::createWindowSurface(
		VkInstance instance,
		VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	VkExtent2D LvWindow::getExtent() 
	{ 
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		
		return { 
			static_cast<uint32_t>(width), 
			static_cast<uint32_t>(height) 
		}; 
	}

	void LvWindow::framebufferResizeCallback(
		GLFWwindow* window,
		int width,
		int height)
	{
		auto currentWindow = 
			reinterpret_cast<LvWindow*>(glfwGetWindowUserPointer(window));

		currentWindow->resized = true;
		currentWindow->setWidth(width);
		currentWindow->setHeight(height);
	}
}