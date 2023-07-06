#include "renderPass.h"

RenderPass::RenderPass(std::shared_ptr<Device> &device, std::shared_ptr<Swapchain> &swapchain)
    : device(device), swapchain(swapchain)
{
    // Swapchain image used for render target in singlesample rendering and as a target for resolution of multisample images
    VkAttachmentDescription2 singlesampleAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = settings.Renderer.enableMSAA ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_CLEAR,
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
        .samples = settings.Renderer.enableMSAA ? settings.Renderer.msaaSampleCount : VK_SAMPLE_COUNT_1_BIT,
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

    // multisample image used as render target for multisampling - discard after resolution
    VkAttachmentDescription2 multisampleAttachment{
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
        .format = swapchain->getImageFormat(),
        .samples = settings.Renderer.msaaSampleCount,
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

    if (settings.Renderer.enableMSAA)
        attachments.push_back(multisampleAttachment);

    // Subpasses
    VkSubpassDescription2 subpass{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = settings.Renderer.enableMSAA ? &multisampleAttachmentRef : &presentAttachmentRef,
        .pResolveAttachments = settings.Renderer.enableMSAA ? &presentAttachmentRef : VK_NULL_HANDLE,
        .pDepthStencilAttachment = nullptr,//&depthAttachmentRef,
    };

    //// Subpass dependencies
    //// - Color attachment
    // VkSubpassDependency2 colorDependency{
    //     .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
    //     .srcSubpass = VK_SUBPASS_EXTERNAL,
    //     .dstSubpass = 0,
    //     .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .srcAccessMask = 0,
    //     .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
    //     .dependencyFlags = 0,
    // };

    //// - Depth attachment
    // VkSubpassDependency2 depthDependency{
    //     .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
    //     .srcSubpass = VK_SUBPASS_EXTERNAL,
    //     .dstSubpass = 0,
    //     .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
    //     .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //     .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    //     .dependencyFlags = 0,
    // };

    // std::vector<VkSubpassDependency2> dependencies;
    // dependencies.push_back(depthDependency);
    // dependencies.push_back(colorDependency);

    // Trying to be very VERY explicit about synchronization and dependencies. Here we go:

    // Memory barrier to synchronize access to color attachment after transition from UNDEFINED to COLOR_ATTACHMENT_OPTIMAL
    // at the beginning of render pass
    // pre-barrier operations executed in COLOR_ATTACHMENT_OUTPUT stage of VK_SUBPASS_EXTERNAL
    // pre-barrier access type is COLOR_ATTACHMENT_WRITE
    // post-barrier operations executed in COLOR_ATTACHMENT_OUTPUT stage of subpass 0
    // post-barrier access is COLOR_ATTACHMENT_WRITE
    // TODO: Do we need a read access in dst as well?
    VkMemoryBarrier2 singlesampleAttachmentMB1{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    };
    VkSubpassDependency2 singlesampleAttachmentDep1{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &singlesampleAttachmentMB1,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };
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

    // depth
    // Firs to synchronize image layout transition and subsequent reads from and writes to a depth buffer
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

    VkMemoryBarrier2 depthAttachmentMB2{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_NONE,
    };

    VkSubpassDependency2 depthAttachmentDep2{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &depthAttachmentMB2,
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };
    /*
    // Memory barrier to synchronize access to multisample image
    VkMemoryBarrier2 multisampleAttachmentMB{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_RESOLVE_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
    };
    VkSubpassDependency2 multisampleAttachmentDep{
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = &multisampleAttachmentMB,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };
    */
    std::vector<VkSubpassDependency2> dependencies;
    dependencies.push_back(singlesampleAttachmentDep1);
    dependencies.push_back(singlesampleAttachmentDep2);
    dependencies.push_back(depthAttachmentDep1);
    //dependencies.push_back(depthAttachmentDep2);
    // dependencies.push_back(multisampleAttachmentDep);

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
