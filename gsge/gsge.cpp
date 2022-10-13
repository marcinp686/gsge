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
    size_t averageFrameCountNumber = 5;
    size_t averageFrameTimeCounter = 0;
    float totalFrameTimeCounter = 0.0f;

    while (!glfwWindowShouldClose(renderer->window))
    {
        float dt = frameTime.resetTimer();
        averageFrameTimeCounter++;
        totalFrameTimeCounter += dt;
        if (averageFrameTimeCounter == averageFrameCountNumber)
        {
            float fps = 1.0f / (totalFrameTimeCounter / averageFrameCountNumber);
            glfwSetWindowTitle(renderer->window, std::to_string(fps).c_str());
            averageFrameTimeCounter = 0;
            totalFrameTimeCounter = 0;
            averageFrameCountNumber = 1 + (fps / 4);
        }

        glfwPollEvents();

        level->update(dt);
        renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
        renderer->updateUniformBufferEx(level->ubo);
        renderer->update();
    }
}
