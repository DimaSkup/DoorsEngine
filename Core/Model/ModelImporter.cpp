////////////////////////////////////////////////////////////////////
// Filename:      ModelImporter.cpp
// 
// Created:       07.07.23
////////////////////////////////////////////////////////////////////
#include <CoreCommon/pch.h>
#include "ModelImporter.h"

#include "ModelMath.h"
#include "../Mesh/MaterialMgr.h"
#include "../Model/ModelLoaderM3D.h"
#include "../Texture/TextureMgr.h"
#include "../Model/ModelImporterHelpers.h"

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
bool ModelImporter::LoadFromFile(
    ID3D11Device* pDevice,
    BasicModel& model,
    const char* filePath)
{
    // this function initializes a new model from the file 
    // of type .blend, .fbx, .3ds, .obj, .m3d, etc.

    if ((filePath == nullptr) || (filePath[0] == '\0'))
    {
        LogErr("can't load file: input path is empty!");
        return false;
    }

    // compute duration of importing process
    auto start = std::chrono::steady_clock::now();

    try
    {
        char fileExt[8]{'\0'};
        FileSys::GetFileExt(filePath, fileExt);

        // define if we want to load (import) .m3d file
        if (strcmp(fileExt, "m3d") == 0)
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
            aiProcess_FixInfacingNormals |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_ImproveCacheLocality |
            aiProcess_Triangulate |
            aiProcess_ConvertToLeftHanded);

        // assert that we successfully read the data file
        if (pScene == nullptr)
        {
            LogErr(LOG, "can't read a model's data from file: %s", filePath);
            return false;
        }

        // compute duration of the scene loading
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

#if 0
        // compute normal vectors
        for (int i = 0; i < model.numIndices_ / 3; ++i)
        {
            // indices of the ith triangle 
            int baseIdx = i * 3;
            const UINT i0 = model.indices_[baseIdx + 0];
            const UINT i1 = model.indices_[baseIdx + 1];
            const UINT i2 = model.indices_[baseIdx + 2];

            // positions of vertices of ith triangle stored as XMVECTOR
            XMVECTOR v0 = DirectX::XMLoadFloat3(&model.vertices_[i0].position);
            XMVECTOR v1 = DirectX::XMLoadFloat3(&model.vertices_[i1].position);
            XMVECTOR v2 = DirectX::XMLoadFloat3(&model.vertices_[i2].position);

            // compute face normal
            XMVECTOR e0 = v1 - v0;
            XMVECTOR e1 = v2 - v0;
            XMVECTOR normalVec = DirectX::XMVector3Cross(e0, e1);
            XMFLOAT3 faceNormal;
            DirectX::XMStoreFloat3(&faceNormal, normalVec);

            model.vertices_[i0].normal = faceNormal;
            model.vertices_[i1].normal = faceNormal;
            model.vertices_[i2].normal = faceNormal;
        }
#endif
#if 0
        // compute tangent for each vertex
        for (int i = 0; i < model.numIndices_ / 3; ++i)
        {
            // get indices
            int baseIdx = i * 3;
            const UINT i0 = model.indices_[baseIdx + 0];
            const UINT i1 = model.indices_[baseIdx + 1];
            const UINT i2 = model.indices_[baseIdx + 2];

            // get vertices by indices
            Vertex3D& v0 = model.vertices_[i0];
            Vertex3D& v1 = model.vertices_[i1];
            Vertex3D& v2 = model.vertices_[i2];

            // compute edge vectors of the triangle
            const XMFLOAT3 e0 = {
                v1.position.x - v0.position.x,
                v1.position.y - v0.position.y,
                v1.position.z - v0.position.z };

            const XMFLOAT3 e1 = {
                v2.position.x - v0.position.x,
                v2.position.y - v0.position.y,
                v2.position.z - v0.position.z };

            // compute deltas of texture coords
            const float du0 = v1.texture.x - v0.texture.x;
            const float dv0 = v1.texture.y - v0.texture.y;
            const float du1 = v2.texture.x - v0.texture.x;
            const float dv1 = v2.texture.y - v0.texture.y;

            // compute interse matrix of texture coords
            const float invDet = 1.0f / (du0 * dv1 - dv0 * du1);

            //  | m00 m01 |
            //  | m10 m11 |
            const float m00 = invDet * +dv1;
            const float m01 = invDet * -dv0;
            const float m10 = invDet * -du1;
            const float m11 = invDet * +du0;

            // compute tangent coords
            float Tx = (m00 * e0.x) + (m01 * e1.x);
            float Ty = (m00 * e0.y) + (m01 * e1.y);
            float Tz = (m00 * e0.z) + (m01 * e0.z);

            const float invTangLen = 1.0f / sqrtf(Tx * Tx + Ty * Ty + Tz * Tz);
            Tx *= invTangLen;
            Ty *= invTangLen;
            Tz *= invTangLen;

            v0.tangent = { Tx, Ty, Tz };
            v1.tangent = { Tx, Ty, Tz };
            v2.tangent = { Tx, Ty, Tz };
        }
#endif

        importer.FreeScene();

        // compute the duration of the nodes processing
        auto nodeEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> nodeElapsed = nodeEnd - nodeStart;
        ModelImporter::s_NodesLoading_ += nodeElapsed.count();

        // compute the duration of the whole process of importing
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;

        ModelImporter::s_ImportDuration_ += elapsed.count();

        LogDbg(LOG, "Model is loaded from file: %s", filePath);
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't import a model from the file: %s", filePath);
        return false;
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
    const char* filePath)                            // full path to the model
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
    const char* filePath)                      // full path to the model
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
        LoadMaterialTextures(pDevice, material.texIds, pAiMat, subset, pScene, filePath);

        // store a material into the material manager and also store its material ID into the subset (mesh)
        strncpy(material.name, pAiMat->GetName().C_Str(), MAX_LENGTH_MATERIAL_NAME);
        subset.materialId = g_MaterialMgr.AddMaterial(std::move(material));
        
    }
    catch (EngineException& e)
    {
        LogErr(e);
        sprintf(g_String, "can't import a mesh \"%s\" for a model from file: %s", subset.name, filePath);
        LogErr(g_String);
        throw EngineException(g_String);
    }
}

///////////////////////////////////////////////////////////

void ModelImporter::LoadMaterialColorsData(aiMaterial* pMaterial, Material& mat)
{
    // read material properties for this mesh

    // global default materials for this .cpp
    const Float4 defaultAmbient (0.3f, 0.3f, 0.3f, 1);
    const Float4 defaultDiffuse (0.8f, 0.8f, 0.8f, 1);
    const Float4 defaultSpecular(0.0f, 0.0f, 0.0f, 1);           // w == spec power
    const Float4 defaultReflect (0.5f, 0.5f, 0.5f, 1);

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
    const char* filePath)
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

  

    std::map<aiTextureType, std::string> texTypeToPath;

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
                // get a path to the texture file
                char texturePath[256]{ '\0' };
                FileSys::GetParentPath(filePath, g_String);
                strcat(texturePath, g_String);
                strcat(texturePath, path.C_Str());

                // store pair: [texture_type => texture_path]
                texTypeToPath.insert_or_assign(type, texturePath);

                break;
            }

            // load an embedded compressed texture
            case TexStoreType::EmbeddedCompressed:
            {
                const aiTexture* pAiTexture = pScene->GetEmbeddedTexture(path.C_Str());
                constexpr bool   mipMapped = true;
                char             textureName[64]{ '\0' };

                FileSys::GetFileName(path.C_Str(), textureName);

                Texture texture(
                    pDevice,
                    textureName,
                    (uint8_t*)(pAiTexture->pcData),         // data of texture
                    pAiTexture->mWidth,                     
                    pAiTexture->mHeight,
                    mipMapped);

                // add into the tex mgr a new texture
                const TexID id = g_TextureMgr.Add(textureName, std::move(texture));


                texIDs[type] = id;

                break;
            }

            // load an embedded indexed compressed texture
            case TexStoreType::EmbeddedIndexCompressed:
            {
                constexpr bool  mipMapped = true;
                const UINT      index = GetIndexOfEmbeddedCompressedTexture(&path);
                char            textureName[64]{ '\0' };

                FileSys::GetFileName(path.C_Str(), textureName);
                aiTexture* pAiTex = pScene->mTextures[index];

                Texture texture(
                    pDevice,
                    textureName,
                    (uint8_t*)(pAiTex->pcData),
                    pAiTex->mWidth,
                    pAiTex->mHeight,
                    mipMapped);

                // add into the tex mgr a new texture
                const TexID id = g_TextureMgr.Add(textureName, std::move(texture));            

                texIDs[type] = id;

                break;

            } // end switch/case
            }
        }
    }


    // load textures from files
    for (const auto& [type, path] : texTypeToPath)
    {
        texIDs[type] = g_TextureMgr.LoadFromFile(path.c_str());
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

#if 1
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
#endif
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
