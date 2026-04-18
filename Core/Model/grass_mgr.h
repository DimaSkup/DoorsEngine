/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_mgr.h
    Desc:     grass (sectors) manager
              (I hope it will optimize my grass rendering :)

    Created:  26.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <cvector.h>
#include <Mesh/vertex.h>
#include <Mesh/vertex_buffer.h>
#include <Mesh/index_buffer.h>
#include <geometry/rect3d.h>

#define NUM_GRASS_CHANNELS 4
#define MAX_LEN_DENSITY_MASK_PATH 64

// forward declarations for global scope (pointer use only)
class Frustum;


namespace Core
{

// forward declarations for Core namespace (pointer use only)
class Model;


//---------------------------------------------------------
// params for initialization of a new grass field
//---------------------------------------------------------
struct GrassFieldInitParams
{
    char name[MAX_LEN_MODEL_NAME];            // name of grass field

    // models for grass channels (each channel can have its own model)
    char modelNames[NUM_GRASS_CHANNELS][MAX_LEN_MODEL_NAME]; 

    char materialName[MAX_LEN_MAT_NAME];      // material for the whole grass field

    // density mask for RGB+A channels of grass for the whole grass field
    char densityMaskRGB[MAX_LEN_DENSITY_MASK_PATH];                  
    char densityMaskAlpha[MAX_LEN_DENSITY_MASK_PATH];

    int centerX;                // center of the grass field in world
    int centerZ;
    int sizeX;                  // size of the grass field in world
    int sizeZ;

    int cellsByX;               // how many times we divide the field along X and Z axis
    int cellsByZ;
    int texSlots;               // how many texture cells (columns) we have in a row of texture atlas
    int texRows;                // how many rows we have in a texture atlas

    int numChannels;            // the same as number of texture slots
    int grassCount;             // how many grass instances we have on this field

    float channelProbability[NUM_GRASS_CHANNELS];

    float channelGrassScaleMin[NUM_GRASS_CHANNELS];
    float channelGrassScaleMax[NUM_GRASS_CHANNELS];
};

//---------------------------------------------------------
// data of a single grass instance (is used for instancing)
//---------------------------------------------------------
struct GrassInstance
{
    Vec3 pos;
    float scale;
    int texColumn;
    int texRow;
};

//---------------------------------------------------------
// grass cell is an element of grass field (we usually have multiple cells per field)
//---------------------------------------------------------
struct GrassCell
{
    cvector<GrassInstance> grassInstances;                  // CPU-side grass instances

    // channels metadata (NOTE: not required to use all 4)
    uint32 channelStart[NUM_GRASS_CHANNELS];            // index where instances begins for particular channel
    uint32 channelInstanceCount[NUM_GRASS_CHANNELS];    // how many instances related to particular channel
};

//---------------------------------------------------------
// a signle field of grass
//---------------------------------------------------------
struct GrassField
{
    // name of this grass field
    char name[MAX_LEN_MODEL_NAME];

    // density mask for RGB+A channels of grass for the whole grass field
    char densityMaskRGB[MAX_LEN_DENSITY_MASK_PATH];
    char densityMaskAlpha[MAX_LEN_DENSITY_MASK_PATH];

    MaterialID  matId;
    uint32      grassCount;                 // number of ALL grass instances of this field

    Rect3d      worldBox;                   // field position and size in 3d space

  
    cvector<GrassCell> cells;               // grass sectors
    cvector<Rect3d>    cellsWorldBoxes;     // world AABB of each cell
    ModelID            grassModelId[NUM_GRASS_CHANNELS];

    uint8              cellsByX;            // number of cells by X-axis
    uint8              cellsByZ;            // number of cells by Z-axis

    uint8              texSlots;            // how many texture cells (columns) we have in a row of texture atlas
    uint8              texRows;             // how many rows we have in a texture atlas

    uint8              numChannels;         // the same as number of texture slots

    uint32             numInstPerChannel[NUM_GRASS_CHANNELS];
    bool               bGeneratedModel[NUM_GRASS_CHANNELS];  // is model for channel (by index 0-3) generated?

    float              channelProbability[NUM_GRASS_CHANNELS];     // chance of grass instance appearing for this channel

    float              channelGrassScaleMin[NUM_GRASS_CHANNELS];  // minimal scale of grass instances for this channel
    float              channelGrassScaleMax[NUM_GRASS_CHANNELS];  // maximal scale of grass instances for this channel


    ID3D11Buffer* pInstancedBuf = nullptr;          // GPU-side buffer for all the visible grass instances
    uint32 instancesBufCounts[NUM_GRASS_CHANNELS];  // number of instances per channel (in the instanced buffer)
};

//---------------------------------------------------------
// visibility data:
// - grass field index
// - its currently visible cells
//---------------------------------------------------------
struct VisibleGrassField
{
    index          fieldIdx;
    cvector<index> cellsIdxs;
};

//---------------------------------------------------------
// class name:  GrassMgr
//---------------------------------------------------------
class GrassMgr
{
public:
    GrassMgr() {}

    void Update(const Vec3 cameraPos, const Frustum* pFrustum);

    bool AddGrassField(const GrassFieldInitParams& params);

    void SetGrassDistFullSize(const float dist);
    void SetGrassVisibilityRange(const float range);

    float GetGrassDistFullSize() const;
    float GetGrassVisibilityRange() const;

    const GrassField&                 GetGrassField(const index index) const;
    const cvector<VisibleGrassField>& GetVisibleFields()               const;

    vsize GetNumGrassFields() const;

private:
    void CalcVisibleGrass(const Vec3 camPos, const Frustum* pWorldFrustum);
    void UpdateGrassInstancedBuf();

private:
    // registered grass fields
    cvector<GrassField> grassFields_;

    // data about each currently visible field
    cvector<VisibleGrassField> visFields_;

    // radius around camera where grass has full size
    float grassDistFullSize_ = 0;

    // grass visibility range
    float grassVisRange_ = 0;
};

//---------------------------------------------------------
// GLOBAL instance of the grass manager
//---------------------------------------------------------
extern GrassMgr g_GrassMgr;

//---------------------------------------------------------
// return a grass field by index
//---------------------------------------------------------
inline const GrassField& GrassMgr::GetGrassField(const index index) const
{
    assert(index >= 0 && index < grassFields_.size());
    return grassFields_[index];
}

//---------------------------------------------------------
// return an array of data about currently visible grass fields
//---------------------------------------------------------
inline const cvector<VisibleGrassField>& GrassMgr::GetVisibleFields() const
{
    return visFields_;
}

//---------------------------------------------------------
// return current number of all the grass fields
//---------------------------------------------------------
inline vsize GrassMgr::GetNumGrassFields() const
{
    return grassFields_.size();
}

//---------------------------------------------------------
// return radius around camera:
// 1. where grass has its full size
// 2. where we see grass
//---------------------------------------------------------
inline float GrassMgr::GetGrassDistFullSize()    const { return grassDistFullSize_; }
inline float GrassMgr::GetGrassVisibilityRange() const { return grassVisRange_; }

} // namespace
