#pragma once

#include <fstream>

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <easy/profiler.h>
#include <easy/arbitrary_value.h>

#include "types.h"
#include "renderer/window.h"
#include "renderer/instance.h"
#include "renderer/surface.h"
#include "renderer/device.h"
#include "renderer/swapchain.h"
#include "renderer/renderPass.h"
#include "renderer/framebuffer.h"
#include "renderer/debugger.h"

class vulkan
{
  public:
    vulkan(std::shared_ptr<Window> &window) : window(window){};
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

    bool viewAspectChanged();
    float getViewAspect();

  private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Surface> surface;
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Framebuffer> framebuffer;
    std::unique_ptr<Debugger> debugger;

    uint32_t currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    bool swapchainAspectChanged = true;

    VkPipelineLayout pipelineLayout;
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

    std::vector<VkSemaphore> imageAquiredSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> drawingFinishedFences;
    std::vector<VkFence> transferFinishedFences;

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

    void cleanup();

    void createVertexBindingDescriptors();
    void createGraphicsPipeline();

    void createGraphicsCommandPool();
    void createTransferCommandPool();
    void createGraphicsCommandBuffers();
    void createTransferCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void createSyncObjects();

    void createVertexBuffer();
    void createIndexBuffer();
    void createVertexNormalsBuffer();
    void createTransformMatricesBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, bool withSemaphores);
    void createDescriptorSetLayouts();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
};
