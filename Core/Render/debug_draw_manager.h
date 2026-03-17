/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: debug_draw_manager.h
    Desc:     the debug drawing API

    Created:  06.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>
#include <math/vec3.h>
#include <assert.h>
#include <cvector.h>


// forward declaration (pointer use only)
class Matrix;

namespace Core
{

//---------------------------------------------------------
// constants
//---------------------------------------------------------
constexpr int MAX_NUM_DBG_LINES             = 16;

constexpr int DEBUG_LINE_BUF_LIMIT          = 512;
constexpr int DEBUG_LINE_VERTEX_BUF_LIMIT   = 512;

constexpr int NUM_AABB_VERTICES             = 8;
constexpr int NUM_AABB_INDICES              = 32;
constexpr int NUM_CIRCLE_VERTICES           = 36;
constexpr int NUM_SPHERE_VERTICES           = NUM_CIRCLE_VERTICES * 3;
constexpr int NUM_SPHERE_INDICES            = NUM_CIRCLE_VERTICES * 6;

//---------------------------------------------------------
// debug geometry shapes definitions
//---------------------------------------------------------
struct DbgLine
{
    Vec3   fromPos      = {0,0,0};
    Vec3   toPos        = {0,0,1};
    uint32 color        = 0;
    uint16 durationMs   = 0;              // 0 is infinite lifetime
    uint8  lineWidth    = 1;              // width in pixels
    uint8  depthEnabled = true;
};

struct DbgLineVertex
{
    Vec3   pos          = {0,0,1};
    uint32 color        = 0;
    uint16 durationMs   = 0;           // 0 is inifinite lifetime
    uint8  lineWidth    = 1;           // width in pixels
    uint8  depthEnabled = true;
};

//---------------------------------------------------------
// class: DbgDrawMgr
//---------------------------------------------------------
class DbgDrawMgr
{
public:

    struct DbgDrawData
    {
        // precomputed sines and cosines for circles and spheres computation
        float sines_[NUM_CIRCLE_VERTICES];
        float cosines_[NUM_CIRCLE_VERTICES];

        // vertices arrays by debug shape types
        DbgLine lines_[MAX_NUM_DBG_LINES];
        DbgLine frustumLines_[12];

        cvector<DbgLineVertex> aabbsVerts_;
        cvector<DbgLineVertex> sphereVerts_;
        cvector<DbgLineVertex> terrainAABBsVerts_;

        cvector<uint16> aabbIndices_;
        cvector<uint16> sphereIndices_;

        int currNumAABBs_;
        int currNumTerrainAABBs_;
        int currNumSpheres_;

        // defines if we want to render all the debug shapes or only
        // some kind of it (only lines, or only AABBs)
        uint16 bRenderLines_       : 1 = true;
        uint16 bRenderCross_       : 1 = true;
        uint16 bRenderSphere_      : 1 = true;
        uint16 bRenderCircle_      : 1 = true;
        uint16 bRenderAxis_        : 1 = true;
        uint16 bRenderTriangle_    : 1 = true;
        uint16 bRenderAABB_        : 1 = true;
        uint16 bRenderOBB_         : 1 = true;
        uint16 bRenderString_      : 1 = true;
        uint16 bRenderTerrainAABB_ : 1 = true;
        uint16 bRenderFrustum_     : 1 = true;
        uint16 bRenderWireframe_   : 1 = true;
    };

    enum class eDbgGeomType
    {
        LINE,
        CROSS,
        SPHERE,
        CIRCLE,
        AXIS,
        TRIANGLE,
        AABB,
        TERRAIN_AABB,
        OBB,
        STRING,
        FRUSTUM,
        MODEL_WIREFRAME,
    };


public:
    DbgDrawMgr();

    void Init();
    void Shutdown();
    void Reset();

    bool IsRenderable       (void) const;
    bool IsRenderableType   (const eDbgGeomType type) const;
    void SwitchRenderingType(const eDbgGeomType type, const bool onOff);

    DbgLine* GetLines()        const;
    DbgLine* GetFrustumLines() const;

    const cvector<DbgLineVertex>& GetAabbVertices()        const;
    const cvector<DbgLineVertex>& GetSphereVertices()      const;
    const cvector<DbgLineVertex>& GetTerrainAabbVertices() const;

    const cvector<uint16>& GetAabbIndices()   const;
    const cvector<uint16>& GetSphereIndices() const;

    int GetNumAABBs()        const;
    int GetNumTerrainAABBs() const;
    int GetNumSpheres()      const;


    // add a line segment to the debug drawing queue
    void AddLine(const Vec3& fromPos,
                 const Vec3& toPos,
                 const Vec3& color,
                 const int lineWidth = 1,
                 const int duration = 0,
                 const bool bDepth = true);

    // add an axis-aligned cross (3 lines converging at a point)
    // to the debug drawing queue
    void AddCross(const Vec3& pos,
                  const Vec3& color,
                  const float size,
                  const float duration = 0.0f,
                  const bool bDepth = true);

    // add a wireframe sphere to the debug drawing queue
    void AddSphere(const Vec3& centerPos,
                   const float radius,
                   const Vec3& color,
                   const int durationMs = 0,
                   const bool bDepth = true);

    // add a circle to the debug drawing queue
    void AddCircle(const Vec3& centerPos,
                   const Vec3& planeNormal,
                   const float radius,
                   const Vec3& color,
                   const float duration = 0.0f,
                   const bool bDepth = true);

    // add a set of coordinate axes depicting the position and orientation
    // of the given transformation to the debug draing queue
    void AddAxes(const Matrix* m,
                 const Vec3& color,
                 const float size,
                 const float duration = 0.0f,
                 const bool bDepth = true);

    // add a wireframe triangle to the debug drawing queue
    void AddTriangle(const Vec3& vertex0,
                     const Vec3& vertex1,
                     const Vec3& vertex2,
                     const Vec3& color,
                     const float lineWidth = 1.0f,
                     const float duration = 0.0f,
                     const bool bDepth = true);

    // add an axis-aligned bounding box to the debug drawing queue
    void AddAABB(const Vec3& minPoint,
                 const Vec3& maxPoint,
                 const Vec3& color,
                 const int lineWidth = 1,
                 const int durationMs = 0,
                 const bool bDepth = true);

    // add an axis-aligned bounding box terrain's patch (sector, chunk)
    // to the debug drawing queue
    void AddTerrainAABB(const Vec3& minPoint,
                        const Vec3& maxPoint,
                        const Vec3& color,
                        const int lineWidth = 1,
                        const int durationMs = 0,
                        const bool bDepth = true);

    // add an oriented bounding box to the debug queue
    void AddOBB(const Matrix* centerTransform,
                const Vec3& scaleXYZ,
                const Vec3& color,
                const float lineWidth = 1.0f,
                const float duration = 0.0f,
                const bool bDepth = true);

    // add a text string to the debug drawing queue
    void AddString(const Vec3& topLeftPos,
                   const char* text,
                   const Vec3& color,
                   const float duration = 0.0f,
                   const bool bDepth = true);

    // add a view frustum to the debug drawing queue
    void AddFrustum(const Vec3& nearTopLeft,
                    const Vec3& nearBottomLeft,
                    const Vec3& nearTopRight,
                    const Vec3& nearBottomRight,
                    const Vec3& farTopLeft,
                    const Vec3& farBottomLeft,
                    const Vec3& farTopRight,
                    const Vec3& farBottomRight,
                    const Vec3& color);

    constexpr inline int GetNumVerticesInAABB()   const { return NUM_AABB_VERTICES; }
    constexpr inline int GetNumIndicesInAABB()    const { return NUM_AABB_INDICES; }

    constexpr inline int GetNumVerticesInSphere() const { return NUM_SPHERE_VERTICES; }
    constexpr inline int GetNumIndicesInSphere()  const { return NUM_SPHERE_INDICES; }

private:

    // private functions...
    void PrecomputeAABB  (void);
    void PrecomputeSphere(void);

    // private data...
    DbgDrawData* pData_;
};

//---------------------------------------------------------
// this global debug drawing manager is configured for drawing 
// in full 3D with a perspective projection
//---------------------------------------------------------
extern DbgDrawMgr g_DebugDrawMgr;

//---------------------------------------------------------
// this global debug drawing manager draws its primitives in
// 2d screen space. The (x,y) coordinates of the point specify
// a 2D location on-screen, and the z coordinate contains a special
// code that indicates whether the (x,y) coordinates are measured
// in absolute pixels or in normalized coordinates that range from 0.0 to 1.0.
// (The latter mode allows drawing to be independent
//  of the actual resolution of the screen)
//---------------------------------------------------------
extern DbgDrawMgr g_DebugDrawMgr2D;


//---------------------------------------------------------
// do we want to render debug geometry at all?
//---------------------------------------------------------
inline bool DbgDrawMgr::IsRenderable(void) const
{
    return pData_ != nullptr;
}

//---------------------------------------------------------
// getters
//---------------------------------------------------------
inline DbgLine* DbgDrawMgr::GetLines()           const { return pData_->lines_; };
inline DbgLine* DbgDrawMgr::GetFrustumLines()    const { return pData_->frustumLines_; }

inline int      DbgDrawMgr::GetNumAABBs()        const { return pData_->currNumAABBs_; }
inline int      DbgDrawMgr::GetNumTerrainAABBs() const { return pData_->currNumTerrainAABBs_; }
inline int      DbgDrawMgr::GetNumSpheres()      const { return pData_->currNumSpheres_; }


inline const cvector<DbgLineVertex>& DbgDrawMgr::GetAabbVertices() const
{
    return pData_->aabbsVerts_;
}

inline const cvector<DbgLineVertex>& DbgDrawMgr::GetSphereVertices() const
{
    return pData_->sphereVerts_;
}

inline const cvector<DbgLineVertex>& DbgDrawMgr::GetTerrainAabbVertices() const
{
    return pData_->terrainAABBsVerts_;
}

inline const cvector<uint16>& DbgDrawMgr::GetAabbIndices()   const
{
    return pData_->aabbIndices_;
}

inline const cvector<uint16>& DbgDrawMgr::GetSphereIndices() const
{
    return pData_->sphereIndices_;
}

} // namespace

