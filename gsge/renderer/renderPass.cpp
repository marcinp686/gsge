#include "renderPass.h"

RenderPass::RenderPass(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain)
    : device(device), swapchain(swapchain)
{
    // Color attachment
    VkAttachmentDescription2 colorAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = settings.Renderer.enableMSAA ? settings.Renderer.msaaSampleCount : VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = settings.Renderer.enableMSAA ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference2 colorAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // Depth attachment
    VkAttachmentDescription2 depthAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = VK_FORMAT_D32_SFLOAT, // TODO: Add method to find supported depth format
        .samples = settings.Renderer.enableMSAA ? settings.Renderer.msaaSampleCount : VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference2 depthAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Attachment for resolving multisampled color attachment to swapchain's color attachment
    // It needs to be have 1 sample count regardless of the settings.Renderer.msaaSampleCount
    VkAttachmentDescription2 resolveAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference2 resolveAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    std::vector<VkAttachmentDescription2> attachments;
    attachments.push_back(colorAttachment);
    attachments.push_back(depthAttachment);

    if (settings.Renderer.enableMSAA)
        attachments.push_back(resolveAttachment);

    // Subpasses
    VkSubpassDescription2 subpass{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = settings.Renderer.enableMSAA ? &resolveAttachmentRef : VK_NULL_HANDLE,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    // Subpass dependencies
    // - Color attachment
    VkSubpassDependency2 colorDependency{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
        .dependencyFlags = 0,
    };

    // - Depth attachment
    VkSubpassDependency2 depthDependency{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        .dependencyFlags = 0,
    };

    std::vector<VkSubpassDependency2> dependencies;
    dependencies.push_back(depthDependency);
    dependencies.push_back(colorDependency);

    VkRenderPassCreateInfo2 renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    if (vkCreateRenderPass2(*device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    SPDLOG_TRACE("[Render pass] Created");
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*device, renderPass, nullptr);

    SPDLOG_TRACE("[Render pass] Destroyed");
}
