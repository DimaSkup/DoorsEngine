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
#include <Render/CRender.h>
#include <geometry/frustum.h>

#include <Timers/game_timer.h>

#include <Mesh/material_mgr.h>
#include <Model/model_mgr.h>
#include <Model/model_creator.h>



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
void CalcFieldYBoundings(GrassField& field);

void InitBuffers(GrassField& field);


void PrintDumpCellInstances(const GrassCell& cell, const index cellIdx)
{
    SetConsoleColor(CYAN);
    printf("Dump instances of cell_%tu:\n", cellIdx);

    for (index i = 0; i < cell.grassInstances.size(); ++i)
    {
        const Vec3& p = cell.grassInstances[i].pos;
        printf("instance[%tu]:   pos(%.2f %.2f %.2f)\n", i, p.x, p.y, p.z);
    }
    printf("\n");
    SetConsoleColor(RESET);
}


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

    field.channelProbability[0] = params.channelProbability[0];
    field.channelProbability[1] = params.channelProbability[1];
    field.channelProbability[2] = params.channelProbability[2];
    field.channelProbability[3] = params.channelProbability[3];


    field.numChannels = field.texSlots;

    // get material for the whole field
    field.matId = g_MaterialMgr.GetMatIdByName(params.materialName);
    if (field.matId == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "no material (%s) to use for grass field (%s)", params.materialName, field.name);
    }

    CalcFieldXZBoundings(field, params);

    CreateCells(field, params);

    CalcFieldYBoundings(field);

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

    assert(!StrHelper::IsEmpty(params.modelNames[0]));
    assert(!StrHelper::IsEmpty(params.modelNames[1]));
    assert(!StrHelper::IsEmpty(params.modelNames[2]));
    assert(!StrHelper::IsEmpty(params.modelNames[3]));

  
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

    assert(params.texSlots > 0 && params.texSlots <= NUM_GRASS_CHANNELS);
    assert(params.texRows  > 0 && params.texSlots <= NUM_GRASS_CHANNELS);

    assert(params.grassCount > 0);

    assert(params.grassMinHeight > 0);
    assert(params.grassMaxHeight > params.grassMinHeight);

    assert(params.channelProbability[0] >= 0.0f);
    assert(params.channelProbability[1] >= 0.0f);
    assert(params.channelProbability[2] >= 0.0f);
    assert(params.channelProbability[3] >= 0.0f);
}


//---------------------------------------------------------
// Desc:  compute world axis-aligned bounding box of the grass field
//---------------------------------------------------------
void CalcFieldXZBoundings(GrassField& field, const GrassFieldInitParams& params)
{
    // compute field's boundings in world
    float minX = (float)(params.centerX - params.sizeX / 2);
    float minZ = (float)(params.centerZ - params.sizeZ / 2);
    float maxX = (float)(params.centerX + params.sizeX / 2);
    float maxZ = (float)(params.centerZ + params.sizeZ / 2);

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
// Desc:  if any channel of grass field requires generated grass model
//        we will generate it
//---------------------------------------------------------
void GenGrassModels(const GrassField& field, const GrassFieldInitParams& params)
{
    bool bNeedGenGrassModel = false;

    // check if we need to generate a grass model at all
    for (int i = 0; i < field.numChannels; ++i)
    {
        bNeedGenGrassModel |= (strcmp(params.modelNames[i], "generated") == 0);
    }

    if (!bNeedGenGrassModel)
        return;


    bool bHasGenGrassModel = g_ModelMgr.HasModelByName("generated_grass");

    if (bNeedGenGrassModel && !bHasGenGrassModel)
    {
        const float planeW = 1.0f;
        const float planeH = 1.0f;

        ModelsCreator creator;
        creator.CreateGrassModel(planeW, planeH);
    }
}

//---------------------------------------------------------
// Desc:  setup model for each channel of grass field according to input params
//---------------------------------------------------------
void SetupModelsForGrassChannels(GrassField& field, const GrassFieldInitParams& params)
{
    for (int i = 0; i < field.numChannels; ++i)
    {
        // if we want to use generated grass geometry...
        if (strcmp(params.modelNames[i], "generated") == 0)
        {
            field.grassModelId[i]    = g_ModelMgr.GetModelIdByName("generated_grass");
            field.bGeneratedModel[i] = true;
        }

        // use some specific model...
        else
        {
            field.grassModelId[i]    = g_ModelMgr.GetModelIdByName(params.modelNames[i]);
            field.bGeneratedModel[i] = false;
        }
    }
}

//---------------------------------------------------------
// Desc:  compute axis-aligned bounding box of each
//        particular cell of input grass field
//---------------------------------------------------------
void CalcGrassCellsBoundings(GrassField& field)
{
    const int numCells = field.cellsByX * field.cellsByZ;

    const float cellBoxSizeX = field.worldBox.SizeX() / (float)field.cellsByX;
    const float cellBoxSizeZ = field.worldBox.SizeZ() / (float)field.cellsByZ;

    // go through each cell and calc boundings...
    for (int i = 0; i < numCells; ++i)
    {
        int cellRow = (i / field.cellsByZ);
        int cellCol = (i % field.cellsByX);

        const float minX = field.worldBox.x0 + (cellCol * cellBoxSizeX);
        const float maxX = minX + cellBoxSizeX;

        const float minZ = field.worldBox.z0 + (cellRow * cellBoxSizeZ);
        const float maxZ = minZ + cellBoxSizeZ;

        // NOTE: Y-boundaries we recalc later, it will be based on height
        //       of lowest and highest position of grass instances (of this cell)
        field.cellsWorldBoxes[i] = Rect3d(minX, maxX, 0, 200, minZ, maxZ);
    }
}

//---------------------------------------------------------
// calc instances count per each channel
//---------------------------------------------------------
void CalcNumInstancesPerChannel(GrassField& field)
{
    float channelsSumProbability = 0;
  
    for (int ch = 0; ch < field.numChannels; ++ch)
        channelsSumProbability += field.channelProbability[ch];

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        const float factor = field.channelProbability[ch] / channelsSumProbability;
        field.numInstPerChannel[ch] = (uint32)(factor * (float)field.grassCount);
    }

    int sumNumInstances =
        field.numInstPerChannel[0] +
        field.numInstPerChannel[1] +
        field.numInstPerChannel[2] +
        field.numInstPerChannel[3];

    // a bit HACKY: make a little correction to make sure that
    // we have a proper number of instances over all the channels
    int correction = (int)field.grassCount - sumNumInstances;

    sumNumInstances += correction;
    field.numInstPerChannel[0] += correction;

    assert(sumNumInstances == field.grassCount);
}

//---------------------------------------------------------
//---------------------------------------------------------
void GenerateGrassInstances(GrassField& field, cvector<GrassInstance>& outGrass)
{
    const Terrain& terrain = g_ModelMgr.GetTerrain();
    Image densityMap;
    Image* densityMaps[NUM_GRASS_CHANNELS]{ nullptr };


    // load density maps for this field
    densityMap.LoadGrayscaleBMP(field.densityMask);

    if (!densityMap.IsLoaded())
    {
        LogErr(LOG, "can't initialize density map for grass field: %s", field.name);
        return;
    }

    // density map per channel
    densityMaps[0] = &densityMap;
    densityMaps[1] = &densityMap;
    densityMaps[2] = &densityMap;
    densityMaps[3] = &densityMap;

    const uint   densityMapWidth = densityMap.GetWidth();
    const uint   densityMapHeight = densityMap.GetHeight();

    //
    // calc random position for each instance
    // (currently instances are grouped by channels)
    //
    const float worldBoxInvDX = 1.0f / (field.worldBox.x1 - field.worldBox.x0);
    const float worldBoxInvDZ = 1.0f / (field.worldBox.z1 - field.worldBox.z0);

    uint32 grassIdx = 0;
    outGrass.resize(field.grassCount);

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        // each channel has its own density map
        const Image& currDensityMap   = *densityMaps[ch];
        


        for (uint i = 0; i < field.numInstPerChannel[ch]; ++i)
        {
            Vec3 pos;

            uint8 density = 0;
            uint8 r = 0;
            uint8 g = 0;
            uint8 b = 0;

            // if we need to regenerate position for this instance
            while (density < 1)
            {
                // random horizontal position
                pos.x = RandF(field.worldBox.x0, field.worldBox.x1);
                pos.z = RandF(field.worldBox.z0, field.worldBox.z1);

                // calc pixel coord for density map
                const float x = (pos.x - field.worldBox.x0) * worldBoxInvDX;
                const float y = (pos.z - field.worldBox.z0) * worldBoxInvDZ;

                const uint px = (uint)(x * densityMapWidth);
                const uint py = (uint)(y * densityMapHeight);

#if 0
                currDensityMap.GetColor(px, py, r, g, b);


                if (ch == 0)        density = r;
                else if (ch == 1)   density = g;
                else if (ch == 2)   density = b;
                else if (ch == 3)   density = r | g | b;
#endif

                density = densityMap.GetPixelGray(px, py);
            }

            // get instance height according to terrain
            pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z);

            assert(pos.x >= 0);
            assert(pos.y >= 0);
            assert(pos.z >= 0);

            outGrass[grassIdx++].pos = pos;
        }
    }

    //
    // setup texture coords for each instance of each cell according to channel
    //
    grassIdx = 0;

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        bool bGeneratedModel = field.bGeneratedModel[ch];
        uint32 numInst = field.numInstPerChannel[ch];

        // if a model for this channel is generated we need to setup
        // for each instance its row and column on texture atlas
        if (bGeneratedModel)
        {
            // setup each instance related to current channel
            for (uint32 i = 0; i < numInst; ++i)
            {
                GrassInstance& grass = outGrass[grassIdx++];

                grass.texColumn = ch;                  // is the same as channel index
                grass.texRow    = (int)RandUint(0, field.texRows);
            }
        }

        // we use some specific model for this channel...
        else
        {
            // setup each instance related to current channel
            for (uint32 i = 0; i < numInst; ++i)
            {
                GrassInstance& grass = outGrass[grassIdx++];

                // set that we will use own tex coords of the model
                grass.texColumn = -1;
                grass.texRow    = -1;
            }
        }
    }

#if 0
    //
    // setup random rotation (in radians) around Y-axis (vertical)
    //
    for (GrassInstance& grass : outGrass)
        grass.rotY = DEG_TO_RAD(RandUint(0, 360));

    //
    // setup random height
    //
    for (GrassInstance& grass : outGrass)
        grass.height = RandF(field.grassMinHeight, field.grassMaxHeight);
#endif
}

//---------------------------------------------------------
// group grass instances into its cells
//---------------------------------------------------------
void GroupGrassInstancesByCells(GrassField& field, const cvector<GrassInstance>& grassInstances)
{
    const vsize numCells = field.cells.size();

    int channel = 0;
    uint32 idx = 0;

    for (GrassCell& cell : field.cells)
        cell.grassInstances.clear();

   // PrintDumpCellInstances(field.cells[1], 1);


    for (const GrassInstance& grass : grassInstances)
    {
        // if we need to switch to the next channel
        if (idx >= field.numInstPerChannel[channel])
        {
            idx = 0;
            channel++;
            assert(channel < field.numChannels);
        }

        const Vec3& p = grass.pos;

        for (index cellIdx = 0; cellIdx < numCells; ++cellIdx)
        {
            const Rect3d& box = field.cellsWorldBoxes[cellIdx];

            const bool bInBox = (p.x >= box.x0) && (p.x <= box.x1) &&
                                (p.z >= box.z0) && (p.z <= box.z1);

            if (!bInBox)
                continue;

           
            // bind instance to cell
            field.cells[cellIdx].grassInstances.push_back(grass);
            field.cells[cellIdx].channelInstanceCount[channel]++;

            cellIdx = numCells;
        }

        idx++;
    }

    // setup instances start index of each channel
    for (GrassCell& cell : field.cells)
        cell.channelStart[0] = 0;

    for (int ch = 1; ch < field.numChannels; ++ch)
    {
        for (GrassCell& cell : field.cells)
        {
            uint32 prevStart = cell.channelStart[ch - 1];
            uint32 prevCount = cell.channelInstanceCount[ch - 1];

            cell.channelStart[ch] = prevStart + prevCount;
        }
    }

    //
    // check yourself
    //
    for (const GrassCell& cell : field.cells)
    {
        uint32 start = 0;

        // >= 0 because cells aren't required to have any grass instances at all
        assert(cell.grassInstances.size() >= 0);

        // the first channel is required
        assert(cell.channelStart[0] == 0);
        start += cell.channelInstanceCount[0];

        // check the rest of channels
        for (int ch = 1; ch < field.numChannels; ++ch)
        {
            assert(cell.channelStart[ch] == start);
            start += cell.channelInstanceCount[ch];
        }
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void CreateCells(GrassField& field, const GrassFieldInitParams& params)
{
    const TimePoint start    = GameTimer::GetTimePoint();
    const Terrain&  terrain  = g_ModelMgr.GetTerrain();
    const int       numCells = field.cellsByX * field.cellsByZ;

    cvector<GrassInstance> grassItems(field.grassCount);


    // alloc memory for cells and its boundings
    field.cells.resize(numCells);
    field.cellsWorldBoxes.resize(numCells);

    // fill memory with zeros
    field.cells.fill_zeros();
    field.cellsWorldBoxes.fill_zeros();

    // generate grass models if necessary
    GenGrassModels(field, params);

    SetupModelsForGrassChannels(field, params);

    CalcGrassCellsBoundings   (field);
    CalcNumInstancesPerChannel(field);

    GenerateGrassInstances    (field, grassItems);
    GroupGrassInstancesByCells(field, grassItems);


    // print log about duration of cells generation
    TimePoint end = GameTimer::GetTimePoint();
    TimeDurationMs dur = end - start;
    SetConsoleColor(MAGENTA);
    LogMsg("grass cells generation took: %.2f sec", dur.count() / 1000.0f);
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// go for each grass instance and use its position to define
// min and max height of grass field's bounding box 
//---------------------------------------------------------
void CalcFieldYBoundings(GrassField& field)
{
    float minY = FLT_MAX;
    float maxY = FLT_MIN;

    // calc Y-boundings of each cell
    for (index i = 0; i < field.cells.size(); ++i)
    {
        const GrassCell& cell = field.cells[i];

        // min/max height is based on position of the lowest/highest grass instance
        for (const GrassInstance& grass : cell.grassInstances)
        {
            minY = Min(minY, grass.pos.y);
            maxY = Max(maxY, grass.pos.y);
        }

        // set Y-boundings
        if (cell.grassInstances.size() > 0)
        {
            field.cellsWorldBoxes[i].y0 = minY;
            field.cellsWorldBoxes[i].y1 = maxY;
        }
        else
        {
            field.cellsWorldBoxes[i].y0 = 0;
            field.cellsWorldBoxes[i].y1 = 0;
        }

        minY = FLT_MAX;
        maxY = FLT_MIN;
    }


    // calc Y-bounding of the whole field is based on lowest/highest cell
    minY = FLT_MAX;
    maxY = FLT_MIN;

    for (int i = 0; const Rect3d& box : field.cellsWorldBoxes)
    {
        minY = Min(minY, box.y0);
        maxY = Max(maxY, box.y1);
    }

    field.worldBox.y0 = minY;
    field.worldBox.y1 = maxY;


    //
    // check yourself
    //
    assert(field.worldBox.x0 >= 0);
    assert(field.worldBox.y0 >= 0);
    assert(field.worldBox.z0 >= 0);

    assert(field.worldBox.x0 < field.worldBox.x1);
    assert(field.worldBox.y0 < field.worldBox.y1);
    assert(field.worldBox.z0 < field.worldBox.z1);

    // check world box of each cell
    for (const Rect3d& box : field.cellsWorldBoxes)
    {
        assert(box.x0 >= 0);
        assert(box.y0 >= 0);
        assert(box.z0 >= 0);

        assert(box.x0 < box.x1);
        assert(box.y0 <= box.y1);   // NOTE: minY can == maxY when there are no grass instances in the cell
        assert(box.z0 < box.z1);
    }
}

//---------------------------------------------------------
// create instances buffer for the input grass field
//---------------------------------------------------------
void InitBuffers(GrassField& field)
{
    HRESULT hr = S_OK;
    ID3D11Device* pDevice = Render::GetD3dDevice();

    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));

    // setup buffer's description
    desc.Usage               = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth           = (UINT)(sizeof(GrassInstance) * field.grassCount);
    desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags           = 0;
    desc.StructureByteStride = 0;

    hr = pDevice->CreateBuffer(&desc, nullptr, &field.pInstancedBuf);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create an instanced buffer for grass field: %s", field.name);
    }

    LogMsg(LOG, "instances buffer for grass field (%s) is initialized successfully!", field.name);
}

//---------------------------------------------------------
// Args:  - camPos:         position of camera in world
//        - pWorldFrustum:  camera's frustum in world space
//---------------------------------------------------------
void GrassMgr::Update(const Vec3 camPos, const Frustum* pWorldFrustum)
{
    assert(pWorldFrustum);

    // reset some rendering data
    for (GrassField& field : grassFields_)
    {
        field.instancesBufCounts[0] = 0;
        field.instancesBufCounts[1] = 0;
        field.instancesBufCounts[2] = 0;
        field.instancesBufCounts[3] = 0;
    }
   
    CalcVisibleGrass(camPos, pWorldFrustum);

    UpdateGrassInstancedBuf();
}

//---------------------------------------------------------
// Desc:   calculate which grass fields and its grass cells are currently visible
//---------------------------------------------------------
void GrassMgr::CalcVisibleGrass(const Vec3 camPos, const Frustum* pWorldFrustum)
{
    assert(grassVisRange_ > 0);

    const float grassVisRange = grassVisRange_;
    const GrassField& field = grassFields_[0];

    visCells_.clear();


    for (index i = 0; i < field.cells.size(); ++i)
    {
        // check if the patch is in the grass visibility range
        const Rect3d& box = field.cellsWorldBoxes[i];

        const Vec3 toEye = camPos - box.MidPoint();
        const Vec3 ext = box.Extents();

        const float distToCam = FastDistance3D(toEye.x, toEye.y, toEye.z);
        const float boxRadius = FastDistance3D(ext.x, ext.y, ext.z);

        //const float distToCenter = Vec3Length(camPos - box.MidPoint()) - Vec3Length();
        const float distToCenter = distToCam - boxRadius;

        // if cell is out of visibility range we don't render it
        if (distToCenter > grassVisRange)
            continue;

        // check if patch is in view frustum
        if (!pWorldFrustum->TestRect(box))
            continue;

        VisibleGrassCell visData;
        visData.fieldIdx = 0;
        visData.cellIdx  = (uint16)i;

        visCells_.push_back(visData);
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void GrassMgr::UpdateGrassInstancedBuf()
{
    ID3D11DeviceContext* pCtx = Render::GetD3dContext();
    D3D11_MAPPED_SUBRESOURCE mappedData;

    GrassField& field = grassFields_[0];
    ID3D11Buffer* pBuf = field.pInstancedBuf;

    // map the instanced buffer to wrote into it
    HRESULT hr = pCtx->Map(pBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    if (FAILED(hr))
    {
        Render::CRender::PrintHRESULT(hr);
        LogErr(LOG, "can't map the instance buffer for grass field: %s", field.name);
        return;
    }

    GrassInstance* data = (GrassInstance*)mappedData.pData;

    // write data
    for (uint32 i = 0, ch = 0; ch < (uint32)field.numChannels; ++ch)
    {
        // go through each visible cell
        for (const VisibleGrassCell& visCell : visCells_)
        {
            const GrassCell& cell = field.cells[visCell.cellIdx];
            const uint32 baseIdx = cell.channelStart[ch];
            const uint32 numInst = cell.channelInstanceCount[ch];

            // push data related only to the current channel
            for (uint32 instanceIdx = 0; instanceIdx < numInst; ++instanceIdx)
            {
                assert(i < field.grassCount);
                data[i++] = cell.grassInstances[baseIdx + instanceIdx];

                // increate a instances number of this channel to render
                field.instancesBufCounts[ch]++;
            }
        }
    }

    // unmap the buffer
    pCtx->Unmap(field.pInstancedBuf, 0);
}

//---------------------------------------------------------
// Desc:  setup radius around camera where grass is visible;
//        after this radius we don't render any grass
//---------------------------------------------------------
void GrassMgr::SetGrassVisibilityRange(const float range)
{
    assert(range > 0);
    grassVisRange_ = range;
}


} // namespace
