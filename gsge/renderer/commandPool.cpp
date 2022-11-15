#include "commandPool.h"

CommandPool::CommandPool(std::shared_ptr<Device> &device, uint32_t queueIndex, const char *name) : device(device)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueIndex;

    if (vkCreateCommandPool(*device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

    if (!strlen(name))
        debugger->setObjectName(pool, name);

    spdlog::trace("Created {}", name);
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(*device, pool, nullptr);
}
