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
}

void gsge::mainLoop()
{
    while (!glfwWindowShouldClose(renderer->window))
    {
        float dt = frameTime.resetTimer();

        glfwSetWindowTitle(renderer->window, std::to_string(1 / dt).c_str());
        glfwPollEvents();

        // level->update(dt);
        //    renderer->updateUniformBuffer(static_cast<void *>(&level->ubo), sizeof(level->ubo));
        renderer->update();
    }
}
