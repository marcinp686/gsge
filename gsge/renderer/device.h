#pragma once
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#include "instance.h"
#include "surface.h"
#include "debugger.h"

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

    VkQueue getGraphicsQueue() const;
    VkQueue getTransferQueue() const;
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

    GSGE_DEBUGGER_INSTANCE_DECL;

    std::shared_ptr<Instance> instance;
    std::shared_ptr<Surface> surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;

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
