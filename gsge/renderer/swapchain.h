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

    VkExtent2D &getExtent();
    uint32_t getImageCount() const;
    VkFormat getImageFormat() const;
    VkImage &getImage(uint32_t index);
    VkImageView &getImageView(uint32_t index);

    inline operator VkSwapchainKHR() const
    {
        return swapchain;
    }

  private:
    VkSwapchainKHR swapchain;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D extent;

    // Color images that shaders render to
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    std::shared_ptr<Device> device;
    std::shared_ptr<Window> window;
    std::shared_ptr<Surface> surface;
    std::shared_ptr<RenderPass> renderPass;

    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkExtent2D chooseSwapExtent();
    VkPresentModeKHR chooseSwapPresentMode();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void initializeSwapchainImages();
    void createSwapchain();
};
