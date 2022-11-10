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
    ~RenderPass();
    
    operator VkRenderPass()
    {
        return renderPass;
    }

  private:
    VkRenderPass renderPass;

    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
};
