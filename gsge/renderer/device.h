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

    void querySurfaceCapabilities();
    void enumerateSurfaceFormats();
    void enumerateSurfacePresentModes();

    uint32_t getGraphicsQueueFamilyIdx();
    uint32_t getTransferQueueFamilyIdx();
    uint32_t getPresentQueueFamilyIdx();

    VkQueue getGraphicsQueue() const;
    VkQueue getTransferQueue() const;
    VkQueue getPresentQueue() const;

    VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const;
    std::vector<VkSurfaceFormatKHR> getSurfaceFormats() const;
    std::vector<VkPresentModeKHR> getSurfacePresentModes() const;

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> surfacePresentModes;

    VkPhysicalDeviceFeatures vulkan10Features{};
    VkPhysicalDeviceVulkan13Features vulkan13features{};
    VkPhysicalDeviceVulkan12Features vulkan12features{};
    VkPhysicalDeviceVulkan11Features vulkan11features{};

    VkPhysicalDeviceFeatures2 deviceFeatures{};

    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_AMD_RASTERIZATION_ORDER_EXTENSION_NAME,
                                                  VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};

    void pickPhysicalDevice();
    uint64_t rateDeviceSuitability(VkPhysicalDevice dev);
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
