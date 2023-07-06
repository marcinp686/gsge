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

    inline VkImageView &getDepthImageView();
    inline VkImageView &getMultisampleImageView();
    inline VkImage &getDepthImage();
    inline VkImage &getMultisampleImage();

    inline VkFramebuffer &operator[](uint32_t index)
    {
        return buffers[index];
    }

  private:
    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

    // Depth buffer resources
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // For MSAA - image that multisampled color image is being resolved to
    VkImage multisampleImage;
    VkDeviceMemory multisampleImageMemory;
    VkImageView multisampleImageView;
    bool msaaEnabledAtCreation;

    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RenderPass> renderPass;
    std::vector<VkFramebuffer> buffers;
    
    void createDepthResources();
    void createResolveResources();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory,
                     VkImageLayout initialLayout);

  

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
