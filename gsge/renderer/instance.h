#pragma once
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

class Instance
{
  public:
    Instance();
    ~Instance();

    operator VkInstance() const
    {
        return instance;
    }

  private:
    VkInstance instance{VK_NULL_HANDLE};

    void prepareLayerList();
    void prepareExtensionList();
    bool checkLayerSupport();

    std::vector<const char *> instanceLayers{};
    std::vector<const char *> instanceExtensions{};
};
