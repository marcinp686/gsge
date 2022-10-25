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
    glfwSetInputMode(window->get_handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window->get_handle()))
    {
        glfwPollEvents();
        frameStats.update();

        glfwGetCursorPos(window->get_handle(), &currentMouseXpos, &currentMouseYpos);
        if (oldMouseXpos != currentMouseXpos)
        {
            mouseDX = oldMouseXpos - currentMouseXpos;
            oldMouseXpos = currentMouseXpos;
        }
        else
            mouseDX = 0;

        if (oldMouseYpos != currentMouseYpos)
        {
            mouseDY = oldMouseYpos - currentMouseYpos;
            oldMouseYpos = currentMouseYpos;
        }
        else
            mouseDY = 0;

        if (mouseDX || mouseDY)
        {
            level->mainCamera.update(frameStats.dt, mouseDX, mouseDY);
        }

        if (glfwGetKey(window->get_handle(), GLFW_KEY_A) == GLFW_PRESS)
        {
            level->mainCamera.strafeLeft(frameStats.dt);
        };
        if (glfwGetKey(window->get_handle(), GLFW_KEY_D) == GLFW_PRESS)
        {
            level->mainCamera.strafeRight(frameStats.dt);
        };
        if (glfwGetKey(window->get_handle(), GLFW_KEY_W) == GLFW_PRESS)
        {
            level->mainCamera.moveForward(frameStats.dt);
        };
        if (glfwGetKey(window->get_handle(), GLFW_KEY_S) == GLFW_PRESS)
        {
            level->mainCamera.moveBackward(frameStats.dt);
        };
        if (glfwGetKey(window->get_handle(), GLFW_KEY_F) == GLFW_PRESS)
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
