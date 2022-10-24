#include "swapchain.h"

Swapchain::Swapchain(Device *device, Window *window, Surface *surface) : device(device), window(window), surface(surface)
{
    device->querySurfaceCapabilities();
    create();
    createImages();
    createImageViews();
    createDepthResources();
}

void Swapchain::createImages()
{
    vkGetSwapchainImagesKHR(device->get_handle(), swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device->get_handle(), swapchain, &imageCount, images.data());
    spdlog::info("Created swapchain images");
}

Swapchain::~Swapchain()
{
    cleanup();
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

    extent = swapExtent;
    imageFormat = surfaceFormat.format;

    spdlog::info("Created swapchain with " + std::to_string(imageCount) + " images");
}

void Swapchain::cleanup()
{
    vkDestroyImageView(device->get_handle(), depthImageView, nullptr);
    vkDestroyImage(device->get_handle(), depthImage, nullptr);
    vkFreeMemory(device->get_handle(), depthImageMemory, nullptr);

    for (auto imageView : imageViews)
    {
        vkDestroyImageView(device->get_handle(), imageView, nullptr);
    }

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

VkImageView &Swapchain::getDepthImageView()
{
    return depthImageView;
}

VkExtent2D &Swapchain::getExtent()
{
    return extent;
}

void Swapchain::createImageViews()
{
    imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); i++)
    {
        imageViews[i] = createImageView(images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    spdlog::info("Created swapchain image views");
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
    if (vkCreateImageView(device->get_handle(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Swapchain::createDepthResources()
{
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    createImage(extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Swapchain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device->get_handle(), &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->get_handle(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->get_handle(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device->get_handle(), image, imageMemory, 0);
}

uint32_t Swapchain::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDeviceHandle(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
