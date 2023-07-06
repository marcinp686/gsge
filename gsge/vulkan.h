#pragma once

#include <fstream>
#include <DirectXMath.h>

#include <vulkan/vulkan.h>
#pragma warning(suppress : 4275 6285 26498 26451 26800)
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
#include "renderer/commandPool.h"
#include "renderer/debugger.h"
#include "renderer/settings.h"

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
    void pushTransformMatricesToGpu(std::vector<DirectX::XMMATRIX>& data);
    void updateUniformBufferEx(UniformBufferObject ubo);
    void updateUniformBuffer(uint32_t currentImage);
    void updateTransformMatrixBuffer(uint32_t currentImage);

    bool viewAspectChanged();
    float getViewAspect();
    void handleMSAAChange();

  private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Surface> surface;
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Framebuffer> framebuffer;   
    std::unique_ptr<CommandPool> graphicsCommandPool;
    std::unique_ptr<CommandPool> transferCommandPool;
    std::unique_ptr<CommandPool> presentCommandPool;

    GSGE_DEBUGGER_INSTANCE_DECL;
    GSGE_SETTINGS_INSTANCE_DECL;

    uint32_t currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    bool swapchainAspectChanged{true};    
    bool isResizing{false};

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkVertexInputBindingDescription> vertexBindingDesc;
    std::vector<VkVertexInputAttributeDescription> vertexAttrDesc;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> graphicsCommandBuffers;
    std::vector<VkCommandBuffer> transferCommandBuffers;
    std::vector<VkCommandBuffer> presentCommandBuffers;

    std::vector<VkSemaphore> imageAquiredSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkSemaphore> prePresentCompleteSemaphores;
    std::vector<VkSemaphore> transferFinishedSemaphores;
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
    std::vector<DirectX::XMMATRIX>* transformMatrices;

    // shaders
    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    void loadShaders();
    VkShaderModule createShaderModule(const std::vector<char> &code);
    
    void destroyCommandPools();

    void createVertexBindingDescriptors();
    void createGraphicsPipeline();

    void createGraphicsCommandBuffers();
    void createTransferCommandBuffers();
    void createPresentCommandBuffers();
    void recordGraphicsCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordPresentCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    
    void handleSurfaceResize();

    void freeCommandBuffers();
    
    void createSyncObjects();
    void destroySyncObjects();

    void initResourceOwnerships();

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
