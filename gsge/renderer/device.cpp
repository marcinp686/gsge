#include "device.h"

Device::Device(std::shared_ptr<Instance> &instance, std::shared_ptr<Surface> &surface) : instance(instance), surface(surface)
{
    pickPhysicalDevice();
    selectPhysicalDevFeatures();
    findQueueFamilies();
    createLogicalDevice();
    createQueues();
    querySurfaceCapabilities();
    enumerateSurfaceFormats();
    enumerateSurfacePresentModes();
}

Device::~Device()
{
    vkDestroyDevice(device, nullptr);

    SPDLOG_TRACE("[Device] Destroyed");
}

VkPhysicalDevice Device::getPhysicalDeviceHandle() const
{
    return physicalDevice;
}

uint32_t Device::getGraphicsQueueFamilyIdx() const
{
    return graphicsQueueFamilyIdx;
}

uint32_t Device::getComputeQueueFamilyIdx() const
{
    return computeQueueFamilyIdx;
}

uint32_t Device::getTransferQueueFamilyIdx() const
{
    return transferQueueFamilyIdx;
}

uint32_t Device::getPresentQueueFamilyIdx() const
{
    return presentQueueFamilyIdx;
}

VkQueue Device::getGraphicsQueue() const
{
    return graphicsQueue;
}

VkQueue Device::getComputeQueue() const
{
    return computeQueue;
}

VkQueue Device::getTransferQueue() const
{
    return transferQueue;
}

VkQueue Device::getPresentQueue() const
{
    return presentQueue;
}

void Device::querySurfaceCapabilities()
{
    GSGE_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, *surface, &surfaceCapabilities));
}

void Device::enumerateSurfaceFormats()
{
    uint32_t formatCount;
    GSGE_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *surface, &formatCount, nullptr));

    if (formatCount != 0)
    {
        surfaceFormats.resize(formatCount);
        GSGE_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, *surface, &formatCount, surfaceFormats.data()));
    }
}

void Device::enumerateSurfacePresentModes()
{
    uint32_t presentModeCount;
    GSGE_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *surface, &presentModeCount, nullptr));

    if (presentModeCount != 0)
    {
        surfacePresentModes.resize(presentModeCount);
        GSGE_CHECK_RESULT(
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, *surface, &presentModeCount, surfacePresentModes.data()));
    }
}

/**
 * @brief Checks if current device surface extent is {0,0}.
 *
 * @details If surface extent is {0,0} then window or app is minimized and we cannot create subsequent dependent objects
 * like images, views etc.
 */
bool Device::isCurrentSurfaceExtentZero() const
{
    if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0)
        return true;
    else
        return false;
}

void Device::pickPhysicalDevice()
{
    // query the number of devices in system
    uint32_t deviceCount = 0;
    GSGE_CHECK_RESULT(vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr));

    if (deviceCount == 0)
    {
        throw std::runtime_error("[Device] failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    GSGE_CHECK_RESULT(vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data()));

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<uint64_t, VkPhysicalDevice> candidates;

    for (const auto &dev : devices)
    {
        uint64_t score = rateDeviceSuitability(dev);
        candidates.insert(std::make_pair(score, dev));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        physicalDevice = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("[Device] Failed to find a suitable GPU!");
    }
}

uint64_t Device::rateDeviceSuitability(VkPhysicalDevice dev)
{
    VkPhysicalDeviceMaintenance4Properties deviceMaintenance4Properties{};
    deviceMaintenance4Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;

    VkPhysicalDeviceProperties2 deviceProperties2{};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &deviceMaintenance4Properties;

    vkGetPhysicalDeviceProperties2(dev, &deviceProperties2);

    // Vulkan 1.3 device features struct
    physDevFeaturesAvailable.v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

    // Vulkan 1.2 device features struct
    physDevFeaturesAvailable.v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    physDevFeaturesAvailable.v12.pNext = &physDevFeaturesAvailable.v13;

    // Vulkan 1.1 device features struct
    physDevFeaturesAvailable.v11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    physDevFeaturesAvailable.v11.pNext = &physDevFeaturesAvailable.v12;

    // Query available features for Vulkan 1.0-1.3
    physDevFeaturesAvailable.v10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physDevFeaturesAvailable.v10.pNext = &physDevFeaturesAvailable.v11;
    vkGetPhysicalDeviceFeatures2(dev, &physDevFeaturesAvailable.v10);

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
    if (!physDevFeaturesAvailable.v10.features.geometryShader)
    {
        return 0;
    }

    SPDLOG_TRACE("[Device] Selected: " + deviceInfoStr.str() + " score: " + std::to_string(score));

    vendorID = deviceProperties2.properties.vendorID;

    return score;
}

void Device::findQueueFamilies()
{
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertiesCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilyProperties(queueFamilyPropertiesCount,
                                                                {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties.data());

    uint32_t i = 0; // index of queue family
    for (const auto &properties : queueFamilyProperties)
    {
        VkBool32 presentationSupport = VK_FALSE;
        GSGE_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, *surface, &presentationSupport));

        queueFamilies.push_back(QueueFamily{.index = i, .properties = properties, .presentSupport = presentationSupport});
        i++;
    }

    // These values are failsafe hardocded for now. Let me know if you have better ideas.
    // Probably it would be best to perform benchmarking on per-device basis to figure out the best values.
    // TODO: Benchmarking
    // TODO: Check if QueueFamilyIdx's are valid based on the above vectors
    float graphicsQueuePriority;
    float computeQueuePriority;
    float presentQueuePriority;
    float transferQueuePriority;

    switch (vendorID)
    {
    case 0x1002: // AMD
        graphicsQueueFamilyIdx = 0;
        computeQueueFamilyIdx = 1;
        transferQueueFamilyIdx = 1;
        presentQueueFamilyIdx = 2;

        graphicsQueuePriority = 1.0f;
        computeQueuePriority = 1.0f;
        presentQueuePriority = 1.0f;
        transferQueuePriority = 1.0f;
        break;

    case 0x10DE: // Nvidia
        graphicsQueueFamilyIdx = 0;
        computeQueueFamilyIdx = 0;
        transferQueueFamilyIdx = 1;
        presentQueueFamilyIdx = 0;

        graphicsQueuePriority = 1.0f;
        computeQueuePriority = 1.0f;
        presentQueuePriority = 1.0f;
        transferQueuePriority = 1.0f;
        break;

    case 0x8086: // Intel
        graphicsQueueFamilyIdx = 0;
        computeQueueFamilyIdx = 1;
        transferQueueFamilyIdx = 2;
        presentQueueFamilyIdx = 3;

        graphicsQueuePriority = 1.0f;
        computeQueuePriority = 1.0f;
        presentQueuePriority = 1.0f;
        transferQueuePriority = 1.0f;
        break;

    default: // Fallback
        graphicsQueueFamilyIdx = 0;
        computeQueueFamilyIdx = 0;
        transferQueueFamilyIdx = 0;
        presentQueueFamilyIdx = 0;

        graphicsQueuePriority = 1.0f;
        computeQueuePriority = 1.0f;
        presentQueuePriority = 1.0f;
        transferQueuePriority = 1.0f;
        break;
    }

    addQueueToCreate(graphicsQueueFamilyIdx, graphicsQueuePriority, &graphicsQueue, "Graphics queue");
    addQueueToCreate(computeQueueFamilyIdx, computeQueuePriority, &computeQueue, "Compute queue");
    addQueueToCreate(transferQueueFamilyIdx, transferQueuePriority, &transferQueue, "Transfer queue");
    addQueueToCreate(presentQueueFamilyIdx, presentQueuePriority, &presentQueue, "Present queue");
}

void Device::addQueueToCreate(uint32_t familyIdx, float priority, VkQueue *handle, const char *queueName)
{
    Queue newQueue{
        .index = static_cast<uint32_t>(queueFamilies[familyIdx].queues.size()),
        .handle = handle,
        .name = queueName,
    };

    queueFamilies[familyIdx].queues.push_back(newQueue);
    queueFamilies[familyIdx].priorities.push_back(priority);
}

void Device::createLogicalDevice()
{
    // Create queues in selected queue family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (auto &family : queueFamilies)
    {
        if (family.queues.size())
        {
            VkDeviceQueueCreateInfo queueCreateInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            queueCreateInfo.queueFamilyIndex = family.index;
            queueCreateInfo.queueCount = family.queues.size();
            queueCreateInfo.pQueuePriorities = family.priorities.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = VK_NULL_HANDLE;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &physDevFeaturesSelected.v10;

    GSGE_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
    GSGE_DEBUGGER_SET_DEVICE(device);
    SPDLOG_TRACE("[Device] Created");
}

void Device::createQueues()
{
    for (auto &family : queueFamilies)
    {
        for (auto &queue : family.queues)
        {
            vkGetDeviceQueue(device, family.index, queue.index, queue.handle);
            GSGE_DEBUGGER_SET_OBJECT_NAME(*queue.handle, queue.name);
        }
    }

    SPDLOG_TRACE("[Device queues] Created");
}

void Device::selectPhysicalDevFeatures()
{
    // TODO: Stupid idea. All available features are enabled. Be more selective.
    physDevFeaturesSelected = physDevFeaturesAvailable;
    physDevFeaturesSelected.v10.features.robustBufferAccess = VK_FALSE;
}

VkSurfaceCapabilitiesKHR Device::getSurfaceCapabilities() const
{
    return surfaceCapabilities;
}

std::vector<VkSurfaceFormatKHR> Device::getSurfaceFormats() const
{
    return surfaceFormats;
}

std::vector<VkPresentModeKHR> Device::getSurfacePresentModes() const
{
    return surfacePresentModes;
}
