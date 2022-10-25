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
    std::unique_ptr<vulkan> renderer;
    std::unique_ptr<scene> level;

    void uploadBuffersToGPU();

    // TODO: temporary - move to specialised class
    double currentMouseXpos{0}, currentMouseYpos{0};
    double mouseDX{0}, mouseDY{0};
    double oldMouseXpos{0}, oldMouseYpos{0};
};
