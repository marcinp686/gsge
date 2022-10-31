#include "scene.h"

void scene::initScene()
{
    // Camera object
    mainCamera.setPosition(glm::vec3(0.0f, 0.0f, -40.0f));

    // Objects in the scene
    suzanne = registry.create();
    suzanne_smooth = registry.create();
    icoSphere = registry.create();
    testCube = registry.create();
    companionCube = registry.create();
    squareFloor = registry.create();
    simpleCube = registry.create();
    // plane = registry.create();

    size_t i = 0;
    for (size_t y = 0; y < c_arraySize; y++)
        for (size_t x = 0; x < c_arraySize; x++)
            for (size_t z = 0; z < c_arraySize; z++)
            {
                cubes[i] = registry.create();
                registry.emplace<component::motion>(cubes[i], glm::vec3(0),
                                                    glm::radians(glm::vec3(30.0f + i / 8000.f, 15.0f + i / 8000.f, 0.0f)));
                registry.emplace<component::transform>(cubes[i++], glm::vec3(x * 2.f, y * 2.f, z * 2.f), glm::vec3(0.0f),
                                                       glm::vec3(0.5f));
            }

    registry.emplace<component::name>(suzanne, "suzanne");
    registry.emplace<component::name>(suzanne_smooth, "suzanne_smooth");
    registry.emplace<component::name>(icoSphere, "icoSphere");
    registry.emplace<component::name>(testCube, "testCube");
    registry.emplace<component::name>(companionCube, "companionCube");
    registry.emplace<component::name>(squareFloor, "squareFloor");
    registry.emplace<component::name>(simpleCube, "simpleCube");
    // scene.emplace<component::name>(plane, "plane");

    registry.emplace<component::mesh>(suzanne);
    registry.emplace<component::mesh>(suzanne_smooth);
    registry.emplace<component::mesh>(icoSphere);
    registry.emplace<component::mesh>(testCube);
    registry.emplace<component::mesh>(companionCube);
    registry.emplace<component::mesh>(squareFloor);
    registry.emplace<component::mesh>(simpleCube);
    // scene.emplace<component::mesh>(plane);

    registry.emplace<component::motion>(suzanne, glm::vec3(0), glm::radians(glm::vec3(0.0f, -60.0f, 0.0f)));
    registry.emplace<component::motion>(suzanne_smooth, glm::vec3(0), glm::radians(glm::vec3(0.0f, 45.0f, 0.0f)));
    registry.emplace<component::motion>(icoSphere, glm::vec3(0), glm::radians(glm::vec3(0.0f, 30.0f, 0.0f)));
    registry.emplace<component::motion>(testCube, glm::vec3(0), glm::radians(glm::vec3(0.0f, 0.0f, -15.0f)));
    registry.emplace<component::motion>(companionCube, glm::vec3(0), glm::radians(glm::vec3(0.0f, -20.0f, 0.0f)));
    registry.emplace<component::motion>(squareFloor, glm::vec3(0), glm::radians(glm::vec3(0.0f, -20.0f, 0.0f)));
    registry.emplace<component::motion>(simpleCube, glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(glm::vec3(30.f, 20.f, 10.f)));
    // scene.emplace<component::motion>(plane);

    registry.emplace<component::transform>(suzanne, glm::vec3(-2.5, -0.5, 0));
    registry.emplace<component::transform>(suzanne_smooth, glm::vec3(2.5, 1, 2));
    registry.emplace<component::transform>(icoSphere, glm::vec3(2.5, -1.2, 0));
    registry.emplace<component::transform>(testCube, glm::vec3(-2.5, 1.5, 2.5));
    registry.emplace<component::transform>(companionCube, glm::vec3(1.5, -1, 1.5));
    registry.emplace<component::transform>(squareFloor, glm::vec3(-2, -3, 8));
    registry.emplace<component::transform>(simpleCube, glm::vec3(-1.5, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0.5));
    // scene.emplace<component::transform>(plane, glm::vec3(0,, 0));

    loadModel(suzanne, "models/suzanne.fbx");
    loadModel(suzanne_smooth, "models/suzanne_smooth.fbx");
    loadModel(icoSphere, "models/icoSphere.fbx");
    loadModel(testCube, "models/testCube.fbx");
    loadModel(companionCube, "models/CompanionCube.fbx");
    loadModel(squareFloor, "models/squareFloor.fbx");
    loadModel(simpleCube, "models/simpleCube.fbx");
    // loadModel(plane, "models/plane_1x1.fbx");

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
        spdlog::info("Loaded " + fileName + " model. Meshes: " + std::to_string(aiScene->mNumMeshes) +
                     ", vertices: " + std::to_string(aiScene->mMeshes[meshId]->mNumVertices));
    }
    else
    {
        spdlog::error("Failed to load " + fileName);
        spdlog::error(std::string(importer.GetErrorString()));
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
    meshComp.indices.assign(pIndices, pIndices + nFaces * 3);
    meshComp.normals.assign(pNormals, pNormals + nVertices);
    meshComp.nVertices = nVertices;
    meshComp.nIndices = nFaces * 3;
    meshComp.nFaces = nFaces;
}

void scene::update(float deltaTime)
{
    updateTransformMatrices(deltaTime);
    updateUniformBuffer();
}

void scene::updateTransformMatrices(float dt)
{
    EASY_FUNCTION();
    auto view = registry.view<component::transform, component::motion>();

    hostTransformMatrixBuffer.resize(view.size_hint());

    for (auto entity : view)
    {
        auto &transform = view.get<component::transform>(entity);
        auto &motion = view.get<component::motion>(entity);

        if (motion.velocity != glm::vec3(0))
            transform.position += motion.velocity * dt;
        transform.rotation += motion.rotation * dt;

        // Translation first - Matrix multiplication is not commutative, which means their order is important
        // transform.transformMatrix = glm::translate(glm::mat4(1.0f), transform.position);

        float sin_g = sin(transform.rotation.x);
        float cos_g = cos(transform.rotation.x);
        float sin_b = sin(transform.rotation.y);
        float cos_b = cos(transform.rotation.y);
        float sin_a = sin(transform.rotation.z);
        float cos_a = cos(transform.rotation.z);

        /*float r11 = cos_y * cos_z;
        float r12 = sin_z * sin_y * cos_x - cos_x * sin_z;
        float r13 = cos_z * sin_y * cos_x + sin_x * sin_z;

        float r21 = cos_y * sin_z;
        float r22 = sin_z * sin_y * sin_x + cos_x * cos_z;
        float r23 = cos_x * sin_y * sin_z - sin_z * cos_x;

        float r31 = -sin_y;
        float r32 = sin_x * cos_y;
        float r33 = cos_x * cos_y;*/

        float r11 = cos_a * cos_b;
        float r12 = cos_a * sin_b * sin_g - sin_a * cos_g;
        float r13 = cos_a * sin_b * cos_g + sin_a * sin_g;

        float r21 = sin_a * cos_b;
        float r22 = sin_a * sin_b * sin_g + cos_a * cos_g;
        float r23 = sin_a * sin_b * cos_g - cos_a * sin_g;

        float r31 = -sin_b;
        float r32 = cos_b * sin_g;
        float r33 = cos_b * cos_g;

        glm::mat4 transformMatrix{{r11 * transform.scale.x, r12 * transform.scale.y, r13 * transform.scale.z, 0.f},
                                  {r21 * transform.scale.x, r22 * transform.scale.y, r23 * transform.scale.z, 0.f},
                                  {r31 * transform.scale.x, r32 * transform.scale.y, r33 * transform.scale.z, 0.f},
                                  {transform.position.x, transform.position.y, transform.position.z, 1.f}};

        // glm::mat4 rot{{r11, r21, r31, 0.f},
        //               {r12, r22, r32, 0.f},
        //               {r13, r23, r33, 0.f},
        //               {transform.position.x, transform.position.y, transform.position.z, 1.f}};

        //// Rotation second
        // transform.transformMatrix = glm::rotate(transform.transformMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        // transform.transformMatrix = glm::rotate(transform.transformMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        // transform.transformMatrix = glm::rotate(transform.transformMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        //// Scale third
        // transform.transformMatrix = glm::scale(transform.transformMatrix, transform.scale);
        //// hostTransformMatrixBuffer[static_cast<uint32_t>(entity)] = transform.transformMatrix;
        hostTransformMatrixBuffer[static_cast<uint32_t>(entity)] = transformMatrix;
    }
}

void scene::prepareFrameData()
{
    auto view = registry.view<component::mesh, component::name>();

    uint32_t totVertices = 0;
    uint32_t totIndices = 0;

    spdlog::info("Preparing vertex, normal and index buffers");

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);
        totVertices += mesh.nVertices;
        totIndices += mesh.nIndices;
    }

    hostVertexBuffer.reserve(totVertices);
    hostVertexNormalBuffer.reserve(totVertices);
    hostIndexBuffer.reserve(totIndices);

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);
        auto &name = view.get<component::name>(entity);

        vertexBufferOffsets.emplace_back(static_cast<uint32_t>(hostVertexBuffer.size()));
        indexBufferOffsets.emplace_back(static_cast<uint32_t>(hostIndexBuffer.size()));

        hostVertexBuffer.insert(hostVertexBuffer.end(), mesh.vertices.begin(), mesh.vertices.end());
        hostVertexNormalBuffer.insert(hostVertexNormalBuffer.end(), mesh.normals.begin(), mesh.normals.end());
        hostIndexBuffer.insert(hostIndexBuffer.end(), mesh.indices.begin(), mesh.indices.end());
    }
    spdlog::info("Total in vectors: totV={}, totN={}, totI={}", hostVertexBuffer.size(), hostVertexNormalBuffer.size(),
                 hostIndexBuffer.size());
}

void scene::updateUniformBuffer()
{
    EASY_FUNCTION();
    auto &tr = registry.get<component::transform>(simpleCube);

    ubo.model = tr.transformMatrix;
    ubo.view = mainCamera.getViewMatrix();
    ubo.proj = mainCamera.getProjMatrix();
    ubo.normal = ubo.model;
    // ubo.normal = glm::inverse(glm::transpose(tr.transformMatrix));
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

std::vector<glm::mat4> &scene::getTransformMatricesLump()
{
    return hostTransformMatrixBuffer;
}
