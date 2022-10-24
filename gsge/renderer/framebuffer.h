#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"
#include "renderPass.h"

class Framebuffer
{
  public:
    Framebuffer(Device *device, Swapchain *swapchain, RenderPass *renderPass);
    ~Framebuffer();
    VkFramebuffer &getBuffer(uint32_t index);

  private:
    Device *device;
    Swapchain *swapchain;
    RenderPass *renderPass;

    std::vector<VkFramebuffer> buffers;

    void createFramebuffers();

    void cleanup();
};
