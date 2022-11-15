#pragma once

#include <memory>

#include "device.h"

class CommandPool
{
  public:
    CommandPool(std::shared_ptr<Device> &device, uint32_t queueIndex, const char *name = "");
    ~CommandPool();

    operator VkCommandPool()
    {
        return pool;
    }

  private:
    std::shared_ptr<Device> device;

    VkCommandPool pool = VK_NULL_HANDLE;
};
