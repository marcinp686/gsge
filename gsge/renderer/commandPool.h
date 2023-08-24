#pragma once

#include <memory>

#include "device.h"
#include "debugger.h"
#include "core/tools.h"

class CommandPool
{
  public:
    CommandPool(std::shared_ptr<Device> &device, uint32_t queueIndex, const char *name = "");
    CommandPool(const CommandPool &) = delete;
    CommandPool &operator=(const CommandPool &) = delete;
    ~CommandPool();

    inline operator VkCommandPool()
    {
        return pool;
    }

  private:
    std::shared_ptr<Device> device;
    
    VkCommandPool pool = VK_NULL_HANDLE;
    
    GSGE_DEBUGGER_INSTANCE_DECL;
};
