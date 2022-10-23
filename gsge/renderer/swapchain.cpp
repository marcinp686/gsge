#include "swapchain.h"

Swapchain::Swapchain(Device *device, Window *window, Surface *surface) : device(device), window(window), surface(surface)
{
    create();
}

Swapchain::~Swapchain()
{
}

void Swapchain::create()
{
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat();
    VkPresentModeKHR presentMode = chooseSwapPresentMode();
    VkExtent2D swapExtent = chooseSwapExtent();

    imageCount = device->getSurfaceCapabilities().minImageCount + 1;
    if (device->getSurfaceCapabilities().maxImageCount > 0 && imageCount > device->getSurfaceCapabilities().maxImageCount)
    {
        imageCount = device->getSurfaceCapabilities().maxImageCount;
    }

    std::vector<uint32_t> qFamilyIndices = {device->getGraphicsQueueFamilyIdx(), device->getPresentQueueFamilyIdx()};

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->get_handle();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // TODO: consider changing to VK_SHARING_MODE_EXCLUSIVE
    createInfo.queueFamilyIndexCount = static_cast<uint32_t>(qFamilyIndices.size());
    createInfo.pQueueFamilyIndices = qFamilyIndices.data();
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->get_handle(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    vkGetSwapchainImagesKHR(device->get_handle(), swapchain, &imageCount, nullptr);

    imageFormat = surfaceFormat.format;
    extent = swapExtent;

    spdlog::info("Swap chain created with " + std::to_string(imageCount) + " vkImage(s)");
}

void Swapchain::recreate()
{
    vkDeviceWaitIdle(device->get_handle());
    device->querySurfaceCapabilities();
    cleanup();
    create();
}

void Swapchain::cleanup()
{
    vkDestroySwapchainKHR(device->get_handle(), swapchain, nullptr);
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat()
{
    for (const auto &availableFormat : device->getSurfaceFormats())
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return device->getSurfaceFormats()[0];
}

VkExtent2D Swapchain::chooseSwapExtent()
{
    if (device->getSurfaceCapabilities().currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return device->getSurfaceCapabilities().currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {window->width, window->height};

        actualExtent.width = std::clamp(actualExtent.width, device->getSurfaceCapabilities().minImageExtent.width,
                                        device->getSurfaceCapabilities().maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, device->getSurfaceCapabilities().minImageExtent.height,
                                         device->getSurfaceCapabilities().maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode()
{
    for (const auto &availablePresentMode : device->getSurfacePresentModes())
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_IMMEDIATE_KHR; //    FIFO_KHR;
}

VkSwapchainKHR &Swapchain::get_handle()
{
    return swapchain;
}

uint32_t Swapchain::getImageCount() const
{
    return imageCount;
}

std::vector<VkImage> Swapchain::getImages()
{
    std::vector<VkImage> images;

    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device->get_handle(), swapchain, &imageCount, images.data());

    return images;
}

VkFormat Swapchain::getImageFormat() const
{
    return imageFormat;
}

VkExtent2D &Swapchain::getExtent()
{
    return extent;
}
