#include "gsge.h"

gsge::~gsge()
{
}

void gsge::init()
{
    window = std::make_shared<Window>();

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

    glfwSetWindowUserPointer(*window, this);
    glfwSetKeyCallback(*window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        gsge *instance = reinterpret_cast<gsge *>(glfwGetWindowUserPointer(window));
        instance->keyCallback(window, key, scancode, action, mods);
    });
}

void gsge::keyCallback(GLFWwindow *glfwWindow, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
        break;
    case GLFW_KEY_F:
        if (action == GLFW_PRESS)
        {
            if (settings.displayMode == ESettings::DisplayMode::Windowed)
                window->setFullScreenMode();
            else
                window->setWindowedMode();
        }
        break;
    case GLFW_KEY_P:
        if (action == GLFW_PRESS)
        {
            if (engineState == EEngine::State::Running)
            {
                engineState = EEngine::State::Paused;
                SPDLOG_INFO("Engine paused");
            }
            else
            {
                engineState = EEngine::State::Running;
                SPDLOG_INFO("Engine running");
            }
        }
        break;
    }
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
        
        mouse->update();

        if (engineState == EEngine::State::Paused)
        {
            Sleep(100);
            continue;
        }

        frameStats.update();

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
        // Q E controls
        if (glfwGetKey(*window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            level->mainCamera.moveDown(frameStats.dt);
        };
        if (glfwGetKey(*window, GLFW_KEY_E) == GLFW_PRESS)
        {
            level->mainCamera.moveUp(frameStats.dt);
        };

        level->update(frameStats.dt);

        renderer->pushTransformMatricesToGpu(level->getTransformMatricesLump());
        renderer->updateUniformBufferEx(level->ubo);
        renderer->update();

        if (renderer->viewAspectChanged())
            level->mainCamera.setAspect(renderer->getViewAspect());
    }
}
