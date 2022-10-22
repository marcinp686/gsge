#include "gsge.h"

gsge::~gsge()
{
}

void gsge::init()
{
    window = std::make_unique<Window>();

    // Fullscreen mode
    // window->settings.windowType = graphicsSettings::windowType::fullScreen;
    // window->settings.windowSize.width = 1920;
    // window->settings.windowSize.height = 1080;

    // windowed mode - default
    window->settings.windowSize.width = 1200;
    window->settings.windowSize.height = 600;

    window->createWindow();
    window->setTitle("Giraffe Game Engine");

    renderer = std::make_unique<vulkan>(window.get());

    level = std::make_unique<scene>();
    level->mainCamera.setAspect(1600.f / 800.f);
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
    while (!glfwWindowShouldClose(window->get_handle()))
    {
        glfwPollEvents();
        frameStats.update();

        level->update(frameStats.dt);
        renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
        renderer->updateUniformBufferEx(level->ubo);
        renderer->update();
    }
}
