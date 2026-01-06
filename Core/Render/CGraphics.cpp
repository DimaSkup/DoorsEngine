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
#include "debug_draw_manager.h"
#include <Render/RenderStates.h>      // Render module
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
    cvector<DirectX::XMVECTOR>  dirLightsDirections;
    cvector<ECS::PointLight>    pointLightsData;
    cvector<DirectX::XMFLOAT3>  pointLightsPositions;
    cvector<ECS::SpotLight>     spotLightsData;
    cvector<DirectX::XMFLOAT3>  spotLightsPositions;
    cvector<DirectX::XMFLOAT3>  spotLightsDirections;

    cvector<ECS::PointLight>    activePointLights;
} s_LightTmpData;

// static arrays for internal purposes
static cvector<DirectX::XMMATRIX> s_BoneTransforms;


//---------------------------------------------------------
// Desc:  default constructor and destructor
//---------------------------------------------------------
CGraphics::CGraphics() :
    texturesBuf_(NUM_TEXTURE_TYPES, nullptr)
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

    // create frustums for frustum culling
    frustums_.push_back(DirectX::BoundingFrustum());
}

//---------------------------------------------------------
// Desc:  update all the graphics stuff here
//---------------------------------------------------------
void CGraphics::Update(
    const float deltaTime,
    const float gameTime)
{
    UpdateHelper(deltaTime, gameTime);
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

    if (enableDepthPrepass_)
        DepthPrepass();
    g_GpuProfiler.Timestamp(GTS_RenderScene_DepthPrepass);

    //if (!enableDepthPrepass_)
    if (!visualizeDepth_)
        ColorLightPass();

    PostFxPass();
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

    pSysState_->numDrawnAllVerts = rndStat_.numDrawnVerts[GEOM_TYPE_ALL];
    pSysState_->numDrawnAllTris = rndStat_.numDrawnTris[GEOM_TYPE_ALL];
    pSysState_->numDrawnEnttsInstances = rndStat_.numDrawnInstances[GEOM_TYPE_ENTTS];
    pSysState_->numDrawCallsEnttsInstances = rndStat_.numDrawCalls[GEOM_TYPE_ENTTS];
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
// Desc:   update all the graphics related stuff for this frame
//---------------------------------------------------------
void CGraphics::UpdateHelper(const float deltaTime, const float totalGameTime)
{
    // check to prevent fuck up
    assert(pSysState_);
    assert(pEnttMgr_);
    assert(pRender_);


    gameTime_ = totalGameTime;
    
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


    ECS::TransformSystem& transformSys = pEnttMgr_->transformSystem_;
    ECS::CameraSystem&    camSys       = pEnttMgr_->cameraSystem_;
    ECS::NameSystem&      nameSys      = pEnttMgr_->nameSystem_;

    TerrainGeomip& terrain   = g_ModelMgr.GetTerrainGeomip();
    const EntityID currCamId = currCameraID_;

    // ---------------------------------------------
    // update the cameras states

    if (sysState.isGameMode)
    {
        ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
        XMFLOAT3 playerPos = player.GetPosition();

        const float terrainSize = (float)terrain.heightMap_.GetWidth();

        // clamp the camera position to be only on the terrain
        if (playerPos.x < 0)
            playerPos.x = 0;
        if (playerPos.x >= terrainSize)
            playerPos.x = terrainSize - 1;

        if (playerPos.z < 0)
            playerPos.z = 0;
        if (playerPos.z >= terrainSize)
            playerPos.z = terrainSize - 1;


        if (player.IsFreeFlyMode())
        {
            // do nothing
        }

        // we aren't in free fly mode
        else
        {
            // make player's offset by Y-axis to be always over the terrain even
            // when jump from lower to higher position
            const float terrainHeight = terrain.GetScaledInterpolatedHeightAtPoint(playerPos.x, playerPos.z);
            const float offsetOverTerrain = player.GetOffsetOverTerrain();
            player.SetMinVerticalOffset(terrainHeight + offsetOverTerrain);
        }

        sysState.cameraPos = { playerPos.x, playerPos.y, playerPos.z };
    }

    // we aren't in the game mode (now is the editor mode)
    else
    {
        sysState.cameraPos = transformSys.GetPosition(currCamId);
    }

    sysState.cameraDir = transformSys.GetDirection(currCamId);

    // update camera's view matrix

    camSys.UpdateView(currCamId);
    sysState.cameraView = camSys.GetView(currCamId);
    sysState.cameraProj = camSys.GetProj(currCamId);
    viewProj_           = sysState.cameraView * sysState.cameraProj;
    const XMMATRIX invView = camSys.GetInverseView(currCamId);

    // build the frustum in view space from the projection matrix
    DirectX::BoundingFrustum::CreateFromMatrix(frustums_[0], sysState.cameraProj);


    // get camera params which will be used for culling of terrain patches 
    CameraParams camParams;

    // setup camera position
    camParams.posX = sysState.cameraPos.x;
    camParams.posY = sysState.cameraPos.y;
    camParams.posZ = sysState.cameraPos.z;

    camParams.fov         = camSys.GetFovX(currCamId);
    camParams.aspectRatio = camSys.GetAspect(currCamId);
    //camParams.aspectRatio = 1.0f / camSys.GetAspect(currCamId);
    camParams.nearZ       = camSys.GetNearZ(currCamId);
    camParams.farZ        = camSys.GetFarZ(currCamId);

    memcpy(camParams.view, (void*)(&sysState.cameraView.r[0].m128_f32), sizeof(float) * 16);
    memcpy(camParams.proj, sysState.cameraProj.r[0].m128_f32, sizeof(float)*16);

    //-------------------------------------------
 
    // get frustum's points in view space, and add for rendering
    const EntityID gameCamId = nameSys.GetIdByName("game_camera");

    // visualize frustum
    AddFrustumToRender(gameCamId);
   
    //-------------------------------------------


    const float farX = 766.50f;
    const float farY = 422.05f;

    DirectX::BoundingBox cameraAABB;
    DirectX::BoundingBox::CreateFromPoints(cameraAABB, XMVECTOR{ -farX,-farY, 0.01f }, XMVECTOR{ farX, farY, 1000.0f });
    cameraAABB.Transform(cameraAABB, invView);

    XMFLOAT3 cameraCorners[8];
    cameraAABB.GetCorners(cameraCorners);

    XMFLOAT3 cameraMinPoint = cameraCorners[4];
    XMFLOAT3 cameraMaxPoint = cameraCorners[2];

#if 0
    //const Rect3d cameraRect = Rect3d(244, 245, 80, 81, 247, 248);
    const Rect3d cameraRect(
        cameraMinPoint.x, cameraMaxPoint.x,
        cameraMinPoint.y, cameraMaxPoint.y,
        cameraMinPoint.z, cameraMaxPoint.z);

    Frustum frustum;
    frustum.CreateFromProjMatrix(camParams.proj);

    g_QuadTree.CalcVisibleEntities(cameraRect, frustum);
#endif


    //-------------------------------------------

    XMFLOAT3 fogColor = {0,0,0};
    float fogStart    = 0;
    float fogRange    = 0;
    bool fogEnabled   = false;

    pRender_->GetFogData(fogColor, fogStart, fogRange, fogEnabled);

    // after this distance all the objects are completely fogged
    float distFogged = fogStart + fogRange;

    //-------------------------------------------
    // update the terrain, grass, sky

    // create a view frustum planes
    Frustum frustum0;
    Frustum worldFrustum;

    frustum0.Init(camParams.fov, camParams.aspectRatio, camParams.nearZ, camParams.farZ);

    // transform frustum from camera space into world space
    const Matrix invView0 = MatrixInverse(nullptr, Matrix(camParams.view));
    frustum0.Transform(worldFrustum, invView0);


    // update LOD and visibility for each terrain's patch
    terrain.Update(camParams, worldFrustum, distFogged);

    // update visibility of grass patches
    g_GrassMgr.Update(&camParams, &worldFrustum);

    SkyPlane& skyPlane = g_ModelMgr.GetSkyPlane();
    skyPlane.Update(deltaTime);

    // perform frustum culling on all of our currently loaded entities
    FrustumCullingEntts      (sysState);
    FrustumCullingParticles  (sysState, worldFrustum);
    FrustumCullingPointLights(sysState, worldFrustum);

    UpdateParticlesVB();

    // prepare data for each entity
    PrepareRenderInstances(sysState.cameraPos);

    // Update shaders common data for this frame
    UpdateShadersDataPerFrame(deltaTime, totalGameTime);


    // push each 2D sprite into render list
    ECS::SpriteSystem& spriteSys = pEnttMgr_->spriteSystem_;
    const EntityID* sprites = spriteSys.GetAllSpritesIds();
    const size numSprites   = spriteSys.GetNumAllSprites();

    for (index i = 0; i < numSprites; ++i)
    {
        TexID texId = 0;
        uint16 left, top, width, height;

        spriteSys.GetData(sprites[i], texId, left, top, width, height);
        SRV* pSRV = g_TextureMgr.GetTexViewsById(texId);

        pRender_->PushSpriteToRender(Render::Sprite2D(pSRV, left, top, width, height));
    }
}

//---------------------------------------------------------
// Desc:   update vertex buffer with update particles before a new frame
//---------------------------------------------------------
void CGraphics::UpdateParticlesVB()
{
    ECS::ParticleSystem& particleSys = pEnttMgr_->particleSystem_;
    const ECS::ParticlesRenderData& particlesData = particleSys.GetParticlesToRender();

    // prepare updated particles data for rendering
    VertexBuffer<BillboardSprite>& vb = g_ModelMgr.GetBillboardsBuffer();

    BillboardSprite* particlesBuf = (BillboardSprite*)particlesData.particles.data();
    const int numParticles        = (int)particlesData.particles.size();


    if (!particlesBuf)
        return;

    // update the vertex buffer with updated particles data
    if (!vb.UpdateDynamic(GetContext(), particlesBuf, numParticles))
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
    cvector<EntityID>& visibleEntts = pEnttMgr_->renderSystem_.GetAllVisibleEntts();

    if (visibleEntts.size() == 0)
        return;

    Render::RenderDataStorage& storage = pRender_->dataStorage_;

    // gather entts data for rendering
    prep_.PrepareEnttsDataForRendering(
        visibleEntts,
        cameraPos,
        pEnttMgr_,
        storage);

    pRender_->UpdateInstancedBuffer(storage.instancesBuf);
}

//---------------------------------------------------------
// Desc:   add a view frustum of camera by id to the debug shapes render list
//---------------------------------------------------------
void CGraphics::AddFrustumToRender(const EntityID camId)
{
    float fov    = 0;
    float aspect = 0;
    float nearZ  = 0;
    float farZ   = 0;

    const ECS::CameraSystem& camSys = pEnttMgr_->cameraSystem_;

    if (!camSys.GetFrustumInitParams(camId, fov, aspect, nearZ, farZ))
    {
        LogErr(LOG, "there is no camera data by entt id: %" PRIu32, camId);
        return;
    }

    // get frustum's points in view space
    Frustum frustum(fov, aspect, nearZ, farZ);

    Vec3 nearTopLeft, nearBottomLeft;
    Vec3 nearTopRight, nearBottomRight;
    Vec3 farTopLeft, farBottomLeft;
    Vec3 farTopRight, farBottomRight;

    frustum.GetPoints(nearTopLeft, nearBottomLeft,
                      nearTopRight, nearBottomRight,
                      farTopLeft, farBottomLeft,
                      farTopRight, farBottomRight);

    // transform frustum's points from view to world space
    const XMMATRIX& invView = camSys.GetInverseView(camId);
    const Matrix    invViewMat(invView.r[0].m128_f32);

    MatrixMulVec3(nearTopLeft,     invViewMat, nearTopLeft);
    MatrixMulVec3(nearBottomLeft,  invViewMat, nearBottomLeft);
    MatrixMulVec3(nearTopRight,    invViewMat, nearTopRight);
    MatrixMulVec3(nearBottomRight, invViewMat, nearBottomRight);

    MatrixMulVec3(farTopLeft,      invViewMat, farTopLeft);
    MatrixMulVec3(farBottomLeft,   invViewMat, farBottomLeft);
    MatrixMulVec3(farTopRight,     invViewMat, farTopRight);
    MatrixMulVec3(farBottomRight,  invViewMat, farBottomRight);

    // add frustum's points for rendering to visualize frustum volume
    const Vec3 color(0, 1, 0);

    g_DebugDrawMgr.AddFrustum(
        nearTopLeft, nearBottomLeft,
        nearTopRight, nearBottomRight,
        farTopLeft, farBottomLeft,
        farTopRight, farBottomRight,
        color);
}

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

// --------------------------------------------------------
// --------------------------------------------------------
void CGraphics::FrustumCullingEntts(SystemState& sysState)
{
    assert(pEnttMgr_);

    ECS::EntityMgr&    mgr       = *pEnttMgr_;
    ECS::RenderSystem& renderSys = mgr.renderSystem_;
    ECS::CameraSystem& camSys    = mgr.cameraSystem_;

    const cvector<EntityID>& enttsRenderable = renderSys.GetAllEnttsIDs();
    const size numRenderableEntts            = enttsRenderable.size();
    size numVisEntts = 0;                                        // the number of currently visible entts

    if (numRenderableEntts == 0)
        return;

    FrustumCullingTmpData& tmpData = s_tmpFrustumCullData;
    tmpData.Resize(numRenderableEntts);

    // get arr of bounding spheres for each renderable entt
    mgr.boundingSystem_.GetBoundSpheres(
        enttsRenderable.data(),
        numRenderableEntts,
        tmpData.boundSpheres);

    mgr.transformSystem_.GetWorlds(
        enttsRenderable.data(),
        numRenderableEntts,
        tmpData.enttsWorlds);

    // transform bound spheres from local space to world space
    for (int i = 0; const XMMATRIX & world : tmpData.enttsWorlds)
    {
        BoundingSphere& sphere = tmpData.boundSpheres[i];
        sphere.Transform(sphere, tmpData.enttsWorlds[i]);
        ++i;
    }

    const EntityID currCamId  = currCameraID_;
    const XMMATRIX& xmInvView = camSys.GetInverseView(currCamId);
    const Matrix invView(xmInvView.r[0].m128_f32);

    Frustum frustum;
    Frustum worldFrustum;

    float fov    = 0;
    float aspect = 0;
    float nearZ  = 0;
    float farZ   = 0;

    camSys.GetFrustumInitParams(currCamId, fov, aspect, nearZ, farZ);
    frustum.Init(fov, aspect, nearZ, farZ);
    frustum.Transform(worldFrustum, invView);

    // go through each entity and define if it is visible
    for (index idx = 0; idx < numRenderableEntts; ++idx)
    {
        tmpData.idxsToVisEntts[numVisEntts] = idx;

        const XMFLOAT3 center = tmpData.boundSpheres[idx].Center;
        const float    radius = tmpData.boundSpheres[idx].Radius;

        numVisEntts += worldFrustum.TestSphere(Sphere(center.x, center.y, center.z, radius));
    }

    // store ids of visible entts
    cvector<EntityID>& visibleEntts = renderSys.GetAllVisibleEntts();
    visibleEntts.resize(numVisEntts);

    for (index i = 0; i < numVisEntts; ++i)
        visibleEntts[i] = enttsRenderable[tmpData.idxsToVisEntts[i]];

    // this number of entities (instances) will be rendered onto the screen
    sysState.numDrawnEnttsInstances = (uint32)numVisEntts;
}

//--------------------------------------------------------
//--------------------------------------------------------
void CGraphics::FrustumCullingParticles(SystemState& sysState, const Frustum& worldFrustum)
{
    assert(pEnttMgr_);

    ECS::EntityMgr&      mgr         = *pEnttMgr_;
    ECS::ParticleSystem& particleSys = mgr.particleSystem_;

    particleSys.visibleEmittersIdxs_.clear();

    const cvector<ECS::ParticleEmitter>& emitters = particleSys.GetEmitters();

    for (index i = 0; i < emitters.size(); ++i)
    {
        const Rect3d aabb = particleSys.GetEmitterAABB(emitters[i].id);

        if (worldFrustum.TestRect(aabb))
            particleSys.visibleEmittersIdxs_.push_back(i);
    }
}

//---------------------------------------------------------
// Desc:  cull invisible point lights so we don't use them
//        when calculate lighting in shaders
//---------------------------------------------------------
void CGraphics::FrustumCullingPointLights(SystemState& sysState, const Frustum& worldFrustum)
{
    assert(pEnttMgr_);

    const ECS::LightSystem&     lightSys          = pEnttMgr_->lightSystem_;
    const ECS::RenderSystem&    renderSys         = pEnttMgr_->renderSystem_;

    const ECS::PointLights&     pointLights       = lightSys.GetPointLights();
    const size                  numAllPointLights = pointLights.data.size();

    cvector<EntityID>&          visPointLights    = renderSys.GetVisiblePointLights();
    cvector<DirectX::XMFLOAT3>& positions         = s_tmpFrustumCullData.positions;
    size                        numVisiblePointL  = 0;


    pEnttMgr_->transformSystem_.GetPositions(
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
// Desc:   pdate shaders common data for this frame: 
//         viewProj matrix, camera position, light sources data, etc.
// Args:   - deltaTime:      the time passed since the prev frame
//         - totalGameTime:  the time passed since the start of the application
//---------------------------------------------------------
void CGraphics::UpdateShadersDataPerFrame(
    const float deltaTime,
    const float totalGameTime)
{
    Render::CRender* pRender   = pRender_;
    ECS::EntityMgr* pEnttMgr   = pEnttMgr_;
    const SkyPlane& skyPlane   = g_ModelMgr.GetSkyPlane();

    ECS::CameraSystem& camSys  = pEnttMgr->cameraSystem_;
    const EntityID currCamId   = currCameraID_;
    const XMMATRIX& invView    = camSys.GetInverseView(currCamId);
    const XMMATRIX  invProj    = DirectX::XMMatrixInverse(nullptr, camSys.GetProj(currCamId));


    pRender->UpdateCbViewProj(DirectX::XMMatrixTranspose(viewProj_));
    pRender->UpdateCbTime(deltaTime, totalGameTime);

    pRender->UpdateCbCamera(
        DirectX::XMMatrixTranspose(camSys.GetView(currCamId)),
        DirectX::XMMatrixTranspose(camSys.GetProj(currCamId)),
        invView,
        invProj,
        camSys.GetPos(currCamId),
        camSys.GetNearZ(currCamId),
        camSys.GetFarZ(currCamId));


    SetupLightsForFrame(pRender_->perFrameData_);

    // update const buffers with new data
    pRender->UpdatePerFrame(pRender_->perFrameData_);


    pRender->UpdateCbSky(
        skyPlane.GetTranslation(0),
        skyPlane.GetTranslation(1),
        skyPlane.GetTranslation(2),
        skyPlane.GetTranslation(3),
        skyPlane.GetBrightness());
}

//---------------------------------------------------------
// Desc:   clear rendering data from the previous frame / instances set
//---------------------------------------------------------
void CGraphics::ClearRenderingDataBeforeFrame()
{
    pRender_->dataStorage_.Clear();
}

//---------------------------------------------------------
// Desc:   switch to another fill mode according to input argument
//---------------------------------------------------------
void SwitchFillMode(
    ID3D11DeviceContext* pContext,
    Render::RenderStates& renderStates,
    const uint32 fillMode)
{
    switch (fillMode)
    {
        case MAT_PROP_FILL_SOLID:
            renderStates.SetRS(pContext, R_FILL_SOLID);
            break;

        case MAT_PROP_FILL_WIREFRAME:
            renderStates.SetRS(pContext, R_FILL_WIREFRAME);
            break;
    }
}

//---------------------------------------------------------
// Desc:   switch to another cull mode according to input argument
//---------------------------------------------------------
void SwitchCullMode(
    ID3D11DeviceContext* pContext,
    Render::RenderStates& renderStates,
    const uint32 cullMode)
{
    switch (cullMode)
    {
        case MAT_PROP_CULL_BACK:
            renderStates.SetRS(pContext, R_CULL_BACK);
            break;

        case MAT_PROP_CULL_FRONT:
            renderStates.SetRS(pContext, R_CULL_FRONT);
            break;

        case MAT_PROP_CULL_NONE:
            renderStates.SetRS(pContext, R_CULL_NONE);
            break;
    }
}

//---------------------------------------------------------
// Desc:   switch to another blend state according to input argument
//---------------------------------------------------------
void SwitchBlendState(
    ID3D11DeviceContext* pContext,
    Render::RenderStates& renderStates,
    const uint32 blendState)
{
    switch (blendState)
    {
        case MAT_PROP_BS_NO_RENDER_TARGET_WRITES:
            renderStates.SetBS(pContext, R_NO_RENDER_TARGET_WRITES);
            break;

        case MAT_PROP_BS_DISABLE:
            renderStates.SetBS(pContext, R_ALPHA_DISABLE);
            break;

        case MAT_PROP_BS_ENABLE:
            renderStates.SetBS(pContext, R_ALPHA_ENABLE);
            break;

        case MAT_PROP_BS_ADD:
            renderStates.SetBS(pContext, R_ADDING);
            break;

        case MAT_PROP_BS_SUB:
            renderStates.SetBS(pContext, R_SUBTRACTING);
            break;

        case MAT_PROP_BS_MUL:
            renderStates.SetBS(pContext, R_MULTIPLYING);
            break;

        case MAT_PROP_BS_TRANSPARENCY:
            renderStates.SetBS(pContext, R_TRANSPARENCY);
            break;

        case MAT_PROP_BS_ALPHA_TO_COVERAGE:
            renderStates.SetBS(pContext, R_ALPHA_TO_COVERAGE);
            break;
    }
}

//---------------------------------------------------------
// Desc:   switch to another depth-stencil state according to input argument
//---------------------------------------------------------
void SwitchDepthStencilState(
    ID3D11DeviceContext* pContext,
    Render::RenderStates& renderStates,
    const uint32 depthStencilState)
{
    switch (depthStencilState)
    {
        case MAT_PROP_DSS_DEPTH_ENABLED:
            renderStates.SetDSS(pContext, R_DEPTH_ENABLED, 0);
            break;

        case MAT_PROP_DSS_DEPTH_DISABLED:
            renderStates.SetDSS(pContext, R_DEPTH_DISABLED, 0);
            break;

        case MAT_PROP_DSS_MARK_MIRROR:
            renderStates.SetDSS(pContext, R_MARK_MIRROR, 0);
            break;

        case MAT_PROP_DSS_DRAW_REFLECTION:
            renderStates.SetDSS(pContext, R_DRAW_REFLECTION, 0);
            break;

        case MAT_PROP_DSS_NO_DOUBLE_BLEND:
            renderStates.SetDSS(pContext, R_NO_DOUBLE_BLEND, 0);
            break;

        case MAT_PROP_DSS_SKY_DOME:
            renderStates.SetDSS(pContext, R_SKY_DOME, 0);
            break;
    }
}

//---------------------------------------------------------
// Desc:  bind input material (textures + render states) 
//---------------------------------------------------------
void CGraphics::BindMaterial(const Material& mat)
{
    BindMaterial(mat.renderStates, mat.texIds);
}

//---------------------------------------------------------
// Desc:  bind a material (textures + render states) by input ID
//---------------------------------------------------------
void CGraphics::BindMaterialById(const MaterialID matId)
{
    const Material& mat = g_MaterialMgr.GetMatById(matId);
    BindMaterial(mat.renderStates, mat.texIds);
}

//---------------------------------------------------------
// Desc:  bind a material (textures + render states) by input name
//---------------------------------------------------------
void CGraphics::BindMaterialByName(const char* matName)
{
    if (!matName || matName[0] == '\0')
    {
        LogErr(LOG, "can't bind a material: input name is empty!");
        return;
    }

    const Material& mat = g_MaterialMgr.GetMatByName("grass_0");
    BindMaterial(mat.renderStates, mat.texIds);
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   - renderStatesBitfield:  bitfield with render states for this material
//         - texIdx:                identifiers to textures which will be bound
//---------------------------------------------------------
void CGraphics::BindMaterial(
    const uint32 renderStatesBitfields,
    const TexID* texIds)
{
    // find texture resource views by input textures ids
    ID3D11ShaderResourceView* texViews[NUM_TEXTURE_TYPES]{ nullptr };
    g_TextureMgr.GetTexViewsByIds(texIds, NUM_TEXTURE_TYPES, texViews);

    BindMaterial(renderStatesBitfields, texViews);
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   - renderStatesBitfield:  bitfield with render states for this material
//         - texViews:              textures to bind
//---------------------------------------------------------
void CGraphics::BindMaterial(
    const uint32 renderStatesBitfield,
    ID3D11ShaderResourceView* const* texViews)
{
    if (!texViews)
    {
        LogErr(LOG, "input arr of textures IDs == nullptr");
        return;
    }

    ID3D11DeviceContext* pContext = GetContext();

    // static bitfield of render states
    static uint32 prevBitfield = 0;

    // bind textures of this material
    pContext->PSSetShaderResources(100U, NUM_TEXTURE_TYPES, texViews);

    // check if we need to switch render states
    if (prevBitfield == renderStatesBitfield)
        return;

 
    Render::RenderStates& renderStates = pRender_->GetRenderStates();
    uint32 prev = 0;
    uint32 curr = 0;


    // switch alpha clipping
    prev = prevBitfield & MAT_PROP_ALPHA_CLIPPING;
    curr = renderStatesBitfield & MAT_PROP_ALPHA_CLIPPING;

    if (prev != curr)
        pRender_->SwitchAlphaClipping(curr);


    // switch fill mode if need
    prev = prevBitfield & ALL_FILL_MODES;
    curr = renderStatesBitfield & ALL_FILL_MODES;

    if (prev != curr)
        SwitchFillMode(pContext, renderStates, curr);


    // switch cull mode if need
    prev = prevBitfield & ALL_CULL_MODES;
    curr = renderStatesBitfield & ALL_CULL_MODES;

    if (prev != curr)
        SwitchCullMode(pContext, renderStates, curr);


    // switch blending state if necessary
    prev = prevBitfield & ALL_BLEND_STATES;
    curr = renderStatesBitfield & ALL_BLEND_STATES;

    if (prev != curr)
        SwitchBlendState(pContext, renderStates, curr);


    // switch depth-stencil state if need
    prev = prevBitfield & ALL_DEPTH_STENCIL_STATES;
    curr = renderStatesBitfield & ALL_DEPTH_STENCIL_STATES;

    if (prev != curr)
        SwitchDepthStencilState(pContext, renderStates, curr);


    // update the static bitfield of render states before the the next material
    prevBitfield = renderStatesBitfield;
}

//---------------------------------------------------------
// Desc:   reset different stuff before rendering of each frame
//---------------------------------------------------------
void CGraphics::ResetBeforeRendering()
{
    // reset textures and render states so we will be able to setup it properly for rendering
    ID3D11ShaderResourceView* zeroTexViews[NUM_TEXTURE_TYPES]{ nullptr };
    BindMaterial(MAT_PROP_DEFAULT, zeroTexViews);
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
    assert(pRender_ != nullptr);
    assert(pEnttMgr_ != nullptr);

    Render::CRender*        pRender       = pRender_;
    ECS::EntityMgr*         pEnttMgr      = pEnttMgr_;
    ID3D11DeviceContext*    pContext      = pRender->GetContext();
    Render::D3DClass&       d3d           = pRender->GetD3D();

    UINT startInstanceLocation = 0;
    const Render::RenderDataStorage& storage = pRender->dataStorage_;

    // check if we have any instances to render
    if (storage.instancesBuf.GetSize() <= 0)
        return;

    Render::RenderStates& renderStates = pRender_->GetRenderStates();
    renderStates.ResetRS(pContext);
    renderStates.ResetBS(pContext);
    renderStates.SetDSS(pContext, R_DEPTH_PREPASS, 0);

    // only depth writing
    pContext->OMSetRenderTargets(0, nullptr, d3d.pDepthStencilView_);


    // render masked geometry: tree branches, bushes, etc.
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DepthPrepassInstanceGroup(storage.masked, GEOM_TYPE_MASKED, startInstanceLocation);

    // render opaque geometry: solid objects
    DepthPrepassInstanceGroup(storage.opaque, GEOM_TYPE_OPAQUE, startInstanceLocation);

    // render terrain
    TerrainDepthPrepass();

    // reset depth-stencil state, and bind back a swap chain's RTV
    renderStates.SetDSS(pContext, R_DEPTH_ENABLED, 0);

    pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// usual rendering: color and lighting
//---------------------------------------------------------
void CGraphics::ColorLightPass()
{
    assert(pRender_ != nullptr);
    assert(pEnttMgr_ != nullptr);

    Render::CRender*     pRender  = pRender_;
    ECS::EntityMgr*      pEnttMgr = pEnttMgr_;
    ID3D11DeviceContext* pContext = pRender->GetContext();
    Render::D3DClass&    d3d      = pRender->GetD3D();

    UINT startInstanceLocation = 0;
    const Render::RenderDataStorage& storage = pRender->dataStorage_;


    // for post effects we have to render into another (non default) render target
    if (IsEnabledPostFxPass() || d3d.IsEnabledFXAA())
    {
        const float clearColor[4] = { 1,1,1,1 };
        ID3D11RenderTargetView* pRTV = nullptr;

        // for MSAA we need to bind sufficient render target view (RTV)
        if (d3d.IsEnabled4xMSAA())
            pRTV = d3d.pMSAARTV_;

        // bind non-MSAA render target
        else
            pRTV = d3d.postFxsPassRTV_[0];

        assert(pRTV != nullptr && "for post process: RTV tex == nullptr");

        pContext->OMSetRenderTargets(1, &pRTV, d3d.pDepthStencilView_);
        pContext->ClearRenderTargetView(pRTV, clearColor);
    }


    // check if we have any instances to render
    if (storage.instancesBuf.GetSize() <= 0)
        return;


    // first of all we render player's weapon
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderPlayerWeapon();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Weapon);

    // render grass
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    RenderGrass();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Grass);

    // render masked geometry: tree branches, bushes, etc.
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderInstanceGroups(storage.masked, GEOM_TYPE_MASKED, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Masked);

    // render opaque geometry: solid objects
    RenderInstanceGroups(storage.opaque, GEOM_TYPE_OPAQUE, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Opaque);


    // render each animated entity separately
    for (const EntityID id : pEnttMgr_->animationSystem_.GetEnttsIds())
    {
        if (pEnttMgr_->renderSystem_.HasEntity(id))
            RenderSkinnedModel(id);
    }
    g_GpuProfiler.Timestamp(GTS_RenderScene_SkinnedModels);


    // render terrain
    RenderTerrainGeomip();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Terrain);

    // render sky and clouds
    RenderSkyDome();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Sky);

    RenderSkyClouds();
    g_GpuProfiler.Timestamp(GTS_RenderScene_SkyPlane);

    // render blended geometry (but not transparent)
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    RenderInstanceGroups(storage.blended, GEOM_TYPE_BLENDED, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Blended);

    // render transparent geometry
    RenderInstanceGroups(storage.blendedTransparent, GEOM_TYPE_TRANSPARENT, startInstanceLocation);
    g_GpuProfiler.Timestamp(GTS_RenderScene_Transparent);

    // render billboards and particles
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    RenderParticles();
    g_GpuProfiler.Timestamp(GTS_RenderScene_Particles);

    RenderDebugShapes();
    g_GpuProfiler.Timestamp(GTS_RenderScene_DbgShapes);
}

//---------------------------------------------------------
// Desc:  render post-effects:  post process, depth visualization, etc.
//        (do it after the scene color and lighting have been rendered)
//---------------------------------------------------------
void CGraphics::PostFxPass()
{
    assert(pRender_ != nullptr);

    Render::D3DClass&     d3d      = GetD3D();
    ID3D11DeviceContext*  pContext = GetContext();
    Render::RenderStates& rs       = d3d.GetRenderStates();

    rs.ResetRS(pContext);
    rs.ResetBS(pContext);
    rs.ResetDSS(pContext);


    if (visualizeDepth_)
    {
        VisualizeDepthBuffer();
        return;
    }

    // FXAA is a kind of post-effects
    if (d3d.IsEnabledFXAA())
    {
        assert(d3d.pSwapChainRTV_     != nullptr && "swap chain RTV is wrong");
        assert(d3d.postFxsPassSRV_[0] != nullptr && "resolved SRV is wrong");

        // bind dst render target (+ unbind depth stencil view) and src shader resource view 
        pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
        pContext->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

        pRender_->BindShaderByName("FXAA");
        pContext->Draw(3, 0);

        // bind depth stencil view back
        pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);

        return;
    }

    // if execution of posts effects is turned off...
    if (!IsEnabledPostFxPass())
        return;


    if (d3d.IsEnabled4xMSAA())
    {
        // resolve MSAA -> single-sample (non MSAA)
        assert(d3d.postFxsPassTex_[0] != nullptr && "resolved tex is wrong");
        assert(d3d.pMSAAColorTex_     != nullptr && "MSAA color tex is wrong");
        pContext->ResolveSubresource(d3d.postFxsPassTex_[0], 0, d3d.pMSAAColorTex_, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
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

    if (!materialBigIconFrameBuf_.Initialize(GetD3D().GetDevice(), fbSpec))
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
        LogErr(LOG, "input icon width or height is wrong, it must be > 0 (current w: %d, h: %d)", width, height);
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
        bool inited = buf.Initialize(GetD3D().GetDevice(), fbSpec);

        if (!inited)
        {
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
        LogErr(LOG, "input shader resource view == nullptr");
        return false;
    }

    assert(pRender_);
    Render::CRender* pRender = pRender_;

    // get a sphere model
    const ModelID basicSphereID      = g_ModelMgr.GetModelIdByName("basic_sphere");
    const BasicModel& sphere         = g_ModelMgr.GetModelById(basicSphereID);
    const MeshGeometry& sphereMesh   = sphere.meshes_;

    const VertexBuffer<Vertex3D>& vb = sphereMesh.vb_;
    const IndexBuffer<UINT>&      ib = sphereMesh.ib_;
    const UINT                offset = 0;

    // change view*proj matrix so we will be able to render material icons properly
    const XMMATRIX world = XMMatrixRotationY(yRotationAngle);
    const XMMATRIX view  = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj  = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    pRender->UpdateCbWorldAndViewProj(
        DirectX::XMMatrixTranspose(world),
        DirectX::XMMatrixTranspose(view * proj));

    // bind shader and prepare IA for rendering
    ID3D11DeviceContext* pContext = GetContext();
    pRender->BindShaderByName("MaterialIconShader");
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), offset);
    pRender->BindIB(ib.Get(), DXGI_FORMAT_R32_UINT);

    // prepare responsible frame buffer for rendering
    materialBigIconFrameBuf_.ClearBuffers(pContext, { 0,0,0,0 });
    materialBigIconFrameBuf_.Bind(pContext);

    // prepare material data and its textures
    const Material& mat = g_MaterialMgr.GetMatById(matID);
    BindMaterial(mat.renderStates, mat.texIds);
    pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

    // render geometry with material into frame buffer
    pContext->DrawIndexed(ib.GetIndexCount(), 0U, 0U);


    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->UpdateCbViewProj(DirectX::XMMatrixTranspose(viewProj_));

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
    assert(pRender_);
    Render::CRender* pRender = pRender_;

    // get a sphere model
    const ModelID basicSphereId    = g_ModelMgr.GetModelIdByName("basic_sphere");
    const BasicModel& sphere       = g_ModelMgr.GetModelById(basicSphereId);
    const MeshGeometry& sphereMesh = sphere.meshes_;

    ID3D11Buffer* vb      = sphereMesh.vb_.Get();
    ID3D11Buffer* ib      = sphereMesh.ib_.Get();
    const UINT indexCount = (UINT)sphereMesh.ib_.GetIndexCount();
    const UINT vertexSize = (UINT)sphereMesh.vb_.GetStride();
    const UINT offset     = 0;

    // change view*proj matrix so we will be able to render material icons properly
    const XMMATRIX view  = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj  = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    // prepare IA for rendering, bind shaders, set matrices
    ID3D11DeviceContext* pContext = GetD3D().GetDeviceContext();
    pRender->BindShaderByName("MaterialIconShader");
    pRender->UpdateCbWorldAndViewProj(DirectX::XMMatrixIdentity(), DirectX::XMMatrixTranspose(view * proj));

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pRender->BindVB(&vb, vertexSize, offset);
    pRender->BindIB(ib, DXGI_FORMAT_R32_UINT);


    // clear all the previous content of frame buffers
    for (FrameBuffer & buf : materialsFrameBuffers_)
        buf.ClearBuffers(pContext, { 0,0,0,0 });


    // render material by idx into responsible frame buffer
    for (int matIdx = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        // bind material data and its textures
        const Material& mat = g_MaterialMgr.GetMatById(matIdx);
        BindMaterial(mat.renderStates, mat.texIds);
        pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

        // render geometry
        buf.Bind(pContext);
        pContext->DrawIndexed(indexCount, 0U, 0U);

        ++matIdx;
    }

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->UpdateCbViewProj(DirectX::XMMatrixTranspose(viewProj_));

    GetD3D().ResetBackBufferRenderTarget();
    GetD3D().ResetViewport();

    return true;
}

//---------------------------------------------------------
// Desc:  just render grass planes
//---------------------------------------------------------
void CGraphics::RenderGrass()
{
    Render::CRender*     pRender  = pRender_;
    ID3D11DeviceContext* pContext = pRender->GetContext();

    // bind shader and material

    const Material& mat = g_MaterialMgr.GetMatByName("grass_0");
    pRender->BindShaderById(mat.shaderId);
    BindMaterial(mat);
    pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

    int numGrassVertices = 0;
    int numDrawCalls = 0;

    for (const uint32 idx : g_GrassMgr.GetVisPatchesIdxs())
    {
        const VertexBuffer<VertexGrass>& vb = g_GrassMgr.GetVertexBufByIdx(idx);
        const int numVerts                  = vb.GetVertexCount();

        // if we have any grass instances in this vertex buffer
        if (numVerts > 0)
        {
            pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
            pContext->Draw(numVerts, 0);

            numGrassVertices += numVerts;
            numDrawCalls++;
        }
    }

    // calc render stats
    rndStat_.numDrawnVerts[GEOM_TYPE_GRASS] += numGrassVertices;
    rndStat_.numDrawnTris [GEOM_TYPE_GRASS] += numGrassVertices * 6;   // from one grass vertex we create 3 grass plane (6 triangles) 

    rndStat_.numDrawnInstances[GEOM_TYPE_GRASS] += numGrassVertices;
    rndStat_.numDrawCalls     [GEOM_TYPE_GRASS] += numDrawCalls;
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
    assert(pRender_ != nullptr);
    Render::CRender* pRender = pRender_;

    uint32 numDrawnVertices = 0;
    uint32 numDrawnTriangles = 0;
    uint32 numDrawnInstances = 0;
    uint32 numDrawCalls = 0;

    for (const Render::InstanceBatch& batch : instanceBatches)
    {
        BindMaterial(batch.renderStates, batch.textures);
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
    assert(pRender_ != nullptr);
    Render::CRender* pRender = pRender_;
    ID3D11DeviceContext* pContext = pRender->GetContext();

    if (geomType == GEOM_TYPE_MASKED)
    {
        for (const Render::InstanceBatch& batch : instanceBatches)
        {
            // stride idx in the instances buffer
            startInstanceLocation += (UINT)batch.numInstances;
        }
        return;
    }

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
    ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
    RenderSkinnedModel(player.GetActiveWeapon());
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
    ID3D11DeviceContext*  pContext      = pRender->GetContext();
    ECS::EntityMgr&       enttMgr       = *pEnttMgr_;
    ECS::TransformSystem& transformSys  = pEnttMgr_->transformSystem_;
    const char*           enttName      = enttMgr.nameSystem_.GetNameById(enttId);

    if (!enttMgr.animationSystem_.HasAnimation(enttId))
    {
        LogErr(LOG, "you try to render entity (id: %" PRIu32 ", name: %s) as skinned (animated) but there is no skeleton/animation for it", enttId, enttName);
        return;
    }

    // prepare model's instance
    const ModelID       modelId = enttMgr.modelSystem_.GetModelIdRelatedToEntt(enttId);
    const BasicModel&   model   = g_ModelMgr.GetModelById(modelId);
    const MeshGeometry& meshes  = model.meshes_;

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
    pEnttMgr_->animationSystem_.GetData(enttId, skeletonId, animationId, timePos);

    // update bone transformations for this frame
    s_BoneTransforms.resize(MAX_NUM_BONES_PER_CHARACTER, DirectX::XMMatrixIdentity());
    AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(skeletonId);

    if (strcmp(skeleton.name_, "ak_74_hud") == 0 && timePos > 0.1f)
    {
        int k = 0;
        k++;
    }

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

    pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);

    //---------------------------------

    // render each mesh of the model separately so we will receive a complete image
    for (int i = 0; i < model.GetNumSubsets(); ++i)
    {
        // bind material and update a const buf with colors
        const Subset& mesh  = meshes.subsets_[i];
        const Material& mat = g_MaterialMgr.GetMatById(mesh.materialId);

        BindMaterial(mat);
        pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

        pContext->DrawIndexed(mesh.indexCount, mesh.indexStart, mesh.vertexStart);
    }
}

//---------------------------------------------------------
// Desc:   render particles onto the screen
//---------------------------------------------------------
void CGraphics::RenderParticles()
{
    assert(pRender_ != nullptr);
    assert(pEnttMgr_ != nullptr);

    Render::CRender&     render                   = *pRender_;
    ID3D11DeviceContext* pContext                 = GetContext();
    ECS::ParticleSystem& particleSys              = pEnttMgr_->particleSystem_;
    const ECS::ParticlesRenderData& particlesData = particleSys.GetParticlesToRender();
    const int numParticles                        = (int)particlesData.particles.size();

    if (numParticles == 0)
        return;

    render.BindShaderByName("ParticleShader");

    // for particles we bind only vertex buffer
    const VertexBuffer<BillboardSprite>& vb = g_ModelMgr.GetBillboardsBuffer();
    render.BindVB(vb.GetAddrOf(), vb.GetStride(), 0);

    // go through particles emitters and render its particles
    int numDrawnVertices = 0;
    int numDrawCalls = 0;

    // render each type of particles separately
    for (index i = 0; i < particlesData.materialIds.size(); ++i)
    {
        BindMaterialById(particlesData.materialIds[i]);

        pContext->Draw(particlesData.numInstances[i], particlesData.baseInstance[i]);

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
    const EntityID skyEnttId = pEnttMgr_->nameSystem_.GetIdByName("sky");

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

    // bind sky material
    BindMaterialById(sky.GetMaterialId());

    // compute a worldViewProj matrix for the sky instance
    const XMFLOAT3 skyOffset     = pEnttMgr_->transformSystem_.GetPosition(skyEnttId);
    const XMFLOAT3 eyePos        = pEnttMgr_->cameraSystem_.GetPos(currCameraID_);
    const XMFLOAT3 translation   = skyOffset + eyePos;
    const XMMATRIX world         = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
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
    assert(pRender_ != nullptr);

    const SkyPlane& skyPlane             = g_ModelMgr.GetSkyPlane();
    const VertexBuffer<VertexPosTex>& vb = skyPlane.GetVB();
    const IndexBuffer<uint16>&        ib = skyPlane.GetIB();

    // bind buffers, material, and shader
    pRender_->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender_->BindIB(ib.Get(), DXGI_FORMAT_R16_UINT);
    BindMaterialById(skyPlane.GetMaterialId());
    pRender_->BindShaderByName("SkyCloudShader");

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
    assert(pRender_ != nullptr);
    Render::CRender*     pRender  = pRender_;
    ID3D11DeviceContext* pContext = pRender->GetContext();

    // prepare the terrain instance
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();

    // vertex/index buffers data
    Render::TerrainInstance instance;
    instance.vertexStride = terrain.GetVertexStride();
    instance.pVB          = terrain.GetVertexBuffer();
    instance.pIB          = terrain.GetIndexBuffer();

    // prepare IA stage and shaders
    pRender->BindShaderByName("TerrainDepthPrepass");
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pRender->BindVB(&instance.pVB, instance.vertexStride, 0);
    pRender->BindIB(instance.pIB, DXGI_FORMAT_R32_UINT);


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
        pContext->DrawIndexed(indexCount, baseIndex, baseVertex);
    }
}

//---------------------------------------------------------
// Desc:  render each terrain's patch (sector) by its index
//---------------------------------------------------------
void RenderTerrainPatches(
    TerrainGeomip& terrain,
    ID3D11DeviceContext* pContext,
    UINT& numDrawnTriangles,
    const cvector<int>& patchesIdxs)
{
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
        pContext->DrawIndexed(indexCount, baseIndex, baseVertex);

        numDrawnTriangles += (indexCount / 3);
    }
}

//---------------------------------------------------------
// Desc:   render the terrain onto the screen
//---------------------------------------------------------
void CGraphics::RenderTerrainGeomip()
{
    assert(pRender_ != nullptr);

    Render::CRender*     pRender  = pRender_;
    ID3D11DeviceContext* pContext = pRender->GetContext();

    // prepare the terrain instance
    Render::TerrainInstance instance;
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    instance.vertexStride  = terrain.GetVertexStride();
    instance.pVB           = terrain.GetVertexBuffer();
    instance.pIB           = terrain.GetIndexBuffer();

    const Material& mat = g_MaterialMgr.GetMatById(terrain.materialID_);

    BindMaterial(mat);
    pRender->UpdateCbMaterialColors(mat.ambient, mat.diffuse, mat.specular, mat.reflect);
    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11ShaderResourceView* texViews[NUM_TEXTURE_TYPES]{ nullptr };
    g_TextureMgr.GetTexViewsByIds(mat.texIds, NUM_TEXTURE_TYPES, texViews);
    pContext->VSSetShaderResources(100U, NUM_TEXTURE_TYPES, texViews);
    
    pRender->BindVB(&instance.pVB, instance.vertexStride, 0);
    pRender->BindIB(instance.pIB, DXGI_FORMAT_R32_UINT);


    UINT numDrawnTris = 0;
    size numDrawnPatches = 0;
    size numDrawCalls = 0;

    const cvector<int>& highDetailed = terrain.GetHighDetailedPatches();
    const cvector<int>& midDetailed  = terrain.GetMidDetailedPatches();
    const cvector<int>& lowDetailed  = terrain.GetLowDetailedPatches();


    pRender->BindShaderByName("TerrainShader");
    RenderTerrainPatches(terrain, pContext, numDrawnTris, highDetailed);

    pRender->BindShaderByName("TerrainMidLodShader");
    RenderTerrainPatches(terrain, pContext, numDrawnTris, midDetailed);

    pRender->BindShaderByName("TerrainLowLodShader");
    RenderTerrainPatches(terrain, pContext, numDrawnTris, lowDetailed);


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

void RenderDebugLines(Render::CRender* pRender, cvector<VertexPosColor>& vertices)
{
    assert(pRender != nullptr);

    // fill arr with lines vertices data and update the vertex buffer with it
    const int vertsPerLine    = 2;
    const int numDbgLineVerts = MAX_NUM_DBG_LINES * vertsPerLine;
    vertices.resize(numDbgLineVerts);


    for (int lineIdx = 0, vertIdx = 0; lineIdx < MAX_NUM_DBG_LINES; ++lineIdx)
    {
        const DebugLine& line = g_DebugDrawMgr.lines_[lineIdx];

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

    // update dynamic VB and draw geometry
    ID3D11DeviceContext*    pContext = pRender->GetContext();
    VertexBuffer<VertexPosColor>& vb = g_ModelMgr.GetDebugLinesVB();

    vb.UpdateDynamic(pContext, vertices.data(), numDbgLineVerts);
    pContext->Draw(numDbgLineVerts, 0);
}


//---------------------------------------------------------

void RenderDebugLinesAABB(
    Render::CRender* pRender,
    cvector<VertexPosColor>& vertices)
{
    assert(pRender != nullptr);

    const DebugDrawMgr& drawMgr = g_DebugDrawMgr;

    if (drawMgr.currNumAABBs_ <= 0)
        return;

    const int numAABBs          = drawMgr.currNumAABBs_;
    const int numVerticesInAABB = drawMgr.GetNumVerticesInAABB();
    const int numIndicesInAABB  = drawMgr.GetNumIndicesInAABB();
    const int numVertices       = numAABBs * numVerticesInAABB;

    // fill arr with lines vertices data and update the vertex buffer with it
    vertices.resize(numVertices);

    // setup vertices for each aabb
    for (int aabbIdx = 0, vertIdx = 0; aabbIdx < numAABBs; ++aabbIdx)
    {
        for (int i = 0; i < numVerticesInAABB; ++i, ++vertIdx)
        {
            const DebugLineVertex& v = drawMgr.aabbsVertices_[vertIdx];

            vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    ID3D11DeviceContext*    pContext = pRender->GetContext();
    VertexBuffer<VertexPosColor>& vb = g_ModelMgr.GetDebugLinesVB();
    IndexBuffer<uint16>&          ib = g_ModelMgr.GetDebugLinesIB();

    vb.UpdateDynamic(pContext, vertices.data(), numAABBs * numVerticesInAABB);
    ib.Update       (pContext, drawMgr.aabbIndices_.data(), numIndicesInAABB);

    // render
    for (int i = 0; i < numAABBs; ++i)
        pContext->DrawIndexed(numIndicesInAABB, 0, i * numVerticesInAABB);
}

//---------------------------------------------------------

void RenderDebugLinesSphere(
    Render::CRender* pRender,
    cvector<VertexPosColor>& vertices)
{
    assert(pRender != nullptr);

    const DebugDrawMgr& drawMgr = g_DebugDrawMgr;

    if (drawMgr.currNumSpheres_ <= 0)
        return;

    const int numSpheres          = drawMgr.currNumSpheres_;
    const int numVerticesInSphere = drawMgr.GetNumVerticesInSphere();
    const int numIndicesInSphere  = drawMgr.GetNumIndicesInSphere();
    const int numVertices         = numSpheres * numVerticesInSphere;

    // fill arr with lines vertices data and update the vertex buffer with it
    vertices.resize(numVertices);

    // setup vertices for each shpere
    for (int sphereIdx = 0, vertIdx = 0; sphereIdx < numSpheres; ++sphereIdx)
    {
        for (int i = 0; i < numVerticesInSphere; ++i, ++vertIdx)
        {
            const DebugLineVertex& v = drawMgr.sphereVertices_[vertIdx];

            vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    VertexBuffer<VertexPosColor>& vb = g_ModelMgr.GetDebugLinesVB();
    IndexBuffer<uint16>&          ib = g_ModelMgr.GetDebugLinesIB();
    ID3D11DeviceContext*    pContext = pRender->GetContext();

    vb.UpdateDynamic(pContext, vertices.data(), numSpheres * numVerticesInSphere);
    ib.Update(pContext, drawMgr.sphereIndices_.data(), numIndicesInSphere);

    // render
    for (int i = 0; i < numSpheres; ++i)
        pContext->DrawIndexed(numIndicesInSphere, 0, i * numVerticesInSphere);
}

//---------------------------------------------------------

void RenderDebugLinesTerrainAABB(
    Render::CRender* pRender,
    cvector<VertexPosColor>& vertices)
{
    assert(pRender != nullptr);

    const DebugDrawMgr& drawMgr = g_DebugDrawMgr;

    if (drawMgr.currNumAABBs_ <= 0)
        return;

    const int numAABBs          = drawMgr.currNumTerrainAABBs_;
    const int numVerticesInAABB = drawMgr.GetNumVerticesInAABB();
    const int numIndicesInAABB  = drawMgr.GetNumIndicesInAABB();
    const int numVertices       = numAABBs * numVerticesInAABB;

    // fill arr with lines vertices data and update the vertex buffer with it
    vertices.resize(numVertices);

    // setup vertices for each aabb
    for (int aabbIdx = 0, vertIdx = 0; aabbIdx < numAABBs; ++aabbIdx)
    {
        for (int i = 0; i < numVerticesInAABB; ++i, ++vertIdx)
        {
            const DebugLineVertex& v = drawMgr.terrainAABBsVertices_[vertIdx];

            vertices[vertIdx].position = { v.pos.x, v.pos.y, v.pos.z };
            vertices[vertIdx].color    = v.color;
        }
    }

    // update buffers
    VertexBuffer<VertexPosColor>& vb = g_ModelMgr.GetDebugLinesVB();
    IndexBuffer<uint16>&          ib = g_ModelMgr.GetDebugLinesIB();
    ID3D11DeviceContext*    pContext = pRender->GetContext();

    vb.UpdateDynamic(pContext, vertices.data(), numAABBs * numVerticesInAABB);
    ib.Update(pContext, drawMgr.aabbIndices_.data(), numIndicesInAABB);

    // render
    for (int i = 0; i < numAABBs; ++i)
        pContext->DrawIndexed(numIndicesInAABB, 0, i * numVerticesInAABB);
}

//---------------------------------------------------------

void RenderDebugLinesFrustum(Render::CRender* pRender)
{
    assert(pRender != nullptr);

    // fill arr with vertices data
    constexpr int numLinesInFrustum = 12;
    constexpr int numVertices = 24;
    VertexPosColor vertices[numVertices];

    for (int lineIdx = 0, vertIdx = 0; lineIdx < numLinesInFrustum; ++lineIdx)
    {
        const DebugLine& line = g_DebugDrawMgr.frustumLines_[lineIdx];

        // setup the first vertex
        vertices[vertIdx].position.x = line.fromPos.x;
        vertices[vertIdx].position.y = line.fromPos.y;
        vertices[vertIdx].position.z = line.fromPos.z;
        vertices[vertIdx].color = line.color;

        vertIdx++;

        // setup the second vertex
        vertices[vertIdx].position.x = line.toPos.x;
        vertices[vertIdx].position.y = line.toPos.y;
        vertices[vertIdx].position.z = line.toPos.z;
        vertices[vertIdx].color = line.color;

        vertIdx++;
    }

    // update the vertex buffer with new data
    ID3D11DeviceContext* pContext = pRender->GetContext();
    g_ModelMgr.GetDebugLinesVB().UpdateDynamic(pContext, vertices, numVertices);

    // render frustum
    pContext->Draw(numVertices, 0);
}

//---------------------------------------------------------

void CGraphics::RenderDebugShapes()
{
    if (!g_DebugDrawMgr.doRendering_)
        return;


    // reset all the render states to default before lines rendering
    ResetRenderStatesToDefault();

    // setup rendering pipeline
    VertexBuffer<VertexPosColor>& vb = g_ModelMgr.GetDebugLinesVB();
    IndexBuffer<uint16>&          ib = g_ModelMgr.GetDebugLinesIB();
    Render::CRender*         pRender = pRender_;
    ID3D11DeviceContext*    pContext = pRender->GetContext();

    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pRender->BindShaderByName("DebugLineShader");

    // we always use the same VB/IB for rendering debug shaped, we just update them
    pRender->BindVB(vb.GetAddrOf(), vb.GetStride(), 0);
    pRender->BindIB(ib.Get(), DXGI_FORMAT_R16_UINT);


    // common arr of vertices for rendering
    cvector<VertexPosColor> vertices(1024);

    if (g_DebugDrawMgr.renderDbgLines_)
        RenderDebugLines(pRender, vertices);

    if (g_DebugDrawMgr.renderDbgAABB_)
        RenderDebugLinesAABB(pRender, vertices);

    if (g_DebugDrawMgr.renderDbgSphere_)
        RenderDebugLinesSphere(pRender, vertices);

    if (!pSysState_->isGameMode && g_DebugDrawMgr.renderDbgFrustum_)
        RenderDebugLinesFrustum(pRender);

    if (g_DebugDrawMgr.renderDbgTerrainAABB_)
        RenderDebugLinesTerrainAABB(pRender, vertices);



    pRender->SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    uint32 startInstanceLocation = 0;

    pRender->BindShaderByName("WireframeShader");

    for (const Render::InstanceBatch& batch : pRender->dataStorage_.masked)
    {
        const UINT instancesBuffElemSize = (UINT)(sizeof(ConstBufType::InstancedData));

        // bind vertex/index buffers for these instances
        ID3D11Buffer* const vbs[2] = { batch.pVB, pRender->pInstancedBuffer_ };
        const UINT strides[2] = { batch.vertexStride, instancesBuffElemSize };
        const UINT offsets[2] = { 0,0 };

        pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
        pContext->IASetIndexBuffer(batch.pIB, DXGI_FORMAT_R32_UINT, 0);


        const Render::Subset& subset = batch.subset;

        pContext->DrawIndexedInstanced(
            subset.indexCount,
            batch.numInstances,
            subset.indexStart,
            subset.vertexStart,
            startInstanceLocation);

        // stride idx in the instances buffer and accumulate render statistic
        startInstanceLocation += (UINT)batch.numInstances;
    }



    for (const Render::InstanceBatch& batch : pRender->dataStorage_.opaque)
    {
        const UINT instancesBuffElemSize = (UINT)(sizeof(ConstBufType::InstancedData));

        // bind vertex/index buffers for these instances
        ID3D11Buffer* const vbs[2] = { batch.pVB, pRender->pInstancedBuffer_ };
        const UINT strides[2] = { batch.vertexStride, instancesBuffElemSize };
        const UINT offsets[2] = { 0,0 };

        pContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);
        pContext->IASetIndexBuffer(batch.pIB, DXGI_FORMAT_R32_UINT, 0);


        const Render::Subset& subset = batch.subset;

        pContext->DrawIndexedInstanced(
            subset.indexCount,
            batch.numInstances,
            subset.indexStart,
            subset.vertexStart,
            startInstanceLocation);

        // stride idx in the instances buffer and accumulate render statistic
        startInstanceLocation += (UINT)batch.numInstances;
    }

    pRender->GetRenderStates().ResetRS(pContext);
    pRender->GetRenderStates().ResetBS(pContext);
    pRender->GetRenderStates().ResetDSS(pContext);
}

//---------------------------------------------------------
// Desc:  bind a shader according to the input post effect's type
//---------------------------------------------------------
void BindPostFxShader(
    ID3D11DeviceContext* pContext,
    Render::CRender* pRender,
    const ePostFxType fxType)
{
    assert(pContext != nullptr);
    assert(pRender != nullptr);

    switch (fxType)
    {
        case POST_FX_VISUALIZE_DEPTH:
            pRender->BindShaderByName("DepthResolveShader");
            pContext->Draw(3, 0);
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
    ID3D11DeviceContext* pContext = GetContext();

    // bind dst render target and src shader resource view
    assert(d3d.pSwapChainRTV_     != nullptr && "swap chain RTV is wrong");
    assert(d3d.postFxsPassSRV_[0] != nullptr && "resolved SRV is wrong");

    pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    pContext->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    BindPostFxShader(pContext, pRender_, postFxsQueue_[0]);
    pContext->Draw(3, 0);

    // bind a depth stencil view back
    pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:  when we have multiple post process passes we have to flip btw
//        two render targets each time when render a pass;
//        for the final pass we do rendering into the swap chain't RTV
//---------------------------------------------------------
void CGraphics::RenderPostFxMultiplePass()
{
    Render::D3DClass&    d3d      = GetD3D();
    ID3D11DeviceContext* pContext = GetContext();

    pContext->OMSetRenderTargets(1, &d3d.postFxsPassRTV_[1], nullptr);
    pContext->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[0]);

    int i = 0;
    int lastDstIdx = 0;

    for (i = 0; i < numPostFxsInQueue_ - 1; ++i)
    {
        // by this index we will get a source SRV for the last post process pass
        lastDstIdx = (i % 2 == 0);

        BindPostFxShader(pContext, pRender_, postFxsQueue_[i]);
        pContext->Draw(3, 0);

        // flip render targets and shader resource views
        // (one target becomes a dst, and another becomes a src)
        const int nextTargetIdx = (i % 2 != 0);
        const int nextSrcIdx    = (i % 2 == 0);

        pContext->OMSetRenderTargets(1, &d3d.postFxsPassRTV_[nextTargetIdx], nullptr);
        pContext->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[nextSrcIdx]);
    }

    // final pass we render directly into swap chain's RTV
    BindPostFxShader(pContext, pRender_, postFxsQueue_[i]);
    pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, nullptr);
    pContext->PSSetShaderResources(TEX_SLOT_POST_FX_SRC, 1, &d3d.postFxsPassSRV_[lastDstIdx]);
    pContext->Draw(3, 0);

    // bind depth stencil back
    pContext->OMSetRenderTargets(1, &d3d.pSwapChainRTV_, d3d.pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:   visualize values from the depth buffer (we do it after usual rendering)
//---------------------------------------------------------
void CGraphics::VisualizeDepthBuffer()
{
    Render::D3DClass&    d3d      = GetD3D();
    ID3D11DeviceContext* pContext = GetContext();

    // unbind depth before depth visualization
    d3d.UnbindDepthBuffer();

    if (d3d.IsEnabled4xMSAA())
    {
        ID3D11ShaderResourceView* pDepthMSAASRV = d3d.GetDepthSRV();
        pContext->PSSetShaderResources(TEX_SLOT_DEPTH_MSAA, 1, &pDepthMSAASRV);

        pRender_->BindShaderByName("DepthResolveShader");
        pContext->Draw(3, 0);
    }
    else
    {
        // setup depth SRV
        ID3D11ShaderResourceView* pDepthSRV = d3d.GetDepthSRV();
        pContext->PSSetShaderResources(TEX_SLOT_DEPTH, 1, &pDepthSRV);

        pRender_->BindShaderByName("VisualizeDepthShader");
        pContext->Draw(3, 0);
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
    assert(pEnttMgr_);
    ECS::EntityMgr& mgr = *pEnttMgr_;

    ECS::LightSystem&     lightSys           = mgr.lightSystem_;
    ECS::RenderSystem&    renderSys          = mgr.renderSystem_;
    ECS::TransformSystem& transformSys       = mgr.transformSystem_;

    const ECS::DirLights&   dirLights        = lightSys.GetDirLights();
    const ECS::PointLights& pointLights      = lightSys.GetPointLights();
    const ECS::SpotLights&  spotLights       = lightSys.GetSpotLights();

    const size numAllDirLights               = dirLights.data.size();
    const size numAllPointLights             = pointLights.data.size();
    const size numAllSpotLights              = spotLights.data.size();

    const cvector<EntityID>& visPointLights  = renderSys.GetVisiblePointLights();
    const size numVisPointLights             = visPointLights.size();

    //printf("num visible point lights: %d\n", (int)numVisPointLights);

    cvector<ECS::PointLight>& activePointL    = s_LightTmpData.activePointLights;
    cvector<ECS::XMFLOAT3>&   pointLPositions = s_LightTmpData.pointLightsPositions;
    cvector<ECS::SpotLight>&  spotLData       = s_LightTmpData.spotLightsData;
    cvector<XMVECTOR>&        dirLDirections  = s_LightTmpData.dirLightsDirections;

    // gather data of currently visible and active point light sources
    lightSys.GetPointLightsData(
        visPointLights.data(),
        visPointLights.size(),
        activePointL,
        pointLPositions);

    //printf("num visible point lights to render: %d\n", (int)activePointL.size());


    lightSys.GetSpotLightsData(
        spotLights.ids.data(),
        numAllSpotLights,
        spotLData,
        s_LightTmpData.spotLightsPositions,
        s_LightTmpData.spotLightsDirections);

    const int numActivePointLights = (int)s_LightTmpData.activePointLights.size();
    const int numActiveSpotLights  = (int)s_LightTmpData.spotLightsData.size();

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

    transformSys.GetDirections(
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

//---------------------------------------------------------
// Desc:  in game mode cast a ray from player to world
//        and compute an intersection point with any stuff on the scene
// 
//---------------------------------------------------------
void CGraphics::GetRayIntersectionPoint(const int sx, const int sy)
{
    // check if we have any entity by input screen coords;
// 
// in:   screen pixel coords
// out:  0  - there is no entity, we didn't select any
//       ID - we selected some entity so return its ID

    XMFLOAT3 intersectionP(0,0,0);

    using namespace DirectX;

    ECS::EntityMgr& enttMgr = *pEnttMgr_;
    const EntityID currCamId = currCameraID_;
    const ECS::CameraSystem& camSys = enttMgr.cameraSystem_;

    const XMFLOAT3  camPos  = camSys.GetPos(currCamId);
    const XMMATRIX& P       = camSys.GetProj(currCamId);
    const XMMATRIX& invView = camSys.GetInverseView(currCamId);

    // TODO: optimize it!
    const float xndc = (+2.0f * sx / GetD3D().GetWindowWidth()  - 1.0f);
    const float yndc = (-2.0f * sy / GetD3D().GetWindowHeight() + 1.0f);

    // compute picking ray in view space
    const float vx = xndc / P.r[0].m128_f32[0];
    const float vy = yndc / P.r[1].m128_f32[1];

    // ray definition in world space
    XMVECTOR rayOriginW = { camPos.x, camPos.y, camPos.z, 1 };
    XMVECTOR rayDirW    = XMVector3Normalize(XMVector3TransformNormal({ vx, vy, 1, 0 }, invView));     // supposed to take a vec (w == 0)

    // normal vector of intersected triangle (if we have such)
    XMFLOAT3 normalVec = { 0,0,0 };

    // assume we have not picked anything yet, 
    // so init the ID to 0 and triangle idx to -1
    uint32_t selectedEnttID = 0;
    int selectedTriangleIdx = -1;
    float tmin = MathHelper::Infinity;  // the distance along the ray where the intersection occurs
    float dist = 0;                     // the length of the ray from origin to the intersection point with the AABB


    // go through each visible entt and check if we have an intersection with it
    for (const EntityID enttId : enttMgr.renderSystem_.GetAllVisibleEntts())
    {
        const char* enttName = enttMgr.nameSystem_.GetNameById(enttId);

        if (strcmp(enttName, "player") == 0)
            continue;

        const ModelID modelID   = enttMgr.modelSystem_.GetModelIdRelatedToEntt(enttId);
        
        const BasicModel& model = g_ModelMgr.GetModelById(modelID);

        // get an inverse world matrix of the current entt

        // transform model's AABB from local space to world space
        const XMMATRIX world = enttMgr.transformSystem_.GetWorld(enttId);
        BoundingBox aabb;
        model.GetModelAABB().Transform(aabb, world);

        // if we hit the bounding box of the model, then we might have picked
        // a model triangle, so do the ray/triangle tests;
        //
        // if we didn't hit the bounding box, then it is impossible that we
        // hit the model, so do not waste effort doing ray/triangle tests
        if (aabb.Intersects(rayOriginW, rayDirW, dist))
        {
            // execute ray/triangle tests
            for (int i = 0; i < model.GetNumIndices() / 3; ++i)
            {
                // indices for this triangle
                UINT i0 = model.indices_[i * 3 + 0];
                UINT i1 = model.indices_[i * 3 + 1];
                UINT i2 = model.indices_[i * 3 + 2];

                // vertices for this triangle
                XMVECTOR v0 = XMLoadFloat3(&model.vertices_[i0].position);
                XMVECTOR v1 = XMLoadFloat3(&model.vertices_[i1].position);
                XMVECTOR v2 = XMLoadFloat3(&model.vertices_[i2].position);

                v0 = XMVector3TransformCoord(v0, world);
                v1 = XMVector3TransformCoord(v1, world);
                v2 = XMVector3TransformCoord(v2, world);

                // we have to iterate over all the triangle in order 
                // to find the nearest intersection
                float t = 0.0f;

                if (DirectX::TriangleTests::Intersects(rayOriginW, rayDirW, v0, v1, v2, t))
                {
                    if (t <= tmin)
                    {
                        // this is the new nearest picked entt and its triangle
                        tmin = t;
                        selectedTriangleIdx = i;
                        selectedEnttID = enttId;
                        normalVec = model.vertices_[i0].normal;

                        XMVECTOR currIntersection = rayOriginW + rayDirW * t;
                        XMStoreFloat3(&intersectionP, currIntersection);
                    }
                }
            }
        }
    }

    // print a msg about selection of the entity
    if (selectedEnttID)
    {
        const EntityID currWeaponId = enttMgr.playerSystem_.GetActiveWeapon();
        DirectX::XMFLOAT3 relPos = enttMgr.hierarchySystem_.GetRelativePos(currWeaponId);

        XMFLOAT3 rayOrig;
        XMStoreFloat3(&rayOrig, rayOriginW);
       
        const Vec3 fromPos(rayOrig.x + relPos.x, rayOrig.y + relPos.y, rayOrig.z + relPos.z);
        const Vec3 toPos(intersectionP.x, intersectionP.y, intersectionP.z);
        const Vec3 color(1, 0, 0);

        g_DebugDrawMgr.AddLine(fromPos, toPos, color);

#if 1
        SetConsoleColor(YELLOW);

        const char* enttName = pEnttMgr_->nameSystem_.GetNameById(selectedEnttID);
        

        printf("hitEntt (id: %" PRIu32 "  '%s'   orig: %.2f %.2f %.2f   i: %.2f %.2f %.2f   d: %.2f   n: %.2f %.2f %.2f)\n",
            selectedEnttID,
            enttName,
            rayOrig.x, rayOrig.y, rayOrig.z,
            intersectionP.x, intersectionP.y, intersectionP.z,
            tmin,
            normalVec.x, normalVec.y, normalVec.z);

        const EntityID splashEmitterId = pEnttMgr_->nameSystem_.GetIdByName("shot_splash");
        pEnttMgr_->AddEvent(ECS::EventTranslate(splashEmitterId, intersectionP.x, intersectionP.y, intersectionP.z));

        XMFLOAT3 rayDir;
        XMStoreFloat3(&rayDir, -rayDirW);

        Vec3 ray  = { rayDir.x, rayDir.y, rayDir.z };
        Vec3 norm = { normalVec.x, normalVec.y, normalVec.z };

        float dot = Vec3Dot(ray, norm);
        if (dot < 0)
            norm = -norm;

        pEnttMgr_->particleSystem_.SetExternForces(splashEmitterId, norm.x*0.001f, -0.006f, norm.z*0.001f);
        pEnttMgr_->particleSystem_.ResetNumSpawnedParticles(splashEmitterId);

        SetConsoleColor(RESET);
#endif
    }
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
    // check if we have any entity by input screen coords;
    // 
    // in:   screen pixel coords
    // out:  0  - there is no entity, we didn't select any
    //       ID - we selected some entity so return its ID

    using namespace DirectX;

    ECS::EntityMgr* pEnttMgr = pEnttMgr_;
    const XMMATRIX& P        = pEnttMgr->cameraSystem_.GetProj(currCameraID_);
    const XMMATRIX& invView  = pEnttMgr->cameraSystem_.GetInverseView(currCameraID_);

    // TODO: optimize it!
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
    uint32_t selectedEnttID = 0;
    int selectedTriangleIdx = -1;
    float tmin = MathHelper::Infinity;  // the distance along the ray where the intersection occurs
    float dist = 0;                     // the length of the ray from origin to the intersection point with the AABB


    // go through each visible entt and check if we have an intersection with it
    for (const EntityID enttID : pEnttMgr->renderSystem_.GetAllVisibleEntts())
    {
        //const EntityID enttID = visEntts[i];
        const ModelID modelID = pEnttMgr->modelSystem_.GetModelIdRelatedToEntt(enttID);
        const BasicModel& model = g_ModelMgr.GetModelById(modelID);

        if (model.type_ == MODEL_TYPE_Terrain)
        {
            continue;
        }
    
        // get an inverse world matrix of the current entt
        const XMMATRIX invWorld = pEnttMgr->transformSystem_.GetInverseWorld(enttID);
        const XMMATRIX toLocal = DirectX::XMMatrixMultiply(invView, invWorld);

        XMVECTOR rayOrigin = XMVector3TransformCoord(rayOrigin_, toLocal);   // supposed to take a point (w == 1)
        XMVECTOR rayDir    = XMVector3TransformNormal(rayDir_, toLocal);  // supposed to take a vec (w == 0)

        // make the ray direction unit length for the intersection tests
        rayDir = XMVector3Normalize(rayDir);


        // if we hit the bounding box of the model, then we might have picked
        // a model triangle, so do the ray/triangle tests;
        //
        // if we didn't hit the bounding box, then it is impossible that we
        // hit the model, so do not waste efford doing ray/triangle tests
        if (model.GetModelAABB().Intersects(rayOrigin, rayDir, dist))
        {
            // execute ray/triangle tests
            for (int i = 0; i < model.GetNumIndices() / 3; ++i)
            {
                // indices for this triangle
                UINT i0 = model.indices_[i * 3 + 0];
                UINT i1 = model.indices_[i * 3 + 1];
                UINT i2 = model.indices_[i * 3 + 2];

                // vertices for this triangle
                XMVECTOR v0 = XMLoadFloat3(&model.vertices_[i0].position);
                XMVECTOR v1 = XMLoadFloat3(&model.vertices_[i1].position);
                XMVECTOR v2 = XMLoadFloat3(&model.vertices_[i2].position);

                // we have to iterate over all the triangle in order 
                // to find the nearest intersection
                float t = 0.0f;

                if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
                {
                    if (t <= tmin)
                    {
                        // this is the new nearest picked entt and its triangle
                        tmin = t;
                        selectedTriangleIdx = i;
                        selectedEnttID = enttID;
                    }
                }
            }
        }
    }

    // print a msg about selection of the entity
    if (selectedEnttID)
    {
        const char* name = pEnttMgr->nameSystem_.GetNameById(selectedEnttID);

        SetConsoleColor(YELLOW);
        LogMsg("picked entt (id: %" PRIu32 "; name: % s)", selectedEnttID, name);
        SetConsoleColor(RESET);
    }

    // return ID of the selected entt, or 0 if we didn't pick any
    return selectedEnttID;
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
    RenderStates& renderStates    = GetD3D().GetRenderStates();
    ID3D11DeviceContext* pContext = GetContext();

    pRender_->SwitchAlphaClipping(false);
    renderStates.ResetRS(pContext);
    renderStates.ResetBS(pContext);
    renderStates.ResetDSS(pContext);
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

    if (!pModelFrameBuf_->Initialize(GetDevice(), fbSpec))
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

    Render::CRender*     pRender  = pRender_;
    ID3D11DeviceContext* pContext = pRender->GetContext();

    // reset render states to prevent rendering bugs
    Render::RenderStates& renderStates = pRender->GetRenderStates();
    renderStates.ResetRS(pContext);
    renderStates.ResetBS(pContext);
    renderStates.SetDSS(pContext, R_DEPTH_ENABLED, 0);

    // prepare model's instance
    const ModelPreviewRenderParams& params = modelPreviewRndParams_;
    BasicModel&                     model  = g_ModelMgr.GetModelById(params.modelId);
    const MeshGeometry&             meshes = model.meshes_;

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

    if (!params.useOrthoMatrix)
        P = XMMatrixPerspectiveFovLH(DEG_TO_RAD(75), pFB->GetAspect(), pFB->GetNearZ(), pFB->GetFarZ());
    else
        P = XMMatrixOrthographicLH(DEG_TO_RAD(75), params.orthoViewHeight, pFB->GetNearZ(), pFB->GetFarZ());

   
    // save pos for later restoring
    const XMFLOAT3 prevCamPos = pRender->GetCameraPos();

    pRender->UpdateCbWorldInvTranspose(MathHelper::InverseTranspose(W));
    pRender->UpdateCbWorldAndViewProj(W, DirectX::XMMatrixTranspose(V * P));
    pRender->UpdateCbCameraPos({ trCamPos.x, trCamPos.y, trCamPos.z });

    // prepare and bind a frame buffer
    const XMFLOAT4 clearColor = { params.bgColor.r, params.bgColor.g, params.bgColor.b, 1 };
    pFB->ClearBuffers(pContext, clearColor);
    pFB->Bind(pContext);

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

        pContext->DrawIndexed(mesh.indexCount, mesh.indexStart, mesh.vertexStart);
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


} // namespace Core
