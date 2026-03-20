/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_mgr.cpp

    Created:  26.10.2025  by DimaSkup
\**********************************************************************************/

#include <CoreCommon/pch.h>
#include "grass_mgr.h"
#include <Render/d3dclass.h>
#include <geometry/frustum.h>
#include <camera_params.h>
#include <set>

#include <Mesh/material_mgr.h>
#include <Model/model_mgr.h>
#include <Model/geometry_generator.h>


namespace Core
{

//---------------------------------------------------------
// GLOBAL instance of the grass manager
//---------------------------------------------------------
GrassMgr g_GrassMgr;


//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void CheckInitParams(const GrassFieldInitParams& params);
void CalcFieldXZBoundings(GrassField& field, const GrassFieldInitParams& params);
void CreateCells(GrassField& field, const GrassFieldInitParams& params);
void ÑalcFieldYBoundings(GrassField& field);

void PushGrassInstancesInCell(
    GrassCell& cell,
    const int numInstances,
    int& instanceIdx,
    Model& grassModel,
    const int channel,
    const int numChannels,
    const int numTexRows);

void InitBuffers(GrassField& field);


//---------------------------------------------------------
// create a new grass field based on input params
//---------------------------------------------------------
bool GrassMgr::AddGrassField(const GrassFieldInitParams& params)
{
    CheckInitParams(params);

    const Terrain& terrain = g_ModelMgr.GetTerrain();

    // create new grass fields
    grassFields_.push_back(GrassField());
    GrassField& field = grassFields_.back();
    memset(&field, 0, sizeof(field));


    // setup a name and path to density mask
    strcpy(field.name, params.name);
    strcpy(field.densityMask, params.densityMask);


    field.grassCount = params.grassCount;
    field.cellsByX   = params.cellsByX;
    field.cellsByZ   = params.cellsByZ;
    field.texSlots   = params.texSlots;
    field.texRows    = params.texSlots;
    field.grassMinHeight = params.grassMinHeight;
    field.grassMaxHeight = params.grassMaxHeight;

    field.numChannels = field.texSlots;

    // get model id for each channel of grass field
    for (uint8 i = 0; i < field.numChannels; i++)
    {
        ModelID&    modelId   = field.channels[i].modelId;
        const char* modelName = params.modelNames[i];

        assert(!StrHelper::IsEmpty(modelName));

        modelId = g_ModelMgr.GetModelIdByName(modelName);
        if (modelId == INVALID_MODEL_ID)
        {
            LogErr(LOG, "no model (%s) to use for grass field (%s), channel (%d)", modelName, field.name, (int)i);
        }
    }
    

    // get material
    field.matId = g_MaterialMgr.GetMatIdByName(params.materialName);
    if (field.matId == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "no material (%s) to use for grass field (%s)", params.materialName, field.name);
    }

    CalcFieldXZBoundings(field, params);

    CreateCells(field, params);

    ÑalcFieldYBoundings(field);

    InitBuffers(field);
    

    LogMsg(LOG, "grass field is initialized: %s", params.name);
    return true;
}


//==================================================================================
// PRIVATE HELPERS
//==================================================================================

//---------------------------------------------------------
// check if we got only valid params for grass field's initialization
//---------------------------------------------------------
void CheckInitParams(const GrassFieldInitParams& params)
{
    assert(!StrHelper::IsEmpty(params.name));
    assert(!StrHelper::IsEmpty(params.materialName));
    assert(!StrHelper::IsEmpty(params.densityMask));

    assert(strlen(params.name)        < MAX_LEN_MODEL_NAME);
    assert(strlen(params.densityMask) < sizeof(GrassField::densityMask));

    assert(params.centerX > 0);
    assert(params.centerZ > 0);

    assert(params.sizeX > 0);
    assert(params.sizeZ > 0);

    assert(params.cellsByX > 0);
    assert(params.cellsByZ > 0);

    // these values must divide by 2
    assert((params.sizeX & 1) == 0);
    assert((params.sizeZ & 1) == 0);

    // size of the field should be divided by cells count exactly.
    // Otherwise, if there is a remainder of a division,
    // it may result in visual artifacts and unexpected visual effects.
    assert(params.sizeX % params.cellsByX == 0);
    assert(params.sizeZ % params.cellsByZ == 0);

    assert(params.texSlots > 0 && params.texSlots <= 4);
    assert(params.texRows  > 0 && params.texSlots <= 4);

    assert(params.grassCount > 0);

    assert(params.grassMinHeight > 0);
    assert(params.grassMaxHeight > params.grassMinHeight);
}


//---------------------------------------------------------
// Desc:  compute world axis-aligned bounding box of the grass field
//---------------------------------------------------------
void CalcFieldXZBoundings(GrassField& field, const GrassFieldInitParams& params)
{
    // compute field's boundings in world
    float minX = (params.centerX - params.sizeX / 2);
    float minZ = (params.centerZ - params.sizeZ / 2);
    float maxX = (params.centerX + params.sizeX / 2);
    float maxZ = (params.centerZ + params.sizeZ / 2);

    const Rect3d& terrainWorldBox = g_ModelMgr.GetTerrain().GetAABB();
    const Vec3    terrainMinP     = terrainWorldBox.MinPoint();
    const Vec3    terrainMaxP     = terrainWorldBox.MaxPoint();

    // clamp grass field bounding box to be within terrain's box
    // to prevent intersection with terrain boundaries
    if (minX < 0)
    {
        // offset grass field along positive X
        maxX += -minX;
        minX = 0;
    }

    if (maxX > terrainMaxP.x)
    {
        // offset grass field along negative X 
        minX -= (maxX - terrainMaxP.x);
        maxX = terrainMaxP.x;
    }

    if (minZ < 0)
    {
        // offset grass field along positive Z
        maxZ += -minZ;
        minZ = 0;
    }

    if (maxZ > terrainMaxP.z)
    {
        // offset grass field along negative Z
        minZ -= (maxZ - terrainMaxP.z);
        maxZ = terrainMaxP.z;
    }

    // check if grass fields is within terrain's boundaries
    assert(minX >= 0 && maxX <= terrainMaxP.x);
    assert(minZ >= 0 && maxZ <= terrainMaxP.z);

    field.worldBox = Rect3d(minX, maxX, 0, 0, minZ, maxZ);
}




//---------------------------------------------------------
//---------------------------------------------------------
void CreateCells(GrassField& field, const GrassFieldInitParams& params)
{
    const Terrain& terrain = g_ModelMgr.GetTerrain();


    Image densityMap;
    densityMap.LoadGrayscaleBMP(field.densityMask);

    if (!densityMap.IsLoaded())
    {
        LogErr(LOG, "can't initialize density map for grass field: %s", field.name);
        return;
    }


    uint densityMapWidth = densityMap.GetWidth();
    uint densityMapHeight = densityMap.GetHeight();

    

    // create cells and its boundings
    int numCells = field.cellsByX * field.cellsByZ;
    field.cells.resize(numCells);
    field.cellsWorldBoxes.resize(numCells);
  

    // generate grass instances:
    // place each grass instance in random position within the grass field
    cvector<GrassInstance> grassItems(field.grassCount);

    const float worldBoxInvDX = 1.0f / (field.worldBox.x1 - field.worldBox.x0);
    const float worldBoxInvDZ = 1.0f / (field.worldBox.z1 - field.worldBox.z0);

    for (GrassInstance& grass : grassItems)
    {
        uint8 density = 0;

        while (density < 1)
        {
            grass.pos.x = RandF(field.worldBox.x0, field.worldBox.x1);
            grass.pos.z = RandF(field.worldBox.z0, field.worldBox.z1);

            // scale grass position to image space
            const float x = (grass.pos.x - field.worldBox.x0) * worldBoxInvDX;
            const float y = (grass.pos.z - field.worldBox.z0) * worldBoxInvDZ;

            const uint px = x * densityMapWidth;
            const uint py = densityMapHeight - y * densityMapHeight;

            density = densityMap.GetPixelGray(px, py);
        }
       
        grass.pos.y = terrain.GetScaledInterpolatedHeightAtPoint(grass.pos.x, grass.pos.z);
    }

    // setup height for each grass instance
    for (GrassInstance& grass : grassItems)
    {
        grass.height = RandF(field.grassMinHeight, field.grassMaxHeight);
    }

    // calc world boundings of each cell
    const float cellBoxSizeX = field.worldBox.SizeX() / (float)field.cellsByX;
    const float cellBoxSizeZ = field.worldBox.SizeZ() / (float)field.cellsByZ;

    for (int i = 0; i < numCells; ++i)
    {
        int row = (i / field.cellsByZ);
        int col = (i % field.cellsByX);

        const float minX = field.worldBox.x0 + (col * cellBoxSizeX);
        const float maxX = minX + cellBoxSizeX;

        const float minZ = field.worldBox.z0 + (row * cellBoxSizeZ);
        const float maxZ = minZ + cellBoxSizeZ;

        // NOTE: Y-boundaries we calc later, it will be based on height
        // of lowest and highest position of grass instances
        field.cellsWorldBoxes[i] = Rect3d(minX, maxX, 0, terrain.GetAABB().y1 + 1, minZ, maxZ);
    }


    // group grass instances into its cells
    for (int cellIdx = 0; const Rect3d& worldBox : field.cellsWorldBoxes)
    {
        GrassCell& cell = field.cells[cellIdx++];

        // go through each grass instance
        for (GrassInstance& grass : grassItems)
        {
            if (worldBox.IsPointInRect(grass.pos))
            {
                cell.grassInstances.push_back(grass);
            }
        }
    }

    //
    // setup vertices for each cell
    //
    GeometryGenerator geomGen;
    Model grassModel;
    geomGen.GenerateGrass(grassModel, 1, 1);
    //geomGen.GenerateTreeLod1(grassModel, 1, 1, true, 0);

   
    // setup vertices for each channel of each cell
    for (GrassCell& cell : field.cells)
    {

        const size numInstances = cell.grassInstances.size();
        const size numVerticesInModel = grassModel.GetNumVertices();
        const size numIndicesInModel = grassModel.GetNumIndices();

        if (numInstances == 0)
            continue;

        int instanceIdx = 0;

        const size numVerticesInCell = numInstances * grassModel.GetNumVertices();
        const size numIndicesInCell = numInstances * grassModel.GetNumIndices();

        cell.vertices.reserve(numVerticesInCell);
        cell.indices.reserve(numIndicesInCell);

        for (int channel = 0; channel < field.numChannels; ++channel)
        {
            // currently we spread grass instances equally between each channel
            const int numInstForChannel = numInstances / field.numChannels;

            PushGrassInstancesInCell(
                cell,
                numInstForChannel,
                instanceIdx,
                grassModel,
                channel,
                field.numChannels,
                field.texRows);
        }
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void PushGrassInstancesInCell(
    GrassCell& cell,
    const int numInstances,
    int& instanceIdx,
    Model& grassModel,
    const int channel,          // index of the current channel [0-3]
    const int numChannels,      // num all possible channels for this cell [1-4]
    const int numTexRows)
{
    assert(numInstances > 0);
    assert(instanceIdx > -1);
    assert(grassModel.GetVertices() != nullptr);
    assert(grassModel.GetIndices() != nullptr);

    assert(numChannels > 0 && numChannels <= 4);
    assert(channel >= 0    && channel < numChannels);
    assert(numTexRows > 0 && numTexRows <= 4);
    

    const float tu = 1.0f / (float)numChannels;
    const float tv = 1.0f / (float)numTexRows;

    const float tu0 = tu * channel;
    const float tu1 = tu0 + tu;

    

    const Vertex3D* modelVerts = grassModel.GetVertices();
    const UINT*     modelIndices  = grassModel.GetIndices();

    const int numVerts = grassModel.GetNumVertices();
    const int numIndices = grassModel.GetNumIndices();


    for (int i = 0; i < numInstances; ++i, ++instanceIdx)
    {
        GrassInstance& inst = cell.grassInstances[instanceIdx];

        UINT baseIdx = (UINT)cell.vertices.size();

        const uint texRow = RandUint(0, 4);
        assert(texRow < 4);

        // push model's each vertex into vertices array of the grass cell
        for (int vIdx = 0; vIdx < numVerts; ++vIdx)
        {
            const Vertex3D& inVert = modelVerts[vIdx];

            cell.vertices.push_back(VertexGrass());
            VertexGrass&   outVert = cell.vertices.back();

            // setup position
            outVert.pos.x = inVert.pos.x + inst.pos.x;
            outVert.pos.y = inVert.pos.y + inst.pos.y;
            outVert.pos.z = inVert.pos.z + inst.pos.z;

            // if original tex coord == 0 (left edge of the texture image)
            if (inVert.tex.x < EPSILON_E5)
                outVert.tex.x = tu0;
            else
                outVert.tex.x = tu1;


          
            // if upper edge of the texture image
            if (inVert.tex.y < EPSILON_E5)
                outVert.tex.y = tv * texRow;
            else
                outVert.tex.y = tv * texRow + tv;

            outVert.normal = inVert.norm;
            
            //printf("v[%d]: %.2f %.2f %.2f\n", vIdx, v.pos.x, v.pos.y, v.pos.z);
        }
        //printf("\n");

        // push indices for this grass instance
       

        for (index i = 0; i < numIndices; ++i)
        {
            cell.indices.push_back(baseIdx + modelIndices[i]);
          //  printf("%zu ", cell.indices.back());
        }
       // printf("\n");
       // exit(0);
    }

#if 0
    printf("indices:\n");
    for (int i = 0; i < 36; ++i)
        printf("%zu ", cell.indices[i]);
    printf("\n");
    exit(0);
#endif
}


//---------------------------------------------------------
// go for each grass instance and use its position to define
// min and max height of grass field's bounding box 
//---------------------------------------------------------
void ÑalcFieldYBoundings(GrassField& field)
{
    float minY = FLT_MAX;
    float maxY = FLT_MIN;

 
    for (int i = 0; i < field.grassCount; ++i)
    {
        minY = 0;
        maxY = 200;
    }

    assert(minY >= 0);

    field.worldBox.y0 = minY;
    field.worldBox.y1 = maxY;
}



//---------------------------------------------------------
// initialize VB/IB for each cell of input grass field
//---------------------------------------------------------
void InitBuffers(GrassField& field)
{
    for (GrassCell& cell : field.cells)
    {
        cell.vb.Init(cell.vertices.data(), cell.vertices.size(), false);
        cell.ib.Init(cell.indices.data(), cell.indices.size(), false);
    }
}

//---------------------------------------------------------
// Desc:   make frustum test for each grass patch
//         if it is visible we will render it later
//---------------------------------------------------------
void GrassMgr::Update(const CameraParams* pCamParams, Frustum* pWorldFrustum)
{

#if 0
    sizeof(GrassField);

    assert(pCamParams);
    assert(pWorldFrustum);

    vsize numVisPatches = 0;
    const CameraParams& cam = *pCamParams;

    //const float sqrGrassRange = SQR(grassRange_);
    const float grassRange = grassRange_;
    const Vec3 camPos = { cam.posX, cam.posY, cam.posZ };


    for (uint32 idx = 0; idx < (uint32)patches_.size(); ++idx)
    {
        // check if the patch is in the grass visibility range
        Sphere& sph = boundSpheres_[idx];
        const float distToPatchCenter = Vec3Length(camPos - sph.center) - sph.radius;

        if (distToPatchCenter > grassRange)
            continue;

        // check if patch is in view frustum
        if (!pWorldFrustum->TestSphere(boundSpheres_[idx]))
            continue;

        visPatchesIdxs_[numVisPatches] = idx;
        ++numVisPatches;
    }

    visPatchesIdxs_.resize(numVisPatches);

#endif
}
#if 0
//---------------------------------------------------------
// Desc:   add a bunch of new grass vertices (central point of each grass instance),
//         separate them by its related grass patches (sectors),
//         and update vertex buffers for these patches
//---------------------------------------------------------
void GrassMgr::AddGrassVertices(const cvector<VertexGrass>& newVertices)
{
    if (newVertices.size() == 0)
        return;

    const float invPatchSize    = 1.0f / patchSize_;
    const int numPatchesPerSide = numPatchesPerSide_;

    std::set<int> updatedPatchIdxs;

    for (const VertexGrass& v : newVertices)
    {
        // compute patch index
        const int px  = (int)(v.pos.x * invPatchSize);
        const int pz  = (int)(v.pos.z * invPatchSize);
        const int idx = (pz * numPatchesPerSide) + px;

        patches_[idx].vertices.push_back(v);
        updatedPatchIdxs.insert(idx);
    }

    // use only necessary memory
    for (int i : updatedPatchIdxs)
        patches_[i].vertices.shrink_to_fit();

    for (int i : updatedPatchIdxs)
        UpdateBoundingsForPatch(i);

    for (int i : updatedPatchIdxs)
        RebuildVB(i);
}

//---------------------------------------------------------
// Desc:   add a single grass vertex (central point of the instance)
//         put it into related grass patch (sector) and update vertex buffer
//---------------------------------------------------------
void GrassMgr::AddGrassVertex(const VertexGrass& vertex)
{
    assert(0 && "TODO ?");
}

//---------------------------------------------------------
// Desc:   setup grass visibility distance
//         (when grass patch is farther we don't render it
//          even if it is in the view frustum)
//---------------------------------------------------------
void GrassMgr::SetGrassVisibilityDist(const float range)
{
    if (range < 0.0f)
    {
        LogErr(LOG, "can't setup a grass visibility distance: input values is < 0");
        return;
    }

    grassRange_ = range;
}

//---------------------------------------------------------
// Desc:   go through each patch and check if we need to rebuild relative
//         vertex buffer (for instance we do it when initialize the scene or
//         when add/remove grass in the editor)
// Args:   - idx:   an index of patch which we will updated
//---------------------------------------------------------
void GrassMgr::RebuildVB(const int idx)
{
    constexpr bool isDynamic = true;

    const size numVertices        = patches_[idx].vertices.size();
    VertexGrass* vertices         = patches_[idx].vertices.data();
    VertexBuffer<VertexGrass>& vb = patches_[idx].vb;

    vb.Shutdown();

    if (!vb.Init(Render::g_pDevice, vertices, (int)numVertices, isDynamic))
    {
        LogErr(LOG, "can't rebuild a grass vertex buffer: %d", idx);
        exit(0);
    }
}
//---------------------------------------------------------
// Desc:   return a grass patch (sector) by input index
//---------------------------------------------------------
const GrassPatch& GrassMgr::GetPatchByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx];
}

//---------------------------------------------------------
// Desc:   return a vertex buffer by input index of grass patch
//---------------------------------------------------------
VertexBuffer<VertexGrass>& GrassMgr::GetVertexBufByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx].vb;
}

//---------------------------------------------------------
// Desc:   return an array of vertices by input index of grass patch
//---------------------------------------------------------
cvector<VertexGrass>& GrassMgr::GetVerticesByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx].vertices;
}

//---------------------------------------------------------
// Desc:   recompute bounding shapes (box + sphere) for grass patch by index
//---------------------------------------------------------
void GrassMgr::UpdateBoundingsForPatch(const int i)
{
    assert(i < (int)patches_.size());

    Vec3 minPoint = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    Vec3 maxPoint = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    // define min and max point for this patch
    for (const VertexGrass& v : patches_[i].vertices)
    {
        minPoint.x = min(v.pos.x, minPoint.x);
        minPoint.y = min(v.pos.y, minPoint.y);
        minPoint.z = min(v.pos.z, minPoint.z);

        maxPoint.x = max(v.pos.x, maxPoint.x);
        maxPoint.y = max(v.pos.y, maxPoint.y);
        maxPoint.z = max(v.pos.z, maxPoint.z);
    }

    // setup bounding box and bounding sphere
    const Vec3 center       = (maxPoint + minPoint) * 0.5f;
    boundSpheres_[i].radius = Vec3Length(maxPoint - center);
    boundSpheres_[i].center = center;

    aabbs_[i] = {
        minPoint.x, maxPoint.x,
        minPoint.y, maxPoint.y,
        minPoint.z, maxPoint.z };
}
#endif

} // namespace
