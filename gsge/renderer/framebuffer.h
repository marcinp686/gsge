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

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> buffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void createImageViews();

    void createDepthResources();
    void createFramebuffers();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void cleanup();
};
