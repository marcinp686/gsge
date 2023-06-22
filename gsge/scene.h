#pragma once

#include <vector>

#include <DirectXMath.h>

#include <entt/entity/registry.hpp>

#pragma warning(push)
#pragma warning(disable : 26451)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure
#pragma warning(pop)

#pragma warning(suppress : 4275 6285 26498 26451 26800)
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
    std::vector<DirectX::XMMATRIX> &getTransformMatricesLump();

    UniformBufferObject ubo;

    camera mainCamera;

  private:
    entt::registry registry;
    entt::entity suzanne, suzanne_smooth, icoSphere, testCube, companionCube, squareFloor, simpleCube, plane, lightGizmo;

    static constexpr int c_arraySize = 40;
    std::array<entt::entity, c_arraySize * c_arraySize * c_arraySize> cubes;

    std::vector<uint32_t> objects;

    std::vector<uint32_t> vertexBufferOffsets;
    std::vector<uint32_t> indexBufferOffsets;
    std::vector<glm::vec3> hostVertexBuffer;
    std::vector<glm::vec3> hostVertexNormalBuffer;
    std::vector<glm::u16> hostIndexBuffer;
    std::vector<DirectX::XMMATRIX> hostTransformMatrixBuffer; // TODO: change model to normal matrix in future
};
