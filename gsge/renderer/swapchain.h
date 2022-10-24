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
    void createImages();
    ~Swapchain();

    void create();
    void cleanup();

    VkSwapchainKHR &get_handle();
    VkExtent2D &getExtent();
    uint32_t getImageCount() const;
    VkFormat getImageFormat() const;
    VkImage &getImage(uint32_t index);
    VkImageView &getImageView(uint32_t index);
    VkImageView &getDepthImageView();

  private:
    VkSwapchainKHR swapchain;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D extent;
    uint32_t imageCount{0};

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    Device *device;
    Window *window;
    Surface *surface;
    RenderPass *renderPass;

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
