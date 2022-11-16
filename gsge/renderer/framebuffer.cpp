#include "framebuffer.h"

Framebuffer::Framebuffer(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain,
                         std::shared_ptr<RenderPass> &renderPass)
    : device(device), swapchain(swapchain), renderPass(renderPass)
{
    createFramebuffers();
}

Framebuffer::~Framebuffer()
{
    cleanup();
}

void Framebuffer::cleanup()
{
    for (auto &buffer : buffers)
    {
        vkDestroyFramebuffer(*device, buffer, nullptr);
    }
}

void Framebuffer::createFramebuffers()
{
    uint32_t imageCount = swapchain->getImageCount();

    buffers.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; i++)
    {
        std::array<VkImageView, 2> attachments = {swapchain->getImageView(i), swapchain->getDepthImageView()};

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
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    SPDLOG_TRACE("Framebuffers created");
}
