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
#include <geometry/sphere.h>


// forward declarations
class Frustum;
struct CameraParams;
struct GrassChannel;
struct GrassCell;


namespace Core
{

struct GrassFieldInitParams
{
    char name[MAX_LEN_MODEL_NAME];            // name of grass field
    char modelNames[4][MAX_LEN_MODEL_NAME];   // model for grass channels (columns of the texture atlas)

    char materialName[MAX_LEN_MAT_NAME];      // material for the whole grass field
    char densityMask[64];                     // grass density mask for the whole grass field

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

    float grassMinHeight;
    float grassMaxHeight;
};

//---------------------------------------------------------
//---------------------------------------------------------
struct GrassInstance
{
    Vec3 pos;
    Vec2 tex0;
    Vec2 tex1;
    float rotY;    // rotation around Y-axis
    float height; 
};

//---------------------------------------------------------
// each grass cell of grass field has its own VB/IB and set of grass vertices
//---------------------------------------------------------
class GrassCell
{
public:
    cvector<GrassInstance>    grassInstances;

    cvector<VertexGrass>      vertices;
    cvector<UINT>             indices;

    VertexBuffer<VertexGrass> vb;
    IndexBuffer<UINT>         ib;
};

//---------------------------------------------------------
// each grass channel (a column from diffuse texture) of a grass field
// has its own set of grass cells
//---------------------------------------------------------
struct GrassChannel
{
    ModelID modelId;                 // what geometry to use as a grass instance for this particular channel
};


//---------------------------------------------------------
// a signle field of grass, each fields can have up to 16 different
// types of grass instances
// so if we want more we can create several grass fields
//---------------------------------------------------------
struct GrassField
{
    char        name[MAX_LEN_MODEL_NAME];
    char        densityMask[64];            // path to a texture with density mask
    MaterialID  matId;
    uint32      grassCount;                 // number of ALL grass instances of this field

    Rect3d      worldBox;                   // field position and size in 3d space

    uint8 cellsByX;                         // how many times we divide the field along X and Z axis
    uint8 cellsByZ;

    uint8 texSlots;                         // how many texture cells (columns) we have in a row of texture atlas
    uint8 texRows;                          // how many rows we have in a texture atlas

    uint8 numChannels;                      // the same as number of texture slots

    cvector<GrassCell> cells;               // grass sectors of this field
    cvector<Rect3d>    cellsWorldBoxes;     // world AABB of each cell
    GrassChannel       channels[4];         // for each field we may have up to 4 grass channels

    float grassMinHeight;
    float grassMaxHeight;
};

//---------------------------------------------------------

class GrassMgr
{
public:
    GrassMgr() {}

    void Update(const CameraParams* pCamParams, Frustum* pFrustum);

    bool AddGrassField(const GrassFieldInitParams& params);

    const GrassField& GetGrassField(const uint index) const;

private:
    cvector<GrassField> grassFields_;

    cvector<uint32>     visPatchesIdxs_;
    cvector<Sphere>     boundSpheres_;

    float grassRange_ = 0;
};

//---------------------------------------------------------
// GLOBAL instance of the grass manager
//---------------------------------------------------------
extern GrassMgr g_GrassMgr;

//---------------------------------------------------------
inline const GrassField& GrassMgr::GetGrassField(const uint index) const
{
    assert(index < grassFields_.size());
    return grassFields_[index];
}

} // namespace
