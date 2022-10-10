#include "scene.h"

void scene::initScene()
{
    // Camera object
    mainCamera.setPosition(glm::vec3(2.0f, -2.0f, -30.0f));
    mainCamera.setCenter(glm::vec3(10.0f, 0.0f, 0.0f));

    // Objects in the scene
    suzanne = registry.create();
    suzanne_smooth = registry.create();
    icoSphere = registry.create();
    testCube = registry.create();
    companionCube = registry.create();
    squareFloor = registry.create();
    simpleCube = registry.create();
    plane = registry.create();

    for (int i = 0; i < cubes.size(); i++)
    {
        cubes[i] = registry.create();      
        registry.emplace<component::name>(cubes[i], "cubematrix");        
    }

    size_t i = 0;
    for (size_t y = 0; y < 10; y++)
        for (size_t x = 0; x < 10; x++)
            for (size_t z = 0; z < 10; z++)
            {
                registry.emplace<component::motion>(cubes[i], glm::vec3(0), glm::vec3(30.0f + i / 100, 15.0f + i / 100, 0.0f));
                registry.emplace<component::transform>(cubes[i++], glm::vec3(x, y, z), glm::vec3(0.0f), glm::vec3(0.2f));
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

    registry.emplace<component::motion>(suzanne, glm::vec3(0), glm::vec3(0.0f, -60.0f, 0.0f));
    registry.emplace<component::motion>(suzanne_smooth, glm::vec3(0), glm::vec3(0.0f, 45.0f, 0.0f));
    registry.emplace<component::motion>(icoSphere, glm::vec3(0), glm::vec3(0.0f, 30.0f, 0.0f));
    registry.emplace<component::motion>(testCube, glm::vec3(0), glm::vec3(0.0f, 0.0f, -15.0f));
    registry.emplace<component::motion>(companionCube, glm::vec3(0), glm::vec3(0.0f, -20.0f, 0.0f));
    registry.emplace<component::motion>(squareFloor, glm::vec3(0), glm::vec3(0.0f, -20.0f, 0.0f));
    registry.emplace<component::motion>(simpleCube, glm::vec3(0.1f, 0.0f, 0.0f), glm::vec3(30.f, 20.f, 10.f));
    // scene.emplace<component::motion>(plane);

    registry.emplace<component::transform>(suzanne, glm::vec3(-2.5, -0.5, 0));
    registry.emplace<component::transform>(suzanne_smooth, glm::vec3(2.5, 1, 2));
    registry.emplace<component::transform>(icoSphere, glm::vec3(2.5, -1.2, 0));
    registry.emplace<component::transform>(testCube, glm::vec3(-2.5, 1.5, 2.5));
    registry.emplace<component::transform>(companionCube, glm::vec3(1.5, -1, 1.5));
    registry.emplace<component::transform>(squareFloor, glm::vec3(0, 3, 0));
    registry.emplace<component::transform>(simpleCube, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0.5));
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
        cubes[i] = registry.create();
        registry.emplace<component::mesh>(cubes[i]) = registry.get<component::mesh>(simpleCube);
        registry.emplace<component::name>(cubes[i], "cubematrix");
        // registry.emplace<component::motion>(cubes[i], glm::vec3(0), glm::vec3(0.0f, 15.0f, 0.0f));
    }

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
    auto view = registry.view<component::transform, component::motion>();

    transformMatrixLump.clear();

    for (auto entity : view)
    {
        auto &transform = view.get<component::transform>(entity);
        auto &motion = view.get<component::motion>(entity);

        transform.position += motion.velocity * dt;
        transform.rotation += motion.rotation * dt;

        // Translation first - Matrix multiplication is not commutative, which means their order is important
        transform.transformMatrix = glm::translate(glm::mat4(1.0f), transform.position);

        // Rotation second
        transform.transformMatrix =
            glm::rotate(transform.transformMatrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform.transformMatrix =
            glm::rotate(transform.transformMatrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform.transformMatrix =
            glm::rotate(transform.transformMatrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 0.1f));

        // Scale third
        transform.transformMatrix = glm::scale(transform.transformMatrix, transform.scale);
        transformMatrixLump.push_back(transform.transformMatrix);
    }
}

void scene::prepareFrameData()
{
    auto view = registry.view<component::mesh, component::name>();

    uint32_t totVertices = 0;
    uint32_t totIndices = 0;

    spdlog::info("==uploadBuffersToGPU==");

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);
        totVertices += mesh.nVertices;
        totIndices += mesh.nIndices;
    }

    vertexLump.reserve(totVertices);
    normalLump.reserve(totVertices);
    indexLump.reserve(totIndices);

    for (auto entity : view)
    {
        auto &mesh = view.get<component::mesh>(entity);
        auto &name = view.get<component::name>(entity);

        vertexOffsets.emplace_back(static_cast<uint32_t>(vertexLump.size()));
        indexOffsets.emplace_back(static_cast<uint32_t>(indexLump.size()));

        vertexLump.insert(vertexLump.end(), mesh.vertices.begin(), mesh.vertices.end());
        normalLump.insert(normalLump.end(), mesh.normals.begin(), mesh.normals.end());
        indexLump.insert(indexLump.end(), mesh.indices.begin(), mesh.indices.end());

        spdlog::info("Name: {}, faces: {}, vertices: {}", name.value, mesh.nFaces, mesh.nVertices);
    }
    spdlog::info("Total calculated: V={}, N={}, I={}", totVertices, totVertices, totIndices);
    spdlog::info("Total in vectors: totV={}, totN={}, totI={}", vertexLump.size(), normalLump.size(), indexLump.size());
}

void scene::updateUniformBuffer()
{
    auto &tr = registry.get<component::transform>(simpleCube);

    ubo.model = tr.transformMatrix;
    // glm::transpose(tr.transformMatrix);
    ubo.view = mainCamera.getViewMatrix();
    ubo.proj = mainCamera.getProjMatrix();
    ubo.normal = ubo.model;
    // ubo.normal = glm::inverse(glm::transpose(tr.transformMatrix));
}

std::vector<glm::vec3> &scene::getVertexLump()
{
    return vertexLump;
}

std::vector<glm::vec3> &scene::getNormalLump()
{
    return normalLump;
}

std::vector<glm::u16> &scene::getIndexLump()
{
    return indexLump;
}

std::vector<glm::uint32_t> &scene::getVertexOffsets()
{
    return vertexOffsets;
}

std::vector<glm::uint32_t> &scene::getIndexOffsets()
{
    return indexOffsets;
}

std::vector<glm::mat4> &scene::getTransformMatricesLump()
{
    return transformMatrixLump;
}
