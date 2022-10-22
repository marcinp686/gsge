#pragma once

#include <algorithm>
#include <limits>

#include <vulkan/vulkan.h>

#include "device.h"
#include "window.h"
#include "surface.h"
#include "renderPass.h"

class RenderPass;

class Swapchain
{
  public:
    Swapchain(Device *device, Window *window, Surface *surface);
    ~Swapchain();

    void create();
    void cleanup();
    void recreate();

    void createFramebuffers();
    void bindRenderPass(RenderPass *bRenderPass);

    VkSwapchainKHR &get_handle();
    VkFormat &getImageFormat();
    VkExtent2D &getExtent();
    VkFramebuffer &getFramebuffer(uint32_t index);

  private:
    VkSwapchainKHR swapchain;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D extent;

    Device *device;
    Window *window;
    Surface *surface;
    RenderPass *renderPass;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkExtent2D chooseSwapExtent();
    VkPresentModeKHR chooseSwapPresentMode();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void createImageViews();

    void createDepthResources();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};
