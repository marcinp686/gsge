#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"

class Swapchain;

class RenderPass
{
  public:
    RenderPass(Device *device, Swapchain *swapchain);
    ~RenderPass();
    VkRenderPass &get_handle();

  private:
    VkRenderPass renderPass;

    Device *device;
    Swapchain *swapchain;
};
