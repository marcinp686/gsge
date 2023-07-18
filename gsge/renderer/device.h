#pragma once
#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm>

#include <string>
#include <sstream>

#include <vulkan/vulkan.h>

#pragma warning(suppress : 4275 6285 26498 26451 26800)
#include <spdlog/spdlog.h>

#include "instance.h"
#include "surface.h"
#include "debugger.h"
#include "core/tools.h"

class Queue
{
  public:
    uint32_t index;   // Queue index in the queue family
    VkQueue *handle;  // Queue handle
    const char* name; // Name of the queue for VK_EXT_debug_utils extension purposes
};

class QueueFamily
{
  public:
    uint32_t index;                      // Queue family index
    std::vector<Queue> queues;           // Queues to be created in the queue family
    std::vector<float> priorities;       // Priorities of the queues in the queue family
    VkQueueFamilyProperties2 properties; // Properties of the queue family
    VkBool32 presentSupport;             // Does the queue family support presenting images to the surface
};

// TODO: Add list of supported extensions and verify if enabled device extensions are present
class Device
{
  public:
    Device(std::shared_ptr<Instance> &instance, std::shared_ptr<Surface> &surface);
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;
    ~Device();

    VkPhysicalDevice getPhysicalDeviceHandle() const;

    void querySurfaceCapabilities();
    void enumerateSurfaceFormats();
    void enumerateSurfacePresentModes();
    bool isCurrentSurfaceExtentZero() const;

    uint32_t getGraphicsQueueFamilyIdx() const;
    uint32_t getTransferQueueFamilyIdx() const;
    uint32_t getPresentQueueFamilyIdx() const;
    uint32_t getComputeQueueFamilyIdx() const;

    VkQueue getGraphicsQueue() const;
    VkQueue getTransferQueue() const;
    VkQueue getComputeQueue() const;
    VkQueue getPresentQueue() const;

    VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const;
    std::vector<VkSurfaceFormatKHR> getSurfaceFormats() const;
    std::vector<VkPresentModeKHR> getSurfacePresentModes() const;

    inline operator VkDevice()
    {
        return device;
    }

  private:
    VkDevice device;
    uint32_t vendorID; // Vendor ID of the selected physical device. Currently AMD, NVIDIA and Intel are supported

    GSGE_DEBUGGER_INSTANCE_DECL;

    std::shared_ptr<Instance> instance;
    std::shared_ptr<Surface> surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    std::vector<QueueFamily> queueFamilies;             // Vector of queue families and queues to create
    std::map<uint32_t, uint32_t> queueFamilyIdxCount; // Count of queues to create for each queue family

    void addQueueToCreate(uint32_t familyIdx, float priority, VkQueue *handle, const char *queueName);

    VkQueue graphicsQueue; // Queue for graphics commands
    VkQueue computeQueue;  // Queue for compute commands
    VkQueue transferQueue; // Queue for transfer commands
    VkQueue presentQueue;  // Queue for presenting images to the swapchain

    uint32_t graphicsQueueFamilyIdx; // Index of the graphics queue family
    uint32_t computeQueueFamilyIdx;  // Index of the compute queue family
    uint32_t transferQueueFamilyIdx; // Index of the transfer queue family
    uint32_t presentQueueFamilyIdx;  // Index of the present queue family

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> surfacePresentModes;

    struct physicalDeviceFeatures
    {
        VkPhysicalDeviceFeatures2 v10{};
        VkPhysicalDeviceVulkan11Features v11{};
        VkPhysicalDeviceVulkan12Features v12{};
        VkPhysicalDeviceVulkan13Features v13{};
    } physDevFeaturesAvailable, physDevFeaturesSelected;

    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void pickPhysicalDevice();
    uint64_t rateDeviceSuitability(VkPhysicalDevice dev);
    void findQueueFamilies();
    void createLogicalDevice();
    void createQueues();
    void selectPhysicalDevFeatures();
};
