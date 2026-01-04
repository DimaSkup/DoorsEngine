/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_exporter.cpp
    Desc:     exports models which were imported or manually generated into
              the .de3d format

    Created:  11.11.2024 by DimaSkup
\**********************************************************************************/

#include <CoreCommon/pch.h>
#include "model_exporter.h"
#include "basic_model.h"

#include "../Texture/enum_texture_types.h"
#include <ImgConverter.h>
#include <Texture/texture_mgr.h>
#include <Mesh/material_mgr.h>
#include <Mesh/material_writer.h>

#include <Render/d3dclass.h>


namespace Core
{

//---------------------------------------------------------
// forward declarations of helpers
//---------------------------------------------------------
void WriteHeader        (FILE* pFile, const BasicModel* pModel, const char* targetName);
void WriteMaterials     (ID3D11Device* pDevice, FILE* pFile, const BasicModel* pModel, const char* matFilePath);
void WriteSubsetsData   (FILE* pFile, const BasicModel* pModel);
void WriteAABBs         (FILE* pFile, const BasicModel* pModel);
void WriteVertices      (FILE* pFile, const Vertex3D* vertices, const int numVertices);
void WriteIndices       (FILE* pFile, const UINT* indices, const int numIndices);
void StoreTextures      (const BasicModel* pModel, ID3D11Device* pDevice, const char* targetDir);


//---------------------------------------------------------
// a little cringe helper to wrap getting of DX11 device
//---------------------------------------------------------
static ID3D11Device* GetDevice()
{
    return Render::g_pDevice;
}

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
ModelExporter::ModelExporter()
{
    // create a directory for this exported model (if not exist)
    if (!fs::exists(g_RelPathAssetsDir))
        fs::create_directory(g_RelPathAssetsDir);
}

//---------------------------------------------------------
// Desc:   store model's data into a .de3d file by path
// Args:   - pDevice:     a ptr to DirectX11 device
//         - pModel:      a ptr to the model
//         - targetDir:   a relative path to target directory (relatively to models assets directory)
//         - targetName:  a name for .de3d and .demat files
//---------------------------------------------------------
bool ModelExporter::ExportIntoDE3D(
    const BasicModel* pModel,
    const char* targetDir,
    const char* targetName)
{
    // check input args
    if (!pModel)
    {
        LogErr(LOG, "input ptr to model == nullptr");
        return false;
    }
    if (!targetDir || targetDir[0] == '\0')
    {
        LogErr(LOG, "target directory path is empty!");
        return false;
    }
    if (!targetName || targetName[0] == '\0')
    {
        LogErr(LOG, "target name is empty!");
        return false;
    }

    const size_t targetDirLen = strlen(targetDir);
    if (targetDir[targetDirLen - 1] != '/' && targetDir[targetDirLen - 1] != '\\')
    {
        LogErr(LOG, "target directory path is invalid: put slash at the end of string");
        return false;
    }

    
    // generate paths to model and material files (relatively to working dir)
    char relTargetDir[256]{ '\0' };
    char modelFilePath[256]{ '\0' };
    char materialFilePath[256]{ '\0' };

    snprintf(relTargetDir, 256, "%s%s", g_RelPathAssetsDir, targetDir);
    snprintf(modelFilePath, 256, "%s%s.de3d", relTargetDir, targetName);
    snprintf(materialFilePath, 256, "%s%s.demat", relTargetDir, targetName);


    // create a directory for this asset model (if dir not exist)
    if (!fs::exists(relTargetDir))
        fs::create_directories(relTargetDir);


    // open .de3d file for writing model's data
    FILE* pFile = fopen(modelFilePath, "wb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for model exporting (into .de3d format): %s", modelFilePath);
        return false;
    }

    LogMsg(LOG, "export model into .de3d: %s", pModel->name_);

    WriteHeader(pFile, pModel, targetName);
    WriteMaterials(GetDevice(), pFile, pModel, materialFilePath);
    WriteSubsetsData(pFile, pModel);
    WriteAABBs(pFile, pModel);
    WriteVertices(pFile, pModel->vertices_, pModel->numVertices_);
    WriteIndices(pFile, pModel->indices_, pModel->numIndices_);
    StoreTextures(pModel, GetDevice(), relTargetDir);

    LogMsg(LOG, "model is converted into .de3d: %s", targetName);
    fclose(pFile);
    return true;
}


// =================================================================================
//                               PRIVATE HELPERS
// =================================================================================
void WriteHeader(FILE* pFile, const BasicModel* pModel, const char* targetName)
{
    assert(pFile != nullptr);
    assert(pModel != nullptr);
    assert(targetName && (targetName[0] != '\0'));

    fprintf(pFile, "***************Header***************\n");
    fprintf(pFile, "Name: %s\n",       targetName);
    fprintf(pFile, "Meshes: %d\n",     pModel->numSubsets_);
    fprintf(pFile, "Vertices: %d\n",   pModel->numVertices_);
    fprintf(pFile, "Indices: %d\n",    pModel->numIndices_);
    fprintf(pFile, "Bones: %d\n",      pModel->numBones_);
    fprintf(pFile, "AnimClips: %d\n",  pModel->numAnimClips_);
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:   write metadata about model's materials
//---------------------------------------------------------
void WriteMaterials(
    ID3D11Device* pDevice,
    FILE* pFile,
    const BasicModel* pModel,
    const char* matFilePath)
{
    assert(pDevice != nullptr);
    assert(pFile != nullptr);
    assert(pModel != nullptr);
    assert(matFilePath && (matFilePath[0] != '\0'));


    const Subset* subsets = pModel->meshes_.subsets_;
    const int numSubsets = (int)pModel->meshes_.numSubsets_;

    char fileName[64]{ '\0' };
    FileSys::GetFileName(matFilePath, fileName);

    fprintf(pFile, "***************Materials*********************\n");
    fprintf(pFile, "MatFile: %s\n", fileName);
    fprintf(pFile, "NumMaterials: %d\n", numSubsets);


    // gather materials and write them into file
    cvector<Material> materials(numSubsets);

    for (int i = 0; i < numSubsets; ++i)
    {
        const MaterialID matId = subsets[i].materialId;
        materials[i] = g_MaterialMgr.GetMatById(subsets[i].materialId);

        fprintf(pFile, "Subset%d_MatName: %s\n", i, materials[i].name);
    }
    fprintf(pFile, "\n");


    // store materials into file
    MaterialWriter matWriter;
    matWriter.Write(materials.data(), numSubsets, matFilePath);
}

//---------------------------------------------------------
// Desc:   write data of each model's subset (mesh)
//---------------------------------------------------------
void WriteSubsetsData(FILE* pFile, const BasicModel* pModel)
{
    assert(pFile != nullptr);
    assert(pModel != nullptr);

    const Subset* subsets = pModel->meshes_.subsets_;
    const int numSubsets = (int)pModel->meshes_.numSubsets_;

    fprintf(pFile, "***************SubsetsData*******************\n");

    for (int i = 0; i < numSubsets; ++i)
    {
        const Subset& mesh = subsets[i];

        // handle mesh name
        char meshName[MAX_LEN_MESH_NAME]{'\0'};
        strncpy(meshName, mesh.name, MAX_LEN_MESH_NAME-1);

        for (size_t i = 0; i < strlen(meshName); ++i)
        {
            if (meshName[i] == ' ')
                meshName[i] = '_';
        }

        fprintf(pFile, "SubsetID: %d\n",    mesh.id);
        fprintf(pFile, "VertexStart: %d\n", (int)mesh.vertexStart);
        fprintf(pFile, "VertexCount: %d\n", (int)mesh.vertexCount);
        fprintf(pFile, "IndexStart: %d\n",  (int)mesh.indexStart);
        fprintf(pFile, "IndexCount: %d\n",  (int)mesh.indexCount);
        fprintf(pFile, "Name: %s\n\n",        meshName);
    }
}

//---------------------------------------------------------
// Desc:   write into file a data of single AABB
// Args:   - pFile:  file descriptor
//         - c:      AABB's center
//         - e:      how much AABB extents
//---------------------------------------------------------
inline void WriteAABB(FILE* pFile, const DirectX::XMFLOAT3& c, const DirectX::XMFLOAT3& e)
{
    fprintf(pFile, "%.2f %.2f %.2f %.2f %.2f %.2f\n", c.x, c.y, c.z, e.x, e.y, e.z);
}

//---------------------------------------------------------
// Desc:  write data about AABB of the whole model and each model's subset
//---------------------------------------------------------
void WriteAABBs(FILE* pFile, const BasicModel* pModel)
{
    assert(pFile != nullptr);
    assert(pModel != nullptr);

    fprintf(pFile, "***************AABB(center,extents)*******************\n");

    // write AABB of the whole model (in local space)
    fprintf(pFile, "Model:    ");
    WriteAABB(pFile, pModel->modelAABB_.Center, pModel->modelAABB_.Extents);

    for (int i = 0; i < pModel->numSubsets_; ++i)
    {
        fprintf(pFile, "Subset_%d: ", i);
        WriteAABB(pFile, pModel->subsetsAABB_[i].Center, pModel->subsetsAABB_[i].Extents);
    }

    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:   write data of each vertex in the model
//---------------------------------------------------------
void WriteVertices(FILE* pFile, const Vertex3D* vertices, const int numVertices)
{
    assert(pFile != nullptr);
    assert(vertices && (numVertices > 0));

    fprintf(pFile, "***************Vertices**********************\n");
    fwrite((void*)vertices, sizeof(Vertex3D), numVertices, pFile);
    fprintf(pFile, "\n\n");
}

//---------------------------------------------------------
// Desc:   write indices data (faces/triangles) of the model 
//---------------------------------------------------------
void WriteIndices(FILE* pFile, const UINT* indices, const int numIndices)
{
    assert(pFile != nullptr);
    assert(indices && (numIndices > 0));

    fprintf(pFile, "***************Indices**********************\n");
    fwrite((void*)indices, sizeof(UINT), numIndices, pFile);
}

//---------------------------------------------------------
// Desc:  check if there is a need to rewrite existed texture
//---------------------------------------------------------
bool IsNeedRewriteTexture(const fs::path& texFullpath, const DXGI_FORMAT targetFormat)
{
    DirectX::TexMetadata metadata;

    HRESULT hr = DirectX::GetMetadataFromDDSFile(
        texFullpath.wstring().c_str(),
        DirectX::DDS_FLAGS_NONE,
        metadata);

    if (FAILED(hr))
    {
        LogErr(LOG, "can't get metadata from texture file: %s", texFullpath.string().c_str());
        return false;
    }

    return (metadata.format != targetFormat) || (metadata.mipLevels == 1);
}

//---------------------------------------------------------
// store texture (if necessary) as .dds texture by texFullPath
// 
// 1. define if a texture by path exist (if not exist we go to step 3)
// 2. if exist we check if we need to rewrite it (if no we just go out)
// 3. load texture from memory and process it
// 4. write a texture into the .dds file
//---------------------------------------------------------
void WriteTextureIntoFile(
    const fs::path& texFullPath,
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    ID3D11Resource* pTexResource)
{
    using namespace DirectX;

    // target image params
    const DXGI_FORMAT      targetFormat = DXGI_FORMAT_BC3_UNORM;
    const TEX_FILTER_FLAGS filter       = TEX_FILTER_DEFAULT;


    if (fs::exists(texFullPath))
    {
        // if there is no need to rewrite this texture file we just go out
        if (!IsNeedRewriteTexture(texFullPath, targetFormat))
            return;
    }

    // ---------------------------------------------

    // load a texture data from memory and store it as .dds file
    ScratchImage srcImage;
    ScratchImage dstImage;
    Img::ImgConverter converter;
    const std::string pathStr(texFullPath.string());
    const char* path = pathStr.c_str();
    size_t mipLevels = 0;

    converter.LoadFromMemory(pDevice, pContext, pTexResource, srcImage);

    if (srcImage.GetMetadata().width == 1 || srcImage.GetMetadata().height == 1)
    {
        LogErr(LOG, "can't create a mipmap for 1x1 texture so skip storing into .dds: %s", path);
        return;
    }

    const bool canGenerateMips  = converter.CalcNumMipLevels(srcImage, mipLevels);
    const bool needGenerateMips = srcImage.GetMetadata().mipLevels != mipLevels;

    if (needGenerateMips && canGenerateMips)
    {
        // generate mip maps, compress/decompress, save into .dds file
        const ScratchImage mipChain(converter.GenMipMaps(srcImage, filter));
        converter.ProcessImage(mipChain, targetFormat, dstImage);

        if (converter.SaveToFile(dstImage, DDS_FLAGS_NONE, texFullPath))
            LogMsg(LOG, "texture is saved: %s", path);
        return;
    }


    // if we need to execute any processing (but don't generate any mipmaps)
    if (srcImage.GetMetadata().format != targetFormat)
    {
        // compress/decompress, save into .dds file
        converter.ProcessImage(srcImage, targetFormat, dstImage);

        if (converter.SaveToFile(dstImage, DDS_FLAGS_NONE, texFullPath))
            LogMsg(LOG, "texture is saved: %s", path);
    }
}

//---------------------------------------------------------
// Desc:   store all the model's textures as DDS files (with mipmaps, etc)
//---------------------------------------------------------
void StoreTextures(
    const BasicModel* pModel,
    ID3D11Device* pDevice,
    const char* targetDir)
{
    assert(pModel != nullptr);
    assert(pDevice != nullptr);
    assert(targetDir && targetDir[0] != '\0');

    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    const Subset*   subsets = pModel->meshes_.subsets_;

    // go through each subset
    for (int i = 0; i < pModel->numSubsets_; ++i)
    {
        const Material& mat = g_MaterialMgr.GetMatById(subsets[i].materialId);
        const TexID* texIds = mat.texIds;

        // go through each texture and store it as .dds file
        for (int texIdx = 0; texIdx < NUM_TEXTURE_TYPES; ++texIdx)
        {
            if (texIds[texIdx] == INVALID_TEXTURE_ID)
                continue;

            Texture& tex = g_TextureMgr.GetTexByID(texIds[texIdx]);

            char texPath[256]{'\0'};
            strcat(texPath, targetDir);
            strcat(texPath, tex.GetName().c_str());
            strcat(texPath, ".dds");

            WriteTextureIntoFile(texPath, pDevice, pContext, tex.GetResource());
        }
    }
}

} // namespace Core
