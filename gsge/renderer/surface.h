#pragma once
#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "window.h"
#include "instance.h"

class Surface
{
  public:
    Surface(std::shared_ptr<Instance> &instance, std::shared_ptr<Window> &window);
    Surface(const Surface &) = delete;
    Surface &operator=(const Surface &) = delete;
    ~Surface();

    inline operator VkSurfaceKHR() const
    {
        return surface;
    }

  private:
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    std::shared_ptr<Window> window{nullptr};
    std::shared_ptr<Instance> instance{nullptr};

    GSGE_DEBUGGER_INSTANCE_DECL;
};
