#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace component
{
struct mesh
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::u16> indices;
    std::vector<glm::vec3> normals;

    uint32_t nVertices = 0;
    uint32_t nIndices = 0;
    uint32_t nFaces = 0;

    uint32_t firstIndex = 0;   // for indexed drawing
    uint32_t vertexOffset = 0; // for indexed drawing
};
} // namespace component
