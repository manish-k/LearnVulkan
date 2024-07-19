#include "src/app.hpp"

#include <stdexcept>
#include <iostream>

int main() {
    //glfwInit();

    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    //uint32_t extensionCount = 0;
    //vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    //std::cout << extensionCount << " extensions supported\n";

    //glm::mat4 matrix;
    //glm::vec4 vec;
    //auto test = matrix * vec;

    //while (!glfwWindowShouldClose(window)) {
    //    glfwPollEvents();
    //}

    //glfwDestroyWindow(window);

    //glfwTerminate();

    try
    {
        lv::App vulkanApp{};
        vulkanApp.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}