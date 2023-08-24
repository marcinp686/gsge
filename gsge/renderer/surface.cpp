#include "surface.h"

Surface::Surface(std::shared_ptr<Instance> &instance, std::shared_ptr<Window> &window)
    : instance(instance), window(window)
{
    GSGE_CHECK_RESULT(glfwCreateWindowSurface(*instance, *window, nullptr, &surface));
    SPDLOG_TRACE("[Surface] Created");
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(*instance, surface, nullptr);
    
    SPDLOG_TRACE("[Surface] Destroyed");
}
