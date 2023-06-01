#include "vulkan.h"

vulkan::~vulkan()
{
    cleanup();
}

void vulkan::update()
{
    EASY_FUNCTION(profiler::colors::Green200);
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

    // 4. create descriptor pool
    createDescriptorPool();

    // 5. create buffers for descriptor sets data
    createSyncObjects();
    createTransferCommandBuffers();
    createVertexBuffer();
    vkQueueWaitIdle(device->getTransferQueue());
    createIndexBuffer();
    vkQueueWaitIdle(device->getTransferQueue());
    createVertexNormalsBuffer();
    vkQueueWaitIdle(device->getTransferQueue());
    createTransformMatricesBuffer();
    createUniformBuffers();

    // 6. create actual descriptor sets - after buffer creation
    createDescriptorSets();

    createGraphicsCommandBuffers();
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

void vulkan::cleanup()
{
    vkDeviceWaitIdle(*device);

    // destroy uniform buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
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

    framebuffer.reset();
    renderPass.reset();

    destroySyncObjects();

    destroyCommandPools();

    swapchain.reset();
    device.reset();
    surface.reset();
    instance.reset();
}

/**
 * @brief Destroy command pools for graphics and transfer queues.
 * Frees command buffers allocated by the pools
 * *
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
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDesc.size());
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttrDesc.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->getExtent().width);
    viewport.height = static_cast<float>(swapchain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->getExtent();

    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    /*VkPipelineRasterizationStateRasterizationOrderAMD orderAMD = {};
    orderAMD.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD;
    orderAMD.rasterizationOrder = VK_RASTERIZATION_ORDER_RELAXED_AMD;*/

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional
    // rasterizer.pNext = &orderAMD;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional - to discard fragments lying outside of range
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    // pipelineLayoutInfo.pushConstantRangeCount = 0;
    // pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(*device, fragShaderModule, nullptr);
    vkDestroyShaderModule(*device, vertShaderModule, nullptr);

    GSGE_DEBUGGER_SET_OBJECT_NAME(graphicsPipeline, "Graphics pipeline");
    SPDLOG_TRACE("Created graphics pipeline");
}

void vulkan::createGraphicsCommandBuffers()
{
    graphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(graphicsCommandBuffers.size());

    if (vkAllocateCommandBuffers(*device, &allocInfo, graphicsCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    GSGE_DEBUGGER_SET_OBJECT_NAME(graphicsCommandBuffers[0], "Graphics command buffer 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(graphicsCommandBuffers[1], "Graphics command buffer 1");

    SPDLOG_TRACE("Created graphics command buffers");
}

void vulkan::createTransferCommandBuffers()
{
    transferCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *transferCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(transferCommandBuffers.size());

    if (vkAllocateCommandBuffers(*device, &allocInfo, transferCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    GSGE_DEBUGGER_SET_OBJECT_NAME(transferCommandBuffers[0], "Transfer command buffer 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(transferCommandBuffers[1], "Transfer command buffer 1");

    SPDLOG_TRACE("Created transfer command buffers");
}

void vulkan::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = 0;                  // Optional
    // beginInfo.pInheritanceInfo = nullptr; // Optional

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN(commandBuffer, "Graphics CB");

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Memory buffer barrier
    VkBufferMemoryBarrier2 memBarrier{};
    memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    memBarrier.pNext = VK_NULL_HANDLE;
    memBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    memBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
    memBarrier.srcQueueFamilyIndex = device->getTransferQueueFamilyIdx();
    memBarrier.dstQueueFamilyIndex = device->getGraphicsQueueFamilyIdx();
    memBarrier.buffer = transformMatricesBuffer[currentFrame];
    memBarrier.offset = 0;
    memBarrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pBufferMemoryBarriers = &memBarrier;
    depInfo.bufferMemoryBarrierCount = 1;

    vkCmdPipelineBarrier2(graphicsCommandBuffers[currentFrame], &depInfo);

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.05f, 0.05f, 0.05f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = *renderPass;
    renderPassInfo.framebuffer = (*framebuffer)[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain->getExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Set viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->getExtent().width);
    viewport.height = static_cast<float>(swapchain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    // Set up scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->getExtent();
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
        uint32_t endingIdx = (i == indexOffsets.size() - 1 ? indices.size() : indexOffsets[i + 1]);
        vkCmdDrawIndexed(commandBuffer, endingIdx - indexOffsets[i], 1, indexOffsets[i], vertexOffsets[i], i);
    }

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // End command buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_END(commandBuffer);
}

void vulkan::drawFrame()
{
    EASY_FUNCTION(profiler::colors::Green400);
    EASY_VALUE("currentFrame", currentFrame);

    uint32_t imageIndex;

    if (isResizing)
    {
        device->querySurfaceCapabilities();
        if (device->isCurrentSurfaceExtentZero())
            return;

        handleSurfaceResize();
        isResizing = false;
    }

    // wait until queue has finished processing previous graphicsCommandBuffers[currentFrame]
    EASY_BLOCK("Wait for Fence");
    VkResult res1 = vkWaitForFences(*device, 1, &drawingFinishedFences[currentFrame], VK_TRUE, UINT64_MAX);
    if (res1 == VK_TIMEOUT)
    {
        SPDLOG_ERROR("Fence timeout");
    }
    EASY_END_BLOCK;

    // Acquire next available image
    EASY_BLOCK("Aquire next img");
    VkResult result =
        vkAcquireNextImageKHR(*device, *swapchain, UINT64_MAX, imageAquiredSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

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

    EASY_BLOCK("Update buffers");
    updateTransformMatrixBuffer(currentFrame);
    updateUniformBuffer(currentFrame);
    EASY_END_BLOCK;

    EASY_BLOCK("Record command buffer");
    //  Reset a fence indicating that drawing has been finished
    vkResetFences(*device, 1, &drawingFinishedFences[currentFrame]);
    recordCommandBuffer(graphicsCommandBuffers[currentFrame], imageIndex);
    EASY_END_BLOCK;
    //  Queue submit info

    EASY_BLOCK("Queue submit");
    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.commandBuffer = graphicsCommandBuffers[currentFrame];

    VkSemaphoreSubmitInfo imageAquiredSemaphoreSubmitInfo{};
    imageAquiredSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    imageAquiredSemaphoreSubmitInfo.semaphore = imageAquiredSemaphores[currentFrame];

    VkSemaphoreSubmitInfo renderFinishedSemaphoreSubmitInfo{};
    renderFinishedSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    renderFinishedSemaphoreSubmitInfo.semaphore = renderFinishedSemaphores[currentFrame];

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &imageAquiredSemaphoreSubmitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &renderFinishedSemaphoreSubmitInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    if (vkQueueSubmit2(device->getGraphicsQueue(), 1, &submitInfo, drawingFinishedFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    EASY_END_BLOCK;

    EASY_BLOCK("Present");
    VkSwapchainKHR swapchains = {*swapchain};
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    // Queue presentation of current frame's image after renderFinishedSemaphore[currentFrame] is signalled
    vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);
    EASY_END_BLOCK;
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 * \brief Recreate swapchain and all dependent objects when surface size changes.
 *
 */
void vulkan::handleSurfaceResize()
{
    vkDeviceWaitIdle(*device);
    freeCommandBuffers();
    swapchain.reset(new Swapchain(device, window, surface));
    renderPass.reset(new RenderPass(device, swapchain));
    framebuffer.reset(new Framebuffer(device, swapchain, renderPass));
    destroySyncObjects();
    createTransferCommandBuffers();
    createGraphicsCommandBuffers();
    createSyncObjects();
    swapchainAspectChanged = true;
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

    SPDLOG_TRACE("Command buffers freed");
}

/**
 *  @brief Create semaphores and fences for each frame in flight.
 *
 */
void vulkan::createSyncObjects()
{
    imageAquiredSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    drawingFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);
    transferFinishedFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &imageAquiredSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(*device, &fenceInfo, nullptr, &drawingFinishedFences[i]) != VK_SUCCESS ||
            vkCreateFence(*device, &fenceInfo, nullptr, &transferFinishedFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    GSGE_DEBUGGER_SET_OBJECT_NAME(drawingFinishedFences[0], "Drawing Finished Fence 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(drawingFinishedFences[1], "Drawing Finished Fence 1");
    GSGE_DEBUGGER_SET_OBJECT_NAME(transferFinishedFences[0], "Transfer Finished Fence 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(transferFinishedFences[1], "Transfer Finished Fence 1");
    GSGE_DEBUGGER_SET_OBJECT_NAME(imageAquiredSemaphores[0], "Image Acquired Semaphore 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(imageAquiredSemaphores[1], "Image Acquired Semaphore 1");
    GSGE_DEBUGGER_SET_OBJECT_NAME(renderFinishedSemaphores[0], "Render Finished Semaphore 0");
    GSGE_DEBUGGER_SET_OBJECT_NAME(renderFinishedSemaphores[1], "Render Finished Semaphore 1");

    SPDLOG_TRACE("Created synchronization objects");
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
        vkDestroyFence(*device, transferFinishedFences[i], nullptr);
        vkDestroyFence(*device, drawingFinishedFences[i], nullptr);
    }

    SPDLOG_TRACE("Destroyed synchronization objects");
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
    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);
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

void vulkan::pushTransformMatricesToGpu(std::vector<glm::mat4> data)
{
    transformMatrices.assign(data.begin(), data.end());
}

void vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                          VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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

void vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, bool withSemaphores)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkWaitForFences(*device, 1, &transferFinishedFences[currentFrame], VK_FALSE, 2000000000);
    vkResetFences(*device, 1, &transferFinishedFences[currentFrame]);

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN(transferCommandBuffers[currentFrame], "transfer CB");

    vkBeginCommandBuffer(transferCommandBuffers[currentFrame], &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;

    vkCmdCopyBuffer(transferCommandBuffers[currentFrame], srcBuffer, dstBuffer, 1, &copyRegion);

    if (withSemaphores)
    {
        // Memory buffer barrier
        VkBufferMemoryBarrier2 memBarrier{};
        memBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        memBarrier.pNext = VK_NULL_HANDLE;
        memBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        memBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        memBarrier.srcQueueFamilyIndex = device->getTransferQueueFamilyIdx();
        memBarrier.dstQueueFamilyIndex = device->getGraphicsQueueFamilyIdx();
        memBarrier.buffer = transformMatricesBuffer[currentFrame];
        memBarrier.offset = 0;
        memBarrier.size = VK_WHOLE_SIZE;

        VkDependencyInfo depInfo{};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.pBufferMemoryBarriers = &memBarrier;
        depInfo.bufferMemoryBarrierCount = 1;

        vkCmdPipelineBarrier2(transferCommandBuffers[currentFrame], &depInfo);
    }
    vkEndCommandBuffer(transferCommandBuffers[currentFrame]);

    GSGE_DEBUGGER_CMD_BUFFER_LABEL_END(transferCommandBuffers[currentFrame]);

    VkCommandBufferSubmitInfo cbSubmitInfo{};
    cbSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cbSubmitInfo.commandBuffer = transferCommandBuffers[currentFrame];

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cbSubmitInfo;

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

    SPDLOG_TRACE("[Uniform buffers] Created");
}

void vulkan::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSize{};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = 20;

    if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    SPDLOG_TRACE("[Descriptor pool] created");
}

void vulkan::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

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
        bufferInfo[1].range = transformMatrices.size() * sizeof(transformMatrices[0]);

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

    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

void vulkan::createTransformMatricesBuffer()
{
    VkDeviceSize bufferSize = sizeof(transformMatrices[0]) * transformMatrices.size();

    transformMatricesBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesStagingBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    transformMatricesStagingBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     transformMatricesStagingBuffer[i], transformMatricesStagingBufferMemory[i]);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformMatricesBuffer[i], transformMatricesBufferMemory[i]);
    }
}

void vulkan::updateTransformMatrixBuffer(uint32_t currentImage)
{
    VkDeviceSize bufferSize = sizeof(transformMatrices[0]) * transformMatrices.size();
    void *data;
    vkMapMemory(*device, transformMatricesStagingBufferMemory[currentImage], 0, bufferSize, 0, &data);
    memcpy(data, transformMatrices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(*device, transformMatricesStagingBufferMemory[currentImage]);
    copyBuffer(transformMatricesStagingBuffer[currentImage], transformMatricesBuffer[currentImage], bufferSize, true);
}

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

    vkDestroyBuffer(*device, stagingBuffer, nullptr);
    vkFreeMemory(*device, stagingBufferMemory, nullptr);
}
