#include "scene.h"

void scene::initScene()
{
    using namespace DirectX;   

    // Camera object
    mainCamera.setPosition(glm::vec3(25.f, -5.0f, -60.0f));

    // Objects in the scene
    suzanne = registry.create();
    suzanne_smooth = registry.create();
    icoSphere = registry.create();
    testCube = registry.create();
    companionCube = registry.create();
    squareFloor = registry.create();
    simpleCube = registry.create();
    lightGizmo = registry.create();

    size_t i = 0;
    for (size_t y = 0; y < c_arraySize; y++)
        for (size_t x = 0; x < c_arraySize; x++)
            for (size_t z = 0; z < c_arraySize; z++)
            {
                cubes[i] = registry.create();
                registry.emplace<component::motion>(
                    cubes[i], XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                    XMFLOAT4A(XMConvertToRadians(-30.0f - i / 8000.f), XMConvertToRadians(-15.0f - i / 8000.f), 0.f, 0.f));
                auto &transform = registry.emplace<component::transform>(cubes[i++]);
                XMStoreFloat4A(&transform.position, {x * 2.f, y * 2.f, z * 2.f, 0.0f});
                XMStoreFloat4A(&transform.rotation, XMVectorZero());
                XMStoreFloat4A(&transform.scale, {0.8f, 0.8f, 0.8f, 0.0f});
            }

    registry.emplace<component::name>(suzanne, "suzanne");
    registry.emplace<component::name>(suzanne_smooth, "suzanne_smooth");
    registry.emplace<component::name>(icoSphere, "icoSphere");
    registry.emplace<component::name>(testCube, "testCube");
    registry.emplace<component::name>(companionCube, "companionCube");
    registry.emplace<component::name>(squareFloor, "squareFloor");
    registry.emplace<component::name>(simpleCube, "simpleCube");
    registry.emplace<component::name>(lightGizmo, "lightGizmo");

    registry.emplace<component::mesh>(suzanne);
    registry.emplace<component::mesh>(suzanne_smooth);
    registry.emplace<component::mesh>(icoSphere);
    registry.emplace<component::mesh>(testCube);
    registry.emplace<component::mesh>(companionCube);
    registry.emplace<component::mesh>(squareFloor);
    registry.emplace<component::mesh>(simpleCube);
    registry.emplace<component::mesh>(lightGizmo);

    registry.emplace<component::motion>(suzanne, XMFLOAT4A(0.6f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(60.0f), 0.0f, 0.0f));
    registry.emplace<component::motion>(suzanne_smooth, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(-60.0f), 0.0f, 0.0f));
    registry.emplace<component::motion>(icoSphere, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(30.0f), 0.0f, 0.0f));
    registry.emplace<component::motion>(testCube, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(10.0f), XMConvertToRadians(-15.0f), 0.0f));
    registry.emplace<component::motion>(companionCube, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(-20.0f), 0.0f, 0.0f));
    registry.emplace<component::motion>(squareFloor, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
                                        XMFLOAT4A(0.0f, XMConvertToRadians(-20.0f), XMConvertToRadians(10.0f), 0.0f));
    registry.emplace<component::motion>(
        simpleCube, XMFLOAT4A(0.f, 0.f, 0.f, 0.f),
        XMFLOAT4A(XMConvertToRadians(30.f), XMConvertToRadians(20.f), XMConvertToRadians(10.f), 0.0f));
    registry.emplace<component::motion>(lightGizmo, XMFLOAT4A(0.f, 0.f, 0.f, 0.f), XMFLOAT4A(0.f, 0.f, 0.f, 0.f));

    registry.emplace<component::transform>(suzanne, XMFLOAT4A(0, -5, 4, 0));
    registry.emplace<component::transform>(suzanne_smooth, XMFLOAT4A(5, -5, 4, 0.f), XMFLOAT4A(0, 0, 0, 0),
                                           XMFLOAT4A(2.f, 2.f, 2.f, 0.f));
    registry.emplace<component::transform>(icoSphere, XMFLOAT4A(10, -5, 4, 0.f));
    registry.emplace<component::transform>(testCube, XMFLOAT4A(15, -5, 4, 0.f));
    registry.emplace<component::transform>(companionCube, XMFLOAT4A(20, -5, 4, 0.f), XMFLOAT4A(0, 0, 0, 0),
                                           XMFLOAT4A(3.f, 3.f, 3.f, 0.f));
    registry.emplace<component::transform>(squareFloor, XMFLOAT4A(27.5, -5, 4, 0.f));
    registry.emplace<component::transform>(simpleCube, XMFLOAT4A(35, -5, 4, 0.f));
    registry.emplace<component::transform>(lightGizmo, XMFLOAT4A(ubo.lightPos.x, ubo.lightPos.y, ubo.lightPos.z, 0.f),
                                           XMFLOAT4A(0, 0, 0, 0), XMFLOAT4A(0.5f, 0.5f, 0.5f, 0.f));

    loadModel(suzanne, "models/suzanne.fbx");
    loadModel(suzanne_smooth, "models/suzanne_smooth.fbx");
    loadModel(icoSphere, "models/icoSphere.fbx");
    loadModel(testCube, "models/testCube.fbx");
    loadModel(companionCube, "models/CompanionCube.fbx");
    loadModel(squareFloor, "models/squareFloor.fbx");
    loadModel(simpleCube, "models/simpleCube.fbx");
    loadModel(lightGizmo, "models/icoSphere.fbx");

    for (int i = 0; i < cubes.size(); i++)
    {
        registry.emplace<component::mesh>(cubes[i]) = registry.get<component::mesh>(simpleCube);
        registry.emplace<component::name>(cubes[i], "cubematrix");
    }

    registry.sort<component::mesh>([](const entt::entity lhs, const entt::entity rhs) { return lhs < rhs; });
    registry.sort<component::transform, component::mesh>();
    registry.sort<component::transform, component::motion>();
}

void scene::loadModel(entt::entity entity, std::string fileName, uint32_t meshId)
{
    Assimp::Importer importer;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u16> indices;
    std::vector<glm::vec3> vertexNormals;

    // Needs to have aiProcess_FlipWindingOrder ENABLED!!!
    // assimp defaults to CW winding order (and normal calculation) while Vulkan has Y axis inverted causing
    // normals to be inverted. Proper setting for vulkan renderer in such situation are:
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    const aiScene *aiScene =
        importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices);

    if (aiScene != nullptr)
    {
        SPDLOG_INFO("[Scene] Loaded " + fileName + " model. Meshes: " + std::to_string(aiScene->mNumMeshes) +
                    ", vertices: " + std::to_string(aiScene->mMeshes[meshId]->mNumVertices));
    }
    else
    {
        SPDLOG_ERROR("[Scene] Failed to load model {}. Error: {}", fileName, importer.GetErrorString());
        throw std::runtime_error("IO error");
    }

    for (size_t i = 0; i < aiScene->mMeshes[meshId]->mNumFaces; i++)
    {
        indices.push_back(aiScene->mMeshes[meshId]->mFaces[i].mIndices[0]);
        indices.push_back(aiScene->mMeshes[meshId]->mFaces[i].mIndices[1]);
        indices.push_back(aiScene->mMeshes[meshId]->mFaces[i].mIndices[2]);
    }

    for (size_t i = 0; i < aiScene->mMeshes[meshId]->mNumVertices; ++i)
    {
        aiVector3D n = aiScene->mMeshes[meshId]->mNormals[i];
        vertexNormals.push_back(glm::vec3(n.x, n.y, n.z));
    }

    glm::vec3 *pVertices = reinterpret_cast<glm::vec3 *>(aiScene->mMeshes[meshId]->mVertices);
    uint32_t nVertices = aiScene->mMeshes[meshId]->mNumVertices;
    uint32_t nFaces = aiScene->mMeshes[meshId]->mNumFaces;

    glm::u16 *pIndices = indices.data();
    glm::vec3 *pNormals = vertexNormals.data();

    auto &meshComp = registry.get<component::mesh>(entity);
    meshComp.vertices.assign(pVertices, pVertices + nVertices);
    meshComp.indices.assign(pIndices, pIndices + static_cast<size_t>(nFaces) * 3);
    meshComp.normals.assign(pNormals, pNormals + nVertices);
    meshComp.nVertices = nVertices;
    meshComp.nIndices = nFaces * 3;
    meshComp.nFaces = nFaces;

    auto view = registry.view<component::transform, component::motion>();
}

void scene::update(float deltaTime)
{
    updateTransformMatrices(deltaTime);
    updateUniformBuffer();
}

void scene::updateTransformMatrices(float dt)
{
    using namespace DirectX;
    ZoneScoped;

    auto view = registry.view<component::transform, component::motion>();

    // load dt to all components of vector
    XMVECTOR dtVec = XMVectorReplicate(dt);

   for ( auto& entity : view)
    {
        auto &transform = view.get<component::transform>(entity);
        auto &motion = view.get<component::motion>(entity);

        XMVECTOR motVelocity = XMLoadFloat4(&motion.velocity);
        XMVECTOR translation = XMLoadFloat4(&transform.position);
        XMVECTOR translationResult = XMVectorMultiplyAdd(motVelocity, dtVec, translation);
        XMStoreFloat4A(&transform.position, translationResult);

        XMVECTOR motRotation = XMLoadFloat4(&motion.rotation);
        XMVECTOR rotation = XMLoadFloat4(&transform.rotation);
        XMVECTOR rotationResult = XMVectorMultiplyAdd(motRotation, dtVec, rotation);
        XMStoreFloat4A(&transform.rotation, rotationResult);

        XMVECTOR scale = XMLoadFloat4(&transform.scale);

        // Not necessary to use translation matrix, just set last row to translation vector
        // XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translationResult);
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationResult);        
        
        XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scale);
        
        // XMMATRIX result = XMMatrixMultiply(XMMatrixMultiply(rotationMatrix, scaleMatrix), translationMatrix);
        // It is faster to just set last row to translation vector
        XMMATRIX result = XMMatrixMultiply(rotationMatrix, scaleMatrix);
        result.r[3] = translationResult;
        result.r[3].m128_f32[3] = 1.0f;

        XMStoreFloat4x4A(reinterpret_cast<XMFLOAT4X4A *>(&hostTransformMatrixBuffer[static_cast<uint32_t>(entity)]), result);
    }
}

void scene::prepareFrameData()
{
    ZoneScoped;
    auto view = registry.view<component::mesh, component::name>();

    uint32_t totVertices = 0;
    uint32_t totIndices = 0;

    size_t totEntities = {0};

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);
        totVertices += mesh.nVertices;
        totIndices += mesh.nIndices;
        totEntities++;
    }

    hostTransformMatrixBuffer.resize(totEntities);
    hostVertexBuffer.reserve(totVertices);
    hostVertexNormalBuffer.reserve(totVertices);
    hostIndexBuffer.reserve(totIndices);

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);

        vertexBufferOffsets.emplace_back(static_cast<uint32_t>(hostVertexBuffer.size()));
        indexBufferOffsets.emplace_back(static_cast<uint32_t>(hostIndexBuffer.size()));

        hostVertexBuffer.insert(hostVertexBuffer.end(), mesh.vertices.begin(), mesh.vertices.end());
        hostVertexNormalBuffer.insert(hostVertexNormalBuffer.end(), mesh.normals.begin(), mesh.normals.end());
        hostIndexBuffer.insert(hostIndexBuffer.end(), mesh.indices.begin(), mesh.indices.end());
    }
    SPDLOG_TRACE("[Scene] Frame data prepared");
    SPDLOG_INFO("[Scene] Total in vectors: totV={}, totN={}, totI={}", hostVertexBuffer.size(), hostVertexNormalBuffer.size(),
                hostIndexBuffer.size());
}

void scene::updateUniformBuffer()
{
    ZoneScoped;
    
    // ubo.model = tr.transformMatrix;
    ubo.view = mainCamera.getViewMatrix();
    ubo.proj = mainCamera.getProjMatrix();
    ubo.normal = ubo.model;
    // ubo.normal = glm::inverse(glm::transpose(tr.transformMatrix));
    // ubo.lightPos += glm::vec3(0.f, -0.005f, 0.02f);
    ubo.viewPos = mainCamera.getPosition();
}

std::vector<glm::vec3> &scene::getVertexLump()
{
    return hostVertexBuffer;
}

std::vector<glm::vec3> &scene::getNormalLump()
{
    return hostVertexNormalBuffer;
}

std::vector<glm::u16> &scene::getIndexLump()
{
    return hostIndexBuffer;
}

std::vector<glm::uint32_t> &scene::getVertexOffsets()
{
    return vertexBufferOffsets;
}

std::vector<glm::uint32_t> &scene::getIndexOffsets()
{
    return indexBufferOffsets;
}

std::vector<DirectX::XMMATRIX> &scene::getTransformMatricesLump()
{
    return hostTransformMatrixBuffer;
}
