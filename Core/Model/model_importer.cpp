////////////////////////////////////////////////////////////////////
// Filename:      model_importer.cpp
// Description:   ASSIMP wrapper
//
//                imports a new model from the file of type:
//                .blend, .fbx, .3ds, .obj, etc.
// 
// Created:       07.07.23
////////////////////////////////////////////////////////////////////
#include <CoreCommon/pch.h>
#include "model_importer.h"

#include "../Mesh/material_mgr.h"
#include "../Texture/texture_mgr.h"

#include "basic_model.h"
#include "model_math.h"
#include "model_importer_helpers.h"
#include "vertices_splitter.h"
#include "animation_importer.h"

using namespace DirectX;


namespace Core
{

// static arrays for transient data
static cvector<Subset>  s_Subsets;
static cvector<RawMesh> s_Meshes;


//---------------------------------------------------------
// forward declarations of helper functions
//---------------------------------------------------------
bool InitFromScene(
    ID3D11Device* pDevice,
    const aiScene* pScene,
    const char* filename,
    BasicModel& model);

void CalcNumVerticesAndIndices(
    const aiScene* pScene,
    int& outNumVertices,
    int& outNumIndices);

void ProcessNode(
    ID3D11Device* pDevice,
    int& subsetIdx,
    const aiNode* pNode,
    const aiScene* pScene,
    const char* filePath);

void ProcessMesh(
    ID3D11Device* pDevice,
    const int subsetIdx,
    const aiMesh* pMesh,
    const aiScene* pScene,
    const char* filePath);

void LoadMaterialColorsData(
    const aiMaterial* pMaterial,
    Material& mat);

void LoadMaterialTextures(
    ID3D11Device* pDevice,
    TexID* texIDs,
    const aiMaterial* pMaterial,
    const Subset& subset,
    const aiScene* pScene,
    const char* filePath);

void GetVerticesIndicesOfMesh(
    const aiMesh* pMesh,
    const int subsetIdx);


//---------------------------------------------------------
// flags to control ASSIMP's import process
//---------------------------------------------------------
#define ASSIMP_LOAD_FLAGS (            \
    aiProcess_OptimizeGraph |          \
    aiProcess_OptimizeMeshes |         \
    aiProcess_JoinIdenticalVertices |  \
    aiProcess_FixInfacingNormals |     \
    aiProcess_GenNormals |             \
    aiProcess_ImproveCacheLocality |   \
    aiProcess_Triangulate |            \
    aiProcess_ConvertToLeftHanded)

//----------------------------------------------------------------------------------
// Desc:   import a new model from the file of type .blend, .fbx, .3ds, .obj, .m3d, etc.
//         (load geometry, load textures and material properties)
//----------------------------------------------------------------------------------
bool ModelImporter::LoadFromFile(
    ID3D11Device* pDevice,
    BasicModel* pModel,
    const char* filePath)
{
    if (StrHelper::IsEmpty(filePath))
    {
        LogErr(LOG, "can't load file: input path is empty!");
        return false;
    }

    LogMsg(LOG, "import model from file: %s", filePath);


    // compute duration of importing process
    auto importStart = std::chrono::steady_clock::now();

    BasicModel& model = *pModel;
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(filePath, ASSIMP_LOAD_FLAGS);


    // failed for some reason
    if (pScene == nullptr)
    {
        LogErr(LOG, "can't read a model from file: %s", filePath);
        LogErr(LOG, "Error parsing '%s': %s", filePath, importer.GetErrorString());

        char fileExt[8]{ '\0' };
        FileSys::GetFileExt(filePath, fileExt);

        if (strcmp(fileExt, ".fbx") == 0)
            LogErr(LOG, "DUDE, maybe your fbx is in text format, if so convert it into binary form");

        return false;
    }


    bool ret = InitFromScene(pDevice, pScene, filePath, model);

    if (ret)
        LogMsg(LOG, "model '%s' is imported from file: %s", model.name_, filePath);
    else
        LogErr(LOG, "didn't manage to import model: %s", filePath);


    // release memory from temp data
    importer.FreeScene();
    s_Meshes.purge();
    s_Subsets.purge();

    return ret;
}

//---------------------------------------------------------
// Desc:  init all the model's data loading it from assimp scene
//---------------------------------------------------------
bool InitFromScene(
    ID3D11Device* pDevice,
    const aiScene* pScene,
    const char* filePath,
    BasicModel& model)
{
    if (pScene->HasAnimations())
    {
        AnimationImporter animImporter;
        animImporter.LoadSkeletonAnimations(pScene, model.name_);
        //animImporter.PrintNodesHierarchy(pScene, false);
    }

    int subsetIdx = 0;
    int numVertices = 0;
    int numIndices = 0;
    int numMeshes = pScene->mNumMeshes;


    // allocate memory for transient data
    s_Meshes.resize(numMeshes);
    s_Subsets.resize(numMeshes);

    // compute how many vertices/indices/materials/subsets this model has
    CalcNumVerticesAndIndices(pScene, numVertices, numIndices);

    // load all the meshes/materials/textures/etc.
    ProcessNode(
        pDevice,
        subsetIdx,
        pScene->mRootNode,
        pScene,
        filePath);


    //
    // now our vertices/indices/subsets are prepared so
    // allocate memory for it in the model
    //
    if (!model.AllocateMemory(numVertices, numIndices, numMeshes))
    {
        LogErr(LOG, "can't alloc memory for model");
        return false;
    }

    // index to vertex/index in the model
    index vIdx = 0;

    // setup vertices of model
    for (int i = 0; i < numMeshes; ++i)
    {
        const RawMesh& mesh  = s_Meshes[i];
        const uint numVerts0 = (uint)s_Subsets[i].vertexCount;
        const uint numVerts1 = (uint)mesh.positions.size();

        if (numVerts0 != numVerts1)
        {
            LogErr(LOG, "the counter of vertices in subset_%d != the number of vertices in the mesh (%u != %u)", i, numVerts0, numVerts1);
            return false;
        }

        for (uint j = 0; j < s_Subsets[i].vertexCount; ++j, ++vIdx)
        {
            Vertex3D& v   = model.vertices_[vIdx];
            const Vec3& p = mesh.positions[j];
            const Vec3& n = mesh.normals[j];
            const Vec2& t = mesh.uvs[j];

            v.position = { p.x, p.y, p.z };
            v.texture  = { t.u, t.v };
            v.normal   = { n.x, n.y, n.z };
        }
    }

    // setup indices of model
    for (uint i = 0, iIdx = 0; i < (uint)numMeshes; ++i)
    {
        const RawMesh& mesh = s_Meshes[i];
        const uint numIdxs0 = (uint)s_Subsets[i].indexCount;
        const uint numIdxs1 = (uint)mesh.indices.size();

        if (numIdxs0 != numIdxs1)
        {
            LogErr(LOG, "counter of indices in the subset_%u != the number of indices in responsible mesh (%u != %u)", i, numIdxs0, numIdxs1);
            return false;
        }

        memcpy(model.indices_ + iIdx, mesh.indices.data(), sizeof(UINT) * numIdxs0);
        iIdx += s_Subsets[i].indexCount;
    }

    // setup subsets (meshes metadata) of model
    for (int i = 0; i < numMeshes; ++i)
    {
        model.meshes_.subsets_[i] = s_Subsets[i];
    }


    // go through each mesh of model and calculate per-vertex tangents
    ModelMath math;

    for (int i = 0; i < numMeshes; ++i)
    {
        const Subset& subset = model.meshes_.subsets_[i];
        Vertex3D* vertices   = &model.vertices_[subset.vertexStart];
        const UINT* indices  = &model.indices_[subset.indexStart];
        int numMeshVert      = subset.vertexCount;
        int numMeshIdxs      = subset.indexCount;

        math.CalcNormals(vertices, indices, numMeshVert, numMeshIdxs);
        math.CalcTangents(vertices, indices, numMeshVert, numMeshIdxs);
    }

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // release memory from temp meshes
    s_Meshes.purge();
    s_Subsets.purge();

    return true;
}


// =================================================================================
//                              PRIVATE METHODS
// =================================================================================
void CalcNumVerticesAndIndices(
    const aiScene* pScene,
    int& numVertices,
    int& numIndices)
{
    for (index i = 0; i < s_Meshes.size(); i++)
    {
        aiMesh* pMesh = pScene->mMeshes[i];

        s_Subsets[i].vertexStart = numVertices;
        s_Subsets[i].vertexCount = pMesh->mNumVertices;
        s_Subsets[i].indexStart  = numIndices;
        s_Subsets[i].indexCount  = pMesh->mNumFaces * 3;

        numVertices += pMesh->mNumVertices;
        numIndices  += pMesh->mNumFaces * 3;
    }
}

//---------------------------------------------------------
// Desc:  go through each node of the scene's tree structure
//        starting from the root node and initializes a mesh using data of this each node;
//
//        created mesh is pushed into the input model
//---------------------------------------------------------
void ProcessNode(
    ID3D11Device* pDevice,
    int& subsetIdx,
    const aiNode* pNode,
    const aiScene* pScene,
    const char* filePath)                            // full path to the model
{

    //const XMMATRIX nodeTransformMatrix = XMMATRIX(&pNode->mTransformation.a1) * parentTransformMatrix;

    // go through all the meshes in the current model's node
    for (UINT i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

        // handle this mesh and push it into the model's meshes array
        ProcessMesh(
            pDevice,
            subsetIdx,
            pMesh,
            pScene,
            filePath);

        ++subsetIdx;
    }

    // go through all the child nodes of the current node and handle it
    for (UINT i = 0; i < pNode->mNumChildren; i++)
    {
        ProcessNode(
            pDevice,
            subsetIdx,
            pNode->mChildren[i],
            pScene,
            filePath);
    }
}

//---------------------------------------------------------
// Desc:  generate a proper name for material
// Out:   - name:  store here a generated name
//---------------------------------------------------------
inline void HandleMaterialName(aiMaterial* pMat, const char* filePath, const int meshIdx, char* name)
{
    assert(pMat && name);
    strncpy(name, pMat->GetName().C_Str(), MAX_LEN_MAT_NAME - 1);


    // if for some reason our name is empty we generate it
    if (StrHelper::IsEmpty(name))
    {
        char filename[64]{ '\0' };
        FileSys::GetFileName(filePath, filename);

        snprintf(name, MAX_LEN_MAT_NAME, "%s_mat_%d", filename, meshIdx);
    }

    // prevent material name to have non digit and non alphabet symbols
    for (size_t i = 0; i < strlen(name); ++i)
    {
        if (!isdigit(name[i]) && !isalpha(name[i]))
            name[i] = '_';
    }
}

//---------------------------------------------------------
// Desc:   load mesh stuff (geometry/textures/materials)
//---------------------------------------------------------
void ProcessMesh(
    ID3D11Device* pDevice,
    const int subsetIdx,
    const aiMesh* pMesh,                       // the current mesh of the model
    const aiScene* pScene,                     // a ptr to the scene of this model type
    const char* filePath)                      // full path to the model
{
    Subset& subset = s_Subsets[subsetIdx];

    // get material data of this mesh
    aiMaterial* pAiMat = pScene->mMaterials[pMesh->mMaterialIndex];

    char matName[MAX_LEN_MAT_NAME]{ '\0' };
    HandleMaterialName(pAiMat, filePath, subsetIdx, matName);

        
    // create a new material in the material manager
    Material& material  = g_MaterialMgr.AddMaterial(matName);

    strncpy(subset.name, pMesh->mName.C_Str(), MAX_LEN_MESH_NAME);
    subset.materialId   = material.id;

    LoadMaterialColorsData(pAiMat, material);

    GetVerticesIndicesOfMesh(pMesh, subsetIdx);

    LoadMaterialTextures(pDevice, material.texIds, pAiMat, subset, pScene, filePath);
}

//---------------------------------------------------------
// Desc:   read material color properties for this mesh
//---------------------------------------------------------
void LoadMaterialColorsData(
    const aiMaterial* pMaterial,
    Material& mat)
{
    aiColor4D ambient;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D reflection;
    float shininess;
    
    // load materials from the aiMaterial
    const aiReturn isAmbientLoaded   = pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    const aiReturn isDiffuseLoaded   = pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    const aiReturn isSpecularLoaded  = pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    const aiReturn isShininessLoaded = pMaterial->Get(AI_MATKEY_SHININESS, shininess);
    const aiReturn isReflectLoaded   = pMaterial->Get(AI_MATKEY_REFLECTIVITY, reflection);

    if (isAmbientLoaded == aiReturn_SUCCESS)
        mat.ambient = Vec4(ambient.r, ambient.g, ambient.b, ambient.a);

    if (isDiffuseLoaded == aiReturn_SUCCESS)
        mat.diffuse = Vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    if (isSpecularLoaded == aiReturn_SUCCESS)
        mat.specular = Vec4(specular.r, specular.g, specular.b, 1.0f);

    // specular power / glossiness
    if (isShininessLoaded == aiReturn_SUCCESS)
        mat.specular.w = shininess;

    if (isReflectLoaded == aiReturn_SUCCESS)
        mat.reflect = Vec4(reflection.r, reflection.g, reflection.b, reflection.a);
}

//---------------------------------------------------------
// Desc:   load all the available textures for this mesh by its material data (from aiMaterial)
//---------------------------------------------------------
void LoadMaterialTextures(
    ID3D11Device* pDevice,
    TexID* texIDs,
    const aiMaterial* pMaterial,
    const Subset& subset,
    const aiScene* pScene,
    const char* filePath)
{
    char modelExt[16]{ '\0' };
    FileSys::GetFileExt(filePath, modelExt);

    aiTextureType texTypesToLoad  [NUM_TEXTURE_TYPES] { aiTextureType_NONE };
    uint          texCountsPerType[NUM_TEXTURE_TYPES]{ 0 };
    uint          numTexTypesToLoad = 0;

    // define what texture types to load
    for (index i = 1; i < NUM_TEXTURE_TYPES; ++i)
    {
        const aiTextureType type = (aiTextureType)i;

        // if there are some textures by this type
        if (UINT texCount = pMaterial->GetTextureCount(type))
        {
            texTypesToLoad[numTexTypesToLoad] = type;
            texCountsPerType[numTexTypesToLoad] = texCount;
            numTexTypesToLoad++;
        }
    }

    // go through available texture type and load responsible texture
    for (uint idx = 0; idx < numTexTypesToLoad; ++idx)
    {
        aiTextureType type = texTypesToLoad[idx];

        // go through each texture of this aiTextureType for this aiMaterial
        for (uint i = 0; i < texCountsPerType[idx]; i++)
        {
            aiString path;

            if (pMaterial->GetTexture(type, i, &path) != AI_SUCCESS)
            {
                LogErr(LOG, "can't get a texture of type: %s", (int)type);
                continue;
            }

            TexStoreType storeType = DetermineTexStoreType(pScene, pMaterial, i, type);

            if (type == aiTextureType_HEIGHT)
                type = aiTextureType_NORMALS;

            switch (storeType)
            {
                // load a tex which is located on the disk
                case TexStoreType::Disk:
                {
                    // get a path to the texture file
                    char texPath[256]{'\0'};
                    char texName[MAX_LEN_TEX_NAME]{'\0'};

                    FileSys::GetParentPath(filePath, g_String);
                    FileSys::GetFileStem(filePath, texName);

                    if (strcmp(modelExt, ".fbx") == 0)
                    {
                        char filename[128]{'\0'};
                        FileSys::GetFileName(path.C_Str(), filename);

                        strcat(texPath, g_String);
                        strcat(texPath, "texture/");
                        strcat(texPath, filename);
                    }
                    else
                    {
                        strcat(texPath, g_String);
                        strcat(texPath, path.C_Str());
                    }

                    // load texture and check it
                    const TexID texId = g_TextureMgr.LoadFromFile(texName, texPath);

                    if (texId == INVALID_TEX_ID)
                        LogErr(LOG, "can't load a texture from file: %s", texPath);

                    texIDs[type] = texId;

                    break;
                }

                // load an embedded compressed texture
                case TexStoreType::EmbeddedCompressed:
                {
                    const aiTexture* pAiTexture = pScene->GetEmbeddedTexture(path.C_Str());
                    constexpr bool   mipMapped = true;
                    char             textureName[128]{ '\0' };

                    FileSys::GetFileName(path.C_Str(), textureName);

                    Texture texture(
                        textureName,
                        (uint8_t*)(pAiTexture->pcData),         // data of texture
                        pAiTexture->mWidth,                     
                        pAiTexture->mHeight,
                        mipMapped);

                    // add a new texture into the texture manager
                    const TexID id = g_TextureMgr.Add(textureName, std::move(texture));

                    texIDs[type] = id;
                    break;
                }

                // load an embedded indexed compressed texture
                case TexStoreType::EmbeddedIndexCompressed:
                {
                    constexpr bool  mipMapped = true;
                    const UINT      index = GetIndexOfEmbeddedCompressedTexture(&path);
                    char            textureName[128]{ '\0' };

                    FileSys::GetFileName(path.C_Str(), textureName);
                    aiTexture* pAiTex = pScene->mTextures[index];

                    Texture texture(
                        textureName,
                        (uint8_t*)(pAiTex->pcData),
                        pAiTex->mWidth,
                        pAiTex->mHeight,
                        mipMapped);

                    // add a new texture into the texture manager
                    const TexID id = g_TextureMgr.Add(textureName, std::move(texture));            

                    texIDs[type] = id;
                    break;

                }
            } // end switch
        } // for
    }
}

//---------------------------------------------------------
// copy vertices/indices data from input mesh into the output
//---------------------------------------------------------
void InitMeshGeometry(const aiMesh* pMesh, RawMesh& outMesh)
{
    const uint numVertices = pMesh->mNumVertices;
    const uint numFaces    = pMesh->mNumFaces;

    outMesh.positions.resize(numVertices);
    outMesh.normals.resize(numVertices);
    outMesh.uvs.resize(numVertices);
    outMesh.indices.resize(numFaces * 3);


    // store positions/normals
    memcpy(outMesh.positions.data(), pMesh->mVertices, sizeof(Vec3) * numVertices);
    memcpy(outMesh.normals.data(),   pMesh->mNormals,  sizeof(Vec3) * numVertices);

    // store texture coords
    for (uint i = 0; i < numVertices; i++)
    {
        outMesh.uvs[i].x = pMesh->mTextureCoords[0][i].x;
        outMesh.uvs[i].y = pMesh->mTextureCoords[0][i].y;
    }

    // store indices
    for (uint faceIdx = 0, idx = 0; faceIdx < numFaces; ++faceIdx)
    {
        outMesh.indices[idx++] = pMesh->mFaces[faceIdx].mIndices[0];
        outMesh.indices[idx++] = pMesh->mFaces[faceIdx].mIndices[1];
        outMesh.indices[idx++] = pMesh->mFaces[faceIdx].mIndices[2];
    }
}


//---------------------------------------------------------
// fill in the arrays with vertices/indices/subset data of the input mesh
//---------------------------------------------------------
void GetVerticesIndicesOfMesh(
    const aiMesh* pMesh,
    const int subsetIdx)   // subset/mesh idx in model
{
#if 0
    RawMesh origMesh;
    InitMeshGeometry(pMesh, origMesh);

    RawMesh& modifiedMesh = s_Meshes[subsetIdx];

    SplitMirroredUVVertices(
        origMesh.positions,
        origMesh.normals,
        origMesh.uvs,
        origMesh.indices,
        modifiedMesh.positions,
        modifiedMesh.normals,
        modifiedMesh.uvs,
        modifiedMesh.indices);
#else

    InitMeshGeometry(pMesh, s_Meshes[subsetIdx]);

#endif

    s_Subsets[subsetIdx].id = (uint16)subsetIdx;
}

} // namespace Core
