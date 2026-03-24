// =================================================================================
// Filename: CGraphics.cpp
// Created:  14.10.22
// =================================================================================
#include <CoreCommon/pch.h>
#include <Engine/gpu_profiler.h>
#include "CGraphics.h"
#include "../Input/inputcodes.h"
#include "../Texture/texture_mgr.h"
#include "../Model/model_mgr.h"
#include "../Mesh/material_mgr.h"

// debug drawing stuff
#include "debug_draw_manager.h"
#include "r_debug_draw.h"

#include <Render/r_states.h>          // Render module
#include <Shaders/Shader.h>           // Render module
#include <geometry/frustum.h>
#include <QuadTree/scene_object.h>
#include <QuadTree/quad_tree.h>
#include <Model/grass_mgr.h>
#include <Mesh/material.h>
#include <Model/animation_mgr.h>


using namespace DirectX;
using namespace Render;


namespace Core
{

// container for the light sources temp data during update process
struct LightTempData
{
    cvector<XMVECTOR>           dirLightsDirections;
    cvector<ECS::PointLight>    pointLightsData;
    cvector<XMFLOAT3>           pointLightsPositions;
    cvector<ECS::SpotLight>     spotLightsData;
    cvector<XMFLOAT3>           spotLightsPositions;
    cvector<XMFLOAT3>           spotLightsDirections;

    cvector<ECS::PointLight>    activePointLights;
} s_LightTmpData;

// static arrays for internal purposes
static cvector<XMMATRIX> s_BoneTransforms;


//---------------------------------------------------------

struct FrustumCullingTmpData
{
    void Resize(const size numRenderableEntts)
    {
        boundSpheres.resize(numRenderableEntts);
        idxsToVisEntts.resize(numRenderableEntts);
        enttsWorlds.resize(numRenderableEntts);
    }

    cvector<BoundingSphere> boundSpheres;
    cvector<index>          idxsToVisEntts;
    cvector<XMMATRIX>       enttsWorlds;
    cvector<XMFLOAT3>       positions;
};

static FrustumCullingTmpData s_tmpFrustumCullData;


//---------------------------------------------------------
// Desc:  default constructor and destructor
//---------------------------------------------------------
CGraphics::CGraphics() : texturesBuf_(NUM_TEXTURE_TYPES, nullptr)
{
    for (int i = 0; i < MAX_NUM_POST_EFFECTS; ++i)
        postFxsQueue_[i] = ePostFxType(-1);

    LogDbg(LOG, "constructor");
}

CGraphics::~CGraphics() 
{
    LogDbg(LOG, "start of destroying");
    Shutdown();
    LogDbg(LOG, "is destroyed");
}


// =================================================================================
//                             PUBLIC METHODS
// =================================================================================
void CGraphics::Init(HWND hwnd, SystemState& systemState)
{
    // assert that ECS and render is already binded
    assert(pRender_  && "did you forget to bind the render?");
    assert(pEnttMgr_ && "did you forget to bind the ECS?");

    pSysState_ = &systemState;
}

//---------------------------------------------------------
// some convertation helpers
//---------------------------------------------------------
inline Vec3 ToVec3(const XMFLOAT3& v)
{
    return Vec3(v.x, v.y, v.z);
}

inline Vec3 ToVec3(const XMVECTOR& v)
{
    return Vec3(v.m128_f32);
}

inline XMFLOAT3 ToFloat3(const Vec3& v)
{
    return XMFLOAT3(v.x, v.y, v.z);
}

//---------------------------------------------------------
// Desc:  add a new axis-aligned bounding box for debug visualization
//---------------------------------------------------------
inline void AddBoundBoxToDebugRender(const BoundingBox& bbox, const Vec3& color)
{
    const Vec3 c = ToVec3(bbox.Center);
    const Vec3 e = ToVec3(bbox.Extents);
    g_DebugDrawMgr.AddAABB(c - e, c + e, color);
}

//---------------------------------------------------------
// gather data for rendering debug shapes
// (visualization of lines, bounding boxes, spheres, wireframes, etc. )
//---------------------------------------------------------
void CGraphics::AddDebugShapesToRender(void)
{
    const Vec3 lightBlue(0, 1, 1);
    const Vec3 yellow(1, 1, 0);
    const Vec3 red(1, 0, 0);
    const Vec3 orange(1, 0.5f, 0.125f);

    const ECS::BoundingSystem& boundSys  = pEnttMgr_->boundingSys_;
    const ECS::NameSystem&     nameSys   = pEnttMgr_->nameSys_;
    const ECS::RenderSystem&   renderSys = pEnttMgr_->renderSys_;

    //
    // add frustum (frustum volume and AABB around it)
    //
    const EntityID gameCamId = nameSys.GetIdByName("game_camera");
    AddFrustumToRender(gameCamId);

    //
    // add AABBs of entities
    //
    if (g_DebugDrawMgr.IsRenderableType(DbgDrawMgr::eDbgGeomType::AABB))
    {
        const Render::RenderDataStorage& storage = pRender_->dataStorage_;

        for (EntityID id : renderSys.GetAllVisibleEntts())
            AddBoundBoxToDebugRender(boundSys.GetWorldBoundBox(id), lightBlue);
    }

    //
    // add AABBs of terrain's patches (sectors)
    //
    if (g_DebugDrawMgr.IsRenderableType(DbgDrawMgr::eDbgGeomType::TERRAIN_AABB))
    {
        const Terrain&         terrain      = g_ModelMgr.GetTerrain();
        const cvector<Rect3d>& patchesAABBs = terrain.GetPatchesAABBs();

        for (const int i : terrain.GetAllVisiblePatches())
        {
            const Rect3d& box = patchesAABBs[i];
            g_DebugDrawMgr.AddTerrainAABB(box.MinPoint(), box.MaxPoint(), yellow);
        }
    }

    //
    // add debug AABB around each visible particles emitter
    //
    if (g_DebugDrawMgr.IsRenderableType(DbgDrawMgr::eDbgGeomType::AABB))
    {
        for (const EntityID id : pEnttMgr_->particleSys_.visEmitters_)
        {
            AddBoundBoxToDebugRender(boundSys.GetWorldBoundBox(id), orange);
        }
    }
}

//---------------------------------------------------------
// Args:  camId:  identifier of a camera to get its params and world frustum
// Out:   1. frustum in world space
//        2. camera parameters
//---------------------------------------------------------
void CGraphics::GetWorldFrustum(
    const EntityID camId,
    Frustum& outFrustum,
    CameraParams& params)
{
    GatherCameraParams(camId, params);

    // create a view frustum planes
    Frustum viewFrustum(params.fov, params.aspect, params.zn, params.zf);

    // transform frustum to world space
    Matrix invView = Matrix(params.invView);
    viewFrustum.Transform(outFrustum, invView);
}

//---------------------------------------------------------
// Desc:  update all the graphics stuff here
// Args:  - deltaTime:  time passed since the prev frame
//        - gameTime:   time passed since the start of the application
//---------------------------------------------------------
void CGraphics::Update(const float deltaTime, const float gameTime)
{
    // check to prevent fuck up
    assert(pSysState_);
    assert(pEnttMgr_);
    assert(pRender_);

    gameTime_ = gameTime;

    ECS::TransformSystem&   transformSys = pEnttMgr_->transformSys_;
    ECS::NameSystem&        nameSys      = pEnttMgr_->nameSys_;

    CameraParams   camParams;
    Frustum        worldFrustum;
    SkyPlane&      skyPlane   = g_ModelMgr.GetSkyPlane();
    float          distFogged = 0;
    Terrain&       terrain    = g_ModelMgr.GetTerrain();

    //const EntityID camId = currCameraId_;
    const EntityID camId = pEnttMgr_->nameSys_.GetIdByName("game_camera");

    ResetRenderStats();
    UpdateCamera();
    GetWorldFrustum(camId, worldFrustum, camParams);


    if (bUseQuadTree_)
    {
        //printf("use quad tree (%f)\n", gameTime);

        //
        // get visible entities: quad-tree search + frustum culling
        //
        QuadTree& quadTree = pEnttMgr_->GetQuadTree();
        Matrix     invView = Matrix(camParams.invView);

        // add frustum's AABB for rendering
        const Rect3d frustumBox = worldFrustum.GetBoundBoxInWorld(&invView);

        // clamp to world
        Rect3d searchRect;
        Rect3d terrainBox = terrain.GetAABB();
        terrainBox.y1 = 160;
        IntersectRect3d(frustumBox, terrainBox, searchRect);

        SceneObject* pObj = quadTree.Search(searchRect, &worldFrustum);
        //SceneObject* pObj = quadTree.Search(searchRect, nullptr);

        //const size numSceneObj = pEnttMgr_->GetNumSceneObjects();
        //const int numFrustumTest = worldFrustum.GetNumTests();

        //printf("tests: %d / %d\n", numFrustumTest, numSceneObj);

        ECS::LightSystem&    lightSys       = pEnttMgr_->lightSys_;
        ECS::RenderSystem&   renderSys      = pEnttMgr_->renderSys_;
        ECS::ParticleSystem& particleSys    = pEnttMgr_->particleSys_;

        cvector<EntityID>& visEntts         = renderSys.GetAllVisibleEntts();
        cvector<EntityID>& visPointLights   = renderSys.GetVisiblePointLights();
        cvector<EntityID>& visEmitters      = particleSys.visEmitters_;

        visEntts.clear();
        visPointLights.clear();
        visEmitters.clear();

        // group entities according to its components
        // (the same entity may be in multiple groups at the same time)
        while (pObj)
        {
            const EntityID id    = pObj->GetId();
            const u32Flags flags = pEnttMgr_->GetAddedComponentsByEntt(id);

            // check if this entity have some components
            if (flags.TestBit(ECS::ModelComponent) && flags.TestBit(ECS::RenderedComponent))
                visEntts.push_back(id);

            else if (flags.TestBit(ECS::LightComponent))
                visPointLights.push_back(id);

            else if (flags.TestBit(ECS::ParticlesComponent))
                visEmitters.push_back(id);

            pObj = pObj->GetNextSearchLink();
        }

        // use as point lights only actual point lights
        for (index i = 0; i < visPointLights.size();)
        {
            if (lightSys.IsPointLight(visPointLights[i]))
            {
                i++;
                continue;
            }

            // swap n pop
            visPointLights[i] = visPointLights.back();
            visPointLights.pop_back();
        }


        // render only active and visible particle emitters
        for (index i = 0; i < visEmitters.size();)
        {
            if (particleSys.IsActive(visEmitters[i]))
            {
                i++;
                continue;
            }

            // swap n pop
            visEmitters[i] = visEmitters.back();
            visEmitters.pop_back();
        }


#if 0

    printf("objects are visible: ");
    while (pObj)
    {
        printf("%d ", pObj->GetId());
        pObj = pObj->GetNextSearchLink();
    }
    printf("   (%f)\n", gameTime);

#endif

    }
    else
    {


        // perform frustum culling on all of our currently loaded entities
        FrustumCullingEntts(worldFrustum);
        FrustumCullingParticles(worldFrustum);
        FrustumCullingPointLights(worldFrustum);

       // const size numAllEntts = pEnttMgr_->GetNumAllEntts();
        //const int numFrustumTest = worldFrustum.GetNumTests();

        //printf("tests: %d / %d\n", numFrustumTest, numAllEntts);
    }


    // after this distance all the objects are completely fogged
    if (pRender_->IsFogEnabled())
        distFogged = pRender_->GetDistFogged();
    else
        distFogged = 1000000;

    // update LOD and visibility for each terrain's patch
    g_ModelMgr.GetTerrain().Update(camParams, worldFrustum, distFogged);

    // update visibility of grass patches
    Vec3 camPos = { camParams.posX, camParams.posY, camParams.posZ };
    g_GrassMgr.Update(camPos, &worldFrustum);


    // update clouds positions
    skyPlane.Update(deltaTime);

    if (g_DebugDrawMgr.IsRenderable())
        AddDebugShapesToRender();

    UpdateParticlesVB();

    // prepare data for each entity
    PrepareRenderInstances(pSysState_->cameraPos);

    // Update shaders common data for this frame
    UpdateShadersDataPerFrame(deltaTime, gameTime);

    // push each 2D sprite into render list
    Push2dSpritesToRender();
}

//---------------------------------------------------------
// Desc:   setup a ptr to render and ECS for using
//---------------------------------------------------------
void CGraphics::BindRender(Render::CRender* pRender)
{
    assert(pRender);
    pRender_ = pRender;
}

void CGraphics::BindECS(ECS::EntityMgr* pEnttMgr)
{
    assert(pEnttMgr);
    pEnttMgr_ = pEnttMgr;
}

//---------------------------------------------------------
// Desc:  render 3D scene
//---------------------------------------------------------
void CGraphics::Render3D()
{
    ResetBeforeRendering();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Reset);

    //if (enableDepthPrepass_)
   //     DepthPrepass();
    g_GpuProfiler.Timestamp(GTS_RenderScene_DepthPrepass);

    //if (!enableDepthPrepass_)
    if (!visualizeDepth_)
        ColorLightPass();

    //PostFxPass();
    g_GpuProfiler.Timestamp(GTS_RenderScene_PostFX);

    // calc frame stats
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_ENTTS];
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_GRASS];
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_TERRAIN];
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_PARTICLE];
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_SKY];
    rndStat_.numDrawnVerts[GEOM_TYPE_ALL] += rndStat_.numDrawnVerts[GEOM_TYPE_SKY_PLANE];

    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_ENTTS];
    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_TERRAIN];
    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_GRASS];
    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_PARTICLE];
    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_SKY];
    rndStat_.numDrawnTris[GEOM_TYPE_ALL] += rndStat_.numDrawnTris[GEOM_TYPE_SKY_PLANE];

    //rndStat_.numDrawCalls[GEOM_TYPE_ALL] = rndStat_.numDrawCalls[GEOM_TYPE_PARTICLE];
#if 0
    SetConsoleColor(CYAN);
    printf("verts (all):      %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_ALL]);
    printf("tris (all):       %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_ALL]);

    printf("verts (entts):    %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_ENTTS]);
    printf("tris (entts):     %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_ENTTS]);

    printf("verts (mask):     %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_MASKED]);
    printf("tris (mask):      %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_MASKED]);

    printf("verts (opaque):   %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_OPAQUE]);
    printf("tris (opaque):    %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_OPAQUE]);

    printf("verts (blend):    %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_BLENDED]);
    printf("tris (blend):     %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_BLENDED]);

    printf("verts (transp):   %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_TRANSPARENT]);
    printf("tris (transp):    %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_TRANSPARENT]);

    printf("verts (grass):    %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_GRASS]);
    printf("tris (grass):     %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_GRASS]);

    printf("verts (trn):      %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_TERRAIN]);
    printf("tris (trn):       %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_TERRAIN]);

    printf("verts (particle): %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_PARTICLE]);
    printf("tris (particle):  %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_PARTICLE]);

    printf("verts (sky):      %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_SKY]);
    printf("tris (sky):       %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_SKY]);

    printf("verts (sky_pl):   %d\n", (int)rndStat_.numDrawnVerts[GEOM_TYPE_SKY_PLANE]);
    printf("tris (sky_pl):    %d\n", (int)rndStat_.numDrawnTris[GEOM_TYPE_SKY_PLANE]);

    SetConsoleColor(YELLOW);
    printf("----------------------------------------\n");
    SetConsoleColor(RESET);
#endif

    pSysState_->numDrawnAllVerts            = rndStat_.numDrawnVerts[GEOM_TYPE_ALL];
    pSysState_->numDrawnAllTris             = rndStat_.numDrawnTris[GEOM_TYPE_ALL];
    pSysState_->numDrawnEnttsInstances      = rndStat_.numDrawnInstances[GEOM_TYPE_ENTTS];
    pSysState_->numDrawCallsEnttsInstances  = rndStat_.numDrawCalls[GEOM_TYPE_ENTTS];
}


//---------------------------------------------------------
// Shutdowns the graphics rendering parts, releases the memory
//---------------------------------------------------------
void CGraphics::Shutdown()
{
    LogDbg(LOG, "graphics shutdown");
    SafeDelete(pModelFrameBuf_);
}


// =================================================================================
// Update / prepare scene
// =================================================================================

//---------------------------------------------------------
//---------------------------------------------------------
void CGraphics::ResetRenderStats(void)
{
    // reset some counters
    SystemState& sysState = *pSysState_;
    sysState.numDrawnTerrainPatches     = 0;
    sysState.numCulledTerrainPatches    = 0;
    sysState.numDrawnAllVerts           = 0;
    sysState.numDrawnAllTris            = 0;
    sysState.numDrawnEnttsInstances     = 0;
    sysState.numDrawCallsEnttsInstances = 0;

    // reset render statistic before the frame
    for (int i = 0; i < NUM_GEOM_TYPES; ++i)
    {
        rndStat_.numDrawnVerts[i]     = 0;
        rndStat_.numDrawnTris[i]      = 0;
        rndStat_.numDrawnInstances[i] = 0;
        rndStat_.numDrawCalls[i]      = 0;
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void CGraphics::UpdatePlayerPos(void)
{
    if (!pSysState_->isGameMode)
        return;

    ECS::PlayerSystem& player = pEnttMgr_->playerSys_;
    XMFLOAT3        playerPos = player.GetPosition();
    Terrain&    terrain = g_ModelMgr.GetTerrain();
    const float   terrainSize = (float)terrain.heightMap_.GetWidth();

    // force the player to be always in world
    playerPos.x = clampf(playerPos.x, 0, terrainSize - 1);
    playerPos.z = clampf(playerPos.z, 0, terrainSize - 1);

    if (!player.IsFreeFlyMode())
    {
        // make player's offset by Y-axis to be always over the terrain even
        // when jump from lower to higher position
        const float groundLevel = terrain.GetScaledInterpolatedHeightAtPoint(playerPos.x, playerPos.z);
        const float minPlayerY  = groundLevel + player.GetOffsetOverTerrain();

        player.SetMinVerticalOffset(minPlayerY);
    }
}

//---------------------------------------------------------
// update the cameras states
//---------------------------------------------------------
void CGraphics::UpdateCamera(void)
{
    ECS::TransformSystem& transformSys  = pEnttMgr_->transformSys_;
    ECS::CameraSystem&    camSys        = pEnttMgr_->cameraSys_;

    if (pSysState_->isGameMode)
    {
        UpdatePlayerPos();

        pSysState_->cameraPos = pEnttMgr_->playerSys_.GetPosition();
        pSysState_->cameraDir = transformSys.GetDirection(currCameraId_);
    }

    // we are in the editor mode
    else
    {
        pSysState_->cameraPos = transformSys.GetPosition(currCameraId_);
        pSysState_->cameraDir = transformSys.GetDirection(currCameraId_);
    }

    // update camera's matrices
    camSys.UpdateView(currCameraId_);

    pSysState_->cameraView = camSys.GetView(currCameraId_);
    pSysState_->cameraProj = camSys.GetProj(currCameraId_);
    viewProj_              = pSysState_->cameraView * pSysState_->cameraProj;
}

//---------------------------------------------------------
// Desc:  collect params of the camera by id
//        so later we will use these params for culling
//---------------------------------------------------------
void CGraphics::GatherCameraParams(const EntityID camId, CameraParams& outParams)
{
    memset(&outParams, 0, sizeof(outParams));

    const ECS::CameraSystem& camSys = pEnttMgr_->cameraSys_;

    const XMFLOAT3& pos       = camSys.GetPos(camId);
    const XMMATRIX& xmViewMat = camSys.GetView(camId);
    const XMMATRIX& xmInvView = camSys.GetInverseView(camId);
    const XMMATRIX& xmProjMat = camSys.GetProj(camId);

    outParams.posX = pos.x;
    outParams.posY = pos.y;
    outParams.posZ = pos.z;

    camSys.GetFrustumInitParams(
        camId,
        outParams.fov,
        outParams.aspect,
        outParams.zn,
        outParams.zf);

    const float* viewMat = xmViewMat.r[0].m128_f32;
    const float* invView = xmInvView.r[0].m128_f32;
    const float* projMat = xmProjMat.r[0].m128_f32;

    memcpy(outParams.view,    viewMat, sizeof(float) * 16);
    memcpy(outParams.invView, invView, sizeof(float) * 16);
    memcpy(outParams.proj,    projMat, sizeof(float) * 16);
}

//---------------------------------------------------------
// push 2d sprite to the render list
//---------------------------------------------------------
void CGraphics::Push2dSpritesToRender(void)
{
    ECS::SpriteSystem& spriteSys = pEnttMgr_->spriteSys_;
    const EntityID*    sprites   = spriteSys.GetAllSpritesIds();
    TexID texId = 0;
    uint16 left, top, width, height;

    for (index i = 0; i < spriteSys.GetNumAllSprites(); ++i)
    {
        spriteSys.GetData(sprites[i], texId, left, top, width, height);
        ID3D11ShaderResourceView* pSRV = g_TextureMgr.GetTexViewsById(texId);

        pRender_->PushSpriteToRender(Render::Sprite2D(pSRV, left, top, width, height));
    }
}

//---------------------------------------------------------
// Desc:   update vertex buffer with update particles before a new frame
//---------------------------------------------------------
void CGraphics::UpdateParticlesVB()
{
    ECS::ParticleSystem& particleSys = pEnttMgr_->particleSys_;
    const ECS::ParticlesRenderData& particlesData = particleSys.GetParticlesToRender();

    // prepare updated particles data for rendering
    VertexBuffer<BillboardSprite>& vb = g_ModelMgr.GetBillboardsBuffer();

    BillboardSprite* particlesBuf = (BillboardSprite*)particlesData.particles.data();
    const int numParticles        = (int)particlesData.particles.size();


    if (!particlesBuf || numParticles == 0)
        return;

    // update the vertex buffer with updated particles data
    if (!vb.UpdateDynamic(particlesBuf, numParticles))
    {
        LogErr(LOG, "didn't manage to update particles VB");
        return;
    }
}

// --------------------------------------------------------
// Desc:   prepare rendering data of entts which have default render states
// --------------------------------------------------------
void CGraphics::PrepareRenderInstances(const DirectX::XMFLOAT3& cameraPos)
{
    cvector<EntityID>& visibleEntts = pEnttMgr_->renderSys_.GetAllVisibleEntts();

    if (visibleEntts.size() == 0)
        return;

    // gather entts data for rendering
    prep_.PrepareEnttsDataForRendering(
        visibleEntts,
        cameraPos,
        pEnttMgr_,
        pRender_->dataStorage_);

    pRender_->UpdateInstancedBuffer(pRender_->dataStorage_.instancesBuf);
}

//---------------------------------------------------------
// Desc:  add a debug AABB of this emitter for rendering (when we turn on dbg rendering)
//---------------------------------------------------------
void PushEmitterForDbgRender(
    const DirectX::XMFLOAT3& emitterPos,
    const DirectX::XMFLOAT3& aabbCenter,
    const DirectX::XMFLOAT3& aabbExtents)
{
    const DirectX::XMFLOAT3 minP(aabbCenter - aabbExtents + emitterPos);   // AABB's min point in world
    const DirectX::XMFLOAT3 maxP(aabbCenter + aabbExtents + emitterPos);   // AABB's max point in world
    const Vec3 yellow = Vec3(0, 1, 1);

    Core::g_DebugDrawMgr.AddAABB(
        Vec3(minP.x, minP.y, minP.z),
        Vec3(maxP.x, maxP.y, maxP.z),
        yellow);
}

//---------------------------------------------------------
// Desc:   add a view frustum of camera by id to the debug shapes render list
// Args:   - camId:  identifier of a camera which frustum we want to visualize
//---------------------------------------------------------
void CGraphics::AddFrustumToRender(const EntityID camId)
{
    const Vec3 green(0, 1, 0);
    const Vec3 red(1, 0, 0);

    CameraParams camParams;
    GatherCameraParams(camId, camParams);

    const Frustum frustum(camParams.fov, camParams.aspect, camParams.zn, camParams.zf);
    const Matrix invView(camParams.invView);

    // 8 corner points of the frustum volume
    Vec3 p[8];

    frustum.GetPointsInWorld(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], &invView);

    // add frustum volume for visualization
    g_DebugDrawMgr.AddFrustum(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], green);

    // add frustum's AABB for rendering
    const Rect3d aabb(p, 8);
    const Terrain& terrain = g_ModelMgr.GetTerrain();

    // clamp to world
    Rect3d aabbInWorld;
    IntersectRect3d(aabb, terrain.GetAABB(), aabbInWorld);
  
    g_DebugDrawMgr.AddAABB(aabbInWorld.MinPoint(), aabbInWorld.MaxPoint(), red);
}

//---------------------------------------------------------
// test all the renderable entities agains the world frustum
//---------------------------------------------------------
void CGraphics::FrustumCullingEntts(const Frustum& worldFrustum)
{
    ECS::EntityMgr&    mgr       = *pEnttMgr_;
    ECS::RenderSystem& renderSys = mgr.renderSys_;

    const cvector<EntityID>& rendEntts = renderSys.GetAllEnttsIDs();
    cvector<EntityID>&       visEntts  = renderSys.GetAllVisibleEntts();

    size numVisEntts = 0;                                        // the number of currently visible entts

    if (rendEntts.size() == 0)
        return;

    FrustumCullingTmpData& tmpData = s_tmpFrustumCullData;
    tmpData.Resize(rendEntts.size());

    // get arr of bounding spheres for each renderable entt
    mgr.boundingSys_.GetBoundSpheres(
        rendEntts.data(),
        rendEntts.size(),
        tmpData.boundSpheres);

    // go through each entity and define if it is visible
    for (index idx = 0; idx < rendEntts.size(); ++idx)
    {
        tmpData.idxsToVisEntts[numVisEntts] = idx;

        const XMFLOAT3& c = tmpData.boundSpheres[idx].Center;
        const float     r = tmpData.boundSpheres[idx].Radius;

        numVisEntts += worldFrustum.TestSphere(Sphere(c.x, c.y, c.z, r));
    }

    // store ids of visible entts
    visEntts.resize(numVisEntts);

    for (index i = 0; i < numVisEntts; ++i)
        visEntts[i] = rendEntts[tmpData.idxsToVisEntts[i]];

    // this number of entities (instances) will be rendered onto the screen
    pSysState_->numDrawnEnttsInstances = (uint32)numVisEntts;
}

//--------------------------------------------------------
// Desc:  test which particles emitters are visible for this frame
//--------------------------------------------------------
void CGraphics::FrustumCullingParticles(const Frustum& worldFrustum)
{
    ECS::ParticleSystem&     particleSys = pEnttMgr_->particleSys_;
    cvector<EntityID>&       visEmitters = particleSys.visEmitters_;
    const cvector<EntityID>& allEmitters = particleSys.GetAllEmitters();

    visEmitters.clear();

    for (index i = 0; i < allEmitters.size(); ++i)
    {
        if (worldFrustum.TestRect(particleSys.GetEmitterWorldAABB(allEmitters[i])))
            visEmitters.push_back(allEmitters[i]);
    }
}

//---------------------------------------------------------
// Desc:  cull invisible point lights so we don't use them
//        when calculate lighting in shaders
//---------------------------------------------------------
void CGraphics::FrustumCullingPointLights(const Frustum& worldFrustum)
{
    const ECS::LightSystem&     lightSys          = pEnttMgr_->lightSys_;
    const ECS::RenderSystem&    renderSys         = pEnttMgr_->renderSys_;

    const ECS::PointLights&     pointLights       = lightSys.GetPointLights();
    const size                  numAllPointLights = pointLights.data.size();

    cvector<EntityID>&          visPointLights    = renderSys.GetVisiblePointLights();
    cvector<DirectX::XMFLOAT3>& positions         = s_tmpFrustumCullData.positions;
    size                        numVisiblePointL  = 0;

    pEnttMgr_->transformSys_.GetPositions(
        pointLights.ids.data(),
        pointLights.ids.size(),
        positions);

    // define if we see point light if so we store its id
    visPointLights.resize(numAllPointLights);

    for (index i = 0; i < numAllPointLights; ++i)
    {
        const Sphere sphere(
            positions[i].x,
            positions[i].y,
            positions[i].z,
            pointLights.data[i].range);

        if (worldFrustum.TestSphere(sphere))
            visPointLights[numVisiblePointL++] = pointLights.ids[i];
    }

    visPointLights.resize(numVisiblePointL);
}

//---------------------------------------------------------
// Desc:   update shaders common data (some const buffers) for this frame: 
//         viewProj matrix, camera position, light sources data, etc.
// Args:   - dt:        the time passed since the prev frame
//         - gameTime:  the time passed since the start of the application
//---------------------------------------------------------
void CGraphics::UpdateShadersDataPerFrame(const float dt, const float gameTime)
{
    const SkyPlane& skyPlane   = g_ModelMgr.GetSkyPlane();

    ECS::CameraSystem& camSys  = pEnttMgr_->cameraSys_;
    const EntityID currCamId   = currCameraId_;

    const XMMATRIX& view       = camSys.GetView(currCamId);
    const XMMATRIX& proj       = camSys.GetProj(currCamId);
    const XMMATRIX& invView    = camSys.GetInverseView(currCamId);
    const XMMATRIX  invProj    = DirectX::XMMatrixInverse(nullptr, proj);

    pRender_->UpdateCbViewProj(DirectX::XMMatrixTranspose(viewProj_));
    pRender_->UpdateCbTime(dt, gameTime);

    pRender_->UpdateCbCamera(
        DirectX::XMMatrixTranspose(view),
        DirectX::XMMatrixTranspose(proj),
        invView,
        invProj,
        camSys.GetPos(currCamId),
        camSys.GetNearZ(currCamId),
        camSys.GetFarZ(currCamId));

    SetupLightsForFrame(pRender_->perFrameData_);

    // update const buffers with new data
    pRender_->UpdatePerFrame(pRender_->perFrameData_);

    pRender_->UpdateCbSky(
        skyPlane.GetTranslation(0),
        skyPlane.GetTranslation(1),
        skyPlane.GetTranslation(2),
        skyPlane.GetTranslation(3),
        skyPlane.GetBrightness());
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   
//         - texIdx:                identifiers to textures which will be bound
//---------------------------------------------------------
void CGraphics::BindMaterial(
    const bool alphaClip,
    const RsID rsId,
    const BsID bsId,
    const DssID dssId,
    const TexID* texIds)
{
    // find texture resource views by input textures ids
    ID3D11ShaderResourceView* texViews[NUM_TEXTURE_TYPES]{ nullptr };
    g_TextureMgr.GetTexViewsByIds(texIds, NUM_TEXTURE_TYPES, texViews);

    BindMaterial(alphaClip, rsId, bsId, dssId, texViews);
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   - texViews:              textures to bind
//---------------------------------------------------------
void CGraphics::BindMaterial(
    const bool alphaClip,
    const RsID rsId,
    const BsID bsId,
    const DssID dssId,
    ID3D11ShaderResourceView* const* texViews)
{
    if (!texViews)
    {
        LogErr(LOG, "input arr of textures IDs == nullptr");
        return;
    }

    static bool  prevAlphaClip = 0;
    static RsID  prevRsId = 0;
    static BsID  prevBsId = 0;
    static DssID prevDssId = 0;

    // bind textures of this material
    GetContext()->PSSetShaderResources(100U, NUM_TEXTURE_TYPES, texViews);

    // check if we need to switch render states
    if (prevAlphaClip == alphaClip &&
        prevRsId == rsId &&
        prevBsId == bsId &&
        prevDssId == dssId)
        return;

 
    Render::RenderStates& renderStates = pRender_->GetRenderStates();

    // switch alpha clipping if need...
    if (prevAlphaClip != alphaClip)
    {
        pRender_->SwitchAlphaClipping(alphaClip);
        prevAlphaClip = alphaClip;
    }

    // switch raster state (fill mode, cull mode, etc.) if need...
    if (prevRsId != rsId)
    {
        renderStates.SetRs(rsId);
        prevRsId = rsId;
    }

    // switch blending state if need...
    if (prevBsId != bsId)
    {
        renderStates.SetBs(bsId);
        prevBsId = bsId;
    }

    // switch depth-stencil state if need...
    if (prevDssId != dssId)
    {
        renderStates.SetDss(dssId, 0);
        prevDssId = dssId;
    }
}

//---------------------------------------------------------
// depth prepass: render the scene to a screen-size normal/depth texture map,
// where RGB stores the view space normal and the alpha channel stores the view
// space depth (z-coordinate);
// 1. normal/depth texture is used for SSAO
// 2. precomputed depth is used for early Z-test
//---------------------------------------------------------
void CGraphics::DepthPrepass()
{
    ID3D11DeviceContext* pCtx = pRender_->GetContext();
    Render::D3DClass&    d3d  = pRender_->GetD3D();

    UINT startInstanceLocation = 0;
    const Render::RenderDataStorage& storage = pRender_->dataStorage_;

    // check if we have any instances to render
    if (storage.instancesBuf.GetSize() <= 0)
        return;

    pRender_->ResetRenderStates();

    // only depth writing
    pCtx->OMSetRenderTargets(0, nullptr, d3d.pDepthStencilView_);

    // render masked geometry: tree branches, bushes, etc.
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DepthPrepassInstanceGroup(storage.masked, GEOM_TYPE_MASKED, startInstanceLocation);

    // render opaque geometry: solid objects
    DepthPrepassInstanceGroup(storage.opaque, GEOM_TYPE_OPAQUE, startInstanceLocation);

    // render terrain
    TerrainDepthPrepass();

    // bind back a swap chain's RTV
    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// usual rendering: color and lighting
//---------------------------------------------------------
void CGraphics::ColorLightPass()
{
    const Render::RenderDataStorage& storage = pRender_->dataStorage_;
    UINT startInstanceLocation = 0;

    // for post effects we have to render into another (non default) render target
    if (IsEnabledPostFxPass() || pRender_->GetD3D().IsEnabledFXAA())
    {
        Render::D3DClass& d3d = pRender_->GetD3D();
        ID3D11RenderTargetView* pRTV = nullptr;
        const float clearColor[4] = { 1,1,1,1 };

        // for MSAA we need to bind sufficient render target view (RTV)
        if (d3d.IsEnabled4xMSAA())
            pRTV = d3d.pMSAARTV_;

        // bind non-MSAA render target
        else
            pRTV = d3d.postFxsPassRTV_[0];

        assert(pRTV && "for post process: RTV tex == nullptr");

        pRender_->GetContext()->OMSetRenderTargets(1, &pRTV, d3d.pDepthStencilView_);
        pRender_->GetContext()->ClearRenderTargetView(pRTV, clearColor);
    }


    // check if we have any instances to render
    if (storage.instancesBuf.GetSize() <= 0)
        return;

    // first of all we render player's weapon
    RenderPlayerWeapon();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Weapon);
    
    // render grass
    RenderGrass();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Grass);

    // render masked geometry: tree branches, bushes, etc.
    RenderInstanceGroups(storage.masked, GEOM_TYPE_MASKED, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Masked);

    // render opaque geometry: solid objects
    RenderInstanceGroups(storage.opaque, GEOM_TYPE_OPAQUE, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Opaque);


    // render each animated entity separately
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#if 1
    const ECS::RenderSystem& renderSys = pEnttMgr_->renderSys_;

    for (const EntityID id : pEnttMgr_->animationSys_.GetEnttsIds())
    {
        if (renderSys.HasEntity(id))
            RenderSkinnedModel(id);
    }
#endif
    g_GpuProfiler.Timestamp(GTS_RenderScene_SkinnedModels);


    // render terrain
    RenderTerrain();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Terrain);

    // render 3d decals
    RenderDecals();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Decals);

    // render sky and clouds
    RenderSkyDome();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Sky);

    //RenderSkyClouds();
    g_GpuProfiler.Timestamp(GTS_RenderScene_SkyPlane);

    // render blended geometry (but not transparent)
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderInstanceGroups(storage.blended, GEOM_TYPE_BLENDED, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Blended);

    // render transparent geometry
    RenderInstanceGroups(storage.blendedTransparent, GEOM_TYPE_TRANSPARENT, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Transparent);

    // render billboards and particles
    RenderParticles();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Particles);

    if (g_DebugDrawMgr.IsRenderable())
        RenderDebugShapes();
    g_GpuProfiler.Timestamp(GTS_RenderScene_DbgShapes);
}

//---------------------------------------------------------
// Desc:  render post-effects:  post process, depth visualization, etc.
//        (do it after the scene color and lighting have been rendered)
//---------------------------------------------------------
void CGraphics::PostFxPass()
{
    if (visualizeDepth_)
    {
        // reset raster state, blend state, depth-stencil state to default
        pRender_->ResetRenderStates();
        VisualizeDepthBuffer();
        return;
    }

    Render::D3DClass&     d3d = GetD3D();
    ID3D11DeviceContext* pCtx = GetContext();

    // FXAA is a kind of post-effects
    if (d3d.IsEnabledFXAA())
    {
        // reset raster state, blend state, depth-stencil state to default
        pRender_->ResetRenderStates();

        assert(d3d.pSwapChainRTV_     && "swap chain RTV is wrong");
        assert(d3d.postFxsPassSRV_[0] && "resolved SRV is wrong");

        // bind dst render target (+ unbind depth stencil view) and src shader resource view 
        pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
        pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

        pRender_->BindShaderByName("FXAA");
        pCtx->Draw(3, 0);

        // bind depth stencil view back
        pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);

        return;
    }

    // if execution of posts effects is turned off...
    if (!IsEnabledPostFxPass())
        return;

    // reset raster state, blend state, depth-stencil state to default
    pRender_->ResetRenderStates();

    if (d3d.IsEnabled4xMSAA())
    {
        // resolve MSAA -> single-sample (non MSAA)
        assert(d3d.postFxsPassTex_[0] && "resolved tex is wrong");
        assert(d3d.pMSAAColorTex_     && "MSAA color tex is wrong");
        pCtx->ResolveSubresource(d3d.postFxsPassTex_[0], 0, d3d.pMSAAColorTex_, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
    }


    // we have only one post process pass
    if (numPostFxsInQueue_ == 1)
        RenderPostFxOnePass();
    else
        RenderPostFxMultiplePass();
}

//---------------------------------------------------------
// Desc:   init a single frame buffer which is used to render material big icon
//         (for the editor's material browser)
// 
// Args:   - width, height:   frame buffer's dimensions
//---------------------------------------------------------
bool CGraphics::InitMatBigIconFrameBuf(const uint width, const uint height)
{
    // check input args
    if (width <= 0 && height <= 0)
    {
        LogErr(LOG, "input icon width or height is wrong, it must be > 0 (current w: %d, h: %d)", width, height);
        return false;
    }

    // setup params for the frame buffers
    FrameBufSpec fbSpec;
    fbSpec.width          = width;
    fbSpec.height         = height;
    fbSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    fbSpec.screenNear     = GetD3D().GetNearZ();
    fbSpec.screenDepth    = GetD3D().GetFarZ();

    // TODO: if input params differ from the frame buffer's params we
    // need to recreate the frame buffer according to new params

    if (!materialBigIconFrameBuf_.Init(fbSpec))
    {
        LogErr(LOG, "can't initialize a material's big icon");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   initialize frame buffers which will be used to render material icons
//         (for material browser in the editor)
// Args:   - numBuffers:             how many buffers we will init
//         - width, height:          frame buffer's dimensions
//         - outShaderResourceViews: output array of ptr to texture resource views
//---------------------------------------------------------
bool CGraphics::InitMatIconFrameBuffers(
    const size numBuffers,
    const uint32 width,
    const uint32 height,
    cvector<ID3D11ShaderResourceView*>& outShaderResourceViews)
{
    // check input args
    if (numBuffers <= 0)
    {
        LogErr(LOG, "input number of icons must be > 0");
        return false;
    }
    if (width <= 0 && height <= 0)
    {
        LogErr(LOG, "input width and height must be > 0 (current w: %zu, h: %zu)", width, height);
        return false;
    }

    // setup params for the frame buffers (we will use the same params for each)
    FrameBufSpec fbSpec;
    fbSpec.width       = (UINT)width;
    fbSpec.height      = (UINT)height;
    fbSpec.format      = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    fbSpec.screenNear  = GetD3D().GetNearZ();
    fbSpec.screenDepth = GetD3D().GetFarZ();

    // release memory from the previous set of frame buffers
    for (FrameBuffer& buf : materialsFrameBuffers_)
        buf.Shutdown();

    // alloc memory for frame buffers and init them
    materialsFrameBuffers_.resize(numBuffers);
    outShaderResourceViews.resize(numBuffers, nullptr);

    for (index i = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        if (!buf.Init(fbSpec))
        {
            // release all the buffers
            for (FrameBuffer& buf : materialsFrameBuffers_)
                buf.Shutdown();

            LogErr(LOG, "can't init a frame buffer (idx: %d)", (int)i);
            return false;
        }

        ++i;
    }

    // copy frame buffers texture ptrs into the output array
    for (int i = 0; FrameBuffer & buf : materialsFrameBuffers_)
        outShaderResourceViews[i++] = buf.GetSRV();

    return true;
}

//---------------------------------------------------------
// Desc:  render a single material preview (when we are in the material's editor)
//---------------------------------------------------------
bool CGraphics::RenderBigMaterialIcon(
    const MaterialID matID,
    const float yRotationAngle,
    ID3D11ShaderResourceView** outMaterialImg)
{
    if (!outMaterialImg)
    {
        LogErr(LOG, "input shader resource view == NULL");
        return false;
    }

    // get a sphere model
    const ModelID       sphereId   = g_ModelMgr.GetModelIdByName("basic_sphere");
    const Model&        sphere     = g_ModelMgr.GetModelById(sphereId);
    const MeshGeometry& sphereMesh = sphere.GetMeshes();

    const VertexBuffer<Vertex3D>& vb = sphereMesh.vb_;
    const IndexBuffer<UINT>&      ib = sphereMesh.ib_;
    const UINT                offset = 0;

    // setup world, view and proj matrices
    const XMMATRIX W = XMMatrixRotationY(yRotationAngle);
    const XMMATRIX V = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX P = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    pRender_->UpdateCbWorldAndViewProj(XMMatrixTranspose(W), XMMatrixTranspose(V * P));

    // bind shader and prepare IA for rendering
    pRender_->BindShaderByName("MaterialIconShader");
    pRender_->SetPrimTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender_->BindVB          (vb.GetAddrOf(), vb.GetStride(), offset);
    pRender_->BindIB          (ib.Get(), DXGI_FORMAT_R32_UINT);

    // prepare responsible frame buffer for rendering
    materialBigIconFrameBuf_.ClearBuffers(0,0,0,0);
    materialBigIconFrameBuf_.Bind();

    // bind material
    const Material& mat = g_MaterialMgr.GetMatById(matID);
    BindMaterial(mat);
    pRender_->UpdateCbMaterialColors(
        mat.ambient,
        mat.diffuse,
        mat.specular,
        mat.reflect);

    // draw
    pRender_->DrawIndexed(ib.GetIndexCount(), 0U, 0U);

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender_->UpdateCbViewProj(XMMatrixTranspose(viewProj_));

    // copy frame buffer texture into the input SRV
    *outMaterialImg = materialBigIconFrameBuf_.GetSRV();

    GetD3D().ResetBackBufferRenderTarget();
    GetD3D().ResetViewport();

    return true;
}

//-----------------------------------------------------------------------------
// Desc:   render material icons (just sphere models with particular material) into
//         frame buffer, so later we will use its shader resource views
//         for visualization of materials (in the editor)
//-----------------------------------------------------------------------------
bool CGraphics::RenderMaterialsIcons()
{
    Render::CRender* pRender = pRender_;

    // get a sphere model
    const ModelID       sphereId   = g_ModelMgr.GetModelIdByName("basic_sphere");
    const Model&        sphere     = g_ModelMgr.GetModelById(sphereId);
    const MeshGeometry& sphereMesh = sphere.GetMeshes();

    const VertexBuffer<Vertex3D>& vb  = sphereMesh.vb_;
    const IndexBuffer<UINT>&      ib  = sphereMesh.ib_;
    const UINT            indexCount  = ib.GetIndexCount();

    // setup world, view and proj matrices
    const XMMATRIX W = XMMatrixIdentity();
    const XMMATRIX V = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX P = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    // prepare IA for rendering, bind shaders, set matrices
    pRender->BindShaderByName("MaterialIconShader");
    pRender->UpdateCbWorldAndViewProj(W, XMMatrixTranspose(V * P));

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender->BindIB(ib.Get(), DXGI_FORMAT_R32_UINT);


    // clear all the previous content of frame buffers
    for (FrameBuffer& buf : materialsFrameBuffers_)
        buf.ClearBuffers(0,0,0,0);

    // render material by idx into responsible frame buffer
    for (MaterialID matId = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        // bind material data and its textures
        const Material& mat = g_MaterialMgr.GetMatById(matId);
        BindMaterial(mat);
        pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

        // render geometry
        buf.Bind();
        pRender->DrawIndexed(indexCount, 0U, 0U);

        ++matId;
    }

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->UpdateCbViewProj(XMMatrixTranspose(viewProj_));

    GetD3D().ResetBackBufferRenderTarget();
    GetD3D().ResetViewport();

    return true;
}

//---------------------------------------------------------
// Desc:  render all the 3d decals from the decals rendering list
//---------------------------------------------------------
void CGraphics::RenderDecals()
{
    Render::CRender* pRender = pRender_;

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindShaderByName("DecalShader");
    BindMaterialById(0);

    // update params which we will use to generate a 2D sprite (in geometry shader)
    const VertexBuffer<VertexDecal3D>& vb = g_ModelMgr.GetDecalsVB();
    const IndexBuffer<uint16>&         ib = g_ModelMgr.GetDecalsIB();

    const Material& mat = g_MaterialMgr.GetMatByName("wallmark");
    BindMaterial(mat);
    pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

    pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender->BindIB(ib.Get(), DXGI_FORMAT_R16_UINT);

    // render each decal
    for (uint32 i = 0; i < g_ModelMgr.GetNumDecals(); ++i)
        pRender->DrawIndexed(ib.GetIndexCount(), 0, i * NUM_VERTS_PER_DECAL);
}

//---------------------------------------------------------
// Desc:  just render grass planes
//---------------------------------------------------------
void CGraphics::RenderGrass()
{

    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ModelID grassModelId              = INVALID_MODEL_ID;
    Model*  pGrass                    = nullptr;
    UINT    startInstanceLocation     = 0;

    const VertexBuffer<Vertex3D>* pVB = nullptr;
    const IndexBuffer<UINT>*      pIB = nullptr;
    const GrassField&           field = g_GrassMgr.GetGrassField(0);



    // bind one shader for the whole field
    pRender_->BindShaderByName("GrassShaderInstance");

    // bind material for the whole field
    const Material& mat = g_MaterialMgr.GetMatByName("grass_0");
    BindMaterial(mat);
    pRender_->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);


    // render each channel of the grass field
    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        // if we need to change geometry (grass model) for this channel
        if (grassModelId != field.grassModelId[ch])
        {
            grassModelId = field.grassModelId[ch];
            pGrass       = &g_ModelMgr.GetModelById(grassModelId);

            pVB = &pGrass->GetVB();
            pIB = &pGrass->GetIB();

            // bind VB/IB and instanced buf
            ID3D11Buffer* const vbs[2] = { pVB->Get(), field.pInstancedBuf };
            const UINT      strides[2] = { pVB->GetStride(), sizeof(GrassInstance) };
            const UINT      offsets[2] = { 0, 0 };

            pRender_->BindVBs(0, 2, vbs, strides, offsets);
            pRender_->BindIB(pIB->Get(), DXGI_FORMAT_R32_UINT);
        }

        pRender_->DrawIndexedInstanced(
            pIB->GetIndexCount(),
            field.instancesBufCounts[ch],        // instances count
            0, 0,                               // index and ertex start
            startInstanceLocation);             // where start getting instances data

        startInstanceLocation += field.instancesBufCounts[ch];
    }

    uint numRenderedInstances = 0;

    for (int ch = 0; ch < field.numChannels; ++ch)
    {
        numRenderedInstances += field.instancesBufCounts[ch];
    }

    //printf("num grass: %u\n", numRenderedInstances);

#if 0
    // render each visible grass patch
    int numGrassVertices = 0;
    int numDrawCalls = 0;

    for (const uint32 idx : g_GrassMgr.GetVisPatchesIdxs())
    {
        const VertexBuffer<VertexGrass>& vb = g_GrassMgr.GetVertexBufByIdx(idx);
        const int                  numVerts = vb.GetVertexCount();

        // if no grass instances in this vb...
        if (numVerts <= 0)
            continue;
     
        pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
        pRender->Draw(numVerts, 0U);

        numGrassVertices += numVerts * 12;
        numDrawCalls++;
    }

    // calc render stats
    rndStat_.numDrawnVerts[GEOM_TYPE_GRASS] += numGrassVertices;
    rndStat_.numDrawnTris [GEOM_TYPE_GRASS] += numGrassVertices * 6;   // from one grass vertex we create 3 grass plane (6 triangles) 

    rndStat_.numDrawnInstances[GEOM_TYPE_GRASS] += numGrassVertices;
    rndStat_.numDrawCalls     [GEOM_TYPE_GRASS] += numDrawCalls;

#endif

    
}

//---------------------------------------------------------
//---------------------------------------------------------
void PrintDumpInstancesBatches(
    const char* groupName,
    const cvector<Render::InstanceBatch>& batches)
{
    assert(!StrHelper::IsEmpty(groupName));

    LogMsg(LOG, "dump instances batches (group: %s):", groupName);

    for (int i = 0; const Render::InstanceBatch& batch : batches)
    {
        printf("\t[%d] %-40s num_inst %-5d rs %d  bs %d  dss %d\n",
            i,
            batch.name,
            (int)batch.numInstances,
            (int)batch.rsId,
            (int)batch.bsId,
            (int)batch.dssId);
        i++;
    }
    printf("\n");
}

//---------------------------------------------------------
// Desc:   render all the instances from the input array
// Args:   - pRender:               a ptr to CRender class from the Render module
//         - instanceBatches:       array of instance batches
//                                  (one batch can represent many instances)
//         - startInstanceLocation: where start to get data in instances buffer
//                                  for currently rendered instances
//         - stat:                  a container for rendering statistic
//---------------------------------------------------------
void CGraphics::RenderInstanceGroups(
    const cvector<Render::InstanceBatch>& instanceBatches,
    const eGeomType geomType,
    UINT& startInstanceLocation)
{
    Render::CRender* pRender = pRender_;

    uint32 numDrawnVertices = 0;
    uint32 numDrawnTriangles = 0;
    uint32 numDrawnInstances = 0;
    uint32 numDrawCalls = 0;

#if 0
    if (geomType == GEOM_TYPE_OPAQUE)
        PrintDumpInstancesBatches("opaque", instanceBatches);
    else if (geomType == GEOM_TYPE_MASKED)
        PrintDumpInstancesBatches("masked", instanceBatches);
    else if (geomType == GEOM_TYPE_BLENDED)
        PrintDumpInstancesBatches("blended", instanceBatches);
#endif
#if 0
    if (geomType == GEOM_TYPE_MASKED)
        pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    else
        pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#endif

    for (const Render::InstanceBatch& batch : instanceBatches)
    {
        BindMaterial(batch.alphaClip, batch.rsId, batch.bsId, batch.dssId, batch.textures);

        pRender->RenderInstances(batch, startInstanceLocation);

        // stride idx in the instances buffer and accumulate render statistic
        startInstanceLocation  += (UINT)batch.numInstances;

        numDrawnVertices  += (batch.numInstances * batch.subset.vertexCount);
        numDrawnTriangles += (batch.numInstances * batch.subset.indexCount / 3);
        numDrawnInstances += batch.numInstances;
        numDrawCalls++;
    }


    // calc render statistic
    rndStat_.numDrawnVerts[GEOM_TYPE_ENTTS]     += numDrawnVertices;
    rndStat_.numDrawnVerts[geomType]            += numDrawnVertices;

    rndStat_.numDrawnTris[GEOM_TYPE_ENTTS]      += numDrawnTriangles;
    rndStat_.numDrawnTris[geomType]             += numDrawnTriangles;

    rndStat_.numDrawnInstances[GEOM_TYPE_ENTTS] += numDrawnInstances;
    rndStat_.numDrawnInstances[geomType]        += numDrawnInstances;

    rndStat_.numDrawCalls[GEOM_TYPE_ENTTS]      += numDrawCalls;
    rndStat_.numDrawCalls[geomType]             += numDrawCalls;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CGraphics::DepthPrepassInstanceGroup(
    const cvector<Render::InstanceBatch>& instanceBatches,
    const eGeomType geomType,
    UINT& startInstanceLocation)
{

    // if we have got masked type we just skip it
    if (geomType == GEOM_TYPE_MASKED)
    {
        for (const Render::InstanceBatch& batch : instanceBatches)
        {
            // stride idx in the instances buffer
            startInstanceLocation += (UINT)batch.numInstances;
        }
        return;
    }

    Render::CRender* pRender = pRender_;

    for (const Render::InstanceBatch& batch : instanceBatches)
    {
        //BindMaterial(pRender, batch.renderStates, batch.textures);
        pRender->DepthPrepassInstances(batch, startInstanceLocation);

        // stride idx in the instances buffer
        startInstanceLocation  += (UINT)batch.numInstances;
    }
}

//---------------------------------------------------------
// Desc:   render currently selected player's weapon
//---------------------------------------------------------
void CGraphics::RenderPlayerWeapon()
{
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderSkinnedModel(pEnttMgr_->playerSys_.GetActiveWeapon());
}

//---------------------------------------------------------
// Helper for work with quaternions
//---------------------------------------------------------
inline XMVECTOR QuatRotAxis(const XMVECTOR& axis, const float angle)
{
    return DirectX::XMQuaternionRotationAxis(axis, angle);
}

inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2)
{
    return DirectX::XMQuaternionMultiply(q1, q2);
}

//---------------------------------------------------------
// Desc:  render animated entity by input ID (using vertex skinning)
//---------------------------------------------------------
void CGraphics::RenderSkinnedModel(const EntityID enttId)
{
    if (enttId == 0)
        return;

    Render::CRender*      pRender       = pRender_;
    ECS::EntityMgr&       enttMgr       = *pEnttMgr_;
    ECS::TransformSystem& transformSys  = pEnttMgr_->transformSys_;
    const char*           enttName      = enttMgr.nameSys_.GetNameById(enttId);

    if (!enttMgr.animationSys_.HasAnimation(enttId))
    {
        LogErr(LOG, "you try to render entity (id: %" PRIu32 ", name: %s) as skinned (animated) but there is no skeleton/animation for it", enttId, enttName);
        return;
    }

    // prepare model's instance
    const ModelID       modelId = enttMgr.modelSys_.GetModelIdRelatedToEntt(enttId);
    const Model&   model   = g_ModelMgr.GetModelById(modelId);
    const MeshGeometry& meshes  = model.GetMeshes();

    const XMVECTOR quatRotZ = QuatRotAxis({ 0,0,1 }, +PIDIV2/2);
    const XMVECTOR quatRotY = QuatRotAxis({ 0,1,0 }, -PIDIV2);
    const XMVECTOR quat     = QuatMul(quatRotZ, quatRotY);
    const XMMATRIX R        = XMMatrixRotationQuaternion(quat);
    const XMMATRIX W        = XMMatrixTranspose(transformSys.GetWorld(enttId));

    //---------------------------------

    // get animation data for current entity
    SkeletonID skeletonId = 0;
    AnimationID animationId = 0;
    float timePos = 0;
    pEnttMgr_->animationSys_.GetData(enttId, skeletonId, animationId, timePos);

    // update bone transformations for this frame
    s_BoneTransforms.resize(MAX_NUM_BONES_PER_CHARACTER, DirectX::XMMatrixIdentity());
    AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(skeletonId);

    skeleton.GetFinalTransforms(animationId, timePos, s_BoneTransforms);

    //---------------------------------

    // update constant buffers
    pRender->UpdateCbBoneTransforms(s_BoneTransforms);
    pRender->UpdateCbWorldInvTranspose(MathHelper::InverseTranspose(W));
    pRender->UpdateCbWorldAndViewProj(W, DirectX::XMMatrixTranspose(viewProj_));

    // bind a shader, prepare AI for rendering
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindShaderByName("SkinnedMeshShader");
    pRender->BindIB(meshes.ib_.Get(), DXGI_FORMAT_R32_UINT);

    const UINT stride0 = sizeof(Vertex3D);
    const UINT offset0 = 0;

    const UINT sizeofUINT4 = 16;
    const UINT stride1 = sizeof(XMFLOAT4) + sizeofUINT4;
    const UINT offset1 = 0;
    
    ID3D11Buffer*   vbs[2] = { meshes.vb_.Get(), skeleton.GetBonesVB() };
    const UINT  strides[2] = { stride0, stride1 };
    const UINT  offsets[2] = { offset0, offset1 };

    ID3D11DeviceContext* pCtx = pRender->GetContext();
    pCtx->IASetVertexBuffers(0, 2, vbs, strides, offsets);

    //---------------------------------

    // render each mesh of the model separately so we will receive a complete image
    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        // bind material and update a const buffer with colors
        const Subset& mesh  = meshes.subsets_[i];
        const Material& mat = g_MaterialMgr.GetMatById(mesh.materialId);

        BindMaterial(mat);
        pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

        pCtx->DrawIndexed(mesh.indexCount, mesh.indexStart, mesh.vertexStart);
    }
}

//---------------------------------------------------------
// Desc:   render particles onto the screen
//---------------------------------------------------------
void CGraphics::RenderParticles()
{
    Render::CRender&                render        = *pRender_;
    ECS::ParticleSystem&            particleSys   = pEnttMgr_->particleSys_;
    const ECS::ParticlesRenderData& particlesData = particleSys.GetParticlesToRender();
    const int                       numParticles  = (int)particlesData.particles.size();

    if (numParticles == 0)
        return;

    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // for particles we bind only vertex buffer
    const VertexBuffer<BillboardSprite>& vb = g_ModelMgr.GetBillboardsBuffer();
    render.BindVB(vb.GetAddrOf(), vb.GetStride(), 0);

    // go through particles emitters and render its particles
    int numDrawnVertices = 0;
    int numDrawCalls = 0;

    // render each type of particles separately
    for (index i = 0; i < particlesData.materialIds.size(); ++i)
    {
        const Material& mat = g_MaterialMgr.GetMatById(particlesData.materialIds[i]);
        BindMaterial(mat);
        render.BindShaderById(mat.shaderId);

        render.Draw(particlesData.numInstances[i], particlesData.baseInstance[i]);

        numDrawnVertices += (particlesData.numInstances[i] * 4);
        numDrawCalls++;
    }

    // calc some render states
    rndStat_.numDrawnVerts[GEOM_TYPE_PARTICLE]     = numDrawnVertices * 4;    // each particle vertex become a billboard quad of 4 vertices
    rndStat_.numDrawnTris[GEOM_TYPE_PARTICLE]      = numDrawnVertices * 2;    // each particle consists of 2 triangles (one quad)
    rndStat_.numDrawnInstances[GEOM_TYPE_PARTICLE] = numParticles;            // how many separate particles we have rendered
    rndStat_.numDrawCalls[GEOM_TYPE_PARTICLE]      = numDrawCalls;
}

//---------------------------------------------------------
// Desc:  render sky shape (skybox, skysphere, skydome) onto the screen
//---------------------------------------------------------
void CGraphics::RenderSkyDome()
{
    // check if we at least have a sky entity
    const EntityID skyEnttId = pEnttMgr_->nameSys_.GetIdByName("sky");

    // if we haven't any sky entity
    if (skyEnttId == INVALID_ENTITY_ID)
        return;

    const SkyModel& sky = g_ModelMgr.GetSky();
    Render::SkyInstance instance;

    instance.vertexStride = sky.GetVertexStride();
    instance.pVB          = sky.GetVB().Get();
    instance.pIB          = sky.GetIB().Get();
    instance.indexCount   = sky.GetNumIndices();
    //instance.colorCenter  = sky.GetColorCenter();
    //instance.colorApex    = sky.GetColorApex();

    // setup IA and bind sky material
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    BindMaterialById(sky.GetMaterialId());

    // compute a worldViewProj matrix for the sky instance
    const XMFLOAT3 skyOffset     = pEnttMgr_->transformSys_.GetPosition(skyEnttId);
    const XMFLOAT3 eyePos        = pEnttMgr_->cameraSys_.GetPos(currCameraId_);
    const XMFLOAT3 tr            = skyOffset + eyePos;
    const XMMATRIX world         = DirectX::XMMatrixTranslation(tr.x, tr.y, tr.z);
    const XMMATRIX worldViewProj = DirectX::XMMatrixTranspose(world * viewProj_);

    pRender_->RenderSkyDome(instance, worldViewProj);

    // calc render stats
    rndStat_.numDrawnVerts[GEOM_TYPE_SKY] = sky.GetNumVertices();
    rndStat_.numDrawnTris [GEOM_TYPE_SKY] = sky.GetNumIndices() / 3;

    rndStat_.numDrawnInstances[GEOM_TYPE_SKY]++;
    rndStat_.numDrawCalls     [GEOM_TYPE_SKY]++;
}

//---------------------------------------------------------
// Desc:  render sky clouds onto the screen
//---------------------------------------------------------
void CGraphics::RenderSkyClouds()
{
    const SkyPlane& skyPlane             = g_ModelMgr.GetSkyPlane();
    const VertexBuffer<VertexPosTex>& vb = skyPlane.GetVB();
    const IndexBuffer<uint16>&        ib = skyPlane.GetIB();

    // bind buffers, material, and shader
    pRender_->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender_->BindIB(ib.Get(), DXGI_FORMAT_R16_UINT);
    pRender_->BindShaderByName("SkyCloudShader");
    BindMaterialById(skyPlane.GetMaterialId());

    // render
    GetContext()->DrawIndexed(skyPlane.GetNumIndices(), 0, 0);

    // calc render stats
    rndStat_.numDrawnVerts[GEOM_TYPE_SKY_PLANE] = vb.GetVertexCount();
    rndStat_.numDrawnTris [GEOM_TYPE_SKY_PLANE] = ib.GetIndexCount() / 3;

    rndStat_.numDrawnInstances[GEOM_TYPE_SKY_PLANE]++;
    rndStat_.numDrawCalls[GEOM_TYPE_SKY_PLANE]++;
}

//---------------------------------------------------------
// Desc:  execute depth pre-pass for terrain's geometry
//---------------------------------------------------------
void CGraphics::TerrainDepthPrepass()
{
    Render::CRender* pRender = pRender_;

    // prepare the terrain instance
    Terrain& terrain = g_ModelMgr.GetTerrain();

    // prepare IA stage and shaders
    pRender->BindShaderByName("TerrainDepthPrepass");
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pRender->BindVB(terrain.GetVbAddr(), terrain.GetVertexStride(), 0);
    pRender->BindIB(terrain.GetIB(), DXGI_FORMAT_R32_UINT);


    const TerrainLodMgr& lodMgr      = terrain.GetLodMgr();
    const int terrainLen             = terrain.GetTerrainLength();
    const int numPatchesPerSide      = terrain.GetNumPatchesPerSide();
    const float invNumPatchesPerSide = 1.0f / numPatchesPerSide;
    const int patchSize              = lodMgr.GetPatchSize();

    // render each visible patch (sector) of the terrain
    for (const int idx : terrain.GetAllVisiblePatches())
    {
        const TerrainLodMgr::PatchLod& plod = lodMgr.GetPatchLodInfo(idx);

        UINT baseIndex = 0;
        UINT indexCount = 0;
        terrain.GetLodInfoByPatch(plod, baseIndex, indexCount);

        const int patchZ = (int)(idx * invNumPatchesPerSide);  // const int patchZ = idx / numPatchesPerSide;
        const int patchX = idx & (numPatchesPerSide - 1);      // const int patchX = idx % numPatchesPerSide;
        const int z      = patchZ * (patchSize - 1);
        const int x      = patchX * (patchSize - 1);

        const UINT baseVertex = (UINT)(z * terrainLen + x);

        // draw a patch
        pRender->DrawIndexed(indexCount, baseIndex, baseVertex);
    }
}

//---------------------------------------------------------
// Desc:  render each terrain's patch (sector) by its index
//---------------------------------------------------------
void RenderTerrainPatches(
    Terrain& terrain,
    Render::CRender* pRender,
    UINT& numDrawnTriangles,
    const cvector<int>& patchesIdxs)
{
    assert(pRender);

    const TerrainLodMgr& lodMgr      = terrain.GetLodMgr();
    const int terrainLen             = terrain.GetTerrainLength();
    const int numPatchesPerSide      = terrain.GetNumPatchesPerSide();
    const float invNumPatchesPerSide = 1.0f / numPatchesPerSide;
    const int patchSize              = lodMgr.GetPatchSize();

    for (const int idx : patchesIdxs)
    {
        const TerrainLodMgr::PatchLod& plod = lodMgr.GetPatchLodInfo(idx);

        UINT baseIndex = 0;
        UINT indexCount = 0;
        terrain.GetLodInfoByPatch(plod, baseIndex, indexCount);

        const int patchZ = (int)(idx * invNumPatchesPerSide);  // const int patchZ = idx / numPatchesPerSide;
        const int patchX = idx & (numPatchesPerSide - 1);      // const int patchX = idx % numPatchesPerSide;
        const int z      = patchZ * (patchSize - 1);
        const int x      = patchX * (patchSize - 1);

        const UINT baseVertex = (UINT)(z * terrainLen + x);

        // draw a patch
        pRender->DrawIndexed(indexCount, baseIndex, baseVertex);

        numDrawnTriangles += (indexCount / 3);
    }
}

//---------------------------------------------------------
// Desc:   render the terrain onto the screen
//---------------------------------------------------------
void CGraphics::RenderTerrain()
{
    Terrain&    terrain = g_ModelMgr.GetTerrain();
    const Material& mat = g_MaterialMgr.GetMatById(terrain.materialID_);

    // bind materials
    BindMaterial(mat);
    pRender_->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);
    pRender_->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // bind textures to VS (because we need to sample terrain's splat map in some VS)
    ID3D11ShaderResourceView* texViews[NUM_TEXTURE_TYPES]{ nullptr };

    g_TextureMgr.GetTexViewsByIds(mat.texIds, NUM_TEXTURE_TYPES, texViews);
    pRender_->GetContext()->VSSetShaderResources(100U, NUM_TEXTURE_TYPES, texViews);

    // bind buffers
    pRender_->BindVB(terrain.GetVbAddr(), terrain.GetVertexStride(), 0);
    pRender_->BindIB(terrain.GetIB(), DXGI_FORMAT_R32_UINT);


    UINT numDrawnTris    = 0;
    size numDrawnPatches = 0;
    size numDrawCalls    = 0;

    const cvector<int>& highDetailed = terrain.GetHighDetailedPatches();
    const cvector<int>& midDetailed  = terrain.GetMidDetailedPatches();
    const cvector<int>& lowDetailed  = terrain.GetLowDetailedPatches();

    pRender_->BindShaderByName("TerrainShader");
    RenderTerrainPatches(terrain, pRender_, numDrawnTris, highDetailed);

    pRender_->BindShaderByName("TerrainMidLodShader");
    RenderTerrainPatches(terrain, pRender_, numDrawnTris, midDetailed);

    pRender_->BindShaderByName("TerrainLowLodShader");
    RenderTerrainPatches(terrain, pRender_, numDrawnTris, lowDetailed);


    // gather some rendering stats
    numDrawnPatches += highDetailed.size();
    numDrawCalls    += highDetailed.size();

    numDrawnPatches += midDetailed.size();
    numDrawCalls    += midDetailed.size();

    numDrawnPatches += lowDetailed.size();
    numDrawCalls    += lowDetailed.size();
    
    rndStat_.numDrawnTris[GEOM_TYPE_TERRAIN]      = (uint32)numDrawnTris;
    rndStat_.numDrawnInstances[GEOM_TYPE_TERRAIN] = (uint32)numDrawnPatches;
    rndStat_.numDrawCalls[GEOM_TYPE_TERRAIN]      = (uint32)numDrawCalls;

    pSysState_->numDrawnTerrainPatches            = (uint32)numDrawnPatches;
    pSysState_->numCulledTerrainPatches           = (uint32)(terrain.GetNumAllPatches() - numDrawnPatches);
}

//---------------------------------------------------------

void CGraphics::RenderDebugShapes()
{
    // reset all the render states to default before lines rendering
    ResetRenderStatesToDefault();

    DbgShapeRender renderer;
    renderer.Render(pRender_, pSysState_->isGameMode);
}

//---------------------------------------------------------
// Desc:  bind a shader according to the input post effect's type
//---------------------------------------------------------
void BindPostFxShader(Render::CRender* pRender, const ePostFxType fxType)
{
    assert(pRender);

    switch (fxType)
    {
        case POST_FX_VISUALIZE_DEPTH:
            pRender->BindShaderByName("DepthResolveShader");
            pRender->GetContext()->Draw(3, 0);
            pRender->BindShaderByName("VisualizeDepthShader");
            break;

        case POST_FX_GRAYSCALE:
        case POST_FX_INVERT_COLORS:
        case POST_FX_BRIGHT_CONTRAST_ADJ:
        case POST_FX_SEPIA:
        case POST_FX_CHROMATIC_ABERRATION:
        case POST_FX_COLOR_TINT:
        case POST_FX_VIGNETTE_EFFECT:
        case POST_FX_BLOOM_BRIGHT_EXTRACT:
        case POST_FX_EDGE_DETECTION:
        case POST_FX_POSTERIZATION:
        case POST_FX_FILM_GRAIN:
        case POST_FX_CRT_SCANLINES:
        case POST_FX_PIXELATION:
        case POST_FX_COLOR_SHIFT:
        case POST_FX_NEGATIVE_GLOW:
        case POST_FX_THERMAL_VISION:
        case POST_FX_NIGHT_VISION:
        case POST_FX_HEAT_DISTORTION:
        case POST_FX_SHOCKWAVE_DISTORTION:
        case POST_FX_FROST_GLASS_BLUR:
        case POST_FX_OLD_TV_DISTORTION:
        case POST_FX_COLOR_SPLIT:
        case POST_FX_RADIAL_BLUR:
        case POST_FX_SWIRL_DISTORTION:
        case POST_FX_GLITCH:
        case POST_FX_DITHERING_ORDERED:
        {
            const char* shaderName = g_PostFxShaderName[fxType];
            pRender->BindShaderByName(shaderName);
            break;
        }

        // gaussian blur we do in 2 passes
        case POST_FX_GAUSSIAN_BLUR:
        {
            const char* shaderName = g_PostFxShaderName[fxType];
            pRender->BindShaderByName(shaderName);
            break;
        }

        //-----------------------------------

        default:
            LogErr(LOG, "wrong post effect type (maybe you add a new postFx but forgot to add a new case here?): %d", (int)fxType);
            return;
    }
}

//---------------------------------------------------------
// Desc:  if we have only one post effect in the post process passes queue we
//        can render it directly into the swap chain's RTV
//---------------------------------------------------------
void CGraphics::RenderPostFxOnePass()
{
    Render::D3DClass&    d3d = GetD3D();
    ID3D11DeviceContext* pCtx = GetContext();

    // bind dst render target and src shader resource view
    assert(d3d.pSwapChainRTV_     && "swap chain RTV is wrong");
    assert(d3d.postFxsPassSRV_[0] && "resolved SRV is wrong");

    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    BindPostFxShader(pRender_, postFxsQueue_[0]);
    pCtx->Draw(3, 0);

    // bind a depth stencil view back
    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:  when we have multiple post process passes we have to flip btw
//        two render targets each time when render a pass;
//        for the final pass we do rendering into the swap chain't RTV
//---------------------------------------------------------
void CGraphics::RenderPostFxMultiplePass()
{
    Render::D3DClass&    d3d  = GetD3D();
    ID3D11DeviceContext* pCtx = GetContext();

    pCtx->OMSetRenderTargets(1, &d3d.postFxsPassRTV_[1], nullptr);
    pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    int i = 0;
    int lastDstIdx = 0;

    for (i = 0; i < numPostFxsInQueue_ - 1; ++i)
    {
        // by this index we will get a source SRV for the last post process pass
        lastDstIdx = (i % 2 == 0);

        BindPostFxShader(pRender_, postFxsQueue_[i]);
        pCtx->Draw(3, 0);

        // flip render targets and shader resource views
        // (one target becomes a dst, and another becomes a src)
        const int nextTargetIdx = (i % 2 != 0);
        const int nextSrcIdx    = (i % 2 == 0);

        pCtx->OMSetRenderTargets(1, &d3d.postFxsPassRTV_[nextTargetIdx], nullptr);
        pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[nextSrcIdx]);
    }

    // final pass we render directly into swap chain's RTV
    BindPostFxShader(pRender_, postFxsQueue_[i]);
    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    pCtx->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[lastDstIdx]);
    pCtx->Draw(3, 0);

    // bind depth stencil back
    pCtx->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:   visualize values from the depth buffer (we do it after usual rendering)
//---------------------------------------------------------
void CGraphics::VisualizeDepthBuffer()
{
    Render::D3DClass&    d3d      = GetD3D();
    ID3D11DeviceContext* pCtx = GetContext();

    // unbind depth before depth visualization
    d3d.UnbindDepthBuffer();

    if (d3d.IsEnabled4xMSAA())
    {
        ID3D11ShaderResourceView* pDepthMSAASRV = d3d.GetDepthSRV();
        pCtx->PSSetShaderResources(TEX_SLOT_DEPTH_MSAA, 1, &pDepthMSAASRV);

        pRender_->BindShaderByName("DepthResolveShader");
        pCtx->Draw(3, 0);
    }
    else
    {
        // setup depth SRV
        ID3D11ShaderResourceView* pDepthSRV = d3d.GetDepthSRV();
        pCtx->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &pDepthSRV);

        pRender_->BindShaderByName("VisualizeDepthShader");
        pCtx->Draw(3, 0);
    }

    // after rendering we bind depth buffer again
    d3d.BindDepthBuffer();
}

//---------------------------------------------------------
// Desc:  convert light source data from the ECS into Render format
//        (they are the same so we simply need to copy data)
//---------------------------------------------------------
void CGraphics::SetupLightsForFrame(Render::PerFrameData& outData)
{
    ECS::LightSystem&       lightSys         = pEnttMgr_->lightSys_;

    const ECS::DirLights&   dirLights        = lightSys.GetDirLights();
    const ECS::PointLights& pointLights      = lightSys.GetPointLights();
    const ECS::SpotLights&  spotLights       = lightSys.GetSpotLights();

    const size numAllDirLights               = lightSys.GetNumDirLights();
    const size numAllPointLights             = lightSys.GetNumPointLights();
    const size numAllSpotLights              = lightSys.GetNumSpotLights();

    const cvector<EntityID>& visPointLights  = pEnttMgr_->renderSys_.GetVisiblePointLights();
    const size numVisPointLights             = visPointLights.size();

    cvector<ECS::PointLight>& activePointL    = s_LightTmpData.activePointLights;
    cvector<XMFLOAT3>&        pointLPositions = s_LightTmpData.pointLightsPositions;
    cvector<ECS::SpotLight>&  spotLData       = s_LightTmpData.spotLightsData;
    cvector<XMVECTOR>&        dirLDirections  = s_LightTmpData.dirLightsDirections;

    // gather data of currently visible and active point light sources
    lightSys.GetPointLightsData(
        visPointLights.data(),
        visPointLights.size(),
        activePointL,
        pointLPositions);

    lightSys.GetSpotLightsData(
        spotLights.ids.data(),
        numAllSpotLights,
        spotLData,
        s_LightTmpData.spotLightsPositions,
        s_LightTmpData.spotLightsDirections);

    const int numActivePointLights = (int)activePointL.size();
    const int numActiveSpotLights  = (int)spotLData.size();

    pSysState_->numVisiblePointLights = (uint32)numActivePointLights;
    pSysState_->numVisibleSpotlights  = (uint32)numActiveSpotLights;

    // prepare enough memory for lights buffer
    outData.ResizeLightData((int)numAllDirLights, numActivePointLights, numActiveSpotLights);


    // ----------------------------------------------------
    // prepare point lights data

    if (numActivePointLights > 0)
    {
        // store light properties, range, and attenuation
        for (int i = 0; i < numActivePointLights; ++i)
        {
            outData.pointLights[i].ambient  = activePointL[i].ambient;
            outData.pointLights[i].diffuse  = activePointL[i].diffuse;
            outData.pointLights[i].specular = activePointL[i].specular;
            outData.pointLights[i].att      = activePointL[i].att;
            outData.pointLights[i].range    = activePointL[i].range;
}

        // store positions
        for (int i = 0; i < numActivePointLights; ++i)
        {
            outData.pointLights[i].position = pointLPositions[i];
        }
    }


    // ----------------------------------------------------
    // prepare data of directed lights

    for (index i = 0; i < numAllDirLights; ++i)
    {
        outData.dirLights[i].ambient  = dirLights.data[i].ambient;
        outData.dirLights[i].diffuse  = dirLights.data[i].diffuse;
        outData.dirLights[i].specular = dirLights.data[i].specular;
    }

    pEnttMgr_->transformSys_.GetDirections(
        dirLights.ids.data(),
        numAllDirLights,
        dirLDirections);

    for (int i = 0; XMVECTOR& direction : dirLDirections)
    {
        direction = DirectX::XMVector3Normalize(direction);
        DirectX::XMStoreFloat3(&outData.dirLights[i].direction, direction);
        ++i;
    }


    // ----------------------------------------------------
    // prepare data of spot lights

    if (numActiveSpotLights > 0)
    {
        for (index i = 0; i < numActiveSpotLights; ++i)
        {
            outData.spotLights[i].ambient   = spotLData[i].ambient;
            outData.spotLights[i].diffuse   = spotLData[i].diffuse;
            outData.spotLights[i].specular  = spotLData[i].specular;
            outData.spotLights[i].range     = spotLData[i].range;
            outData.spotLights[i].spot      = spotLData[i].spot;
            outData.spotLights[i].att       = spotLData[i].att;
        }

        for (int i = 0; const XMFLOAT3 & pos : s_LightTmpData.spotLightsPositions)
            outData.spotLights[i++].position = pos;

        for (int i = 0; XMFLOAT3& dir : s_LightTmpData.spotLightsDirections)
        {
            // normalize its direction here so we don't need to do it in our HLSL
            XMFloat3Normalize(dir);
            outData.spotLights[i++].direction = dir;
        }
    }
}


//==================================================================================
// POST EFFECTS CONTROL
//==================================================================================

//---------------------------------------------------------
// Desc:  push a new post fx at the end of post effects rendering queue
//---------------------------------------------------------
void CGraphics::PushPostFx(const ePostFxType type)
{
    // check input args
    if (numPostFxsInQueue_ >= MAX_NUM_POST_EFFECTS)
    {
        LogErr(LOG, "can't push post fx (%d): you can't use more than %d post effects at once", (int)type, MAX_NUM_POST_EFFECTS);
        return;
    }

    if (type >= NUM_POST_EFFECTS)
    {
        LogErr(LOG, "lol, your post effect type is wrong (%d)", (int)type);
        return;
    }


    // push back this post fx
    postFxsQueue_[numPostFxsInQueue_] = type;
    numPostFxsInQueue_++;
}

//---------------------------------------------------------
// Desc:  remove post fx by input order number from the post-effects queue
//---------------------------------------------------------
void CGraphics::RemovePostFx(const uint8 orderNum)
{
    if (orderNum >= numPostFxsInQueue_)
    {
        LogErr(LOG, "wrong order num (%d)", (int)orderNum);
    }

    // shirt left
    for (int i = (int)orderNum; i < numPostFxsInQueue_-1; ++i)
        postFxsQueue_[i] = postFxsQueue_[i + 1];

    numPostFxsInQueue_--;
}

//---------------------------------------------------------
// Desc:  reset all the render states:
//        raster state, blend state, depth-stencil state, etc. to default
//---------------------------------------------------------
void CGraphics::ResetRenderStatesToDefault()
{
    pRender_->SwitchAlphaClipping(false);
    pRender_->ResetRenderStates();
}


//==================================================================================
// MODEL PREVIEW (for model editor, or model screenshot tool)
//==================================================================================

//---------------------------------------------------------
// Desc:   create and initialize a frame buffer for models rendering
//         (it is mainly used in the editor)
//---------------------------------------------------------
bool CGraphics::InitModelFrameBuf(const uint width, const uint height)
{
    // setup params for the frame buffer
    FrameBufSpec fbSpec;
    fbSpec.width       = width;
    fbSpec.height      = height;
    fbSpec.format      = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    fbSpec.screenNear  = GetD3D().GetNearZ();
    fbSpec.screenDepth = GetD3D().GetFarZ();

    modelPreviewRndParams_.frameBufWidth  = width;
    modelPreviewRndParams_.frameBufHeight = height;


    if (pModelFrameBuf_ == nullptr)
        pModelFrameBuf_ = new FrameBuffer();

    if (!pModelFrameBuf_->Init(fbSpec))
    {
        LogErr(LOG, "can't init a frame buffer for model rendering");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  render a model into responsible frame buffer;
//        later we can use a texture with rendered model for any our purposes
//---------------------------------------------------------
bool CGraphics::RenderModelIntoFrameBuf()
{
    FrameBuffer* pFB = pModelFrameBuf_;
    if (!pFB)
    {
        LogErr(LOG, "you have to create and init a model's frame buffer");
        return false;
    }

    Render::CRender* pRender  = pRender_;

    // reset render states to prevent rendering bugs
    pRender->ResetRenderStates();

    // prepare model's instance
    const ModelPreviewRenderParams& params = modelPreviewRndParams_;
    Model&                          model  = g_ModelMgr.GetModelById(params.modelId);
    const MeshGeometry&             meshes = model.GetMeshes();

    // calc model world matrix
    const XMMATRIX S = XMMatrixScaling             (params.modelScale, params.modelScale, params.modelScale);
    const XMMATRIX R = XMMatrixRotationRollPitchYaw(params.modelRot.x, params.modelRot.y, params.modelRot.z);
    const XMMATRIX T = XMMatrixTranslation         (params.modelPos.x, params.modelPos.y, params.modelPos.z);
    const XMMATRIX W = DirectX::XMMatrixTranspose(S * R * T);

    // transform camera and calc view matrix 
    const XMVECTOR eyePos   = { params.camPos.x, params.camPos.y, params.camPos.z };
    const XMMATRIX VR       = XMMatrixRotationRollPitchYaw(params.camRot.x, params.camRot.y, params.camRot.z);

    const XMVECTOR focusPos = { 0,0,0 };
    const XMVECTOR upDir    = { 0,1,0 };
    XMFLOAT3 trCamPos;
    XMStoreFloat3(&trCamPos, XMVector3Transform(eyePos, VR));

    const XMMATRIX V = XMMatrixLookAtLH(XMVector3Transform(eyePos, VR), focusPos, upDir);


    // calc projection / ortho matrix
    XMMATRIX P;
    const float fov = DEG_TO_RAD(75);
    const float zn  = pFB->GetNearZ();
    const float zf  = pFB->GetFarZ();

    if (!params.useOrthoMatrix)
        P = XMMatrixPerspectiveFovLH(fov, pFB->GetAspect(), zn, zf);
    else
        P = XMMatrixOrthographicLH(fov, params.orthoViewHeight, zn, zf);

   
    // save pos for later restoring
    const XMFLOAT3 prevCamPos = pRender->GetCameraPos();

    pRender->UpdateCbWorldInvTranspose(MathHelper::InverseTranspose(W));
    pRender->UpdateCbWorldAndViewProj(W, DirectX::XMMatrixTranspose(V * P));
    pRender->UpdateCbCameraPos({ trCamPos.x, trCamPos.y, trCamPos.z });

    // prepare and bind a frame buffer
    const Vec3& col = params.bgColor;
    pFB->ClearBuffers(col.r, col.g, col.b, 1);
    pFB->Bind();

    // bind a shader, prepare IA for rendering
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindShaderByName("DebugModelShader");
    pRender->BindVB(meshes.vb_.GetAddrOf(), meshes.vb_.GetStride(), 0);
    pRender->BindIB(meshes.ib_.Get(), DXGI_FORMAT_R32_UINT);


    // render each mesh of the model separately so we will receive a complete image
    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        const Subset& mesh  = meshes.subsets_[i];
        const Material& mat = g_MaterialMgr.GetMatById(mesh.materialId);

        BindMaterial(mat);
        pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);
        pRender->DrawIndexed(mesh.indexCount, mesh.indexStart, mesh.vertexStart);
    }

    // reset camera position
    pRender->UpdateCbCameraPos(prevCamPos);

    // reset to default render target and viewport
    GetD3D().ResetBackBufferRenderTarget();
    GetD3D().ResetViewport();

    return true;
}

//---------------------------------------------------------
// Desc:  setup a model preview's parameter by input type
//
// NOTE:  input param type is casted to uint8 to we
//        need to cast it to our enum
//---------------------------------------------------------
void CGraphics::SetModelPreviewParam(const uint8 param, const float val)
{
    const eModelPreviewParams type   = (eModelPreviewParams)param;
    ModelPreviewRenderParams& params = modelPreviewRndParams_;

    if (type >= NUM_MODEL_PREVIEW_PARAMS)
    {
        LogErr(LOG, "invalid input parameter type: %d", (int)param);
        return;
    }


     switch (param)
    {
        // which model to render
        case MODEL_ID:                  params.modelId = (ModelID)val;      break;

        case MODEL_POS_X:               params.modelPos.x = val;            break;
        case MODEL_POS_Y:               params.modelPos.y = val;            break;
        case MODEL_POS_Z:               params.modelPos.z = val;            break;

        case MODEL_ROT_X:               params.modelRot.x = val;            break;
        case MODEL_ROT_Y:               params.modelRot.y = ClampYaw(val);  break;
        case MODEL_ROT_Z:               params.modelRot.z = val;            break;
        case MODEL_SCALE:               params.modelScale = val;            break;

        case CAMERA_POS_X:              params.camPos.x = val;              break;
        case CAMERA_POS_Y:              params.camPos.y = val;              break;
        case CAMERA_POS_Z:              params.camPos.z = val;              break;
        case CAMERA_ROT_X:              params.camRot.x = ClampPitch(val);  break;
        case CAMERA_ROT_Y:              params.camRot.y = ClampYaw(val);    break;
        case CAMERA_ROT_Z:              params.camRot.z = val;              break;

        case FRAME_BUF_WIDTH:           params.frameBufWidth = (uint)val;   break;
        case FRAME_BUF_HEIGHT:          params.frameBufHeight = (uint)val;  break;

        case ORTHO_MATRIX_VIEW_HEIGHT:  params.orthoViewHeight = val;       break;
        case USE_ORTHO_MATRIX:          params.useOrthoMatrix = (bool)val;  break;

        case BG_COLOR_R:                params.bgColor.r = val;             break;
        case BG_COLOR_G:                params.bgColor.g = val;             break;
        case BG_COLOR_B:                params.bgColor.b = val;             break;

        default:
            LogErr(LOG, "invalid input parameter type: %d", (int)param);
    }
}

//---------------------------------------------------------
// Desc:  get a model preview's parameter by input type
//
// NOTE:  input param type is casted to uint8 to we
//        need to cast it to our enum
//---------------------------------------------------------
float CGraphics::GetModelPreviewParam(const uint8 param) const
{
    const eModelPreviewParams       type   = (eModelPreviewParams)param;
    const ModelPreviewRenderParams& params = modelPreviewRndParams_;

    if (type >= NUM_MODEL_PREVIEW_PARAMS)
    {
        LogErr(LOG, "invalid input parameter type: %d", (int)param);
        return FLT_MIN;
    }


    switch (param)
    {
        // which model to render
        case MODEL_ID:                  return (float)params.modelId;

        case MODEL_POS_X:               return params.modelPos.x;
        case MODEL_POS_Y:               return params.modelPos.y;
        case MODEL_POS_Z:               return params.modelPos.z;
        case MODEL_ROT_X:               return params.modelRot.x;
        case MODEL_ROT_Y:               return params.modelRot.y;
        case MODEL_ROT_Z:               return params.modelRot.z;
        case MODEL_SCALE:               return params.modelScale;

        case CAMERA_POS_X:              return params.camPos.x;
        case CAMERA_POS_Y:              return params.camPos.y;
        case CAMERA_POS_Z:              return params.camPos.z;
        case CAMERA_ROT_X:              return params.camRot.x;
        case CAMERA_ROT_Y:              return params.camRot.y;
        case CAMERA_ROT_Z:              return params.camRot.z;

        case FRAME_BUF_WIDTH:           return (float)params.frameBufWidth;
        case FRAME_BUF_HEIGHT:          return (float)params.frameBufHeight;

        case ORTHO_MATRIX_VIEW_HEIGHT:  return (float)params.orthoViewHeight;
        case USE_ORTHO_MATRIX:          return (float)params.useOrthoMatrix;

        case BG_COLOR_R:                return params.bgColor.r;
        case BG_COLOR_G:                return params.bgColor.g;
        case BG_COLOR_B:                return params.bgColor.b;

        default:
            LogErr(LOG, "invalid input parameter type: %d", (int)param);
    }

    return FLT_MIN;
}



//==================================================================================
//                               COLLISIONS
//==================================================================================

//---------------------------------------------------------
// Desc:  calculate intersection between a ray and some entity or terrain;
//        the ray goes from camera pos to coordinate calculated
//        by input screen sx, sy pixel coordinate;
//
// Out:   - outData:  output container for calculated intersection data
//---------------------------------------------------------
bool CGraphics::GetRayIntersectionData(
    const int sx,
    const int sy,
    IntersectionData& outData)
{
    using namespace DirectX;

    // reset output data
    memset(&outData, 0, sizeof(outData));

    ECS::EntityMgr& enttMgr   = *pEnttMgr_;
    const EntityID  playerId  = enttMgr.nameSys_.GetIdByName("player");

    const ECS::CameraSystem& camSys = enttMgr.cameraSys_;

    const XMFLOAT3  camPos  = camSys.GetPos(currCameraId_);
    const XMMATRIX& proj    = camSys.GetProj(currCameraId_);
    const XMMATRIX& invView = camSys.GetInverseView(currCameraId_);

    const float xndc = (+2.0f * sx / GetD3D().GetWindowWidth()  - 1.0f);
    const float yndc = (-2.0f * sy / GetD3D().GetWindowHeight() + 1.0f);

    // compute picking ray in view space
    const float vx = xndc / proj.r[0].m128_f32[0];
    const float vy = yndc / proj.r[1].m128_f32[1];
    const XMVECTOR rayDirV = { vx, vy, 1, 0 };

    // ray definition (origin, direction) in world space...
    XMVECTOR rayOrigW = { camPos.x, camPos.y, camPos.z, 1 };
    XMVECTOR rayDirW  = XMVector3Normalize(XMVector3TransformNormal(rayDirV, invView));     // supposed to take a vec (w == 0)

    // ...and in local space
    XMVECTOR rayOrigL = {0,0,0};
    XMVECTOR rayDirL  = {0,0,1};


    // the distance along the ray where the intersection occurs
    float tmin = FLT_MAX;  

    // go through each visible entt and check if we have an intersection with it
    for (const EntityID enttId : enttMgr.renderSys_.GetAllVisibleEntts())
    {
        if (enttId == playerId)
            continue;

        RayEnttTest(enttId, rayOrigW, rayDirW, tmin, outData, rayOrigL, rayDirL);
    }

    // if we didn't intersect any entity...
    if (outData.enttId == 0)
        return false;

    GatherIntersectionData(rayOrigL, rayDirL, rayOrigW, rayDirW, tmin, outData);
    return true;
}

//---------------------------------------------------------
// Desc:  calculate a normal vector by 3 input positions
//---------------------------------------------------------
Vec3 CalcNormalVec(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2)
{
    XMVECTOR pos0 = XMLoadFloat3(&p0);
    XMVECTOR pos1 = XMLoadFloat3(&p1);
    XMVECTOR pos2 = XMLoadFloat3(&p2);

    XMVECTOR e0 = pos1 - pos0;
    XMVECTOR e1 = pos2 - pos0;

    XMVECTOR n = XMVector3Normalize(XMVector3Cross(e1, e0));

    return ToVec3(n);
}

//---------------------------------------------------------
// Desc:  ray/entity test (test ray agains each mesh of entity's model)
//---------------------------------------------------------
void CGraphics::RayEnttTest(
    const EntityID enttId,
    const XMVECTOR& rayOrigW,
    const XMVECTOR& rayDirW,
    float& tmin,
    IntersectionData& outData,
    XMVECTOR& outRayOrigL,
    XMVECTOR& outRayDirL)
{
    const ModelID    modelId = pEnttMgr_->modelSys_.GetModelIdRelatedToEntt(enttId);
    const Model&  model = g_ModelMgr.GetModelById(modelId);

    // transform ray to model's local space
    const XMMATRIX& invWorld = pEnttMgr_->transformSys_.GetInvWorld(enttId);
    const XMVECTOR  rayOrigL = XMVector3Transform(rayOrigW, invWorld);
    const XMVECTOR  rayDirL  = XMVector3Normalize(XMVector3TransformNormal(rayDirW, invWorld));

    // the length of the ray from origin to the intersection point with the AABB
    float dist = 0;

    // ray/AABB test
    if (!model.GetModelAABB().Intersects(rayOrigL, rayDirL, dist))
        return;

    // ray/model test (test ray agains each mesh of the model)
    if (!RayModelTest(&model, rayOrigL, rayDirL, tmin, outData.triangleIdx))
        return;

    outData.enttId = enttId;
    outData.modelId = modelId;

    outRayOrigL = rayOrigL;
    outRayDirL = rayDirL;
}
 
//---------------------------------------------------------
// Desc:  execute ray/model test
//---------------------------------------------------------
bool CGraphics::RayModelTest(
    const Model* pModel,
    const XMVECTOR& rayOrigin,
    const XMVECTOR& rayDir,
    float& tmin,
    uint& intersectedTriangleIdx)
{
    assert(pModel);

    const Vertex3D* vertices = pModel->GetVertices();
    const UINT*     indices  = pModel->GetIndices();
    bool           intersect = false;

    // ray/triangle tests
    for (int i = 0; i < pModel->GetNumIndices() / 3; ++i)
    {
        // indices for this triangle
        const UINT i0 = indices[i*3 + 0];
        const UINT i1 = indices[i*3 + 1];
        const UINT i2 = indices[i*3 + 2];

        // vertices for this triangle
        const XMVECTOR v0 = XMLoadFloat3(&vertices[i0].pos);
        const XMVECTOR v1 = XMLoadFloat3(&vertices[i1].pos);
        const XMVECTOR v2 = XMLoadFloat3(&vertices[i2].pos);

        // we have to iterate over all the triangle in order 
        // to find the nearest intersection
        float t = 0.0f;

        if (!DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
            continue;

        if (t > tmin)
            continue;

        // this is a new nearest picked entt and its triangle
        intersectedTriangleIdx = i;
        tmin = t;
        intersect = true;
    }

    return intersect;
}

//---------------------------------------------------------
// Desc:  gather and calculate data about intersection of a ray and some entity
// 
// Args:  - rayOrigL, rayDirL:  ray's origin and direction in local space
//        - rayOrigW, rayDirW:  ... in world space
//        - t:                  the distance along the ray where the intersection occurs
// Out:   - outData:            output container for calculated intersection data
//---------------------------------------------------------
void CGraphics::GatherIntersectionData(
    const XMVECTOR rayOrigL,
    const XMVECTOR rayDirL,
    const XMVECTOR rayOrigW,
    const XMVECTOR rayDirW,
    const float t,
    IntersectionData& outData)
{
    const Model& model = g_ModelMgr.GetModelById(outData.modelId);
    const Vertex3D*   verts = model.GetVertices();
    const UINT*     indices = model.GetIndices();
    const uint      baseIdx = outData.triangleIdx * 3;

    const uint           i0 = indices[baseIdx + 0];
    const uint           i1 = indices[baseIdx + 1];
    const uint           i2 = indices[baseIdx + 2];

    const Vertex3D&      v0 = verts[i0];
    const Vertex3D&      v1 = verts[i1];
    const Vertex3D&      v2 = verts[i2];

    
    // transform triangle's points from local to world space
    const XMMATRIX& world = pEnttMgr_->transformSys_.GetWorld(outData.enttId);

    // vertices positions in local space
    const XMVECTOR posL[3] = {
        XMLoadFloat3(&v0.pos),
        XMLoadFloat3(&v1.pos),
        XMLoadFloat3(&v2.pos)
    };

    // vertices positions in world space
    const XMVECTOR posW[3] = {
        XMVector3TransformCoord(posL[0], world),
        XMVector3TransformCoord(posL[1], world),
        XMVector3TransformCoord(posL[2], world)
    };

    // store endpoints of the intersected triangle
    XMFLOAT3 pos0, pos1, pos2;

    XMStoreFloat3(&pos0, posW[0]);
    XMStoreFloat3(&pos1, posW[1]);
    XMStoreFloat3(&pos2, posW[2]);

    outData.vx0 = pos0.x;
    outData.vy0 = pos0.y;
    outData.vz0 = pos0.z;

    outData.vx1 = pos1.x;
    outData.vy1 = pos1.y;
    outData.vz1 = pos1.z;

    outData.vx2 = pos2.x;
    outData.vy2 = pos2.y;
    outData.vz2 = pos2.z;

    // calc intersection point in local space and transform it to world space
    const XMVECTOR intersectPL = rayOrigL + rayDirL * t;
    const XMVECTOR intersectPW = XMVector3TransformCoord(intersectPL, world);
    XMFLOAT3 intersect;
    XMStoreFloat3(&intersect, intersectPW);

    // store ray origin...
    XMFLOAT3 rayOrig;
    XMStoreFloat3(&rayOrig, rayOrigW);

    outData.rayOrigX = rayOrig.x;
    outData.rayOrigY = rayOrig.y;
    outData.rayOrigZ = rayOrig.z;

    // ...intersection point 
    outData.px = intersect.x;
    outData.py = intersect.y;
    outData.pz = intersect.z;

    // ...and normal vec of intersected triangle (in world space)
    Vec3 normal = CalcNormalVec(pos0, pos1, pos2);
    Vec3 rayDir = ToVec3(rayDirW);

    if (Vec3Dot(normal, rayDir) > 0)
        normal = -normal;

    outData.nx = normal.x;
    outData.ny = normal.y;
    outData.nz = normal.z;

#if 0
    // PRINT COLLISION STATS
    printf("\nlocal:\n");
    printf("p0 %.2f %.2f %.2f     p1 %.2f %.2f %.2f    p2 %.2f %.2f %.2f\n",
        v0.position.x, v0.position.y, v0.position.z,
        v1.position.x, v1.position.y, v1.position.z,
        v2.position.x, v2.position.y, v2.position.z);

    printf("n0 %.2f %.2f %.2f     n1 %.2f %.2f %.2f    n2 %.2f %.2f %.2f\n",
        v0.normal.x, v0.normal.y, v0.normal.z,
        v1.normal.x, v1.normal.y, v1.normal.z,
        v2.normal.x, v2.normal.y, v2.normal.z);

    printf("world:\n");
    printf("p0 %.2f %.2f %.2f     p1 %.2f %.2f %.2f    p2 %.2f %.2f %.2f\n",
        pos0.x, pos0.y, pos0.z,
        pos1.x, pos1.y, pos1.z,
        pos1.x, pos1.y, pos1.z);

    printf("n0 %.2f %.2f %.2f\n", normal.x, normal.y, normal.z);
#endif
}

//---------------------------------------------------------
// check if we have any entity by input screen coords;
// 
// in:   screen pixel coords
// out:  0  - there is no entity, we didn't select any
//       ID - we selected some entity so return its ID
//---------------------------------------------------------
int CGraphics::TestEnttSelection(const int sx, const int sy)
{
    using namespace DirectX;

    ECS::EntityMgr* pEnttMgr = pEnttMgr_;
    const XMMATRIX& P        = pEnttMgr->cameraSys_.GetProj(currCameraId_);
    const XMMATRIX& invView  = pEnttMgr->cameraSys_.GetInverseView(currCameraId_);

    const float xndc = (+2.0f * sx / GetD3D().GetWindowWidth() - 1.0f);
    const float yndc = (-2.0f * sy / GetD3D().GetWindowHeight() + 1.0f);

    // compute picking ray in view space
    const float vx = xndc / P.r[0].m128_f32[0];
    const float vy = yndc / P.r[1].m128_f32[1];

    // ray definition in view space
    XMVECTOR rayOrigin_ = { 0,0,0,1 };
    XMVECTOR rayDir_    = { vx, vy, 1, 0 };

    // assume we have not picked anything yet, 
    // so init the ID to 0 and triangle idx to -1
    uint32_t selectedEnttId = 0;
    int selectedTriangleIdx = -1;
    float tmin = FLT_MAX;               // the distance along the ray where the intersection occurs
    float dist = 0;                     // the length of the ray from origin to the intersection point with the AABB


    // go through each visible entt and check if we have an intersection with it
    for (const EntityID enttId : pEnttMgr->renderSys_.GetAllVisibleEntts())
    {
        // get an inverse world matrix of the current entt
        const XMMATRIX invWorld = pEnttMgr->transformSys_.GetInvWorld(enttId);
        const XMMATRIX toLocal  = DirectX::XMMatrixMultiply(invView, invWorld);

        XMVECTOR rayOrigin = XMVector3TransformCoord(rayOrigin_, toLocal);   // supposed to take a point (w == 1)
        XMVECTOR rayDir    = XMVector3TransformNormal(rayDir_, toLocal);     // supposed to take a vec (w == 0)

        // make the ray direction unit length for the intersection tests
        rayDir = XMVector3Normalize(rayDir);


        // if we hit the bounding box of the model, then we might have picked
        // a model triangle, so do the ray/triangle tests;
        //
        // if we didn't hit the bounding box, then it is impossible that we
        // hit the model, so do not waste efford doing ray/triangle tests
        const ModelID   modelId = pEnttMgr->modelSys_.GetModelIdRelatedToEntt(enttId);
        const Model& model = g_ModelMgr.GetModelById(modelId);
        
        if (!model.GetModelAABB().Intersects(rayOrigin, rayDir, dist))
            continue;

        const Vertex3D* verts = model.GetVertices();
        const UINT*   indices = model.GetIndices();

        // execute ray/triangle tests
        for (int i = 0; i < model.GetNumIndices() / 3; ++i)
        {
            // indices for this triangle
            const UINT i0 = indices[i*3 + 0];
            const UINT i1 = indices[i*3 + 1];
            const UINT i2 = indices[i*3 + 2];

            // vertices for this triangle
            const XMVECTOR v0 = XMLoadFloat3(&verts[i0].pos);
            const XMVECTOR v1 = XMLoadFloat3(&verts[i1].pos);
            const XMVECTOR v2 = XMLoadFloat3(&verts[i2].pos);

            float t = 0.0f;

            if (!DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
                continue;

            if (t > tmin)
                continue;

            // this is the new nearest picked entt and its triangle
            tmin = t;
            selectedTriangleIdx = i;
            selectedEnttId = enttId;
        }
    }

    // print a msg about selection of the entity
    if (selectedEnttId)
    {
        const char* name = pEnttMgr->nameSys_.GetNameById(selectedEnttId);

        SetConsoleColor(YELLOW);
        LogMsg("picked entt (id: %" PRIu32 "; name: % s)", selectedEnttId, name);
        SetConsoleColor(RESET);
    }

    // return ID of the selected entt, or 0 if we didn't pick any
    return selectedEnttId;
}

} // namespace Core
