#include "renderPass.h"

RenderPass::RenderPass(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain)
    : device(device), swapchain(swapchain)
{
    // Swapchain image used for render target in singlesample rendering and as a target for resolution of multisample images
    VkAttachmentDescription2 singlesampleAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = settings.Renderer.msaa.enabled ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference2 presentAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    };

    // Depth image - discard after rendering
    VkAttachmentDescription2 depthAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = VK_FORMAT_D32_SFLOAT, // TODO: Add method to find supported depth format
        .samples = settings.Renderer.msaa.enabled ? settings.Renderer.msaa.sampleCount : VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference2 depthAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    };

    // Multisample image used as render target for multisampling - discard after resolution
    VkAttachmentDescription2 multisampleAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = settings.Renderer.msaa.sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference2 multisampleAttachmentRef{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    };

    std::vector<VkAttachmentDescription2> attachments;
    attachments.push_back(singlesampleAttachment);
    attachments.push_back(depthAttachment);

    if (settings.Renderer.msaa.enabled)
        attachments.push_back(multisampleAttachment);

    VkSubpassDescription2 subpass{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = settings.Renderer.msaa.enabled ? &multisampleAttachmentRef : &presentAttachmentRef,
        .pResolveAttachments = settings.Renderer.msaa.enabled ? &presentAttachmentRef : VK_NULL_HANDLE,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    // Memory barrier to synchronize access to single sample color attachment after transition
    // from UNDEFINED to COLOR_ATTACHMENT_OPTIMAL at the beginning of render pass (VK_SUBPASS_EXTERNAL -> subpass #0)
    //   * For singlesample color attachment, we do not care about content of the image at the beginning of the render pass,
    //     becasue multisample image will be resolved into singlesample image at the end of the render pass
    //     thus LOAD_OP_DONT_CARE is used and this barrier has no effect
    //   * For multisample color attachment, we need to wait for the image to be cleared before we start wrting to it
    VkMemoryBarrier2 singlesampleAttachmentMB1{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    };
    VkSubpassDependency2 singlesampleAttachmentDep1{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &singlesampleAttachmentMB1,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    // Memory barrier to synchronize transition from COLOR_ATTACHMENT_OPTIMAL to PRESENT_SRC_KHR
    // at the end of render pass (subpass #0 -> VK_SUBPASS_EXTERNAL)
    VkMemoryBarrier2 singlesampleAttachmentMB2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    };
    VkSubpassDependency2 singlesampleAttachmentDep2{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &singlesampleAttachmentMB2,
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    // Synchronize image layout transition and subsequent reads from and writes to a depth buffer
    // We need to wait for the transition from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    // at (VK_SUBPASS_EXTERNAL->subpass #0) and clearing due to LOAD_OP_CLEAR
    VkMemoryBarrier2 depthAttachmentMB1{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkSubpassDependency2 depthAttachmentDep1{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &depthAttachmentMB1,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    // Memory barrier to synchronize access to multisample image
    VkMemoryBarrier2 multisampleAttachmentMB{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_RESOLVE_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
    };
    // https://vulkan.lunarg.com/doc/view/1.3.250.1/windows/1.3-extensions/vkspec.html#synchronization-access-masks
    VkSubpassDependency2 multisampleAttachmentDep{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &multisampleAttachmentMB,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    std::vector<VkSubpassDependency2> dependencies;

    //if (!settings.Renderer.msaa.enabled)
        dependencies.push_back(singlesampleAttachmentDep1);
    dependencies.push_back(singlesampleAttachmentDep2);
    dependencies.push_back(depthAttachmentDep1);
    if (settings.Renderer.msaa.enabled)
        dependencies.push_back(multisampleAttachmentDep);

    VkRenderPassCreateInfo2 renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    GSGE_CHECK_RESULT(vkCreateRenderPass2(*device, &renderPassInfo, nullptr, &renderPass));
    SPDLOG_TRACE("[Render pass] Created");
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*device, renderPass, nullptr);

    SPDLOG_TRACE("[Render pass] Destroyed");
}
