/*********************************************************************
 * \file   swapchain.cpp
 * \brief  Swapchain class implementation
 *
 * \author Marcin Plichta
 * \date   May 2023
 *********************************************************************/

#include "swapchain.h"

Swapchain::Swapchain(std::shared_ptr<Device> &device, std::shared_ptr<Window> &window, std::shared_ptr<Surface> &surface)
    : device(device), window(window), surface(surface)
{
    device->querySurfaceCapabilities();

    createSwapchain();
    initializeSwapchainImages();
}

Swapchain::~Swapchain()
{
    for (auto imageView : imageViews)
    {
        vkDestroyImageView(*device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(*device, swapchain, nullptr);

    SPDLOG_TRACE("[Swapchain] Destroyed");
}

void Swapchain::initializeSwapchainImages()
{
    uint32_t imgCount = static_cast<uint32_t>(images.size());

    vkGetSwapchainImagesKHR(*device, swapchain, &imgCount, images.data());

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(images, "Singlesample image");
    SPDLOG_TRACE("[Swapchain / Images] Created");

    for (size_t i = 0; i < images.size(); i++)
    {
        imageViews[i] = createImageView(images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(imageViews, "Singlesample image view");
    SPDLOG_TRACE("[Swapchain / Image views] Created");
}

void Swapchain::createSwapchain()
{
    uint32_t colorImageCount{0};

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat();
    VkPresentModeKHR presentMode = chooseSwapPresentMode();
    VkExtent2D swapExtent = chooseSwapExtent();

    colorImageCount = device->getSurfaceCapabilities().minImageCount + 1;
    if (device->getSurfaceCapabilities().maxImageCount > 0 && colorImageCount > device->getSurfaceCapabilities().maxImageCount)
    {
        colorImageCount = device->getSurfaceCapabilities().maxImageCount;
    }

    std::vector<uint32_t> qFamilyIndices = {device->getPresentQueueFamilyIdx()};

    // If graphics and present queues are different, then swapchain must let images be shared between queues
    if (device->getGraphicsQueueFamilyIdx() != device->getPresentQueueFamilyIdx())
        qFamilyIndices.push_back(device->getGraphicsQueueFamilyIdx());

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = *surface,
        .minImageCount = colorImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = swapExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = qFamilyIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = static_cast<uint32_t>(qFamilyIndices.size()),
        .pQueueFamilyIndices = qFamilyIndices.data(),
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE, // TODO: consider changing to use old swapchain
    };
    if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    images.resize(colorImageCount);
    imageViews.resize(colorImageCount);

    extent = swapExtent;
    imageFormat = surfaceFormat.format;

    SPDLOG_TRACE("[Swapchain] Created");
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
    // Get surface extent - With Win32, minImageExtent, maxImageExtent, and currentExtent must always equal the window size
    VkExtent2D currentExtent = device->getSurfaceCapabilities().currentExtent;

    // If SurfaceCapabilities.currentExtent is {0xFFFFFFFF,0xFFFFFFFF} then
    // the surface size will be determined by the extent of a swapchain targeting the surface.
    if ((currentExtent.width != 0xFFFFFFFF) && (currentExtent.height != 0xFFFFFFFF))
    {
        return currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {settings.displaySize.width, settings.displaySize.height};
        VkExtent2D minImageExtent = device->getSurfaceCapabilities().minImageExtent;
        VkExtent2D maxImageExtent = device->getSurfaceCapabilities().maxImageExtent;

        actualExtent.width = std::clamp(actualExtent.width, minImageExtent.width, maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, minImageExtent.height, maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode()
{
    for (const auto &availablePresentMode : device->getSurfacePresentModes())
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            SPDLOG_TRACE("[Swapchain] Present mode MAILBOX");
            return availablePresentMode;
        }
    }

    SPDLOG_TRACE("[Swapchain] Present mode IMMEDIATE");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

uint32_t Swapchain::getImageCount() const
{
    return static_cast<uint32_t>(images.size());
}

VkFormat Swapchain::getImageFormat() const
{
    return imageFormat;
}

VkImage &Swapchain::getImage(uint32_t index)
{
    return images[index];
}

VkImageView &Swapchain::getImageView(uint32_t index)
{
    return imageViews[index];
}

VkExtent2D &Swapchain::getExtent()
{
    return extent;
}

VkImageView Swapchain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}