#include "debugger.h"

Debugger::Debugger(std::shared_ptr<Instance> &instance) : instance(instance)
{
    setupDebugFuctionPointers();
    createDebugMessanger();
}

Debugger::~Debugger()
{
    vkDestroyDebugUtilsMessengerEXT(*instance, debugMessenger, nullptr);
}

void Debugger::setupDebugFuctionPointers()
{
    vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");

    vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugUtilsMessengerEXT");

    vkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(*instance, "vkSetDebugUtilsObjectNameEXT");

    vkCmdBeginDebugUtilsLabelEXT =
        (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(*instance, "vkCmdBeginDebugUtilsLabelEXT");

    vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(*instance, "vkCmdEndDebugUtilsLabelEXT");

    vkQueueBeginDebugUtilsLabelEXT =
        (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(*instance, "vkQueueBeginDebugUtilsLabelEXT");

    vkQueueEndDebugUtilsLabelEXT =
        (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(*instance, "vkQueueEndDebugUtilsLabelEXT");
}

void Debugger::setDevice(std::shared_ptr<Device> &device)
{
    this->device = device;
}

void Debugger::setObjectName(VkFence &fence, const char *name)
{
    VkDebugUtilsObjectNameInfoEXT objectNameInfo{};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    objectNameInfo.objectType = VK_OBJECT_TYPE_FENCE;
    objectNameInfo.objectHandle = reinterpret_cast<uint64_t>(fence);   
    objectNameInfo.pObjectName = name;
    vkSetDebugUtilsObjectNameEXT(*device, &objectNameInfo);
}

void Debugger::setObjectName(VkCommandBuffer &commandBuffer, const char *name)
{
    VkDebugUtilsObjectNameInfoEXT objectNameInfo{};
    objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    objectNameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    objectNameInfo.objectHandle = reinterpret_cast<uint64_t>(commandBuffer);
    objectNameInfo.pObjectName = name;
    vkSetDebugUtilsObjectNameEXT(*device, &objectNameInfo);
}

void Debugger::commandBufferLabelBegin(VkCommandBuffer &commandBuffer, const std::string &labelName)
{
    VkDebugUtilsLabelEXT duLabel{};
    duLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    duLabel.pLabelName = labelName.c_str();

    vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &duLabel);
}

void Debugger::commandBufferLabelEnd(VkCommandBuffer &commandBuffer)
{
    vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void Debugger::queueLabelBegin(VkQueue &queue, const std::string &labelName)
{
    VkDebugUtilsLabelEXT duLabel{};
    duLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    duLabel.pLabelName = labelName.c_str();

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

    if (vkCreateDebugUtilsMessengerEXT(*instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Debugger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{    
    switch (messageSeverity)
    {
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(pCallbackData->pMessage);
        break;
    case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::debug(pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}