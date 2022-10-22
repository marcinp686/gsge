#pragma once
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

class Device
{
  public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();
    VkDevice get_handle() const;
    VkPhysicalDevice getPhysicalDeviceHandle() const;

    uint32_t getGraphicsQueueFamilyIdx();
    uint32_t getTransferQueueFamilyIdx();
    uint32_t getPresentQueueFamilyIdx();

    VkQueue getGraphicsQueue() const;
    VkQueue getTransferQueue() const;
    VkQueue getPresentQueue() const;

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceFeatures deviceFeatures{};
    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void pickPhysicalDevice();
    uint64_t rateDeviceSuitability(VkPhysicalDevice device);
    void findQueueFamilies();
    void createLogicalDevice();
    void createQueues();

    struct FamilyIndices
    {
        std::vector<uint32_t> graphics;
        std::vector<uint32_t> compute;
        std::vector<uint32_t> transfer;
        std::vector<uint32_t> sparse_binding;
        std::vector<uint32_t> protectedMem;
        std::vector<uint32_t> present;
    } queueFamilyIndices;
};
