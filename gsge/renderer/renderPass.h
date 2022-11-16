#pragma once
#include <memory>

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

class Swapchain;

class RenderPass
{
  public:
    RenderPass(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain);
    RenderPass(const RenderPass &) = delete;
    RenderPass &operator=(const RenderPass &) = delete;
    ~RenderPass();
    
    inline operator VkRenderPass() const
    {
        return renderPass;
    }

  private:
    VkRenderPass renderPass;

    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
};
