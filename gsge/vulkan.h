#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include <vulkan/vulkan.h>
#include "vkProxies.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GLFW/glfw3.h>

#include "timer.h"

class vulkan
{
  public:
    vulkan();
    ~vulkan();

    void init();
    void update();
    bool framebufferResized = false;
    void prepareVertexData(glm::vec3 *dataPtr, size_t length);
    void prepareIndexData(glm::u16 *dataPtr, size_t length);
    void prepareNormalsData(glm::vec3 *dataPtr, size_t len);
    void prepareIndexOffsets(std::vector<uint32_t> data);
    void prepareVertexOffsets(std::vector<uint32_t> data);
    // void updateUniformBuffer(void *ubo, size_t size);
    void updateUniformBuffer(uint32_t currentImage);

    GLFWwindow *window;

  private:
    const uint32_t WINDOW_WIDTH = 800;
    const uint32_t WINDOW_HEIGHT = 600;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    timer systemTimer;
    float dt = 0;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkDevice device;
    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue transferQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D swapChainExtent;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char *> instanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkVertexInputBindingDescription> vertexBindingDesc;
    std::vector<VkVertexInputAttributeDescription> vertexAttrDesc;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer vertexNormalsBuffer;
    VkDeviceMemory vertexNormalsBufferMemory;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u16> indices;
    std::vector<glm::vec3> vertexNormals;
    std::vector<uint32_t> indexOffsets;

    std::vector<uint32_t> vertexOffsets;

    // shaders
    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    void loadShaders();
    VkShaderModule createShaderModule(const std::vector<char> &code);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    void initWindow();
    void createInstance();
    bool checkValidationLayerSupport();
    void setupDebugMessanger();
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
    void createDescriptors();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void createSyncObjects();
    void cleanupSwapchain();
    void recreateSwapChain();
    void createVertexBuffer();
    void createIndexBuffer();
    void createVertexNormalsBuffer();
    void createTransformBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createDescriptorSetLayout();
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

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 normal;
        alignas(16) glm::vec3 lightPos;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
