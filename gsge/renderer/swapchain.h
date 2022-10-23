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

    VkSwapchainKHR &get_handle();
    VkExtent2D &getExtent();
    uint32_t getImageCount() const;
    VkFormat getImageFormat() const;
    std::vector<VkImage> getImages();

  private:
    VkSwapchainKHR swapchain;
    VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D extent;
    uint32_t imageCount{0};

    Device *device;
    Window *window;
    Surface *surface;
    RenderPass *renderPass;

    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkExtent2D chooseSwapExtent();
    VkPresentModeKHR chooseSwapPresentMode();
};
