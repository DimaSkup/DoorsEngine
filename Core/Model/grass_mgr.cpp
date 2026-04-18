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

struct GrassTimeStats
{
    TimeDurationMs fullTime;
    TimeDurationMs timeCellsGen;

    TimeDurationMs timeCellsPrepare;
    TimeDurationMs timeGenInstances;
    TimeDurationMs timeGenPositions;
    TimeDurationMs timeGroupByCells;
};

GrassTimeStats s_TimeStats;

//---------------------------------------------------------
// GLOBAL instance of the grass manager
//---------------------------------------------------------
GrassMgr g_GrassMgr;


//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void CheckInitParams        (const GrassFieldInitParams& params);
void CalcFieldXZBoundings   (GrassField& field, const GrassFieldInitParams& params);
void CreateCells            (GrassField& field, const GrassFieldInitParams& params);
void GenGrassRandPositions  (GrassField& field, cvector<GrassInstance>& outGrass);
void GenGrassRandPositions2 (GrassField& field, cvector<GrassInstance>& outGrass);
void CalcFieldYBoundings    (GrassField& field);
void InitBuffers            (GrassField& field);


//---------------------------------------------------------
// create a new grass field based on input params
//---------------------------------------------------------
bool GrassMgr::AddGrassField(const GrassFieldInitParams& params)
{
    TimePoint start = GetTimePoint();

    CheckInitParams(params);

    // create a new grass field
    grassFields_.push_back(GrassField());
    GrassField& field = grassFields_.back();
    memset(&field, 0, sizeof(field));

    // setup a name and path to density mask
    strcpy(field.name, params.name);
    strcpy(field.densityMaskRGB, params.densityMaskRGB);
    strcpy(field.densityMaskAlpha, params.densityMaskAlpha);

    // number of texture slots (number of horizontal frames on atlas) defines
    // the number of grass channels for this field
    field.numChannels = params.texSlots;

    field.grassCount = params.grassCount;
    field.cellsByX   = params.cellsByX;
    field.cellsByZ   = params.cellsByZ;
    field.texSlots   = params.texSlots;
    field.texRows    = params.texRows;

    // setup chance of grass appearance per channel
    for (int i = 0; i < field.numChannels; ++i)
        field.channelProbability[i] = params.channelProbability[i];

    // setup min/max scale
    for (int i = 0; i < field.numChannels; ++i)
        field.channelGrassScaleMin[i] = params.channelGrassScaleMin[i];

    for (int i = 0; i < field.numChannels; ++i)
        field.channelGrassScaleMax[i] = params.channelGrassScaleMax[i];

    // get material for the whole field
    field.matId = g_MaterialMgr.GetMatIdByName(params.materialName);
    if (field.matId == INVALID_MAT_ID)
    {
        LogErr(LOG, "no material (%s) to use for grass field (%s)", params.materialName, field.name);
    }

    CalcFieldXZBoundings(field, params);

    const TimePoint startCellsGen = GetTimePoint();
    CreateCells(field, params);
    s_TimeStats.timeCellsGen = GetTimePoint() - startCellsGen;

    CalcFieldYBoundings(field);

    InitBuffers(field);

    s_TimeStats.fullTime = GetTimePoint() - start;

    // print time statistic for this field
    SetConsoleColor(MAGENTA);
    printf("\n\n");
    LogMsg("grass field (%s) generation took: %.2f sec", params.name, s_TimeStats.fullTime.count() / 1000.0f);
    LogMsg("cells generation took:            %.2f sec", s_TimeStats.timeCellsGen.count() / 1000.0f);

    LogMsg("cells preparation took:           %.2f sec", s_TimeStats.timeCellsPrepare.count() / 1000.0f);
    LogMsg("cells gen instances took:         %.2f sec", s_TimeStats.timeGenInstances.count() / 1000.0f);
    LogMsg("cells gen positions took:         %.2f sec", s_TimeStats.timeGenPositions.count() / 1000.0f);
    LogMsg("cells group by cells took:        %.2f sec", s_TimeStats.timeGroupByCells.count() / 1000.0f);
    printf("\n\n");
    SetConsoleColor(RESET);

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
    const int numChannels = params.texSlots;

    assert(!StrHelper::IsEmpty(params.name));
    assert(!StrHelper::IsEmpty(params.materialName));

    assert(!StrHelper::IsEmpty(params.densityMaskRGB));
    assert(!StrHelper::IsEmpty(params.densityMaskAlpha));

    assert(!StrHelper::IsEmpty(params.modelNames[0]));
    assert(!StrHelper::IsEmpty(params.modelNames[1]));
    assert(!StrHelper::IsEmpty(params.modelNames[2]));
    assert(!StrHelper::IsEmpty(params.modelNames[3]));

  
    assert(strlen(params.name) < MAX_LEN_MODEL_NAME);
    assert(sizeof(params.densityMaskRGB)   == sizeof(GrassField::densityMaskRGB));
    assert(sizeof(params.densityMaskAlpha) == sizeof(GrassField::densityMaskAlpha));

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

    for (int i = 0; i < numChannels; ++i)
    {
        assert(params.channelGrassScaleMin[i] > 0);
        assert(params.channelGrassScaleMin[i] < params.channelGrassScaleMax[i]);
    }

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
//        we will generate this model
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
// Desc:  check if our density maps are valid
//---------------------------------------------------------
bool CheckDensityMap(const Image& map, const GrassField& field)
{
    // check channels number
    const uint numChannelsInMap = map.GetBPP() >> 3;

    if (numChannelsInMap < field.numChannels)
    {
        LogErr(LOG, "number of channels in density map < number of channels in grass field (%u < %u)", numChannelsInMap, field.numChannels);
        return false;
    }


    // check if we have at least one non-zero value for each channel
    const uint w = map.GetWidth();
    const uint h = map.GetHeight();
    const uint8* pixels = map.GetPixels();

  
    if (numChannelsInMap == 4)
    {
        bool hasR = false;
        bool hasG = false;
        bool hasB = false;
        bool hasA = false;

        for (uint x = 0; x < w; ++x)
        {
            for (uint y = 0; y < h; ++y)
            {
                uint8 r, g, b, a;
                map.GetPixelColor(x, y, r, g, b, a);

                hasR |= (r > 0);
                hasG |= (g > 0);
                hasB |= (b > 0);
                hasA |= (a > 0);
            }
        }

        if (field.numInstPerChannel[0] > 0 && !hasR)
        {
            LogErr(LOG, "you have grass instances (%u) for channel R, but there is no place in density map to put it!", field.numInstPerChannel[0]);
            return false;
        }

        if (field.numInstPerChannel[1] > 0 && !hasG)
        {
            LogErr(LOG, "you have grass instances (%u) for channel G, but there is no place in density map to put it!", field.numInstPerChannel[1]);
            return false;
        }

        if (field.numInstPerChannel[2] > 0 && !hasB)
        {
            LogErr(LOG, "you have grass instances (%u) for channel B, but there is no place in density map to put it!", field.numInstPerChannel[2]);
            return false;
        }

        if (field.numInstPerChannel[3] > 0 && !hasA)
        {
            LogErr(LOG, "you have grass instances (%u) for channel A, but there is no place in density map to put it!", field.numInstPerChannel[3]);
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void GenerateGrassInstances(GrassField& field, cvector<GrassInstance>& outGrass)
{
    //
    // calc random position for each instance
    // (currently instances are grouped by channels)
    //
    GenGrassRandPositions(field, outGrass);

    //
    // setup texture coords for each instance of each cell according to channel
    //
    int grassIdx = 0;

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        bool bGeneratedModel = field.bGeneratedModel[ch];

        // if a model for this channel is generated we need to setup
        // for each instance its row and column on texture atlas
        if (bGeneratedModel)
        {
            // setup each instance related to current channel
            for (uint32 i = 0; i < field.numInstPerChannel[ch]; ++i)
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
            for (uint32 i = 0; i < field.numInstPerChannel[ch]; ++i)
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

  
#endif

    //
    // setup random scale
    //
    grassIdx = 0;

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        const float minS = field.channelGrassScaleMin[ch];
        const float maxS = field.channelGrassScaleMax[ch];

        // setup each instance related to current channel
        for (uint32 i = 0; i < field.numInstPerChannel[ch]; ++i)
        {
            GrassInstance& grass = outGrass[grassIdx++];
            grass.scale = RandF(minS, maxS);
        }
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void GenGrassRandPositions(GrassField& field, cvector<GrassInstance>& outGrass)
{
    const Terrain& terrain = g_ModelMgr.GetTerrain();
    Image densityMapRGB;
    Image densityMapAlpha;
    Image* densityMaps[NUM_GRASS_CHANNELS]{ nullptr };


    // load density maps for this grass field
    densityMapRGB.LoadRgbBMP(field.densityMaskRGB);

    if (!densityMapRGB.IsLoaded())
    {
        LogErr(LOG, "can't initialize density map for grass field: %s", field.name);
        return;
    }

    // load a density map for field's alpha channel if need
    if (field.numChannels == 4)
    {
        densityMapAlpha.LoadGrayscaleBMP(field.densityMaskAlpha);

        if (!densityMapAlpha.IsLoaded())
        {
            LogErr(LOG, "can't load density map for alpha channel of grass field: %s", field.name);
            return;
        }

        if ((densityMapAlpha.GetWidth()  != densityMapRGB.GetWidth()) ||
            (densityMapAlpha.GetHeight() != densityMapRGB.GetHeight()))
        {
            LogErr(LOG, "density map both for RGB and alpha channels don't have the same dimensions for grass field: %s", field.name);
            return;
        }
    }

#if 0
    if (!CheckDensityMap(densityMap, field))
    {
        LogErr(LOG, "can't generate grass instances");
        return;
    }
#endif

    // density map per channel
    densityMaps[0] = &densityMapRGB;
    densityMaps[1] = &densityMapRGB;
    densityMaps[2] = &densityMapRGB;
    densityMaps[3] = &densityMapAlpha;

    const float worldBoxInvDX = 1.0f / (field.worldBox.x1 - field.worldBox.x0);
    const float worldBoxInvDZ = 1.0f / (field.worldBox.z1 - field.worldBox.z0);

    uint32 grassIdx = 0;
    outGrass.resize(field.grassCount);

    TimePoint start = GetTimePoint();

    int numAllAttempts = 0;
    int numWasted = 0;

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        // each channel has its own density map
        const Image& densityMap = *densityMaps[ch];

        const uint densityMapWidth  = densityMap.GetWidth();
        const uint densityMapHeight = densityMap.GetHeight();

        for (uint i = 0; i < field.numInstPerChannel[ch]; ++i)
        {
            Vec3 pos;
            uint8 density = 0;

            // if we need to regenerate position for this instance
            while (density < RandUint(1, 255))
            {
                // random horizontal position
                pos.x = RandF(field.worldBox.x0 + 0.1f, field.worldBox.x1 - 0.1f);
                pos.z = RandF(field.worldBox.z0 + 0.1f, field.worldBox.z1 - 0.1f);

                // calc pixel coord for density map
                const float x = (pos.x - field.worldBox.x0) * worldBoxInvDX;
                const float y = 1.0f - (pos.z - field.worldBox.z0) * worldBoxInvDZ;  // flip vertically texture coord

                const uint px = (uint)(x * densityMapWidth);
                const uint py = (uint)(y * densityMapHeight);

                // get density value according to the current grass field's channel
                if (ch == 0)        density = densityMap.GetPixelRed(px, py);
                else if (ch == 1)   density = densityMap.GetPixelGreen(px, py);
                else if (ch == 2)   density = densityMap.GetPixelBlue(px, py);
                else if (ch == 3)   density = densityMap.GetPixelGray(px, py);

                numAllAttempts++;
                numWasted++;
            }

            numWasted--;

            // get instance height according to terrain
            pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z);

            assert(pos.x >= 0);
            assert(pos.y >= 0);
            assert(pos.z >= 0);

            outGrass[grassIdx++].pos = pos;
        }
    }

    s_TimeStats.timeGenPositions = GetTimePoint() - start;

    LogMsg(LOG, "grass stats:  %d / %d", numWasted, numAllAttempts);
    LogMsg(LOG, "percent wasted: %d", (int)((float)numWasted / (float)numAllAttempts * 100.0f));
}

struct Rect2d
{
    float minX, maxX;
    float minZ, maxZ;
};

//---------------------------------------------------------
// Desc:  get available positions for channel
//        (for each density value [0-255], 0-low, 255-high density)
// Args:  channel      - index of current grass channel
//        outPositions - available positions per each density value
//        densityMap   - texture with encoded density values
//        fieldMinP    - minimal point for curr field boundaries
//        fieldMaxP    - maximal point for curr field boundaries
//---------------------------------------------------------
void GetAvailPosForChannel(
    const int channel,
    cvector<Rect2d>* outPositions,
    const Image& densityMap,
    const Vec3& fieldMinP,
    const Vec3& fieldMaxP)
{
    assert(channel >= 0 && channel <= 3);
    assert(outPositions);
    assert(densityMap.IsLoaded());

    const float fieldSizeX = fieldMaxP.x - fieldMinP.x;
    const float fieldSizeZ = fieldMaxP.z - fieldMinP.z;

    const uint8* pixels      = densityMap.GetPixels();
    const uint   width       = densityMap.GetWidth();
    const uint   height      = densityMap.GetHeight();
    //const uint   numPixels   = width * height;
    const uint   pixelStride = densityMap.GetBPP() / 8;

    // calc how many space in world is covered by a single pixel from density map
    const float densityMapCellWidth  = fieldSizeX / (float)width;
    const float densityMapCellHeight = fieldSizeZ / (float)height;

    for (uint y = 0; y < height; ++y)
    {
        for (uint x = 0; x < width; ++x)
        {
            uint8 density = pixels[(y * width + x) * pixelStride + channel];

            // no intensity for this channel on density map - no grass here
            if (density == 0)
                continue;

            // calc "world position" for this density pixel
            Rect2d densityCellPos;
            densityCellPos.minX = densityMapCellWidth * (float)x + fieldMinP.x;
            densityCellPos.maxX = densityCellPos.minX + densityMapCellWidth;
            densityCellPos.minZ = densityMapCellHeight * (float)y + fieldMinP.z;
            densityCellPos.maxZ = densityCellPos.minZ + densityMapCellHeight;

            assert(densityCellPos.minX >= fieldMinP.x);
            assert(densityCellPos.maxX <= fieldMaxP.x);
            assert(densityCellPos.minZ >= fieldMinP.z);
            assert(densityCellPos.maxZ <= fieldMaxP.z);

            outPositions[density].push_back(densityCellPos);
        }
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void GenGrassRandPositions2(GrassField& field, cvector<GrassInstance>& outGrass)
{
    const Terrain& terrain = g_ModelMgr.GetTerrain();
    const float terrainLen = (float)terrain.GetTerrainLength();

    Image densityMapRGB;
    Image densityMapAlpha;
    Image* densityMaps[NUM_GRASS_CHANNELS]{ nullptr };

    // density map per channel
    densityMaps[0] = &densityMapRGB;
    densityMaps[1] = &densityMapRGB;
    densityMaps[2] = &densityMapRGB;
    densityMaps[3] = &densityMapAlpha;

    //
    // load density maps for this grass field
    //
    densityMapRGB.LoadRgbBMP(field.densityMaskRGB);

    if (!densityMapRGB.IsLoaded())
    {
        LogErr(LOG, "can't initialize density map (RGB) for grass field: %s", field.name);
        return;
    }

    // load a density map for field's alpha channel if need
    if (field.numChannels == 4)
    {
        densityMapAlpha.LoadGrayscaleBMP(field.densityMaskAlpha);

        if (!densityMapAlpha.IsLoaded())
        {
            LogErr(LOG, "can't load density map (alpha) for grass field: %s", field.name);
            return;
        }

        if ((densityMapAlpha.GetWidth()  != densityMapRGB.GetWidth()) ||
            (densityMapAlpha.GetHeight() != densityMapRGB.GetHeight()))
        {
            LogErr(LOG, "density maps (both RGB and alpha) must have the same dimensions for grass field: %s", field.name);
            return;
        }
    }

    uint grassIdx = 0;
    TimePoint start = GetTimePoint();
   
    constexpr int NUM_DENSITIES = 256;
    cvector<Rect2d> densityMapCells[NUM_DENSITIES];

    const Vec3 fieldMinP = field.worldBox.MinPoint();
    const Vec3 fieldMaxP = field.worldBox.MaxPoint();

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        // get available positions for channel
        // (for each density value [0-255], 0-low, 255-high density)
        GetAvailPosForChannel(ch, densityMapCells, *densityMaps[ch], fieldMinP, fieldMaxP);

        // generate pos for each grass instance of this channel
        for (uint i = 0; i < field.numInstPerChannel[ch]; ++i)
        {
            const uint8            density        = RandUint(1, 255);
            const cvector<Rect2d>& densityCells   = densityMapCells[density];
            const uint             densityCellIdx = RandUint(0, (UINT)(densityCells.size()-1));
            const Rect2d&          densityCell    = densityCells[densityCellIdx];

            // gen rand pos within density cell
            Vec3& pos = outGrass[grassIdx++].pos;
            pos.x = RandF(densityCell.minX, densityCell.maxX);
            pos.z = RandF(densityCell.minZ, densityCell.maxZ);
            pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z);

            assert(pos.x >= fieldMinP.x);
            assert(pos.y >= fieldMinP.y);
            assert(pos.z >= fieldMinP.z);

            assert(pos.x <= fieldMaxP.x);
            assert(pos.z <= fieldMaxP.z);
        }

        // clear before handling the next channel
        for (int i = 0; i < NUM_DENSITIES; ++i)
            densityMapCells[i].clear();
    }

    s_TimeStats.timeGenPositions = GetTimePoint() - start;
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

#if 0
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

#else
    const uint32 grassStartIdxs[4] = {
        0,
        grassStartIdxs[0] + field.numInstPerChannel[0],
        grassStartIdxs[1] + field.numInstPerChannel[1],
        grassStartIdxs[2] + field.numInstPerChannel[2],
    };

    const Vec3 fieldMidP = field.worldBox.MidPoint();

    struct CellsGroup
    {
        index cellsIdxs[64];
        size count;
        float minX, maxX;
        float minZ, maxZ;
    };

    constexpr int numGroupsByX = 8;
    constexpr int numGroupsByZ = 8;
    constexpr int numGroups = numGroupsByX * numGroupsByZ;
    assert(IS_POW2(numGroups));

    CellsGroup groups[numGroups];
    memset(&groups, 0, sizeof(groups));

    float groupSizeX = field.worldBox.SizeX() / (float)numGroupsByX;
    float groupSizeZ = field.worldBox.SizeZ() / (float)numGroupsByZ;

    // calc boundings for each group
    for (int row = 0; row < numGroupsByZ; ++row)
    {
        for (int col = 0; col < numGroupsByX; ++col)
        {
            int groupIdx = (row * numGroupsByX) + col;

            groups[groupIdx].minX = groupSizeX * col;
            groups[groupIdx].maxX = groups[groupIdx].minX + groupSizeX;

            groups[groupIdx].minZ = groupSizeZ * row;
            groups[groupIdx].maxZ = groups[groupIdx].minZ + groupSizeZ;
        }
    }

    // group cells by its positions
    for (index cellIdx = 0; cellIdx < numCells; ++cellIdx)
    {
        const Rect3d& cellBox = field.cellsWorldBoxes[cellIdx];

        for (int groupIdx = 0; groupIdx < numGroups; ++groupIdx)
        {
            if (cellBox.x0 >= groups[groupIdx].minX &&
                cellBox.x1 <= groups[groupIdx].maxX &&
                cellBox.z0 >= groups[groupIdx].minZ &&
                cellBox.z1 <= groups[groupIdx].maxZ)
            {
                index i = groups[groupIdx].count++;
                groups[groupIdx].cellsIdxs[i] = cellIdx;
                groupIdx = numGroups;
            }
        }
    }

    const size numCellsPerGroup = field.cells.size() / numGroups;
    for (int groupIdx = 0; groupIdx < numGroups; ++groupIdx)
    {
        assert(groups[groupIdx].count == numCellsPerGroup);
    }


    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        const uint32 idxOffset = grassStartIdxs[ch];

        // for each grass related to the current channel
        for (uint32 i = 0; i < field.numInstPerChannel[ch]; ++i)
        {
            const GrassInstance& grassInst = grassInstances[i + idxOffset];
            const Vec3& p = grassInst.pos;
            index groupIdx = -1;

            // define cells group
            for (int idx = 0; idx < numGroups; ++idx)
            {
                if (p.x >= groups[idx].minX &&
                    p.x <= groups[idx].maxX &&
                    p.z >= groups[idx].minZ &&
                    p.z <= groups[idx].maxZ)
                {
                    groupIdx = idx;
                    idx = numGroups;
                }
            }

            assert(groupIdx != -1);

            size   numCells  = groups[groupIdx].count;
            index* cellsIdxs = groups[groupIdx].cellsIdxs;

            for (index c = 0; c < numCells; ++c)
            {
                const index cellIdx = cellsIdxs[c];
                const Rect3d& box = field.cellsWorldBoxes[cellIdx];

                // if grass is inside this cell's boundaries
                if ((p.x >= box.x0) && (p.x <= box.x1) &&
                    (p.z >= box.z0) && (p.z <= box.z1))
                {
                    // bind instance to cell
                    field.cells[cellIdx].grassInstances.push_back(grassInst);
                    field.cells[cellIdx].channelInstanceCount[ch]++;
                    c = numCells;
                }
            }
        }
    }
   
#endif

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
    const Terrain& terrain  = g_ModelMgr.GetTerrain();
    const int      numCells = field.cellsByX * field.cellsByZ;

    cvector<GrassInstance> grassItems(field.grassCount);


    // alloc memory for cells and its boundings
    field.cells.resize(numCells);
    field.cellsWorldBoxes.resize(numCells);

    // fill memory with zeros
    field.cells.fill_zeros();
    field.cellsWorldBoxes.fill_zeros();

    // generate grass models if necessary
    TimePoint start1 = GetTimePoint();
    GenGrassModels(field, params);

    SetupModelsForGrassChannels(field, params);
    CalcGrassCellsBoundings   (field);
    CalcNumInstancesPerChannel(field);
    s_TimeStats.timeCellsPrepare = GetTimePoint() - start1;


    TimePoint start2 = GetTimePoint();
    GenerateGrassInstances(field, grassItems);
    s_TimeStats.timeGenInstances = GetTimePoint() - start2;
    printf("here\n");

    TimePoint start3 = GetTimePoint();
    GroupGrassInstancesByCells(field, grassItems);
    s_TimeStats.timeGroupByCells = GetTimePoint() - start3;
    printf("here2\n");
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


    // calc Y-bounding of the whole field (is based on lowest/highest cell)
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
// Desc:   calculate visible grass fields and its cells
//---------------------------------------------------------
void GrassMgr::CalcVisibleGrass(const Vec3 camPos, const Frustum* pWorldFrustum)
{
    assert(grassVisRange_ > 0);

    const float grassVisRange = grassVisRange_;
    visFields_.clear();


    // gather visible grass fields
    for (index i = 0; i < grassFields_.size(); ++i)
    {
        if (!pWorldFrustum->TestRect(grassFields_[i].worldBox))
            continue;

        visFields_.push_back(VisibleGrassField());
        visFields_.back().fieldIdx = i;
    }


    // go through each visible grass field...
    for (index i = 0; i < visFields_.size(); ++i)
    {
        VisibleGrassField& visField = visFields_[i];
        const GrassField&  field    = grassFields_[visField.fieldIdx];

        // ... and gather visible cells
        for (index cellIdx = 0; cellIdx < field.cells.size(); ++cellIdx)
        {
            // check if this cell is in the visibility range
            const Rect3d& box = field.cellsWorldBoxes[cellIdx];

            const Vec3 toEye = camPos - box.MidPoint();
            const Vec3 ext   = box.Extents();

            const float distToCam = FastDistance3D(toEye.x, toEye.y, toEye.z);
            const float boxRadius = FastDistance3D(ext.x, ext.y, ext.z);

            const float distToCenter = distToCam - boxRadius;

            // if cell is out of visibility range we don't render it
            if (distToCenter > grassVisRange)
                continue;

            // check if patch is in view frustum
            if (!pWorldFrustum->TestRect(box))
                continue;

            // this cell is visible
            visField.cellsIdxs.push_back(cellIdx);
        }
    }
}

//---------------------------------------------------------
// Desc:  update instanced buffer per visible grass field
//---------------------------------------------------------
void GrassMgr::UpdateGrassInstancedBuf()
{
    ID3D11DeviceContext* pCtx = Render::GetD3dContext();
    D3D11_MAPPED_SUBRESOURCE mappedData;


    for (const VisibleGrassField& visField : visFields_)
    {
        GrassField& field = grassFields_[visField.fieldIdx];

        ID3D11Buffer* pBuf = field.pInstancedBuf;

        //
        // map the instanced buffer to wrote into it
        //
        HRESULT hr = pCtx->Map(pBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        if (FAILED(hr))
        {
            Render::CRender::PrintHRESULT(hr);
            LogErr(LOG, "can't map the instance buffer for grass field: %s", field.name);
            return;
        }

        GrassInstance* data = (GrassInstance*)mappedData.pData;

        //
        // write instances data into buffer
        //
        for (uint32 i = 0, ch = 0; ch < (uint32)field.numChannels; ++ch)
        {
            // go through each visible cell
            for (const index cellIdx : visField.cellsIdxs)
            {
                const GrassCell& cell = field.cells[cellIdx];
                const uint32  baseIdx = cell.channelStart[ch];
                const uint32  numInst = cell.channelInstanceCount[ch];

                // push data related only to the current channel
                for (uint32 instanceIdx = 0; instanceIdx < numInst; ++instanceIdx)
                {
                    assert(i < field.grassCount);
                    data[i++] = cell.grassInstances[baseIdx + instanceIdx];

                    // increase a number of instances for this channel to render
                    field.instancesBufCounts[ch]++;
                }
            }
        }

        //
        // unmap the buffer
        //
        pCtx->Unmap(field.pInstancedBuf, 0);
    }
}

//---------------------------------------------------------
// Desc:  setup radius around the camera where grass has its full size;
//        after this radius grass is getting smaller;
//---------------------------------------------------------
void GrassMgr::SetGrassDistFullSize(const float dist)
{
    assert(dist < grassVisRange_);
    grassDistFullSize_ = dist;
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
