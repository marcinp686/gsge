#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject
{
    alignas(16) glm::mat4 model{1.0f};
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 normal;
    alignas(16) glm::vec3 lightPos{glm::vec3(-1.2, -1.2, -1.2)};
};
