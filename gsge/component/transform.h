#pragma once

#include <glm/glm.hpp>

namespace component
{
struct transform
{
    // transform(glm::vec3 pos) : position(pos){};

    glm::vec3 position{0.f, 0.f, 0.f};
    glm::vec3 rotation{0.f, 0.f, 0.f}; // in radians
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::mat4 transformMatrix{1.0f};
};
} // namespace component
