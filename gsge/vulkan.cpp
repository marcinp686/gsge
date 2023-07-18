#include "vulkan.h"

vulkan::~vulkan()
{
    vkDeviceWaitIdle(*device);

    // destroy uniform buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkUnmapMemory(*device, transformMatricesStagingBufferMemory[i]);

        vkDestroyBuffer(*device, uniformBuffers[i], nullptr);
        vkFreeMemory(*device, uniformBuffersMemory[i], nullptr);

        vkDestroyBuffer(*device, transformMatricesBuffer[i], nullptr);
        vkFreeMemory(*device, transformMatricesBufferMemory[i], nullptr);

        vkDestroyBuffer(*device, transformMatricesStagingBuffer[i], nullptr);
        vkFreeMemory(*device, transformMatricesStagingBufferMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(*device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(*device, vertexBuffer, nullptr);
    vkFreeMemory(*device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(*device, indexBuffer, nullptr);
    vkFreeMemory(*device, indexBufferMemory, nullptr);

    vkDestroyBuffer(*device, vertexNormalsBuffer, nullptr);
    vkFreeMemory(*device, vertexNormalsBufferMemory, nullptr);

    vkDestroyPipeline(*device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);

    destroySyncObjects();
    destroyCommandPools();
}

void vulkan::update()
{
    EASY_FUNCTION(profiler::colors::Green200);
    EASY_BLOCK("Update buffers");
    updateTransformMatrixBuffer(currentFrame);
    updateUniformBuffer(currentFrame);
    EASY_END_BLOCK;
    drawFrame();
}

void vulkan::init()
{
    instance = std::make_shared<Instance>();
    surface = std::make_shared<Surface>(instance, window);
    device = std::make_shared<Device>(instance, surface);
    swapchain = std::make_shared<Swapchain>(device, window, surface);
    renderPass = std::make_shared<RenderPass>(device, swapchain);
    framebuffer = std::make_shared<Framebuffer>(device, swapchain, renderPass);

    // 1. Create vertex binding descriptors for vertex stage buffers
    createVertexBindingDescriptors();

    // 2. Create descriptor set layouts (to bind descriptor sets later on)
    createDescriptorSetLayouts();

    // 3. pass (2) as parameter to create pipeline layout and (1) to bind vertex buffers to pipeline
    createGraphicsPipeline();

    graphicsCommandPool = std::make_unique<CommandPool>(device, device->getGraphicsQueueFamilyIdx(), "Graphics command pool");
    transferCommandPool = std::make_unique<CommandPool>(device, device->getTransferQueueFamilyIdx(), "Transfer command pool");
    presentCommandPool = std::make_unique<CommandPool>(device, device->getPresentQueueFamilyIdx(), "Present command pool");

    // 4. create descriptor pool
    createDescriptorPool();

    // 5. create buffers for descriptor sets data
    createSyncObjects();
    createTransferCommandBuffers();
    createVertexBuffer();
    createIndexBuffer();
    createVertexNormalsBuffer();
    createTransformMatricesBuffer();
    createUniformBuffers();

    // 6. create actual descriptor sets - after buffer creation
    createDescriptorSets();

    createGraphicsCommandBuffers();
    createPresentCommandBuffers();

    // initResourceOwnerships();
}

void vulkan::loadShaders()
{
    /*const std::string vertShaderFilename = "shaders/per_vertex_light_shader.vert.spv";
    const std::string fragShaderFilename = "shaders/per_vertex_light_shader.frag.spv";*/

    const std::string vertShaderFilename = "shaders/per_fragment_light_shader.vert.spv";
    const std::string fragShaderFilename = "shaders/per_fragment_light_shader.frag.spv";

    std::ifstream vertShaderFile(vertShaderFilename, std::ios::ate | std::ios::binary);
    if (!vertShaderFile.is_open())
    {
        throw std::runtime_error("failed to open vertex shader!");
    }
    size_t fileSize = static_cast<size_t>(vertShaderFile.tellg());
    vertShaderCode.resize(fileSize);
    vertShaderFile.seekg(0);
    vertShaderFile.read(vertShaderCode.data(), fileSize);
    vertShaderFile.close();

    std::ifstream fragShaderFile(fragShaderFilename, std::ios::ate | std::ios::binary);
    if (!fragShaderFile.is_open())
    {
        throw std::runtime_error("failed to open fragment shader!");
    }
    fileSize = static_cast<size_t>(fragShaderFile.tellg());
    fragShaderCode.resize(fileSize);
    fragShaderFile.seekg(0);
    fragShaderFile.read(fragShaderCode.data(), fileSize);
    fragShaderFile.close();
}

VkShaderModule vulkan::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

/**
 * @brief Destroy command pools for graphics and transfer queues.
 *
 * @details Frees command buffers allocated by the pools as well.
 * */
void vulkan::destroyCommandPools()
{
    graphicsCommandPool.reset();
    transferCommandPool.reset();
}

void vulkan::createGraphicsPipeline()
{
    loadShaders();
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // create shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDesc.size()),
        .pVertexBindingDescriptions = vertexBindingDesc.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDesc.size()),
        .pVertexAttributeDescriptions = vertexAttrDesc.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchain->getExtent().width),
        .height = static_cast<float>(swapchain->getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain->getExtent(),
    };
    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };
    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = settings.Renderer.msaa.enabled ? settings.Renderer.msaa.sampleCount : VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f, // Optional - to discard fragments lying outside of rang,
        .maxDepthBounds = 1.0f,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = *renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(*device, fragShaderModule, nullptr);
    vkDestroyShaderModule(*device, vertShaderModule, nullptr);

    GSGE_DEBUGGER_SET_OBJECT_NAME(graphicsPipeline, "Graphics pipeline");
    SPDLOG_TRACE("[Graphics pipeline] Created");
}

void vulkan::createGraphicsCommandBuffers()
{
    graphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = *graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(*device, &allocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(graphicsCommandBuffers, "Graphics Command Buffer");
    SPDLOG_TRACE("[Graphics command buffers] Created");
}

void vulkan::createTransferCommandBuffers()
{
    transferCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = *transferCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(transferCommandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(*device, &allocInfo, transferCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(transferCommandBuffers, "Transfer command buffer");
    SPDLOG_TRACE("[Transfer command buffers] Created");
}

void vulkan::createPresentCommandBuffers()
{
    presentCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = *presentCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(presentCommandBuffers.size()),
    };

    if (vkAllocateCommandBuffers(*device, &allocInfo, presentCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(presentCommandBuffers, "Present command buffer");
    SPDLOG_TRACE("[Present command buffers] Created");
}

void vulkan::recordPresentCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    if (device->getGraphicsQueueFamilyIdx() != device->getPresentQueueFamilyIdx())
    {
        // A layout transition which happens as part of an ownership transfer needs to be specified twice; one for the
        // release, and one for the acquire. No srcStage/AccessMask is needed, waiting for a semaphore does that
        // automatically (all commands in submitted queue need to be finished before semaphore signal).
        // No dstStage/AccessMask is needed, signalling a semaphore does that automatically.
        VkImageMemoryBarrier2 presentImageAO_MB = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            //.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, Renderpass handles that
            //.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, Renderpass handles that
            .srcQueueFamilyIndex = device->getGraphicsQueueFamilyIdx(),
            .dstQueueFamilyIndex = device->getPresentQueueFamilyIdx(),
            .image = swapchain->getImage(imageIndex),
            .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        VkDependencyInfo presentImageAOMBdepInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &presentImageAO_MB,
        };

        vkCmdPipelineBarrier2(presentCommandBuffers[currentFrame], &presentImageAOMBdepInfo);
    }

    // End command buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record present command buffer!");
    }
}

void vulkan::recordGraphicsCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN(commandBuffer, "Graphics CB");

    // ---- MEMORY BARRIERS
    // Buffer memory barrier to acquire ownership of transformation matrices in this queue family
    VkBufferMemoryBarrier2 transformMatricesBufferMB_AO{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = VK_NULL_HANDLE,
        .srcStageMask = 0,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
        .srcQueueFamilyIndex = device->getTransferQueueFamilyIdx(),
        .dstQueueFamilyIndex = device->getGraphicsQueueFamilyIdx(),
        .buffer = transformMatricesBuffer[currentFrame],
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    /* Queue ownership transfer is only required when we need the content to remain valid across queues.
    Since we are transitioning from UNDEFINED -- and therefore discarding the image contents to begin with --
    we are not required to perform an ownership transfer from the presentation queue to graphics.
    This transition could also be made as an EXTERNAL -> subpass #0 render pass dependency as shown earlier.
    */
    // VkImageMemoryBarrier2 singlesampleImageAcquireMB{
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //     .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .srcAccessMask = VK_ACCESS_2_NONE,
    //     .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    //     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    //     .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .image = swapchain->getImage(imageIndex),
    //     .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    // };

    VkDependencyInfo depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &transformMatricesBufferMB_AO,
        //.imageMemoryBarrierCount = 1,
        //.pImageMemoryBarriers = &singlesampleImageAcquireMB,
    };

    vkCmdPipelineBarrier2(commandBuffer, &depInfo);

    // Begin render pass
    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = {0.02f, 0.02f, 0.02f, 1.0f};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = *renderPass,
        .framebuffer = (*framebuffer)[imageIndex],
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain->getExtent(),
            },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Set viewport
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchain->getExtent().width),
        .height = static_cast<float>(swapchain->getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Set up scissor
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain->getExtent(),
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind vertex and index buffers
    std::vector<VkBuffer> vertexBuffers = {vertexBuffer, vertexNormalsBuffer};
    std::vector<VkDeviceSize> vertexBuffersOffsets = {0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers.data(), vertexBuffersOffsets.data());
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Bind descriptors (uniform buffers, etc)
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame],
                            0, nullptr);

    // Draw commands
    for (uint32_t i = 0; i < indexOffsets.size(); i++)
    {
        uint32_t endingIdx = (i == indexOffsets.size() - 1 ? indices.size() : indexOffsets[(size_t)i + 1]);
        vkCmdDrawIndexed(commandBuffer, endingIdx - indexOffsets[i], 1, indexOffsets[i], vertexOffsets[i], i);
    }

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // ---- MEMORY BARRIERS
    // Release ownership of an image and transition image layout for presentation
    // But what happens if QF are equal? No ownership transfer? does ownership apply only to queue family and not queue itself?
    // If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer is performed,
    // and the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED.
    // if (device->getGraphicsQueueFamilyIdx() != device->getPresentQueueFamilyIdx())
    //{
    //    VkImageMemoryBarrier2 presentImageReleaseMB{
    //        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    //        .dstStageMask = VK_PIPELINE_STAGE_2_NONE, // or 0 - this is ownership release
    //        .dstAccessMask = VK_ACCESS_2_NONE,        // or 0 - this is ownership release
    //        //.oldLayout = COLOR_ATTACHMENT_OPTIMAL,  // This is done in renderpass automatically
    //        //.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // This is done in renderpass automatically
    //        .srcQueueFamilyIndex = device->getGraphicsQueueFamilyIdx(),
    //        .dstQueueFamilyIndex = device->getPresentQueueFamilyIdx(),
    //        .image = swapchain->getImage(imageIndex),
    //        .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    //    };

    //    VkDependencyInfo depInfo2{
    //        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //        .imageMemoryBarrierCount = 1,
    //        .pImageMemoryBarriers = &presentImageReleaseMB,
    //    };

    //    vkCmdPipelineBarrier2(commandBuffer, &depInfo2);
    //}

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_END(commandBuffer);

    // End command buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record graphics command buffer!");
    }
}

void vulkan::drawFrame()
{
    EASY_FUNCTION(profiler::colors::Green400);
    EASY_VALUE("currentFrame", currentFrame);

    if (isResizing)
    {
        device->querySurfaceCapabilities();
        if (device->isCurrentSurfaceExtentZero())
            return;

        handleSurfaceResize();
        isResizing = false;
    }

    // wait until queue has finished processing previous graphicsCommandBuffers[currentFrame]
    // TODO: Maybe we should wait for presentCompleteFence instead?
    EASY_BLOCK("Wait for Fence");
    VkResult res1 = vkWaitForFences(*device, 1, &drawingFinishedFences[currentFrame], VK_TRUE, UINT64_MAX);
    if (res1 == VK_TIMEOUT)
    {
        SPDLOG_ERROR("Fence timeout");
    }
    EASY_END_BLOCK;
    // Acquire next available image
    EASY_BLOCK("Aquire next img");
    uint32_t swapchainImageIndex;
    VkAcquireNextImageInfoKHR acquireInfo{
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext = nullptr,
        .swapchain = *swapchain,
        .timeout = UINT64_MAX,
        .semaphore = imageAquiredSemaphores[currentFrame],
        .fence = VK_NULL_HANDLE,
        .deviceMask = 1, // TODO: For now assuming only device with bit 0 set
    };

    VkResult result = vkAcquireNextImage2KHR(*device, &acquireInfo, &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        isResizing = true;
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swapchain image!");
    }
    EASY_END_BLOCK;

    EASY_BLOCK("Record command buffers");
    // Reset a fence indicating that drawing has been finished
    vkResetFences(*device, 1, &drawingFinishedFences[currentFrame]);
    recordGraphicsCommandBuffer(graphicsCommandBuffers[currentFrame], swapchainImageIndex);
    recordPresentCommandBuffer(presentCommandBuffers[currentFrame], swapchainImageIndex);
    EASY_END_BLOCK;

    // Graphics queue submit info
    EASY_BLOCK("Queue submit");
    VkCommandBufferSubmitInfo graphicsCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = graphicsCommandBuffers[currentFrame],
    };

    // Sempahore to wait on until image acquisition is complete. stageMask defines second synchronization scope
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#synchronization-semaphores-waiting
    // The second synchronization scope includes every command submitted in the same batch. In the case of vkQueueSubmit,
    // the second synchronization scope is limited to operations on the pipeline stages determined by the destination stage mask
    // specified by the corresponding element of pWaitDstStageMask. In the case of vkQueueSubmit2, the second synchronization
    // scope is limited to the pipeline stage specified by VkSemaphoreSubmitInfo::stageMask. Also, in the case of either
    // vkQueueSubmit2 or vkQueueSubmit, the second synchronization scope additionally includes all commands that occur later in
    // submission order.
    VkSemaphoreSubmitInfo imageAquiredSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = imageAquiredSemaphores[currentFrame],
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSemaphoreSubmitInfo renderFinishedSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = renderFinishedSemaphores[currentFrame],
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSemaphoreSubmitInfo transferFinishedSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = transferFinishedSemaphores[currentFrame],
        .stageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
    };

    std::array<VkSemaphoreSubmitInfo, 2> waitSemaphoresInfos = {
        imageAquiredSemaphoreSubmitInfo,
        transferFinishedSemaphoreSubmitInfo,
    };

    VkSubmitInfo2 graphicsQueueSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoresInfos.size()),
        .pWaitSemaphoreInfos = waitSemaphoresInfos.data(),
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &graphicsCommandBufferSubmitInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &renderFinishedSemaphoreSubmitInfo,
    };

    VkResult reult = vkQueueSubmit2(device->getGraphicsQueue(), 1, &graphicsQueueSubmitInfo, drawingFinishedFences[currentFrame]);
    if (result != VK_SUCCESS)
    {
        SPDLOG_CRITICAL("Error: {}", static_cast<int>(result));
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    EASY_END_BLOCK;

    //// present queue submission
    // VkSemaphoreSubmitInfo prePresentCompleteSemaphoreInfo{
    //     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    //     .semaphore = prePresentCompleteSemaphores[currentFrame],
    //     .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    // };

    // VkCommandBufferSubmitInfo presentCommandBufferInfo{
    //     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
    //     .commandBuffer = presentCommandBuffers[currentFrame],
    // };

    // VkSubmitInfo2 prePresentSubmitInfo{
    //     .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
    //     .waitSemaphoreInfoCount = 1,
    //     .pWaitSemaphoreInfos = &renderFinishedSemaphoreSubmitInfo,
    //     .commandBufferInfoCount = 1,
    //     .pCommandBufferInfos = &presentCommandBufferInfo,
    //     .signalSemaphoreInfoCount = 1,
    //     .pSignalSemaphoreInfos = &prePresentCompleteSemaphoreInfo,
    // };

    ////result = vkQueueSubmit2(device->getPresentQueue(), 1, &prePresentSubmitInfo, nullptr);
    // if (result != VK_SUCCESS)
    //{
    //     SPDLOG_CRITICAL("Error: {}", static_cast<int>(result));
    //     throw std::runtime_error("failed to submit present command buffer!");
    // }
    EASY_BLOCK("Present");

    VkSwapchainKHR swapchains = {*swapchain};
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &swapchains,
        .pImageIndices = &swapchainImageIndex,
        .pResults = nullptr,
    };

    // Presentation of current frame's image after renderFinishedSemaphore[currentFrame] is signalled
    vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);
    EASY_END_BLOCK;
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 * @brief Recreate swapchain and all dependent objects when surface size changes.
 *
 */
void vulkan::handleSurfaceResize()
{
    vkDeviceWaitIdle(*device);
    freeCommandBuffers();
    destroySyncObjects();

    {
        swapchain.reset();
        renderPass.reset();
        framebuffer.reset();
    }

    swapchain.reset(new Swapchain(device, window, surface));
    renderPass.reset(new RenderPass(device, swapchain));
    framebuffer.reset(new Framebuffer(device, swapchain, renderPass));

    createTransferCommandBuffers();
    createGraphicsCommandBuffers();
    createSyncObjects();
    swapchainAspectChanged = true;
    vkDeviceWaitIdle(*device);
}

/**
 * @brief Recreate frame resources when number of samples per pixel changes.
 *
 * When MSAA changes, framebuffer resources need to be recreated as their sample count is declared
 * during creation.
 */
void vulkan::handleMSAAChange()
{
    vkDeviceWaitIdle(*device);
    freeCommandBuffers();
    vkDestroyPipeline(*device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
    destroySyncObjects();

    {
        swapchain.reset();
        renderPass.reset();
        framebuffer.reset();
    }

    swapchain.reset(new Swapchain(device, window, surface));
    renderPass.reset(new RenderPass(device, swapchain));
    framebuffer.reset(new Framebuffer(device, swapchain, renderPass));

    createGraphicsPipeline();
    createTransferCommandBuffers();
    createGraphicsCommandBuffers();
    createSyncObjects();
    vkDeviceWaitIdle(*device);
}

/**
 * @brief Free allocated command buffers.
 *
 */
void vulkan::freeCommandBuffers()
{
    vkFreeCommandBuffers(*device, *graphicsCommandPool, static_cast<uint32_t>(graphicsCommandBuffers.size()),
                         graphicsCommandBuffers.data());
    vkFreeCommandBuffers(*device, *transferCommandPool, static_cast<uint32_t>(transferCommandBuffers.size()),
                         transferCommandBuffers.data());

    SPDLOG_TRACE("[Command buffers] Freed");
}

/**
 *  @brief Create semaphores and fences for each frame in flight.
 *
 */
void vulkan::createSyncObjects()
{
    imageAquiredSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    prePresentCompleteSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    transferFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    drawingFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);
    transferFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &imageAquiredSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &prePresentCompleteSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &transferFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(*device, &fenceInfo, nullptr, &drawingFinishedFences[i]) != VK_SUCCESS ||
            vkCreateFence(*device, &fenceInfo, nullptr, &transferFinishedFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(drawingFinishedFences, "Drawing finished fence");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(transferFinishedFences, "Transfer finished fence");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(imageAquiredSemaphores, "Image acquired semaphore");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(renderFinishedSemaphores, "Render finished semaphore");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(prePresentCompleteSemaphores, "Pre-present complete semaphore");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(transferFinishedSemaphores, "Transfer finished semaphore");
    SPDLOG_TRACE("[Synchronization objects] Created");
}

/**
 * @brief Destroy syncronization objects.
 *
 */
void vulkan::destroySyncObjects()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(*device, imageAquiredSemaphores[i], nullptr);
        vkDestroySemaphore(*device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(*device, prePresentCompleteSemaphores[i], nullptr);
        vkDestroySemaphore(*device, transferFinishedSemaphores[i], nullptr);
        vkDestroyFence(*device, transferFinishedFences[i], nullptr);
        vkDestroyFence(*device, drawingFinishedFences[i], nullptr);
    }

    SPDLOG_TRACE("[Synchronization objects] Destroyed");
}

void vulkan::createVertexBindingDescriptors()
{
    // vertex coords
    vertexBindingDesc.emplace_back();
    vertexBindingDesc[0].binding = 0;
    vertexBindingDesc[0].stride = sizeof(glm::vec3);
    vertexBindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // vertex normals
    vertexBindingDesc.emplace_back();
    vertexBindingDesc[1].binding = 1;
    vertexBindingDesc[1].stride = sizeof(glm::vec3);
    vertexBindingDesc[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // vertex coords
    vertexAttrDesc.emplace_back();
    vertexAttrDesc[0].binding = 0;
    vertexAttrDesc[0].location = 0;
    vertexAttrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDesc[0].offset = 0; // ewentualnie jako offsetof(struct,field)

    // vertex normals
    vertexAttrDesc.emplace_back();
    vertexAttrDesc[1].binding = 1;
    vertexAttrDesc[1].location = 1;
    vertexAttrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrDesc[1].offset = 0; // ewentualnie jako offsetof(struct,field)
}

void vulkan::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(*device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize, false);

    vkWaitForFences(*device, 1, &transferFinishedFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);

    GSGE_DEBUGGER_SET_OBJECT_NAME(vertexBuffer, "Vertex buffer");
}

uint32_t vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

void vulkan::prepareVertexData(glm::vec3 *dataPtr, size_t len)
{
    vertices.assign(dataPtr, dataPtr + len);
}

void vulkan::prepareIndexData(glm::u16 *dataPtr, size_t len)
{
    indices.assign(dataPtr, dataPtr + len);
}

void vulkan::prepareNormalsData(glm::vec3 *dataPtr, size_t len)
{
    vertexNormals.assign(dataPtr, dataPtr + len);
}

void vulkan::prepareIndexOffsets(std::vector<uint32_t> data)
{
    indexOffsets.assign(data.begin(), data.end());
}

void vulkan::prepareVertexOffsets(std::vector<uint32_t> data)
{
    vertexOffsets.assign(data.begin(), data.end());
}

void vulkan::pushTransformMatricesToGpu(std::vector<DirectX::XMMATRIX> &data)
{
    transformMatrices = &data;
}

void vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                          VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(*device, buffer, bufferMemory, 0);
}

void vulkan::initResourceOwnerships()
{
    // Dummy memory barrier to release ownership of transform matrices buffer from graphics queue family
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkBeginCommandBuffer(graphicsCommandBuffers[i], &beginInfo);

        VkBufferMemoryBarrier2 transformMatricesBufferMB_RO{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = VK_NULL_HANDLE,
            .srcStageMask = 0,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT,
            .dstAccessMask = 0,
            .srcQueueFamilyIndex = device->getGraphicsQueueFamilyIdx(),
            .dstQueueFamilyIndex = device->getTransferQueueFamilyIdx(),
            .buffer = transformMatricesBuffer[i],
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        };

        VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &transformMatricesBufferMB_RO,
        };

        vkCmdPipelineBarrier2(graphicsCommandBuffers[i], &depInfo);
        vkEndCommandBuffer(graphicsCommandBuffers[i]);

        VkCommandBufferSubmitInfo cbsi{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = graphicsCommandBuffers[i],
        };

        VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cbsi,
        };

        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = 0,
        };

        VkFence fence;
        vkCreateFence(*device, &fenceInfo, nullptr, &fence);
        vkQueueSubmit2(device->getGraphicsQueue(), 1, &submitInfo, fence);
        vkWaitForFences(*device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(*device, fence, nullptr);
    }

    SPDLOG_TRACE("Initialized ownership of resources");
}

void vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, bool withSemaphores)
{
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkWaitForFences(*device, 1, &transferFinishedFences[currentFrame], VK_FALSE, UINT64_MAX);
    vkResetFences(*device, 1, &transferFinishedFences[currentFrame]);

    vkBeginCommandBuffer(transferCommandBuffers[currentFrame], &beginInfo);
    GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN(transferCommandBuffers[currentFrame], "transfer CB");

    VkBufferCopy2 copyRegion{
        .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .pNext = VK_NULL_HANDLE,
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };

    VkCopyBufferInfo2 cbi{
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .pNext = VK_NULL_HANDLE,
        .srcBuffer = srcBuffer,
        .dstBuffer = dstBuffer,
        .regionCount = 1,
        .pRegions = &copyRegion,
    };

    /*
    If buffer was created with VK_SHARING_MODE_EXCLUSIVE, and srcQueueFamilyIndex is not equal to dstQueueFamilyIndex,
    this memory barrier defines a queue family transfer operation.

    When executed on a queue in the family identified by srcQueueFamilyIndex,
    this barrier defines a queue family release operation for the specified buffer range, and the second synchronization
    and access scopes do not synchronize operations on that queue.

    When executed on a queue in the family identified by dstQueueFamilyIndex,
    this barrier defines a queue family acquire operation for the specified buffer range, and the first synchronization and access
    scopes do not synchronize operations on that queue.

    If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer is performed,
    and the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED.

    NOTE: If an application does not need the contents of a resource to remain valid when transferring from one queue family to
    another, then the ownership transfer should be skipped. - since transform matrices buffer is updated every frame we do not
    care about it's contents from previous frame and do not need to transfer ownership back from graphics queue family

    https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap7.html#synchronization-queue-transfers
    */

    VkBufferMemoryBarrier2 host_write_complete{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = VK_NULL_HANDLE,
        .srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
        .srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
        .buffer = srcBuffer,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    VkDependencyInfo host_write_depInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &host_write_complete,
    };

    vkCmdPipelineBarrier2(transferCommandBuffers[currentFrame], &host_write_depInfo);

    vkCmdCopyBuffer2(transferCommandBuffers[currentFrame], &cbi);

    if (withSemaphores)
    {
        // Release buffer ownership from transfer queue family
        // Second synchronization and access scopes do not synchronize operations on that queue
        VkBufferMemoryBarrier2 memBarrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = VK_NULL_HANDLE,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = 0,
            .dstAccessMask = 0,
            .srcQueueFamilyIndex = device->getTransferQueueFamilyIdx(),
            .dstQueueFamilyIndex = device->getGraphicsQueueFamilyIdx(),
            .buffer = dstBuffer,
            .offset = 0,
            .size = VK_WHOLE_SIZE,
        };

        VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &memBarrier,
        };

        vkCmdPipelineBarrier2(transferCommandBuffers[currentFrame], &depInfo);
    }

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_END(transferCommandBuffers[currentFrame]);
    vkEndCommandBuffer(transferCommandBuffers[currentFrame]);

    VkCommandBufferSubmitInfo cbSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = transferCommandBuffers[currentFrame],
    };

    // Semaphore to signal completion of transfer operation
    VkSemaphoreSubmitInfo transferSemaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = transferFinishedSemaphores[currentFrame],
        .stageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    };

    VkSubmitInfo2 submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cbSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<uint32_t>(withSemaphores ? 1 : 0),
        .pSignalSemaphoreInfos = withSemaphores ? &transferSemaphoreInfo : nullptr,
    };

    vkQueueSubmit2(device->getTransferQueue(), 1, &submitInfo, transferFinishedFences[currentFrame]);
}

void vulkan::createDescriptorSetLayouts()
{
    // uniform buffer descriptor set
    std::array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBinding{};
    descriptorSetLayoutBinding[0].binding = 0;
    descriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding[0].descriptorCount = 1;
    descriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorSetLayoutBinding[0].pImmutableSamplers = nullptr;

    // ssbo descriptor set
    descriptorSetLayoutBinding[1].binding = 1;
    descriptorSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding[1].descriptorCount = 1;
    descriptorSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorSetLayoutBinding[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
    layoutInfo.pBindings = descriptorSetLayoutBinding.data();

    if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    SPDLOG_TRACE("[Descriptor set layouts] Created");
}

void vulkan::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
                     uniformBuffersMemory[i]);
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(uniformBuffers, "Uniform buffer");
    SPDLOG_TRACE("[Uniform buffers] Created");
}

void vulkan::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSize{};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = 20,
        .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
        .pPoolSizes = poolSize.data(),
    };

    if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    SPDLOG_TRACE("[Descriptor pool] created");
}

void vulkan::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts.data(),
    };

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(*device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::array<VkDescriptorBufferInfo, 2> bufferInfo{};
        // UBO buffer info
        bufferInfo[0].buffer = uniformBuffers[i];
        bufferInfo[0].offset = 0;
        bufferInfo[0].range = sizeof(UniformBufferObject);

        // SSBO Buffer with object data (model transform for now)
        bufferInfo[1].buffer = transformMatricesBuffer[i];
        bufferInfo[1].offset = 0;
        bufferInfo[1].range = (*transformMatrices).size() * sizeof((*transformMatrices)[0]);

        std::array<VkWriteDescriptorSet, 2> descriptorWrite{};
        // UBO buffer descriptor write
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].dstSet = descriptorSets[i];
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].pBufferInfo = &bufferInfo[0];

        // SSBO buffer descriptor write
        descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[1].dstSet = descriptorSets[i];
        descriptorWrite[1].dstBinding = 1;
        descriptorWrite[1].dstArrayElement = 0;
        descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite[1].descriptorCount = 1;
        descriptorWrite[1].pBufferInfo = &bufferInfo[1];

        vkUpdateDescriptorSets(*device, 2, descriptorWrite.data(), 0, nullptr);
    }

    SPDLOG_TRACE("[Descriptor sets] created");
}

void vulkan::updateUniformBufferEx(UniformBufferObject ubo)
{
    local_ubo = ubo;
}

void vulkan::updateUniformBuffer(uint32_t currentImage)
{
    void *data;
    vkMapMemory(*device, uniformBuffersMemory[currentImage], 0, sizeof(local_ubo), 0, &data);
    memcpy(data, &local_ubo, sizeof(local_ubo));
    vkUnmapMemory(*device, uniformBuffersMemory[currentImage]);
}

void vulkan::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(*device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize, false);

    vkWaitForFences(*device, 1, &transferFinishedFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);

    GSGE_DEBUGGER_SET_OBJECT_NAME(indexBuffer, "Index buffer");
}

void vulkan::createTransformMatricesBuffer()
{
    VkDeviceSize bufferSize = sizeof((*transformMatrices)[0]) * (*transformMatrices).size();

    transformMatricesBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesStagingBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesStagingBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesMappedMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                     transformMatricesStagingBuffer[i], transformMatricesStagingBufferMemory[i]);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformMatricesBuffer[i], transformMatricesBufferMemory[i]);

        vkMapMemory(*device, transformMatricesStagingBufferMemory[i], 0, bufferSize, 0, &transformMatricesMappedMemory[i]);
    }

    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(transformMatricesBuffer, "Transform matrices buffer");
    GSGE_DEBUGGER_SET_INDEXED_OBJECT_NAME(transformMatricesStagingBuffer, "Transform matrices staging buffer");
}

void vulkan::updateTransformMatrixBuffer(uint32_t currentImage)
{
    VkDeviceSize bufferSize = sizeof((*transformMatrices)[0]) * (*transformMatrices).size();

    memcpy(transformMatricesMappedMemory[currentImage], (*transformMatrices).data(), static_cast<size_t>(bufferSize));

    // Flush memory from host cache
    VkMappedMemoryRange memoryRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = transformMatricesStagingBufferMemory[currentImage],
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    vkFlushMappedMemoryRanges(*device, 1, &memoryRange);

    copyBuffer(transformMatricesStagingBuffer[currentImage], transformMatricesBuffer[currentImage], bufferSize, true);
}

/**
 * \brief Check if view aspect of current surface has changed.
 *
 * \return true if view aspect changed
 * \return false if view aspect has not changed
 */
bool vulkan::viewAspectChanged()
{
    if (swapchainAspectChanged)
    {
        swapchainAspectChanged = false;
        return true;
    }
    else
    {
        return false;
    }
}

float vulkan::getViewAspect()
{
    return static_cast<float>(swapchain->getExtent().width) / static_cast<float>(swapchain->getExtent().height);
}

void vulkan::createVertexNormalsBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertexNormals[0]) * vertexNormals.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertexNormals.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(*device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexNormalsBuffer, vertexNormalsBufferMemory);

    copyBuffer(stagingBuffer, vertexNormalsBuffer, bufferSize, false);

    vkWaitForFences(*device, 1, &transferFinishedFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);

    GSGE_DEBUGGER_SET_OBJECT_NAME(vertexNormalsBuffer, "Vertex normals buffer");
}