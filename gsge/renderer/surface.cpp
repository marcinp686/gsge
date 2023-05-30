#include "surface.h"

Surface::Surface(std::shared_ptr<Instance> &instance, std::shared_ptr<Window> &window)
    : instance(instance), window(window)
{
    if (glfwCreateWindowSurface(*instance, *window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
    
    SPDLOG_TRACE("[Surface] created");
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(*instance, surface, nullptr);
    
    SPDLOG_TRACE("[Surface] destroyed");
}
