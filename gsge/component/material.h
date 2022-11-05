#pragma once
#include <glm/glm.hpp>

namespace component
{
struct alignas(32) material
{
    glm::vec4 color{1.f};
};
} // namespace component
