#include "framebuffer.h"

Framebuffer::Framebuffer(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain,
                         std::shared_ptr<RenderPass> &renderPass)
    : device(device), swapchain(swapchain), renderPass(renderPass)
{
    if (settings.Renderer.enableMSAA)
        createMultisampleResources();
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
            attachments.push_back(depthImageView[i]);
            attachments.push_back(multisampleImageView[i]);
        }
        else
        {
            attachments.push_back(swapchain->getImageView(i));
            attachments.push_back(depthImageView[i]);
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
        for (size_t i = 0; i < multisampleImage.size(); ++i)
        {
            vkDestroyImageView(*device, multisampleImageView[i], nullptr);
            vkDestroyImage(*device, multisampleImage[i], nullptr);
            vkFreeMemory(*device, multisampleImageMemory[i], nullptr);
        }
    }

    for (size_t i = 0; i < depthImage.size(); ++i)
    {
        vkDestroyImageView(*device, depthImageView[i], nullptr);
        vkDestroyImage(*device, depthImage[i], nullptr);
        vkFreeMemory(*device, depthImageMemory[i], nullptr);
    }

    for (auto &buffer : buffers)
    {
        vkDestroyFramebuffer(*device, buffer, nullptr);
    }

    SPDLOG_TRACE("[Frambuffer(s)] Destroyed");
}

VkImageView &Framebuffer::getDepthImageView(size_t index)
{
    return depthImageView[index];
}

VkImageView &Framebuffer::getMultisampleImageView(size_t index)
{
    return multisampleImageView[index];
}

VkImage &Framebuffer::getDepthImage(size_t index)
{
    return depthImage[index];
}

VkImage &Framebuffer::getMultisampleImage(size_t index)
{
    return multisampleImage[index];
}

void Framebuffer::createDepthResources()
{
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    uint32_t imageCount = swapchain->getImageCount();

    depthImage.resize(imageCount);
    depthImageView.resize(imageCount);
    depthImageMemory.resize(imageCount);

    for (size_t i = 0; i < imageCount; ++i)
    {
        createImage(swapchain->getExtent().width, swapchain->getExtent().height,
                    settings.Renderer.enableMSAA ? settings.Renderer.msaaSampleCount : VK_SAMPLE_COUNT_1_BIT, depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage[i], depthImageMemory[i], VK_IMAGE_LAYOUT_UNDEFINED);
        depthImageView[i] = createImageView(depthImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        std::stringstream di_name, diview_name;
        di_name << "Depth image " << i;
        diview_name << "Depth image view " << i;

        GSGE_DEBUGGER_SET_OBJECT_NAME(depthImage[i], di_name.str().c_str());
        GSGE_DEBUGGER_SET_OBJECT_NAME(depthImageView[i], diview_name.str().c_str());
    }

    SPDLOG_TRACE("[Framebuffer / Depth resources ] Created");
}

void Framebuffer::createMultisampleResources()
{
    uint32_t imageCount = swapchain->getImageCount();

    multisampleImage.resize(imageCount);
    multisampleImageView.resize(imageCount);
    multisampleImageMemory.resize(imageCount);

    for (size_t i = 0; i < imageCount; ++i)
    {
        createImage(swapchain->getExtent().width, swapchain->getExtent().height, settings.Renderer.msaaSampleCount,
                    swapchain->getImageFormat(), VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, multisampleImage[i], multisampleImageMemory[i],
                    VK_IMAGE_LAYOUT_UNDEFINED);
        multisampleImageView[i] = createImageView(multisampleImage[i], swapchain->getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

        std::stringstream mi_name, miview_name;
        mi_name << "Multisample image " << i;
        miview_name << "Multisample image view " << i;

        GSGE_DEBUGGER_SET_OBJECT_NAME(multisampleImage[i], mi_name.str().c_str());
        GSGE_DEBUGGER_SET_OBJECT_NAME(multisampleImageView[i], miview_name.str().c_str());
    }
    SPDLOG_TRACE("[Framebuffer / Multisample resources ] Created");
}

void Framebuffer::createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits sampleCount, VkFormat format,
                              VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                              VkDeviceMemory &imageMemory, VkImageLayout initialLayout)
{
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = sampleCount,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = initialLayout,
    };

    if (vkCreateImage(*device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(*device, image, imageMemory, 0);
}

VkImageView Framebuffer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {.aspectMask = aspectFlags, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
    };

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