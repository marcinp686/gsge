#pragma once

#include <algorithm>
#include <limits>
#include <memory>

#include <vulkan/vulkan.h>

#include "device.h"
#include "window.h"
#include "surface.h"
#include "renderPass.h"
#include "settings.h"

class RenderPass;

class Swapchain
{
  public:
    Swapchain(std::shared_ptr<Device> &device, std::shared_ptr<Window> &window, std::shared_ptr<Surface> &surface);
    Swapchain(const Swapchain &) = delete;
    Swapchain &operator=(const Swapchain &) = delete;
    ~Swapchain();

    void createImages();
    void create();
    void cleanup();

    VkExtent2D &getExtent();
    uint32_t getImageCount() const;
    VkFormat getImageFormat() const;
    VkImage &getImage(uint32_t index);
    VkImageView &getImageView(uint32_t index);
    VkImageView &getDepthImageView();

    inline operator VkSwapchainKHR() const
    {
        return swapchain;
    }

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

    std::shared_ptr<Device> device;
    std::shared_ptr<Window> window;
    std::shared_ptr<Surface> surface;
    std::shared_ptr<RenderPass> renderPass;

    GSGE_SETTINGS_INSTANCE_DECL;

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
