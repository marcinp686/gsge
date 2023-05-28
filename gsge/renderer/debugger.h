#pragma once

#include <memory>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#if !defined NDEBUG
#define GSGE_DEBUGGER_INSTANCE_DECL Debugger &debugger = Debugger::getInstance()
#define GSGE_DEBUGGER_SET_OBJECT_NAME(object, name) debugger.setObjectName(object, name)
#define GSGE_DEBUGGER_SET_INSTANCE(instance) debugger.setInstance(instance)
#define GSGE_DEBUGGER_SET_DEVICE(device) debugger.setDevice(device)
#define GSGE_DEBUGGER_DESTROY debugger.destroy()
#define GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN(commandBuffer, labelName) debugger.commandBufferLabelBegin(commandBuffer, labelName)
#define GSGE_DEBUGGER_CMD_BUFFER_LABEL_END(commandBuffer) debugger.commandBufferLabelEnd(commandBuffer)
#else
#define GSGE_DEBUGGER_INSTANCE_DECL
#define GSGE_DEBUGGER_SET_OBJECT_NAME __noop
#define GSGE_DEBUGGER_SET_INSTANCE __noop
#define GSGE_DEBUGGER_SET_DEVICE __noop
#define GSGE_DEBUGGER_DESTROY __noop
#define GSGE_DEBUGGER_CMD_BUFFER_LABEL_BEGIN __noop
#define GSGE_DEBUGGER_CMD_BUFFER_LABEL_END __noop
#endif

class Debugger
{
  public:
    static Debugger &getInstance();
    void destroy();
    void setInstance(VkInstance &instance);
    void setDevice(VkDevice &device);

    template <typename T> void setObjectName(T object, const char *name);

    void commandBufferLabelBegin(VkCommandBuffer &commandBuffer, const char *labelName);
    void commandBufferLabelEnd(VkCommandBuffer &commandBuffer);
    void queueLabelBegin(VkQueue &queue, const char *labelName);
    void queueLabelEnd(VkQueue &queue);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

  private:   

    Debugger();
    Debugger(Debugger &&) = delete;
    Debugger &operator=(Debugger &&) = delete;
    Debugger(const Debugger &) = delete;
    Debugger &operator=(const Debugger &) = delete;
    
    VkInstance instance;
    VkDevice device;

    VkDebugUtilsMessengerEXT debugMessenger;

    void setupDebugFuctionPointers();
    void createDebugMessanger();

    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = VK_NULL_HANDLE;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = VK_NULL_HANDLE;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT = VK_NULL_HANDLE;
    PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT = VK_NULL_HANDLE;
};