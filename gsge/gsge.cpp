#include "gsge.h"

gsge::~gsge()
{
}

void gsge::init()
{
    renderer = std::make_unique<vulkan>();
    level = std::make_unique<scene>();
    level->initScene();
    level->prepareFrameData();
    level->update(0.0f);
    uploadBuffersToGPU();
    renderer->init();
}

void gsge::cleanup()
{
}

void gsge::uploadBuffersToGPU()
{
    renderer->prepareVertexData(level->getVertexLump().data(), level->getVertexLump().size());
    renderer->prepareIndexData(level->getIndexLump().data(), level->getIndexLump().size());
    renderer->prepareNormalsData(level->getNormalLump().data(), level->getNormalLump().size());
    renderer->prepareIndexOffsets(level->getIndexOffsets());
    renderer->prepareVertexOffsets(level->getVertexOffsets());
    renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
}

void gsge::mainLoop()
{
    while (!glfwWindowShouldClose(renderer->window))
    {
        float dt = frameTime.resetTimer();

        glfwSetWindowTitle(renderer->window, std::to_string(1 / dt).c_str());
        glfwPollEvents();

        level->update(dt);
        renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
        renderer->updateUniformBufferEx(level->ubo);
        renderer->update();
    }
}
