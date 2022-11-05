#pragma once
#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#include "../vkProxies.h"

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

class Instance
{
  public:
    Instance();
    ~Instance();

    VkInstance get_handle() const;

  private:
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger;

    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2"};
    std::vector<const char *> instanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_get_physical_device_properties2"};

    bool checkValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);
    void setupDebugMessanger();

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
