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

#include <easy/profiler.h>

#include "component/component.h"
#include "component/camera.h"
#include "component/material.h"
#include "types.h"
#include "timer.h"

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
    entt::entity suzanne, suzanne_smooth, icoSphere, testCube, companionCube, squareFloor, simpleCube, plane, lightGizmo;

    int c_arraySize = 20;
    std::array<entt::entity, 20 * 20 * 20> cubes;

    std::vector<uint32_t> objects;

    std::vector<uint32_t> vertexBufferOffsets;
    std::vector<uint32_t> indexBufferOffsets;
    std::vector<glm::vec3> hostVertexBuffer;
    std::vector<glm::vec3> hostVertexNormalBuffer;
    std::vector<glm::u16> hostIndexBuffer;
    std::vector<glm::mat4> hostTransformMatrixBuffer; // TODO: change model to normal matrix in future
};
