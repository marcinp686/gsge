#pragma once

#include <memory>
#include <string>
#include <vector>
#include <format>

#include "vulkan.h"

#include <entt/entity/registry.hpp>

#include <glm/glm.hpp>
#include <easy/profiler.h>

#include "scene.h"
#include "renderer/window.h"
#include "renderer/settings.h"
#include "core/stats.h"
#include "controller/mouse.h"
#include <enums.h>

class gsge
{
  public:
    ~gsge();
    void init();
    void cleanup();
    void mainLoop();

  private:
    stats frameStats;
    GSGE_SETTINGS_INSTANCE_DECL;

    std::shared_ptr<Window> window;
    std::unique_ptr<Mouse> mouse;
    std::unique_ptr<vulkan> renderer;
    std::unique_ptr<scene> level;

    EEngine::State engineState{EEngine::State::Running};

    void uploadBuffersToGPU();

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};
