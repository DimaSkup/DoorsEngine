////////////////////////////////////////////////////////////////////
// Filename:      ModelImporter.cpp
// 
// Created:       07.07.23
////////////////////////////////////////////////////////////////////
#include "ModelImporter.h"
#include "ModelMath.h"
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>

#include "../Mesh/MaterialMgr.h"
#include "../Model/ModelLoaderM3D.h"
#include "../Texture/TextureMgr.h"
#include "../Model/ModelImporterHelpers.h"

#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using namespace DirectX;

namespace Core
{

// duration time of importing process
double ModelImporter::s_ImportDuration_     = 0.0;
double ModelImporter::s_TexLoadingDuration_ = 0.0;
double ModelImporter::s_VerticesLoading_    = 0.0;
double ModelImporter::s_NodesLoading_       = 0.0;
double ModelImporter::s_SceneLoading_       = 0.0;


// =================================================================================
//                              PUBLIC METHODS
// =================================================================================
void ModelImporter::LoadFromFile(
    ID3D11Device* pDevice,
    BasicModel& model,
    const std::string& filePath)
{
    // this function initializes a new model from the file 
    // of type .blend, .fbx, .3ds, .obj, .m3d, etc.

    auto start = std::chrono::steady_clock::now();

    fs::path path = filePath;

    Assert::True(fs::exists(path), "there is no file by path: " + filePath);

    try
    {
        fs::path ext = path.extension();

        // define if we want to load a .m3d file
        if (ext == L".m3d")
        {
            assert(0 && "FIXME: can't import .m3d format");
#if 0
            ModelLoaderM3D loader;
            loader.LoadM3d(filePath, model);
            return;
#endif
        }

        // --------------------------------------

        // else we want to import a model of some another type
        Assimp::Importer importer;

        auto sceneStart = std::chrono::steady_clock::now();

        const aiScene* pScene = importer.ReadFile(
            filePath,
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            //aiProcess_ImproveCacheLocality |
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded);

        // assert that we successfully read the data file 
        Assert::NotNullptr(pScene, "can't read a model's data file: " + filePath);

        auto sceneEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> sceneElapsed = sceneEnd - sceneStart;
        ModelImporter::s_SceneLoading_ += sceneElapsed.count();

        // compute how many vertices/indices/materials/subsets this model has
        ComputeNumOfData(model, pScene->mRootNode, pScene);

        // allocate memory for vertices/indices/etc.
        model.AllocateMemory(model.numVertices_, model.numIndices_, model.numSubsets_);

        int subsetIdx = 0;

        auto nodeStart = std::chrono::steady_clock::now();

        // load all the meshes/materials/textures/etc.
        ProcessNode(
            pDevice,
            model,
            subsetIdx,
            pScene->mRootNode,
            pScene,
            DirectX::XMMatrixIdentity(),
            filePath);

        importer.FreeScene();

        // compute the duration of the nodes processing
        auto nodeEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> nodeElapsed = nodeEnd - nodeStart;
        ModelImporter::s_NodesLoading_ += nodeElapsed.count();

        // compute the duration of the whole process of importing
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        ModelImporter::s_ImportDuration_ += elapsed.count();


        Log::Debug("Asset is loaded from file: " + filePath);
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        throw EngineException("can't initialize a model from the file: " + filePath);
    }
}


// =================================================================================
//                              PRIVATE METHODS
// =================================================================================
void ModelImporter::ComputeNumOfData(
    BasicModel& model,
    const aiNode* pNode,
    const aiScene* pScene)
{
    // go through all the meshes in the current model's node
    for (UINT i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

        // accumulate the amount of vertices/indices/subsets in this model
        model.numVertices_ += pMesh->mNumVertices;
        model.numIndices_ += pMesh->mNumFaces * 3;
        model.numSubsets_++;
    }

    // go through all the child nodes of the current node and handle it
    for (UINT i = 0; i < pNode->mNumChildren; i++)
    {
        ComputeNumOfData(model, pNode->mChildren[i], pScene);
    }
}

///////////////////////////////////////////////////////////

void ModelImporter::ProcessNode(
    ID3D11Device* pDevice,
    BasicModel& model,
    int& subsetIdx,
    const aiNode* pNode,
    const aiScene* pScene,
    const DirectX::XMMATRIX& parentTransformMatrix,  // a matrix which is used to transform position of this mesh to the proper location
    const std::string& filePath)                     // full path to the model
{
    //
    // this function goes through each node of the scene's tree structure
    // starting from the root node and initializes a mesh using data of this each node;
    //
    // created mesh is pushed into the input meshes array
    //

    const XMMATRIX nodeTransformMatrix = XMMATRIX(&pNode->mTransformation.a1) * parentTransformMatrix;

    // go through all the meshes in the current model's node
    for (UINT i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

        // handle this mesh and push it into the model's meshes array
        ProcessMesh(
            pDevice,
            model,
            subsetIdx,
            pMesh,
            pScene,
            nodeTransformMatrix,
            filePath);

        ++subsetIdx;
    }

    // go through all the child nodes of the current node and handle it
    for (UINT i = 0; i < pNode->mNumChildren; i++)
    {
        ProcessNode(
            pDevice,
            model,
            subsetIdx,
            pNode->mChildren[i],
            pScene,
            nodeTransformMatrix,
            filePath);
    }
}

///////////////////////////////////////////////////////////

void ModelImporter::ProcessMesh(
    ID3D11Device* pDevice,
    BasicModel& model,
    int& subsetIdx,
    const aiMesh* pMesh,                                    // the current mesh of the model
    const aiScene* pScene,                            // a ptr to the scene of this model type
    const DirectX::XMMATRIX& transformMatrix,        // a matrix which is used to transform position of this mesh to the proper location
    const std::string& filePath)                      // full path to the model
{
    
    MeshGeometry::Subset& subset = model.meshes_.subsets_[subsetIdx];

    try
    {
        Material material;

        model.meshes_.SetSubsetName(subsetIdx, pMesh->mName.C_Str());

        // load vertices/indices/subsets data
        GetVerticesIndicesOfMesh(pMesh, model, subset, subsetIdx);

        // get material data of this mesh
        aiMaterial* pAiMat = pScene->mMaterials[pMesh->mMaterialIndex];
        LoadMaterialColorsData(pAiMat, material);

        // load available textures for this mesh and store IDs of these texture into the material
        LoadMaterialTextures(pDevice, material.textureIDs, pAiMat, subset, pScene, filePath);

        // store a material into the material manager and also store its material ID into the subset (mesh)
        subset.materialID = g_MaterialMgr.AddMaterial(std::move(material));
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error(std::format("can't import a mesh \"{}\" from file: {}", subset.name, filePath));
    }
}

///////////////////////////////////////////////////////////

void ModelImporter::LoadMaterialColorsData(aiMaterial* pMaterial, Material& mat)
{
    // read material properties for this mesh

    // global default materials for this .cpp
    constexpr Float4 defaultAmbient (0.3f, 0.3f, 0.3f, 1);
    constexpr Float4 defaultDiffuse (0.8f, 0.8f, 0.8f, 1);
    constexpr Float4 defaultSpecular(0.0f, 0.0f, 0.0f, 1);           // w == spec power
    constexpr Float4 defaultReflect (0.5f, 0.5f, 0.5f, 1);

    aiColor4D ambient;
    aiColor4D diffuse;
    aiColor4D specular;
    float shininess;

    // load materials from the aiMaterial
    const aiReturn isAmbientLoaded   = pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    const aiReturn isDiffuseLoaded   = pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    const aiReturn isSpecularLoaded  = pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    const aiReturn isShininessLoaded = pMaterial->Get(AI_MATKEY_SHININESS, shininess);

    mat.ambient    = (isAmbientLoaded == aiReturn_SUCCESS)   ? Float4(ambient.r, ambient.g, ambient.b, ambient.a) : defaultAmbient;
    mat.diffuse    = (isDiffuseLoaded == aiReturn_SUCCESS)   ? Float4(diffuse.r, diffuse.g, diffuse.b, diffuse.a) : defaultDiffuse;
    mat.specular   = (isSpecularLoaded == aiReturn_SUCCESS)  ? Float4(specular.r, specular.g, specular.b, 1.0f) : defaultSpecular;
    mat.specular.w = (isShininessLoaded == aiReturn_SUCCESS) ? shininess : defaultSpecular.w;
    mat.reflect    = defaultReflect;
}

///////////////////////////////////////////////////////////

void ModelImporter::LoadMaterialTextures(
    ID3D11Device* pDevice,
    TexID* texIDs,
    const aiMaterial* pMaterial,
    const MeshGeometry::Subset& subset,
    const aiScene* pScene,
    const std::string& filePath)
{
    //
    // load all the available textures for this mesh by its material data
    //

    std::vector<aiTextureType> texTypesToLoad;
    std::vector<UINT> texCounts;

    // define what texture types to load
    for (u32 i = 1; i < NUM_TEXTURE_TYPES; ++i)
    {
        const aiTextureType type = (aiTextureType)i;

        // if there are some textures by this type
        if (UINT texCount = pMaterial->GetTextureCount(type))
        {
            texTypesToLoad.push_back(type);
            texCounts.push_back(texCount);
        }
    }

    // get path to the directory which contains a model's data file
    const fs::path fullPath = filePath;
    const fs::path modelDirPath = fullPath.parent_path();
    const std::string parentPath = modelDirPath.string() + "/";

    std::pair<aiTextureType, std::string> texTypeToPath[22];
    int numTexLoadFromFile = 0;

    // compute the duration of textures loading for this model
    auto start = std::chrono::steady_clock::now();


    // go through available texture type and load responsible texture
    for (size idx = 0; idx < std::ssize(texTypesToLoad); ++idx)
    {
        aiTextureType type = texTypesToLoad[idx];

        // go through each texture of this aiTextureType for this aiMaterial
        for (UINT i = 0; i < texCounts[idx]; i++)
        {
            // get path to the texture file
            aiString path;
            pMaterial->GetTexture(type, i, &path);

            // determine what the texture storage type is
            const TexStoreType storeType = DetermineTextureStorageType(
                pScene,
                pMaterial,
                i, type);

            type = (type == aiTextureType_HEIGHT) ? aiTextureType_NORMALS : type;

            switch (storeType)
            {
                // load a tex which is located on the disk
            case TexStoreType::Disk:
            {
                // make and store a pair [texture_type => texture_filepath]
                texTypeToPath[numTexLoadFromFile] = { type, std::string(parentPath + path.C_Str()) };
                numTexLoadFromFile++;

                break;
            }

            // load an embedded compressed texture
            case TexStoreType::EmbeddedCompressed:
            {
                const fs::path texPath = path.C_Str();
                const aiTexture* pAiTexture = pScene->GetEmbeddedTexture(path.C_Str());
                const std::string texName = texPath.stem().string();

                // add into the tex mgr a new texture
                const TexID id = g_TextureMgr.Add(texName, Texture(
                    pDevice,
                    texName,
                    (uint8_t*)(pAiTexture->pcData),         // data of texture
                    pAiTexture->mWidth));                  // size of texture);

                texIDs[type] = id;

                break;
            }

            // load an embedded indexed compressed texture
            case TexStoreType::EmbeddedIndexCompressed:
            {
                const fs::path texPath = path.C_Str();
                const UINT index = GetIndexOfEmbeddedCompressedTexture(&path);
                const std::string texName = texPath.stem().string();

                // add into the tex mgr a new texture
                const TexID id = g_TextureMgr.Add(texName, Texture(
                    pDevice,
                    texName,
                    (uint8_t*)(pScene->mTextures[index]->pcData),   // data of texture
                    pScene->mTextures[index]->mWidth));             // size of texture

                texIDs[type] = id;

                break;

            } // end switch/case
            }
        }
    }


    // load textures from files
    for (int i = 0; i < numTexLoadFromFile; ++i)
    {
        const aiTextureType type = texTypeToPath[i].first;
        const std::string& path = texTypeToPath[i].second;

        texIDs[type] = g_TextureMgr.LoadFromFile(path);
    }

    // compute the duration of textures loading for this model
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    ModelImporter::s_TexLoadingDuration_ += elapsed.count();
}

///////////////////////////////////////////////////////////

void ComputeMeshVertices(
    Vertex3D* vertices,
    const aiMesh* pMesh,
    const int vertexStart,
    const int numVertices)
{
    // load data for each vertex of the model

    for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
    {
        vertices[vIdx].position.x = pMesh->mVertices[i].x;
        vertices[vIdx].position.y = pMesh->mVertices[i].y;
        vertices[vIdx].position.z = pMesh->mVertices[i].z;
    }

    const aiVector3D* texCoords = pMesh->mTextureCoords[0];

    for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
    {
        vertices[vIdx].texture.x = texCoords[i].x;
        vertices[vIdx].texture.y = texCoords[i].y;
    }

    for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
    {
        vertices[vIdx].normal.x = pMesh->mNormals[i].x;
        vertices[vIdx].normal.y = pMesh->mNormals[i].y;
        vertices[vIdx].normal.z = pMesh->mNormals[i].z;
    }

    for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
    {
        vertices[vIdx].tangent.x = pMesh->mTangents[i].x;
        vertices[vIdx].tangent.y = pMesh->mTangents[i].y;
        vertices[vIdx].tangent.z = pMesh->mTangents[i].z;
    }

    for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
    {
        vertices[vIdx].binormal.x = pMesh->mBitangents[i].x;
        vertices[vIdx].binormal.y = pMesh->mBitangents[i].y;
        vertices[vIdx].binormal.z = pMesh->mBitangents[i].z;
    }
}

///////////////////////////////////////////////////////////

void ComputeMeshIndices(
    UINT* indices,
    aiFace* faces,
    const int indexStart,
    const int numFaces)
{
    // compute value for each index of all the faces in the mesh

    for (int faceIdx = 0, idx = indexStart; faceIdx < numFaces; ++faceIdx)
    {
        indices[idx++] = faces[faceIdx].mIndices[0];
        indices[idx++] = faces[faceIdx].mIndices[1];
        indices[idx++] = faces[faceIdx].mIndices[2];
    }
}

///////////////////////////////////////////////////////////

void ComputeMeshAABB(
    DirectX::BoundingBox& aabb,
    aiVector3D* verticesPos,
    const int numVertices)
{
    // compute the bounding box of the mesh
    XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
    XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

    for (int i = 0; i < numVertices; ++i)
    {
        XMVECTOR P{ verticesPos[i].x, verticesPos[i].y, verticesPos[i].z };
        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }

    // convert min/max representation to center and extents representation
    XMStoreFloat3(&aabb.Center,  0.5f * (vMin + vMax));
    XMStoreFloat3(&aabb.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void ModelImporter::GetVerticesIndicesOfMesh(
    const aiMesh* pMesh,
    BasicModel& model,
    MeshGeometry::Subset& currSubset,
    const int subsetIdx)
{
    // fill in the arrays with vertices/indices/subset data of the input mesh

    auto start = std::chrono::steady_clock::now();

    // compute the number of previous vertices/indices so we will know a starting
    // position to continue writing data
    for (int i = 0; i < subsetIdx; ++i)
    {
        const MeshGeometry::Subset& subset = model.meshes_.subsets_[i];
        currSubset.vertexStart += subset.vertexCount;
        currSubset.indexStart += subset.indexCount;
    }

    // define how many vertices/indices this subset (mesh) has
    currSubset.vertexCount = pMesh->mNumVertices;
    currSubset.indexCount = pMesh->mNumFaces * 3;

    ComputeMeshVertices(
        model.vertices_,
        pMesh,
        currSubset.vertexStart,
        currSubset.vertexCount);

    ComputeMeshIndices(
        model.indices_,
        pMesh->mFaces,
        currSubset.indexStart,
        (int)pMesh->mNumFaces);

    ComputeMeshAABB(
        model.subsetsAABB_[subsetIdx],
        pMesh->mVertices,
        currSubset.vertexCount);


    // compute the duration of the vertices/indices loading process for the whole model
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    ModelImporter::s_VerticesLoading_ += elapsed.count();
}

} // namespace Core
