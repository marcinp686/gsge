#include "commandPool.h"

CommandPool::CommandPool(std::shared_ptr<Device> &device, uint32_t queueIndex, const char *name) : device(device)
{
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueIndex,
    };

    if (vkCreateCommandPool(*device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("[Command pool] failed to create command pool!");
    }

    if (!strlen(name))
        GSGE_DEBUGGER_SET_OBJECT_NAME(pool, name);

    SPDLOG_TRACE("[Command pool] Created {}", name);
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(*device, pool, nullptr);

    SPDLOG_TRACE("[Command pool] Destroyed");
}
