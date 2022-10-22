#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include "vkProxies.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GLFW/glfw3.h>

#include "timer.h"
#include "types.h"
#include "renderer/window.h"
#include "renderer/instance.h"
#include "renderer/surface.h"

class vulkan
{
  public:
    vulkan(Window *_window) : window(_window){};
    ~vulkan();

    void init();
    void update();

    void prepareVertexData(glm::vec3 *dataPtr, size_t length);
    void prepareIndexData(glm::u16 *dataPtr, size_t length);
    void prepareNormalsData(glm::vec3 *dataPtr, size_t len);
    void prepareIndexOffsets(std::vector<uint32_t> data);
    void prepareVertexOffsets(std::vector<uint32_t> data);
    void pushTransformMatricesToGpu(std::vector<glm::mat4> data);
    void updateUniformBufferEx(UniformBufferObject ubo);
    void updateUniformBuffer(uint32_t currentImage);
    void updateTransformMatrixBuffer(uint32_t currentImage);

    Window *window;

  private:
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkDevice device;
    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;
    // VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D swapChainExtent;

    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transferCommandPool;

    std::vector<VkVertexInputBindingDescription> vertexBindingDesc;
    std::vector<VkVertexInputAttributeDescription> vertexAttrDesc;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> graphicsCommandBuffers;
    std::vector<VkCommandBuffer> transferCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer vertexNormalsBuffer;
    VkDeviceMemory vertexNormalsBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    // transform matrices buffers
    std::vector<VkBuffer> transformMatricesStagingBuffer;
    std::vector<VkDeviceMemory> transformMatricesStagingBufferMemory;
    std::vector<VkBuffer> transformMatricesBuffer;
    std::vector<VkDeviceMemory> transformMatricesBufferMemory;

    UniformBufferObject local_ubo;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u16> indices;
    std::vector<glm::vec3> vertexNormals;
    std::vector<uint32_t> indexOffsets;
    std::vector<uint32_t> vertexOffsets;
    std::vector<glm::mat4> transformMatrices;

    // shaders
    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    void loadShaders();
    VkShaderModule createShaderModule(const std::vector<char> &code);

    void pickPhysicalDevice();
    uint64_t rateDeviceSuitability(VkPhysicalDevice device);
    void findQueueFamilies();
    void printQueueFamilies();
    void createLogicalDevice();
    void createQueues();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createVertexBindingDescriptors();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createGraphicsCommandPool();
    void createTransferCommandPool();
    void createGraphicsCommandBuffers();
    void createTransferCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void createSyncObjects();
    void cleanupSwapchain();
    void recreateSwapChain();
    void createVertexBuffer();
    void createIndexBuffer();
    void createVertexNormalsBuffer();
    void createTransformMatricesBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createDepthResources();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void cleanup();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    struct FamilyIndices
    {
        std::vector<uint32_t> graphics;
        std::vector<uint32_t> compute;
        std::vector<uint32_t> transfer;
        std::vector<uint32_t> sparse_binding;
        std::vector<uint32_t> protectedMem;
        std::vector<uint32_t> present;
    } queueFamilyIndices;

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
