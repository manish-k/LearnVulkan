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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(
			width, 
			height, 
			name.c_str(), 
			nullptr, 
			nullptr);

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
		return { 
			static_cast<uint32_t>(width), 
			static_cast<uint32_t>(height) 
		}; 
	}
}