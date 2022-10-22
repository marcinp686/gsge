#include "surface.h"

Surface::Surface(VkInstance instance, GLFWwindow *window) : instance(instance), window(window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkSurfaceKHR Surface::get_handle() const
{
    return surface;
}
