#include "gsge.h"

gsge::~gsge()
{
}

void gsge::init()
{
    window = std::make_shared<Window>();

    // Fullscreen mode
    // window->settings.windowType = graphicsSettings::windowType::fullScreen;
    // window->settings.windowSize.width = 1920;
    // window->settings.windowSize.height = 1080;

    // windowed mode - default
    window->settings.windowSize.width = 1000;
    window->settings.windowSize.height = 600;

    window->createWindow();
    window->setTitle("Giraffe Game Engine");

    mouse = std::make_unique<Mouse>(window);
    renderer = std::make_unique<vulkan>(window);

    level = std::make_unique<scene>();
    level->initScene();
    level->prepareFrameData();
    level->update(0.0f);

    uploadBuffersToGPU();

    renderer->init();

    level->mainCamera.setAspect(renderer->getViewAspect());
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
    EASY_MAIN_THREAD;
    while (!glfwWindowShouldClose(*window))
    {
        EASY_BLOCK("mainLoop", profiler::colors::Blue200);
        glfwPollEvents();
        frameStats.update();
        mouse->update();

        if (mouse->dx != 0 || mouse->dy != 0)
        {
            level->mainCamera.update(frameStats.dt, mouse->dx, mouse->dy);
        }

        // W S A D controls
        if (glfwGetKey(*window, GLFW_KEY_A) == GLFW_PRESS)
        {
            level->mainCamera.strafeLeft(frameStats.dt);
        };
        if (glfwGetKey(*window, GLFW_KEY_D) == GLFW_PRESS)
        {
            level->mainCamera.strafeRight(frameStats.dt);
        };
        if (glfwGetKey(*window, GLFW_KEY_W) == GLFW_PRESS)
        {
            level->mainCamera.moveForward(frameStats.dt);
        };
        if (glfwGetKey(*window, GLFW_KEY_S) == GLFW_PRESS)
        {
            level->mainCamera.moveBackward(frameStats.dt);
        };

        // ESC exits application
        if (glfwGetKey(*window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            break;
        };

        // Toggle full screen and windowed mode
        if (glfwGetKey(*window, GLFW_KEY_F) == GLFW_PRESS)
        {
            if (window->settings.windowType == graphicsSettings::windowType::windowed)
                window->setFullScreenMode();
            else
                window->setWindowedMode();
        };

        level->update(frameStats.dt);

        renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
        renderer->updateUniformBufferEx(level->ubo);
        renderer->update();

        if (renderer->viewAspectChanged())
            level->mainCamera.setAspect(renderer->getViewAspect());
    }
}
