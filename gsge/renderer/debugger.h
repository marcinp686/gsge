#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "instance.h"
#include "device.h"

class Debugger
{
  public:
    Debugger(std::shared_ptr<Instance> &instance);
    ~Debugger();

    void setDevice(std::shared_ptr<Device> &device);

    void setObjectName(VkFence &fence, const char *name);
    void setObjectName(VkCommandBuffer &commandBuffer, const char *name);

    void commandBufferLabelBegin(VkCommandBuffer &commandBuffer, const std::string &labelName);
    void commandBufferLabelEnd(VkCommandBuffer &commandBuffer);
    void queueLabelBegin(VkQueue &queue, const std::string &labelName);
    void queueLabelEnd(VkQueue &queue);

  private:
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Device> device;

    VkDebugUtilsMessengerEXT debugMessenger;

    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    void setupDebugFuctionPointers();
    void createDebugMessanger();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = VK_NULL_HANDLE;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = VK_NULL_HANDLE;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT = VK_NULL_HANDLE;
};
