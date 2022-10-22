#pragma once
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "window.h"

class Surface
{
  public:
    Surface(VkInstance instance, GLFWwindow *window);
    ~Surface();
    VkSurfaceKHR get_handle() const;

  private:
    GLFWwindow *window{nullptr};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkInstance instance{VK_NULL_HANDLE};
};
