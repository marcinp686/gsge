#include "surface.h"

Surface::Surface(std::shared_ptr<Instance> &instance, std::shared_ptr<Window> &window)
    : instance(instance), window(window)
{
    if (glfwCreateWindowSurface(*instance, *window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(*instance, surface, nullptr);
}
