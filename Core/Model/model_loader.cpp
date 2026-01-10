/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_loader.cpp

    Created:  19.10.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "model_loader.h"
#include "basic_model.h"
#include "FileSystemPaths.h"
#include <Mesh/material_mgr.h>
#include <Mesh/material_reader.h>

namespace Core
{

//---------------------------------------------------------
// helpers forward declaration
//---------------------------------------------------------
void ReadHeader   (FILE* pFile, BasicModel& model);
void ReadMaterials(FILE* pFile, BasicModel& model, const char* path);
void ReadSubsets  (FILE* pFile, BasicModel& model);
void ReadAABBs    (FILE* pFile, BasicModel& model);
void ReadVertices (FILE* pFile, BasicModel& model);
void ReadIndices  (FILE* pFile, BasicModel& model);


//---------------------------------------------------------
// Desc:   init input model with data loaded from file
//---------------------------------------------------------
bool ModelLoader::Load(const char* filePath, BasicModel* pModel)
{
    // check input args
    if (!filePath || filePath[0] == '\0')
    {
        LogErr(LOG, "input filepath is empty");
        return false;
    }
    if (!pModel)
    {
        LogErr(LOG, "input ptr to model == nullptr");
        return false;
    }


    // open file for reading
    char path[512]{ '\0' };
    strcat(path, g_RelPathAssetsDir);
    strcat(path, filePath);

    FILE* pFile = fopen(path, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for model reading: %s", path);
        return false;
    }

    LogMsg(LOG, "load model: %s", filePath);

    ReadHeader(pFile, *pModel);

    // allocate memory for the model and some temp data
    pModel->AllocateMemory(pModel->numVertices_, pModel->numIndices_, pModel->numSubsets_);

    ReadMaterials(pFile, *pModel, path);
    ReadSubsets(pFile, *pModel);
    ReadAABBs(pFile, *pModel);
    ReadVertices(pFile, *pModel);
    ReadIndices(pFile, *pModel);

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:   read common params for this model
//---------------------------------------------------------
void ReadHeader(FILE* pFile, BasicModel& model)
{
    assert(pFile != nullptr);

    fscanf(pFile, "%s",                    g_String);  // skip chunk/block header
    fscanf(pFile, "\nName: %s",            model.name_);
    fscanf(pFile, "\nMeshes: %"    SCNu16, &model.numSubsets_);
    fscanf(pFile, "\nVertices: %d" SCNu32, &model.numVertices_);
    fscanf(pFile, "\nIndices: %"   SCNu32, &model.numIndices_);
    fscanf(pFile, "\nBones: %"     SCNu16, &model.numBones_);
    fscanf(pFile, "\nAnimClips: %" SCNu16, &model.numAnimClips_);
    fscanf(pFile, "\n\n");
}

//---------------------------------------------------------
// Desc:   load materials from file and bind them to this model
//---------------------------------------------------------
void ReadMaterials(
    FILE* pFile,
    BasicModel& model,
    const char* relFilePath)
{
    assert(pFile != nullptr);
    assert(relFilePath && (relFilePath[0] != '\0'));

    char relMatFilePath[256]{ '\0' };
    char matFileName[128]{'\0'};
    char matName[MAX_LEN_MAT_NAME]{ '\0' };
    int meshIdx = 0;
    int numMats = 0;
    int count = 0;

    // skip chunk/block header
    fgets(g_String, sizeof(g_String), pFile);

    
    count = fscanf(pFile, "MatFile: %s\n", matFileName);
    assert(count == 1);

    // generate a relative path to the materials file
    FileSys::GetParentPath(relFilePath, relMatFilePath);
    strcat(relMatFilePath, matFileName);

    // read materials from file and add them into the material manager
    MaterialReader matReader;
    matReader.Read(relMatFilePath);

    // setup material ID for each subset (mesh)
    count = fscanf(pFile, "NumMaterials: %d\n", &numMats);
    assert(count == 1);

    Subset* subsets = model.GetSubsets();

    for (int i = 0; i < numMats; ++i)
    {
        count = fscanf(pFile, "Subset%d_MatName: %s\n", &meshIdx, matName);
        assert(count == 2);

        subsets[i].materialId = g_MaterialMgr.GetMatIdByName(matName);
    }

    fscanf(pFile, "\n\n");
}

//---------------------------------------------------------
// read in each subset (mesh) data of this model
//---------------------------------------------------------
void ReadSubsets(FILE* pFile, BasicModel& model)
{
    // skip block comment
    int c;
    while ((c = fgetc(pFile)) != EOF && c != '\n')
        continue;

    Subset* subsets = model.GetSubsets();

    for (int i = 0; i < model.numSubsets_; ++i)
    {
        fscanf(pFile, "\nSubsetID: %"    SCNu16, &subsets[i].id);
        fscanf(pFile, "\nVertexStart: %" SCNu32, &subsets[i].vertexStart);
        fscanf(pFile, "\nVertexCount: %" SCNu32, &subsets[i].vertexCount);
        fscanf(pFile, "\nIndexStart: %"  SCNu32, &subsets[i].indexStart);
        fscanf(pFile, "\nIndexCount: %"  SCNu32, &subsets[i].indexCount);
        fscanf(pFile, "\nName: %s",              subsets[i].name);
        fscanf(pFile, "\n");
    }
}

//---------------------------------------------------------
// Desc:   read AABB for the whole model and for each mesh
//---------------------------------------------------------
void ReadAABBs(FILE* pFile, BasicModel& model)
{
    assert(pFile != nullptr);

    // skip block comment
    int sym;
    while ((sym = fgetc(pFile)) != EOF && sym != '\n')
        continue;

    // read AABB of the whole model (in local space)
    DirectX::XMFLOAT3& c = model.modelAABB_.Center;
    DirectX::XMFLOAT3& e = model.modelAABB_.Extents;

    int count = fscanf(pFile, "Model: %f %f %f %f %f %f\n", &c.x, &c.y, &c.z, &e.x, &e.y, &e.z);
    assert(count == 6);

    // read AABB for each subset (mesh)
    for (int i = 0; i < model.numSubsets_; ++i)
    {
        DirectX::XMFLOAT3& sc = model.subsetsAABB_[i].Center;
        DirectX::XMFLOAT3& se = model.subsetsAABB_[i].Extents;
        int idx = 0;

        count = fscanf(pFile, "Subset_%d: %f %f %f %f %f %f\n", &idx, &sc.x, &sc.y, &sc.z, &se.x, &se.y, &se.z);

        assert(count == 7);
        assert(idx == i);
    }
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:   read in vertices in binary representation
//---------------------------------------------------------
void ReadVertices(FILE* pFile, BasicModel& model)
{
    assert(pFile != nullptr);

    // skip block comment
    int c;
    while ((c = fgetc(pFile)) != EOF && c != '\n')
        continue;

    size_t res = fread(model.vertices_, sizeof(Vertex3D), model.numVertices_, pFile);
    if (res != model.numVertices_)
    {
        LogErr(LOG, "can't read vertices for model: %s", model.GetName());
        return;
    }
}

//---------------------------------------------------------
// Desc:   read in indices in binary representation
//---------------------------------------------------------
void ReadIndices(FILE* pFile, BasicModel& model)
{
    assert(pFile != nullptr);

    fscanf(pFile, "%s\n", g_String);     // skip chunk/block header

    size_t res = fread(model.indices_, sizeof(UINT), model.numIndices_, pFile);
    if (res != model.numIndices_)
    {
        LogErr(LOG, "can't read indices for model: %s", model.GetName());
        return;
    }
}

} // namespace
