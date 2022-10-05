#pragma once

#include <glm/glm.hpp>
namespace component
{

// Motion description of an object
// Velocity/speed of rotation unit is units/s and degrees/s

struct motion
{
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
};
} // namespace component
