#include "framebuffer.h"

Framebuffer::Framebuffer(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain,
                         std::shared_ptr<RenderPass> &renderPass)
    : device(device), swapchain(swapchain), renderPass(renderPass)
{
    uint32_t imageCount = swapchain->getImageCount();

    buffers.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; i++)
    {        
        std::vector<VkImageView> attachments;
        if (settings.Renderer.enableMSAA)
        {
            // TODO: Why the hell is it in reverse order compared to renderpass attachments???
            attachments.push_back(swapchain->getColorImageView());
            attachments.push_back(swapchain->getDepthImageView());
            attachments.push_back(swapchain->getImageView(i));
        }
        else
        {
            attachments.push_back(swapchain->getImageView(i));
            attachments.push_back(swapchain->getDepthImageView());
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
    for (auto &buffer : buffers)
    {
        vkDestroyFramebuffer(*device, buffer, nullptr);
    }

    SPDLOG_TRACE("[Frambuffer(s)] Destroyed");
}