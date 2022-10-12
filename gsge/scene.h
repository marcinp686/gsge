#pragma once

#include <vector>

#include <entt/entity/registry.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "component/component.h"
#include "component/camera.h"
#include "types.h"

class scene
{
  public:
    scene(){};

    void initScene();
    void loadModel(entt::entity entity, std::string fileName, uint32_t meshId = 0);
    void update(float deltaTime);
    void updateTransformMatrices(float dt);
    void prepareFrameData();
    void updateUniformBuffer();

    std::vector<glm::vec3> &getVertexLump();
    std::vector<glm::vec3> &getNormalLump();
    std::vector<glm::u16> &getIndexLump();
    std::vector<uint32_t> &getVertexOffsets();
    std::vector<uint32_t> &getIndexOffsets();
    std::vector<glm::mat4> &getTransformMatricesLump();

    UniformBufferObject ubo;

    camera mainCamera;

  private:
    entt::registry registry;
    entt::entity suzanne, suzanne_smooth, icoSphere, testCube, companionCube, squareFloor, simpleCube, plane;

    int c_arraySize = 5;
    std::array<entt::entity, 5 * 5 * 5> cubes;

    std::vector<uint32_t> vertexOffsets;
    std::vector<uint32_t> indexOffsets;
    std::vector<glm::vec3> vertexLump;
    std::vector<glm::vec3> normalLump;
    std::vector<glm::u16> indexLump;
    std::vector<glm::mat4> transformMatrixLump; // TODO: change model to normal matrix in future
};
