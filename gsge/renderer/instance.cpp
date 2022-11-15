#include "instance.h"

Instance::Instance()
{
    prepareLayerList();

    if (!checkLayerSupport())
    {
        throw std::runtime_error("[ERROR] Instance layers not suppoprted.");
    }

    prepareExtensionList();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "GSGE Demo";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    appInfo.pEngineName = "Giraffe Studio Game Engine Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    createInfo.ppEnabledLayerNames = instanceLayers.data();

#ifndef NDEBUG

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugger->debugCallback;

    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

#endif

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("[ERROR] Vulkan instance creation failed.");
    }

#ifndef NDEBUG
    debugger->setInstance(instance);
#endif
}

Instance::~Instance()
{
#ifndef NDEBUG
    debugger->destroy();
#endif
    vkDestroyInstance(instance, nullptr);
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

    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

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
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    // Second call to actually enumerate layers
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
            return false;
        }
    }

    return true;
}