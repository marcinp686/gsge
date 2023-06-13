#pragma once

#include <vulkan/vulkan.h>

#include "device.h"
#include "swapchain.h"
#include "renderPass.h"

class Framebuffer
{
  public:
    Framebuffer(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain, std::shared_ptr<RenderPass> &renderPass);
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;
    ~Framebuffer();   

    inline VkFramebuffer &operator[](uint32_t index)
    {
        return buffers[index];
    }

  private:
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RenderPass> renderPass;

    std::vector<VkFramebuffer> buffers;

    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

};
