#include "framebuffer.h"

Framebuffer::Framebuffer(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain,
                         std::shared_ptr<RenderPass> &renderPass)
    : device(device), swapchain(swapchain), renderPass(renderPass)
{
    if (settings.Renderer.enableMSAA)
        createResolveResources();
    createDepthResources();

    msaaEnabledAtCreation = settings.Renderer.enableMSAA;

    uint32_t imageCount = swapchain->getImageCount();

    buffers.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; i++)
    {
        std::vector<VkImageView> attachments;
        if (settings.Renderer.enableMSAA)
        {
            attachments.push_back(swapchain->getImageView(i));
            attachments.push_back(depthImageView);
            attachments.push_back(multisampleImageView);
        }
        else
        {
            attachments.push_back(swapchain->getImageView(i));
            attachments.push_back(depthImageView);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain->getExtent().width;
        framebufferInfo.height = swapchain->getExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &buffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("[Frambuffer] Failed to create framebuffer!");
        }
    }

    SPDLOG_TRACE("[Frambuffer(s)] Created");
}

Framebuffer::~Framebuffer()
{
    if (msaaEnabledAtCreation)
    {
        vkDestroyImageView(*device, multisampleImageView, nullptr);
        vkDestroyImage(*device, multisampleImage, nullptr);
        vkFreeMemory(*device, multisampleImageMemory, nullptr);
    }

    vkDestroyImageView(*device, depthImageView, nullptr);
    vkDestroyImage(*device, depthImage, nullptr);
    vkFreeMemory(*device, depthImageMemory, nullptr);

    for (auto &buffer : buffers)
    {
        vkDestroyFramebuffer(*device, buffer, nullptr);
    }

    SPDLOG_TRACE("[Frambuffer(s)] Destroyed");
}

VkImageView &Framebuffer::getDepthImageView()
{
    return depthImageView;
}

VkImageView &Framebuffer::getMultisampleImageView()
{
    return multisampleImageView;
}

VkImage &Framebuffer::getDepthImage()
{
    return depthImage;
}

VkImage &Framebuffer::getMultisampleImage()
{
    return multisampleImage;
}

void Framebuffer::createDepthResources()
{
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    createImage(swapchain->getExtent().width, swapchain->getExtent().height,
                settings.Renderer.enableMSAA ? settings.Renderer.msaaSampleCount : VK_SAMPLE_COUNT_1_BIT, depthFormat,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_UNDEFINED);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    GSGE_DEBUGGER_SET_OBJECT_NAME(depthImage, "Depth image");
    GSGE_DEBUGGER_SET_OBJECT_NAME(depthImageView, "Depth image view");
    SPDLOG_TRACE("[Framebuffer / Depth resources ] Created");
}

void Framebuffer::createResolveResources()
{
    createImage(swapchain->getExtent().width, swapchain->getExtent().height, settings.Renderer.msaaSampleCount,
                swapchain->getImageFormat(),
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, multisampleImage, multisampleImageMemory, VK_IMAGE_LAYOUT_UNDEFINED);
    multisampleImageView = createImageView(multisampleImage, swapchain->getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

    GSGE_DEBUGGER_SET_OBJECT_NAME(multisampleImage, "Multisample image");
    GSGE_DEBUGGER_SET_OBJECT_NAME(multisampleImageView, "Multisample image view");
    SPDLOG_TRACE("[Framebuffer / Multisample resources ] Created");
}

void Framebuffer::createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits sampleCount, VkFormat format,
                              VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                              VkDeviceMemory &imageMemory, VkImageLayout initialLayout)
{
    std::array<uint32_t, 2> dd = {0, 1};

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
    imageInfo.initialLayout = initialLayout;
    imageInfo.usage = usage;
    imageInfo.samples = sampleCount;
    // TODO: Is it necessary in all configurations? It needs to take into account if image is shared between queue families
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.pQueueFamilyIndices = dd.data();
    imageInfo.queueFamilyIndexCount = 2;

    if (vkCreateImage(*device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(*device, image, imageMemory, 0);
}

VkImageView Framebuffer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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

uint32_t Framebuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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