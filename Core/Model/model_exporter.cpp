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
#include "model.h"

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
void WriteHeader        (FILE* pFile, const Model* pModel, const char* targetName);
void WriteMaterials     (FILE* pFile, const Model* pModel, const char* matFilePath);
void WriteSubsetsData   (FILE* pFile, const Model* pModel);
void WriteAABBs         (FILE* pFile, const Model* pModel);
void WriteVertices      (FILE* pFile, const Vertex3D* vertices, const int numVertices);
void WriteIndices       (FILE* pFile, const UINT* indices, const int numIndices);
void StoreTextures      (const Model* pModel, const char* targetDir);

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
    const Model* pModel,
    const char* targetDir,
    const char* targetName)
{
    // check input args
    if (!pModel)
    {
        LogErr(LOG, "ptr to model == NULL");
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

    LogMsg(LOG, "export model into .de3d: %s", pModel->GetName());

    WriteHeader     (pFile, pModel, targetName);
    WriteMaterials  (pFile, pModel, materialFilePath);
    WriteSubsetsData(pFile, pModel);
    WriteAABBs      (pFile, pModel);

    WriteVertices   (pFile, pModel->GetVertices(), pModel->GetNumVertices());
    WriteIndices    (pFile,  pModel->GetIndices(), pModel->GetNumIndices());
    StoreTextures   (pModel, relTargetDir);

    LogMsg(LOG, "model is converted into .de3d: %s", targetName);
    fclose(pFile);
    return true;
}


// =================================================================================
//                               PRIVATE HELPERS
// =================================================================================
void WriteHeader(FILE* pFile, const Model* pModel, const char* targetName)
{
    assert(pFile);
    assert(pModel);
    assert(!StrHelper::IsEmpty(targetName));

    fprintf(pFile, "***************Header***************\n");
    fprintf(pFile, "Name: %s\n",       targetName);
    fprintf(pFile, "Meshes: %d\n",     pModel->GetNumSubsets());
    fprintf(pFile, "Vertices: %d\n",   pModel->GetNumVertices());
    fprintf(pFile, "Indices: %d\n",    pModel->GetNumIndices());
    fprintf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:   write metadata about model's materials
//---------------------------------------------------------
void WriteMaterials(FILE* pFile, const Model* pModel, const char* matFilePath)
{
    assert(pFile);
    assert(pModel);
    assert(!StrHelper::IsEmpty(matFilePath));

    const Subset* subsets = pModel->GetSubsets();
    const int  numSubsets = pModel->GetNumSubsets();

    char fileName[64]{ '\0' };
    FileSys::GetFileName(matFilePath, fileName);

    fprintf(pFile, "***************Materials*********************\n");
    fprintf(pFile, "MatFile: %s\n", fileName);
    fprintf(pFile, "NumMaterials: %d\n", numSubsets);


    // gather materials and write them into file
    cvector<Material> materials(numSubsets);

    for (int i = 0; i < numSubsets; ++i)
    {
        materials[i] = g_MaterialMgr.GetMatById(subsets[i].materialId);

        fprintf(pFile, "Subset%d_MatName: %s\n", i, materials[i].name);
    }
    fprintf(pFile, "\n");


    // store materials of the model into a separate file
    MaterialWriter matWriter;
    matWriter.Write(materials.data(), numSubsets, matFilePath);
}

//---------------------------------------------------------
// Desc:   write data of each model's subset (mesh)
//---------------------------------------------------------
void WriteSubsetsData(FILE* pFile, const Model* pModel)
{
    assert(pFile);
    assert(pModel);

    const Subset* subsets = pModel->GetSubsets();
    const int  numSubsets = pModel->GetNumSubsets();

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
void WriteAABBs(FILE* pFile, const Model* pModel)
{
    assert(pFile);
    assert(pModel);

    const DirectX::BoundingBox& modelAABB    = pModel->GetModelAABB();
    const DirectX::BoundingBox* subsetsAABBs = pModel->GetSubsetsAABB();

    fprintf(pFile, "***************AABB(center,extents)*******************\n");

    // write AABB of the whole model (in local space)
    fprintf(pFile, "Model:    ");
    WriteAABB(pFile, modelAABB.Center, modelAABB.Extents);

    for (int i = 0; i < pModel->GetNumSubsets(); ++i)
    {
        fprintf(pFile, "Subset_%d: ", i);
        WriteAABB(pFile, subsetsAABBs[i].Center, subsetsAABBs[i].Extents);
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
    assert(pFile);
    assert(indices);
    assert(numIndices > 0);

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
void WriteTextureIntoFile(const fs::path& texFullPath, ID3D11Resource* pTexResource)
{
    assert(!texFullPath.empty());
    assert(pTexResource);

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


    // load a texture data from VRAM into srcImage
    converter.LoadFromMemory(
        Render::GetD3dDevice(),
        Render::GetD3dContext(),
        pTexResource,
        srcImage);

    if (srcImage.GetMetadata().width == 1 || srcImage.GetMetadata().height == 1)
    {
        LogErr(LOG, "can't create a mipmap for 1x1 texture so skip storing into .dds: %s", path);
        return;
    }

    const bool bCanGenerateMips  = converter.CalcNumMipLevels(srcImage, mipLevels);
    const bool bNeedGenerateMips = srcImage.GetMetadata().mipLevels != mipLevels;

    if (bNeedGenerateMips && bCanGenerateMips)
    {
        // generate mip maps, compress/decompress, save into .dds file
        const ScratchImage mipChain(converter.GenMipMaps(srcImage, filter));
        converter.ProcessImage(mipChain, targetFormat, dstImage);

        if (converter.SaveToFile(dstImage, DDS_FLAGS_NONE, texFullPath))
            LogMsg(LOG, "texture is saved: %s", path);
        else
            LogErr(LOG, "can't save a texture into file: %s", path);

        return;
    }


    // if we need to execute any processing (but don't generate any mipmaps)
    if (srcImage.GetMetadata().format != targetFormat)
    {
        // compress/decompress, save into .dds file
        converter.ProcessImage(srcImage, targetFormat, dstImage);

        if (converter.SaveToFile(dstImage, DDS_FLAGS_NONE, texFullPath))
            LogMsg(LOG, "texture is saved: %s", path);
        else
            LogErr(LOG, "can't save a texture into file: %s", path);
    }
}

//---------------------------------------------------------
// Desc:   store all the model's textures as DDS files (with mipmaps, etc)
//---------------------------------------------------------
void StoreTextures(const Model* pModel, const char* targetDir)
{
    assert(pModel);
    assert(targetDir && targetDir[0] != '\0');

    const Subset* subsets = pModel->GetSubsets();

    // go through each subset
    for (int i = 0; i < pModel->GetNumSubsets(); ++i)
    {
        const Material& mat = g_MaterialMgr.GetMatById(subsets[i].materialId);
        const TexID* texIds = mat.texIds;

        // go through each texture and store it as .dds file
        for (int texIdx = 0; texIdx < NUM_TEXTURE_TYPES; ++texIdx)
        {
            if (texIds[texIdx] == INVALID_TEX_ID)
                continue;

            Texture& tex = g_TextureMgr.GetTexById(texIds[texIdx]);

            char texPath[256]{'\0'};
            strcat(texPath, targetDir);
            strcat(texPath, tex.GetName().c_str());
            strcat(texPath, ".dds");

            WriteTextureIntoFile(texPath, tex.GetResource());
        }
    }
}

} // namespace Core
