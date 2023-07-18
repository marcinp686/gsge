#pragma once
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#pragma warning(suppress : 4275 6285 26498 26451 26800)
#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#include "debugger.h"
#include "settings.h"
#include "core/tools.h"

class Instance
{
  public:
    Instance();
    Instance(const Instance &) = delete;
    Instance &operator=(const Instance &) = delete;
    ~Instance();

    inline operator VkInstance() const
    {
        return instance;
    }

  private:
    VkInstance instance{VK_NULL_HANDLE};

    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

    void prepareLayerList();
    void prepareExtensionList();
    bool checkLayerSupport();

    std::vector<const char *> instanceLayers{};
    std::vector<const char *> instanceExtensions{};
};
