// implementation of functional for drawing debug shapes
#include <CoreCommon/pch.h>
#include "r_debug_draw.h"
#include "debug_draw_manager.h"
#include "../Model/model_mgr.h"

namespace Core
{

// static arr of vertices (for internal purposes)
cvector<VertexPosColor> s_Vertices;


//---------------------------------------------------------
// private internal helpers
//---------------------------------------------------------
inline VertexBuffer<VertexPosColor>& GetVB(void)
{
    return g_ModelMgr.GetDebugLinesVB();
}

inline IndexBuffer<uint16>& GetIB(void)
{
    return g_ModelMgr.GetDebugLinesIB();
}

//---------------------------------------------------------
// render all the visible debug shapes onto the screen
//---------------------------------------------------------
void DbgShapeRender::Render(Render::CRender* pRender, const bool bGameMode)
{
    assert(pRender);

    s_Vertices.resize(1024);

    // we always use the same VB/IB for rendering debug shaped,
    // we just dynamically update them
    VertexBuffer<VertexPosColor>& vb = GetVB();
    IndexBuffer<uint16>&          ib = GetIB();

    pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender->BindIB(ib.Get(), DXGI_FORMAT_R16_UINT);

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pRender->BindShaderByName("DebugLineShader");


    DbgDrawMgr& mgr = g_DebugDrawMgr;
    using enum DbgDrawMgr::eDbgGeomType;

    if (mgr.IsRenderableType(LINE))
        RenderLines(pRender);

    if (mgr.IsRenderableType(AABB))
        RenderAABBs(pRender);

    if (mgr.IsRenderableType(SPHERE))
        RenderSpheres(pRender);

    if (!bGameMode && mgr.IsRenderableType(FRUSTUM))
        RenderFrustum(pRender);

    if (mgr.IsRenderableType(TERRAIN_AABB))
        RenderTerrainAABBs(pRender);

    if (mgr.IsRenderableType(MODEL_WIREFRAME))
        RenderModelsWireframes(pRender);

    mgr.Reset();
    s_Vertices.purge();
}

//---------------------------------------------------------
// (for instance: render shooting traces)
//---------------------------------------------------------
void DbgShapeRender::RenderLines(Render::CRender* pRender)
{
    assert(pRender);

    constexpr int vertsPerLine    = 2;
    constexpr int numDbgLineVerts = MAX_NUM_DBG_LINES * vertsPerLine;

    const DbgLine* lines = g_DebugDrawMgr.GetLines();

    // fill arr with lines vertices data and update the vertex buffer with it
    VertexPosColor vertices[numDbgLineVerts];
    memset(vertices, 0, sizeof(vertices));

    for (int lineIdx = 0, vertIdx = 0; lineIdx < MAX_NUM_DBG_LINES; ++lineIdx)
    {
        const DbgLine& line = lines[lineIdx];

        // setup the first vertex
        vertices[vertIdx].position.x = line.fromPos.x;
        vertices[vertIdx].position.y = line.fromPos.y;
        vertices[vertIdx].position.z = line.fromPos.z;
        vertices[vertIdx].color      = line.color;

        vertIdx++;

        // setup the second vertex
        vertices[vertIdx].position.x = line.toPos.x;
        vertices[vertIdx].position.y = line.toPos.y;
        vertices[vertIdx].position.z = line.toPos.z;
        vertices[vertIdx].color      = line.color;

        vertIdx++;
    }

    // update dynamic VB and render lines
    GetVB().UpdateDynamic(vertices, numDbgLineVerts);
    pRender->Draw(numDbgLineVerts, 0);
}


//---------------------------------------------------------
// Desc:  visualize axis-aligned bounding boxes (as a set of lines)
//---------------------------------------------------------
void DbgShapeRender::RenderAABBs(Render::CRender* pRender)
{
    assert(pRender);

    if (g_DebugDrawMgr.GetNumAABBs() <= 0)
        return;

    const int numBoxes      = g_DebugDrawMgr.GetNumAABBs();
    const int numVertsInBox = g_DebugDrawMgr.GetNumVerticesInAABB();
    const int numIndices    = g_DebugDrawMgr.GetNumIndicesInAABB();
    const int numVertices   = numBoxes * numVertsInBox;

    const DbgLineVertex* aabbVerts = g_DebugDrawMgr.GetAabbVertices().data();

    // fill arr with lines vertices data and update the vertex buffer with it
    s_Vertices.resize(numVertices);

    // setup vertices for each aabb
    for (int boxIdx = 0, vertIdx = 0; boxIdx < numBoxes; ++boxIdx)
    {
        for (int i = 0; i < numVertsInBox; ++i, ++vertIdx)
        {
            const DbgLineVertex& v = aabbVerts[vertIdx];

            s_Vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            s_Vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    VertexPosColor* vertices = s_Vertices.data();
    uint16*         indices  = g_DebugDrawMgr.GetAabbIndices().data();

    GetVB().UpdateDynamic(vertices, numVertices);
    GetIB().Update       (indices, numIndices);

    // render
    for (int i = 0; i < numBoxes; ++i)
        pRender->DrawIndexed(numIndices, 0, i * numVertsInBox);
}


//---------------------------------------------------------
// Desc:  visualize debug spheres (3 crossed circles)
//---------------------------------------------------------
void DbgShapeRender::RenderSpheres(Render::CRender* pRender)
{
    assert(pRender);

    if (g_DebugDrawMgr.GetNumSpheres() <= 0)
        return;

    const int numSpheres       = g_DebugDrawMgr.GetNumSpheres();
    const int numVertsInSphere = g_DebugDrawMgr.GetNumVerticesInSphere();
    const int numIndices       = g_DebugDrawMgr.GetNumIndicesInSphere();
    const int numVertices      = numSpheres * numVertsInSphere;

    const DbgLineVertex* sphereVerts = g_DebugDrawMgr.GetSphereVertices().data();

    // fill arr with lines vertices data and update the vertex buffer with it
    s_Vertices.resize(numVertices);

    // setup vertices for each shpere
    for (int sphereIdx = 0, vertIdx = 0; sphereIdx < numSpheres; ++sphereIdx)
    {
        for (int i = 0; i < numVertsInSphere; ++i, ++vertIdx)
        {
            const DbgLineVertex& v = sphereVerts[vertIdx];

            s_Vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            s_Vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    VertexPosColor* vertices = s_Vertices.data();
    uint16*         indices  = g_DebugDrawMgr.GetSphereIndices().data();

    GetVB().UpdateDynamic(vertices, numVertices);
    GetIB().Update       (indices, numIndices);

    // render each sphere
    for (int i = 0; i < numSpheres; ++i)
        pRender->DrawIndexed(numIndices, 0, i * numVertsInSphere);
}

//---------------------------------------------------------
// render axis-aligned bounding box (AABB) around each visible terrain's patch
//---------------------------------------------------------
void DbgShapeRender::RenderTerrainAABBs(Render::CRender* pRender)
{
    assert(pRender);

    if (g_DebugDrawMgr.GetNumTerrainAABBs() <= 0)
        return;

    const int numAABBs      = g_DebugDrawMgr.GetNumTerrainAABBs();
    const int numVertsInBox = g_DebugDrawMgr.GetNumVerticesInAABB();
    const int numIndices    = g_DebugDrawMgr.GetNumIndicesInAABB();
    const int numVertices   = numAABBs * numVertsInBox;

    const DbgLineVertex* verts = g_DebugDrawMgr.GetTerrainAabbVertices().data();

    // fill arr with lines vertices data and update the vertex buffer with it
    s_Vertices.resize(numVertices);

    // setup vertices for each aabb
    for (int aabbIdx = 0, vertIdx = 0; aabbIdx < numAABBs; ++aabbIdx)
    {
        for (int i = 0; i < numVertsInBox; ++i, ++vertIdx)
        {
            const DbgLineVertex& v = verts[vertIdx];

            s_Vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            s_Vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    VertexPosColor* vertices = s_Vertices.data();
    uint16*         indices  = g_DebugDrawMgr.GetAabbIndices().data();

    GetVB().UpdateDynamic(vertices, numVertices);
    GetIB().Update       (indices, numIndices);

    // render each AABB
    for (int i = 0; i < numAABBs; ++i)
        pRender->DrawIndexed(numIndices, 0, i * numVertsInBox);
}

//---------------------------------------------------------
// Desc:  visualize frustum volume
//---------------------------------------------------------
void DbgShapeRender::RenderFrustum(Render::CRender* pRender)
{
    assert(pRender);

    constexpr int numLinesInFrustum = 12;
    constexpr int numVertices = 24;
    VertexPosColor vertices[numVertices];

    const DbgLine* lines = g_DebugDrawMgr.GetFrustumLines();

    // fill arr with vertices data
    for (int lineIdx = 0, vertIdx = 0; lineIdx < numLinesInFrustum; ++lineIdx)
    {
        const DbgLine& line = lines[lineIdx];

        // setup the first vertex
        vertices[vertIdx].position.x = line.fromPos.x;
        vertices[vertIdx].position.y = line.fromPos.y;
        vertices[vertIdx].position.z = line.fromPos.z;
        vertices[vertIdx].color      = line.color;

        vertIdx++;

        // setup the second vertex
        vertices[vertIdx].position.x = line.toPos.x;
        vertices[vertIdx].position.y = line.toPos.y;
        vertices[vertIdx].position.z = line.toPos.z;
        vertices[vertIdx].color      = line.color;

        vertIdx++;
    }

    // update VB and render
    GetVB().UpdateDynamic(vertices, numVertices);

    pRender->Draw(numVertices, 0);
}

//---------------------------------------------------------
// Desc:  visualize wireframe for each instance from input instances group (batch)
//---------------------------------------------------------
void RenderInstancesDebugWireframe(
    const cvector<Render::InstanceBatch>& instancesGroup,
    Render::CRender* pRender,
    UINT& startInstanceLocation)
{
    assert(pRender);

    constexpr UINT offset = 0;
    constexpr UINT startSlot = 1;
    constexpr UINT instBufElemSize = (UINT)(sizeof(Render::ConstBufType::InstancedData));

    // bind the instanced buffer
    pRender->BindVB(startSlot, &pRender->pInstancedBuffer_, instBufElemSize, offset);

    // bind and render each instances batch
    for (const Render::InstanceBatch& batch : instancesGroup)
    {
        pRender->BindVB(&batch.pVB, batch.vertexStride, offset);
        pRender->BindIB(batch.pIB, DXGI_FORMAT_R32_UINT);
        pRender->DrawIndexedInstanced(batch, startInstanceLocation);
    }
}

//---------------------------------------------------------
// Desc:  visualize wireframe of each entity on the scene
//---------------------------------------------------------
void DbgShapeRender::RenderModelsWireframes(Render::CRender* pRender)
{
    assert(pRender);

    UINT startInstanceLocation = 0;
    const Render::RenderDataStorage& storage = pRender->dataStorage_;

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindShaderByName("WireframeShader");

    RenderInstancesDebugWireframe(storage.masked, pRender, startInstanceLocation);
    RenderInstancesDebugWireframe(storage.opaque, pRender, startInstanceLocation);

    pRender->ResetRenderStates();
}

} // namespace
