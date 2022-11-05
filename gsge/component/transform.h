#pragma once

#include <glm/glm.hpp>

namespace component
{
struct alignas(32) transform
{
    // transform(glm::vec3 pos) : position(pos){};

    alignas(16) glm::vec3 position{0.f, 0.f, 0.f};
    // float p1{1.f};                                 // padding
    alignas(16) glm::vec3 rotation{0.f, 0.f, 0.f}; // in radians
    // float p2{0.f};                                 // padding
    alignas(16) glm::vec3 scale{1.0f, 1.0f, 1.0f};
    // float p3{0.f}; // padding
    alignas(16) glm::mat4 transformMatrix{1.0f};
};
} // namespace component
