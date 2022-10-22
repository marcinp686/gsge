#include "device.h"

Device::Device(VkInstance instance, VkSurfaceKHR surface) : instance(instance), surface(surface)
{
    pickPhysicalDevice();
    findQueueFamilies();
    createLogicalDevice();
    createQueues();
}

Device::~Device()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice Device::get_handle() const
{
    return device;
}

VkPhysicalDevice Device::getPhysicalDeviceHandle() const
{
    return physicalDevice;
}

uint32_t Device::getGraphicsQueueFamilyIdx()
{
    return queueFamilyIndices.graphics[0];
}

uint32_t Device::getTransferQueueFamilyIdx()
{
    return queueFamilyIndices.transfer[2];
}

uint32_t Device::getPresentQueueFamilyIdx()
{
    return queueFamilyIndices.present[1];
}

VkQueue Device::getGraphicsQueue() const
{
    return graphicsQueue;
}

VkQueue Device::getTransferQueue() const
{
    return transferQueue;
}

VkQueue Device::getPresentQueue() const
{
    return presentQueue;
}

void Device::pickPhysicalDevice()
{
    // query the number of devices in system
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<uint64_t, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        uint64_t score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

uint64_t Device::rateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties2 deviceProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

    VkPhysicalDeviceMaintenance4Properties deviceMaintenance4Properties{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES};
    deviceProperties2.pNext = &deviceMaintenance4Properties;

    vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    uint64_t score = 0;

    std::stringstream deviceInfoStr;

    deviceInfoStr << deviceProperties2.properties.deviceName << " ("
                  << VK_API_VERSION_MAJOR(deviceProperties2.properties.apiVersion) << "."
                  << VK_API_VERSION_MINOR(deviceProperties2.properties.apiVersion) << "."
                  << VK_API_VERSION_PATCH(deviceProperties2.properties.apiVersion)
                  << "), maxBufferSize=" << static_cast<uint64_t>(deviceMaintenance4Properties.maxBufferSize) / 1024l / 1024l
                  << " ";

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    score += static_cast<uint64_t>(deviceMaintenance4Properties.maxBufferSize) / 1024l / 1024l;

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties2.properties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        return 0;
    }

    spdlog::info("Device " + deviceInfoStr.str() + " score: " + std::to_string(score));

    // deviceFeatures.wideLines = VK_TRUE;

    return score;
}

void Device::findQueueFamilies()
{
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertyCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyPropertyCount);

    for (auto &familyProp : queueFamilies) // set s_type for each entry;
    {
        familyProp.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        // Check for presentation support in Queue i;

        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentationSupport);

        if (presentationSupport == VK_TRUE)
        {
            queueFamilyIndices.present.push_back(i);
        }
        if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndices.graphics.push_back(i);
        }
        if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            queueFamilyIndices.compute.push_back(i);
        }
        if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            queueFamilyIndices.transfer.push_back(i);
        }
        if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
        {
            queueFamilyIndices.sparse_binding.push_back(i);
        }
        if (queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_PROTECTED_BIT)
        {
            queueFamilyIndices.protectedMem.push_back(i);
        }

        i++;
    }
}

void Device::createLogicalDevice()
{
    float graphicsQueuePriority = 1.0f;
    float presentQueuePriority = 1.0f;
    float transferQueuePriority = 1.0f;

    // Graphics queue
    VkDeviceQueueCreateInfo queueCreateInfo[3]{};
    queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[0].queueFamilyIndex = queueFamilyIndices.graphics[0];
    queueCreateInfo[0].queueCount = 1;
    queueCreateInfo[0].pQueuePriorities = &graphicsQueuePriority;

    // Transfer queue
    queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[1].queueFamilyIndex = queueFamilyIndices.transfer[2];
    queueCreateInfo[1].queueCount = 1;
    queueCreateInfo[1].pQueuePriorities = &transferQueuePriority;

    // Presentation queue
    queueCreateInfo[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[2].queueFamilyIndex = queueFamilyIndices.present[1];
    queueCreateInfo[2].queueCount = 1;
    queueCreateInfo[2].pQueuePriorities = &presentQueuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfo;
    createInfo.queueCreateInfoCount = 3;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkPhysicalDeviceShaderDrawParameterFeatures pf{};
    pf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    pf.shaderDrawParameters = VK_TRUE;
    pf.pNext = nullptr;

    createInfo.pNext = &pf;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
}

void Device::createQueues()
{
    vkGetDeviceQueue(device, queueFamilyIndices.present[1], 0, &presentQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.graphics[0], 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.transfer[2], 0, &transferQueue);
    std::stringstream infoMsg;
    infoMsg << "Device queues created: Presentation=" << presentQueue << "   Graphics=" << graphicsQueue
            << "   Transfer=" << transferQueue;
    spdlog::info(infoMsg.str());
}
