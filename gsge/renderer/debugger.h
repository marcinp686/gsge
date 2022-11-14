#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "instance.h"

class Debugger
{
  public:
    Debugger(std::shared_ptr<Instance> &instance);
    ~Debugger();

  private:
    std::shared_ptr<Instance> instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    void createDebugMessanger();

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);
};
