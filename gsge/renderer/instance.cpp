#include "instance.h"

Instance::Instance()
{
    prepareLayerList();

    if (!checkLayerSupport())
    {
        throw std::runtime_error("[Instance] Instance layers not suppoprted.");
    }

    prepareExtensionList();

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "GSGE Demo",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0),
        .pEngineName = "Giraffe Studio Game Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (settings.Debugger.debugInstanceCreation)
    {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugger.debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
#endif

    GSGE_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
    GSGE_DEBUGGER_SET_INSTANCE(instance);
    SPDLOG_TRACE("[Instance] Created");
}

Instance::~Instance()
{
    GSGE_DEBUGGER_DESTROY;
    vkDestroyInstance(instance, nullptr);
    SPDLOG_TRACE("[Instance] Destroyed");
}

void Instance::prepareLayerList()
{
#ifndef NDEBUG
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
}

void Instance::prepareExtensionList()
{
#ifndef NDEBUG
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // !NDEBUG

    // get required extensions for glfw, basically VK_KHR_SURFACE and win32_something
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (size_t i = 0; i < glfwExtensionCount; ++i)
    {
        instanceExtensions.push_back(glfwExtensions[i]);
    }
}

bool Instance::checkLayerSupport()
{
    uint32_t layerCount;

    // First call to check layer count
    GSGE_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

    // Second call to actually enumerate layers
    std::vector<VkLayerProperties> availableLayers(layerCount);
    GSGE_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    for (const char *layerName : instanceLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            SPDLOG_ERROR("[Instance] \"{}\" instance layer not found", layerName);
            return false;
        }
    }
    SPDLOG_TRACE("[Instance] All selected instance layers are supported");
    return true;
}