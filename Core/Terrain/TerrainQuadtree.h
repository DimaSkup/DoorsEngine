// =================================================================================
// Filename:   TerrainQuadtree.h
// Desc:       quadtree implementation for terrain
//
// Created:    05.07.2025 by DimaSkup
// =================================================================================
#pragma once

#include "TerrainBase.h"
#include "../Mesh/Vertex3dTerrain.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"
#include "../Render/d3dclass.h"


namespace Core
{


// =================================================================================
// Constants
// =================================================================================

// QT - quad tree
// LR - lower right
// LL - lower left
// UL - upper left
// UR - upper right
#define QT_LR_NODE 0
#define QT_LL_NODE 1
#define QT_UL_NODE 2
#define QT_UR_NODE 3

// bit codes
#define QT_COMPLETE_FAN 0
#define QT_LL_UR        10
#define QT_LR_UL        5
#define QT_NO_FAN       15

// =================================================================================
// Data structures
// =================================================================================
struct SqtVertex
{
    float height = 0;
};

// =================================================================================
// Class
// =================================================================================
class TerrainQuadtree : public TerrainBase
{
public:
    TerrainQuadtree(void) {}
    ~TerrainQuadtree(void) { Release(); }

    bool Init(void);
    void Release(void);

    bool AllocateMemory(const int numVertices, const int numIndices);

    bool InitBuffers(
        const Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    void Update(const CameraParams& params);


    //-----------------------------------------------------
    // inline getters
    //-----------------------------------------------------
    inline Vertex3dTerrain*  GetVertices()            { return vertices_; }
    inline Vertex3dTerrain** GetVerticesAddrOf()      { return &vertices_; }
    inline UINT*             GetIndices()             { return indices_; }
    inline UINT**            GetIndicesAddrOf()       { return &indices_; }


    inline int              GetNumVertices()    const { return numVertices_; }
    inline int              GetNumIndices()     const { return numIndices_; }
    inline uint             GetVertexStride()   const { vb_.GetStride(); }

    //-----------------------------------------------------
    // setters
    //-----------------------------------------------------
    void SetNumVertices(const int num);
    void SetNumIndices (const int num);


    //-----------------------------------------------------
    // Desc:   set the quadtree's desired detail level
    // Args:   - res: detail level to set
    //-----------------------------------------------------
    inline void SetDesiredResolution(const float res)
    {   desiredResolution_ = res;   }

    //-----------------------------------------------------
    // Desc:   set the quadtree's global resolution values
    // Args:   - res: minimum global resolution to set
    //-----------------------------------------------------
    inline void SetMinResolution(const float res)
    {   minResolution_ = res;   }

    //-----------------------------------------------------
    // Desc:   retrieve a value from the quadtree matrix
    // Args:   - x, z: vertex pos to get data from
    // Ret:    the value stored in the quadtree matrix
    //-----------------------------------------------------
    inline uint8 GetQuadMatrixData(const int x, const int z)
    {   return quadMatrix_[(z*heightMap_.GetWidth()) + x]; }

private:
    void PropagateRoughness(void);

    void RefineNode(
        const float x,
        const float z,
        const int edgeLen,
        const CameraParams& params);

    void GenerateNode(
        const float x,
        const float z,
        const int edgeLen);

    //-----------------------------------------------------
    // Desc:   calculate the index value to access the quadtree matrix
    // Args:   - x, z:  vertex position to calculate the index
    // Ret:    an index to access info in quadtree matrix
    //-----------------------------------------------------
    inline int GetMatrixIdx(const int x, const int z)
    {   return (z*heightMap_.GetWidth() + x); }


private:
    char                name_[32]           = "terrain_quadtree";
    uint8               vertexStride_       = sizeof(Vertex3dTerrain);
    uint32              numVertices_        = 0;
    uint32              numIndices_         = 0;
    MaterialID          materialID_         = 0;
    uint8*              quadMatrix_         = nullptr;

    // keep CPU copy of the vertices/indices data to read from
    Vertex3dTerrain*    vertices_           = nullptr;
    UINT*               indices_            = nullptr;

    uint32              verticesOffset_     = 0;
    uint32              indicesOffset_      = 0;

    // detail level variables
    float               desiredResolution_  = 50.0f;
    float               minResolution_      = 10.0f;

    VertexBuffer<Vertex3dTerrain> vb_;
    IndexBuffer<UINT>             ib_;

};

}; // namespace Core
