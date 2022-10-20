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

    VkInstance get() const;

  private:
    VkInstance instance{VK_NULL_HANDLE};

    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char *> instanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

    bool checkValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
