#pragma once

#include <string>

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

    inline VkImage &getDepthImage(size_t index);
    inline VkImageView &getDepthImageView(size_t index);
    
    inline VkImage &getMultisampleImage(size_t index);
    inline VkImageView &getMultisampleImageView(size_t index);

    inline VkFramebuffer &operator[](uint32_t index)
    {
        return buffers[index];
    }

  private:
    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

    // Depth buffer resources
    std::vector<VkImage> depthImage;
    std::vector<VkDeviceMemory> depthImageMemory;
    std::vector<VkImageView> depthImageView;

    // For MSAA - image that multisampled color image is being resolved to
    std::vector<VkImage> multisampleImage;
    std::vector<VkDeviceMemory> multisampleImageMemory;
    std::vector<VkImageView> multisampleImageView;
    bool msaaEnabledAtCreation;

    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RenderPass> renderPass;
    std::vector<VkFramebuffer> buffers;
    
    void createDepthResources();
    void createMultisampleResources();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory,
                     VkImageLayout initialLayout);

  

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
