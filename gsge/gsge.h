#pragma once

#include <memory>
#include <string>
#include <vector>
#include <format>

#include "vulkan.h"

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure

#include <entt/entity/registry.hpp>

#include <glm/glm.hpp>

#include "scene.h"
#include "renderer/window.h"
#include "renderer/settings.h"
#include "core/stats.h"
#include "controller/mouse.h"

class gsge
{
  public:
    ~gsge();
    void init();
    void cleanup();
    void mainLoop();

  private:
    stats frameStats;

    std::unique_ptr<Window> window;
    std::unique_ptr<Mouse> mouse;
    std::unique_ptr<vulkan> renderer;
    std::unique_ptr<scene> level;

    void uploadBuffersToGPU();
};
