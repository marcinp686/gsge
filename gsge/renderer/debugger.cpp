#include "debugger.h"

Debugger::Debugger()
{
}

void Debugger::setupDebugFuctionPointers()
{
    vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    vkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

    vkCmdBeginDebugUtilsLabelEXT =
        (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");

    vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");

    vkQueueBeginDebugUtilsLabelEXT =
        (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");

    vkQueueEndDebugUtilsLabelEXT =
        (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");

    SPDLOG_TRACE("[Debugger] Debug function pointers set up");
}

Debugger &Debugger::getInstance()
{   
    static Debugger debugUtilsInstance;
    return debugUtilsInstance;
}

void Debugger::destroy()
{
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    SPDLOG_TRACE("[Debugger] Destroyed");
}

void Debugger::setInstance(VkInstance &instance)
{
    this->instance = instance;
    setupDebugFuctionPointers();
    createDebugMessanger();
}

void Debugger::setDevice(VkDevice &device)
{
    this->device = device;

    SPDLOG_TRACE("[Debugger] Device set");
}

template void Debugger::setObjectName(VkFence object, const char *name);
template void Debugger::setObjectName(VkCommandPool object, const char *name);
template void Debugger::setObjectName(VkCommandBuffer object, const char *name);
template void Debugger::setObjectName(VkPipeline object, const char *name);
template void Debugger::setObjectName(VkQueue object, const char *name);
template void Debugger::setObjectName(VkSemaphore object, const char *name);
template void Debugger::setObjectName(VkImage object, const char *name);
template void Debugger::setObjectName(VkImageView object, const char *name);
template void Debugger::setObjectName(VkBuffer object, const char *name);

template <typename T> void Debugger::setObjectName(T object, const char *name)
{
    VkDebugUtilsObjectNameInfoEXT objectNameInfo{};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;

    if (std::is_same<VkFence, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_FENCE;
    if (std::is_same<VkCommandBuffer, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    if (std::is_same<VkCommandPool, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
    if (std::is_same<VkPipeline, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
    if (std::is_same<VkQueue, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_QUEUE;
    if (std::is_same<VkSemaphore, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    if (std::is_same<VkImage, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    if (std::is_same<VkImageView, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
    if (std::is_same<VkBuffer, T>::value)
        objectNameInfo.objectType = VK_OBJECT_TYPE_BUFFER;

    objectNameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
    objectNameInfo.pObjectName = name;
    vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
}

void Debugger::commandBufferLabelBegin(VkCommandBuffer &commandBuffer, const char *labelName)
{
    VkDebugUtilsLabelEXT duLabel{};
    duLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    duLabel.pLabelName = labelName;

    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &duLabel);
}

void Debugger::commandBufferLabelEnd(VkCommandBuffer &commandBuffer)
{
    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void Debugger::queueLabelBegin(VkQueue &queue, const char *labelName)
{
    VkDebugUtilsLabelEXT duLabel{};
    duLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    duLabel.pLabelName = labelName;

    vkQueueBeginDebugUtilsLabelEXT(queue, &duLabel);
}

void Debugger::queueLabelEnd(VkQueue &queue)
{
    vkQueueEndDebugUtilsLabelEXT(queue);
}

void Debugger::createDebugMessanger()
{
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    if (vkCreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }

    SPDLOG_TRACE("[Debugger] Debug messenger created");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Debugger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    switch (messageSeverity)
    {
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        SPDLOG_ERROR(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        SPDLOG_INFO(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        SPDLOG_WARN(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        SPDLOG_DEBUG(pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}