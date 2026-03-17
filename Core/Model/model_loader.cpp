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
#include "model.h"
#include "FileSystemPaths.h"
#include <Mesh/material_mgr.h>
#include <Mesh/material_reader.h>

namespace Core
{

//---------------------------------------------------------
// helpers forward declaration
//---------------------------------------------------------
void ReadHeaderAndAllocMem(FILE* pFile, Model& model);
void ReadMaterials        (FILE* pFile, Model& model, const char* path);
void ReadSubsets          (FILE* pFile, Model& model);
void ReadAABBs            (FILE* pFile, Model& model);
void ReadVertices         (FILE* pFile, Model& model);
void ReadIndices          (FILE* pFile, Model& model);


//---------------------------------------------------------
// Desc:   init input model with data loaded from file
//---------------------------------------------------------
bool ModelLoader::Load(const char* filePath, Model* pModel)
{
    // check input args
    if (StrHelper::IsEmpty(filePath))
    {
        LogErr(LOG, "empty filepath");
        return false;
    }
    if (!pModel)
    {
        LogErr(LOG, "ptr to model == NULL");
        return false;
    }


    // open file for reading
    char path[512]{ '\0' };
    strcat(path, g_RelPathAssetsDir);
    strcat(path, filePath);

    FILE* pFile = fopen(path, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for model loading: %s", path);
        return false;
    }

    LogMsg(LOG, "load model: %s", filePath);

    ReadHeaderAndAllocMem(pFile, *pModel);
    ReadMaterials        (pFile, *pModel, path);
    ReadSubsets          (pFile, *pModel);
    ReadAABBs            (pFile, *pModel);
    ReadVertices         (pFile, *pModel);
    ReadIndices          (pFile, *pModel);

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:   read common params for this model
//         and allocate memory for model's vertices, indices, and meshes (subsets)
//---------------------------------------------------------
void ReadHeaderAndAllocMem(FILE* pFile, Model& model)
{
    assert(pFile);

    int numVertices, numIndices, numSubsets;

    // skip chunk/block header
    fscanf(pFile, "%s", g_String);

    fscanf(pFile, "\nName: %s", g_String);
    model.SetName(g_String);

    fscanf(pFile, "\nMeshes:   %d", &numSubsets);
    fscanf(pFile, "\nVertices: %d", &numVertices);
    fscanf(pFile, "\nIndices:  %d", &numIndices);
    fscanf(pFile, "\n\n");

    // allocate memory for the model and some temp data
    model.AllocMem(numVertices, numIndices, numSubsets);
}

//---------------------------------------------------------
// Desc:   load materials from file and bind them to this model
//---------------------------------------------------------
void ReadMaterials(FILE* pFile, Model& model, const char* relFilePath)
{
    assert(pFile);
    assert(!StrHelper::IsEmpty(relFilePath));

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
void ReadSubsets(FILE* pFile, Model& model)
{
    // skip block comment
    int c;
    while ((c = fgetc(pFile)) != EOF && c != '\n')
        continue;

    Subset* subsets = model.GetSubsets();

    for (int i = 0; i < model.GetNumSubsets(); ++i)
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
void ReadAABBs(FILE* pFile, Model& model)
{
    assert(pFile);

    // skip block comment
    int sym;
    while ((sym = fgetc(pFile)) != EOF && sym != '\n')
        continue;

    // center, extents
    DirectX::XMFLOAT3 c;
    DirectX::XMFLOAT3 e;

    // read model's local AABB
    int count = fscanf(pFile, "Model: %f %f %f %f %f %f\n", &c.x, &c.y, &c.z, &e.x, &e.y, &e.z);
    assert(count == 6);
    model.SetModelAABB(DirectX::BoundingBox(c, e));


    // setup model's local bounding sphere
    DirectX::BoundingSphere sphere;
    DirectX::BoundingSphere::CreateFromBoundingBox(sphere, model.GetModelAABB());
    model.SetModelBoundSphere(sphere);


    // read AABB for each subset (mesh)
    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        int idx = 0;
        count = fscanf(pFile, "Subset_%d: %f %f %f %f %f %f\n", &idx, &c.x, &c.y, &c.z, &e.x, &e.y, &e.z);

        assert(count == 7 && "invalid count of data elements");
        assert(idx == i   && "read data for invalid subset");

        model.SetSubsetAABB(i, DirectX::BoundingBox(c, e));
    }
    fscanf(pFile, "\n");
}

//---------------------------------------------------------
// Desc:   read in vertices in binary representation
//---------------------------------------------------------
void ReadVertices(FILE* pFile, Model& model)
{
    assert(pFile);

    // skip block comment
    int c;
    while ((c = fgetc(pFile)) != EOF && c != '\n')
        continue;

    Vertex3D* verts = model.GetVertices();
    int    numVerts = model.GetNumVertices();

    if (fread(verts, sizeof(Vertex3D), numVerts, pFile) != numVerts)
    {
        assert(!StrHelper::IsEmpty(model.GetName()));
        LogErr(LOG, "can't read vertices for model: %s", model.GetName());
    }
}

//---------------------------------------------------------
// Desc:   read in indices in binary representation
//---------------------------------------------------------
void ReadIndices(FILE* pFile, Model& model)
{
    assert(pFile);

    fscanf(pFile, "%s\n", g_String);     // skip chunk/block header

    UINT*  indices = model.GetIndices();
    int numIndices = model.GetNumIndices();

    if (fread(indices, sizeof(UINT), numIndices, pFile) != numIndices)
    {
        assert(!StrHelper::IsEmpty(model.GetName()));
        LogErr(LOG, "can't read indices for model: %s", model.GetName());
    }
}

} // namespace
