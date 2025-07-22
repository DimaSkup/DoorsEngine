// =================================================================================
// Filename: CGraphics.cpp
// Created:  14.10.22
// =================================================================================
#include <CoreCommon/pch.h>
#include "CGraphics.h"
#include "../Input/inputcodes.h"
#include "../Texture/TextureMgr.h"
#include "../Model/ModelMgr.h"
#include "../Mesh/MaterialMgr.h"

using namespace DirectX;

#define TERRAIN_V1 false

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
} s_LightTmpData;

//---------------------------------------------------------

CGraphics::CGraphics() :
    texturesBuf_(NUM_TEXTURE_TYPES, nullptr)
{
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
bool CGraphics::Initialize(
    HWND hwnd,
    SystemState& systemState,
    const EngineConfigs& cfg,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    return InitHelper(hwnd, systemState, cfg, pEnttMgr, pRender);
}

void CGraphics::Update(
    SystemState& sysState,
    const float deltaTime,
    const float gameTime,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    UpdateHelper(sysState, deltaTime, gameTime, pEnttMgr, pRender);
}

///////////////////////////////////////////////////////////

void CGraphics::Render3D(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender)
{
    RenderHelper(pEnttMgr, pRender);
}

///////////////////////////////////////////////////////////

bool CGraphics::InitHelper(
    HWND hwnd, 
    SystemState& systemState,
    const EngineConfigs& cfg,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    try
    {
        bool result = false;

        SetConsoleColor(YELLOW);
        LogMsg("\n");
        LogMsg("------------------------------------------------------------");
        LogMsg("               INITIALIZATION: GRAPHICS SYSTEM              ");
        LogMsg("------------------------------------------------------------\n");
        SetConsoleColor(RESET);

        pSysState_ = &systemState;

        result = d3d_.Initialize(
            hwnd,
            cfg.GetBool("VSYNC_ENABLED"),
            cfg.GetBool("FULL_SCREEN"),
            cfg.GetBool("ENABLE_4X_MSAA"),
            cfg.GetFloat("NEAR_Z"),
            cfg.GetFloat("FAR_Z"));         // how far we can see

        CAssert::True(result, "can't initialize the D3DClass");

        // setup the rasterizer state to default params
        d3d_.SetRS({ eRenderState::CULL_BACK, eRenderState::FILL_SOLID });

        // after initialization of the DirectX we can use pointers to the device and device context
        d3d_.GetDeviceAndContext(pDevice_, pDeviceContext_);

        // initializer the textures global manager (container)
        g_TextureMgr.Initialize(pDevice_);
    
        // create frustums for frustum culling
        frustums_.push_back(DirectX::BoundingFrustum());

        BuildGeometryBuffers();
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize the graphics class");
        this->Shutdown();
        return false;
    }

    LogMsg(" is successfully initialized");
    return true;
}

///////////////////////////////////////////////////////////

void CGraphics::Shutdown()
{
    // Shutdowns all the graphics rendering parts, releases the memory
    LogDbg(LOG, "graphics shutdown");
    d3d_.Shutdown();
}


// =================================================================================
// Update / prepare scene
// =================================================================================

void CGraphics::UpdateHelper(
    SystemState& sysState,
    const float deltaTime,
    const float totalGameTime,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    // update all the graphics related stuff for this frame


    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    

    // ---------------------------------------------
    // update the cameras states

    const EntityID currCamID = currCameraID_;

    if (isGameMode_)
    {
        ECS::PlayerSystem& player = pEnttMgr->playerSystem_;
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
            // make player's offset by Y-axis to be at fixed height over the terrain
            const float terrainHeight = terrain.GetScaledInterpolatedHeightAtPoint(playerPos.x, playerPos.z);
            const float offsetOverTerrain = 2;

            // moving up and down the camera during movement
            //float f = sinf(playerPos.x * 4.0f) + cosf(playerPos.z * 4.0f);
            //f /= 35.0f;
            //player.SetMinVerticalOffset(terrainHeight + offsetOverTerrain + f);

            player.SetMinVerticalOffset(terrainHeight + offsetOverTerrain);
        }


        sysState.cameraPos = { playerPos.x, playerPos.y, playerPos.z };
    }
    else
    {
        sysState.cameraPos = pEnttMgr->transformSystem_.GetPosition(currCamID);
    }

    sysState.cameraDir = pEnttMgr->transformSystem_.GetDirection(currCamID);

    pEnttMgr->cameraSystem_.UpdateView(currCamID);

    sysState.cameraView = pEnttMgr->cameraSystem_.GetView(currCamID);
    sysState.cameraProj = pEnttMgr->cameraSystem_.GetProj(currCamID);
    viewProj_ = sysState.cameraView * sysState.cameraProj;

    // build the frustum in view space from the projection matrix
    DirectX::BoundingFrustum::CreateFromMatrix(frustums_[0], sysState.cameraProj);


    // ---------------------------------------------
    // update the terrain

    // get camera params which will be used for culling of terrain patches 
    CameraParams camParams;

    // setup camera position
    camParams.posX = sysState.cameraPos.x;
    camParams.posY = sysState.cameraPos.y;
    camParams.posZ = sysState.cameraPos.z;

    // setup view matrix
    memcpy(camParams.viewMatrix, &sysState.cameraView.r->m128_f32, 16 * sizeof(float));
    memcpy(camParams.projMatrix, &sysState.cameraProj.r->m128_f32, 16 * sizeof(float));

    camParams.fovX = pEnttMgr->cameraSystem_.GetFovX(currCamID);
    camParams.fovY = pEnttMgr->cameraSystem_.GetFovY(currCamID);

    camParams.nearZ = pEnttMgr->cameraSystem_.GetNearZ(currCamID);
    camParams.farZ = pEnttMgr->cameraSystem_.GetFarZ(currCamID);


    // 6 planes representation of frustum
    DirectX::XMVECTOR planes[6];
    frustums_[0].GetPlanes(&planes[0], &planes[1], &planes[2], &planes[3], &planes[4], &planes[5]);

    // setup params for each frustum plane 
    for (int i = 0; i < 6; ++i)
    {
        // normalized plane (normal: x,y,z; w is d = -dot(n,p0))
        camParams.planes[i][0] = -planes[i].m128_f32[0];
        camParams.planes[i][1] = -planes[i].m128_f32[1];
        camParams.planes[i][2] = -planes[i].m128_f32[2];
        camParams.planes[i][3] = -planes[i].m128_f32[3];
    }

    // recompute terrain patches and load them into GPU
    terrain.Update(camParams);

#if TERRAIN_V1
    terrain.vb_.UpdateDynamic(pDeviceContext_, terrain.vertices_, terrain.verticesOffset_);
    terrain.ib_.Update(pDeviceContext_, terrain.indices_, terrain.indicesOffset_);
#endif

    static XMFLOAT3 prevCamPos = { 0,0,0 };
    bool updateTerrain = false;

    if (prevCamPos != sysState.cameraPos)
    {
        updateTerrain = true;
        prevCamPos = sysState.cameraPos;
    }

    // recompute terrain's quadtree
    if (updateTerrain)
    {
        //TerrainQuadtree& trnQuadtree = g_ModelMgr.GetTerrainQuadtree();
        //trnQuadtree.Update(camParams);
    }



    // ------------------------------------------
    // perform frustum culling on all of our currently loaded entities
    ComputeFrustumCulling(sysState, pEnttMgr);
    ComputeFrustumCullingOfLightSources(sysState, pEnttMgr);

    // Update shaders common data for this frame
    UpdateShadersDataPerFrame(pEnttMgr, pRender);

    // prepare all the visible entities data for rendering
    const cvector<EntityID>& visibleEntts = pEnttMgr->renderSystem_.GetAllVisibleEntts();

    // separate entts into opaque, entts with alpha clipping, blended, etc.
    pEnttMgr->renderStatesSystem_.SeparateEnttsByRenderStates(visibleEntts, rsDataToRender_);

    pSysState_->visibleVerticesCount = 0;


#if 0
    //
    // separate entts by distance
    //

    // render as lit entts only that entts which are closer than some distance
    // all entts that are farther will be rendered as fully fogged with a single fog color
    cvector<EntityID>& alphaClippedEntts = rsDataToRender_.enttsAlphaClipping_.ids_;
    const size numVisEntts = alphaClippedEntts.size();

  
    if (numVisEntts > 0)
    {
        using namespace DirectX;


        size count = 0;
        std::vector<index> idxs(numVisEntts);
        cvector<XMFLOAT3> positions;


        pEnttMgr->transformSystem_.GetPositions(alphaClippedEntts.data(), numVisEntts, positions);
        const int fullFogDistSqr = (fullFogDistance_ * fullFogDistance_);
        const XMVECTOR camPos = XMLoadFloat3(&sysState.cameraPos);

        // check if entity by idx is farther than fog range if so we store its idx
        for (index i = 0; i < numVisEntts; ++i)
        {
            const XMVECTOR enttPos      = XMLoadFloat3(&positions[i]);
            const XMVECTOR camToEnttVec = DirectX::XMVectorSubtract(enttPos, camPos);
            const int distSqr           = (int)DirectX::XMVectorGetX(DirectX::XMVector3Dot(camToEnttVec, camToEnttVec));

            if (distSqr > fullFogDistSqr)
            {
                idxs[count] = i;
                ++count;
            }
        }

        idxs.resize(count);
        cvector<EntityID>& foggedEntts = rsDataToRender_.enttsFogged_.ids_;
        foggedEntts.resize(count);

        // store IDs of entts which are farther than fog range
        for (index i = 0; const index enttIdx : idxs)
        {
            foggedEntts[i++] = alphaClippedEntts[enttIdx];
        }

        // erase IDs of entts which are farther than fog range from the origin IDs array
        for (index i = 0; const index idx : idxs)
        {
            alphaClippedEntts.erase(idx - i);
            i++;
        }

        idxs.clear();
    }
#endif

    // ----------------------------------------------------
    // prepare data for each entts set
   
    PrepBasicInstancesForRender(pEnttMgr, pRender);
    PrepAlphaClippedInstancesForRender(pEnttMgr, pRender);
}

///////////////////////////////////////////////////////////

void CGraphics::PrepBasicInstancesForRender(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender)
{
    // prepare rendering data of entts which have default render states

    const EntityID* ids = rsDataToRender_.enttsDefault_.ids_.data();
    const size numEntts = rsDataToRender_.enttsDefault_.ids_.size();

    if (numEntts == 0)
        return;

    Render::RenderDataStorage& storage = pRender->dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        pEnttMgr,
        storage.modelInstBuffer,
        storage.modelInstances);

    // compute how many vertices will we render
    for (const Render::Instance& inst : storage.modelInstances)
        pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void CGraphics::PrepAlphaClippedInstancesForRender(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender)
{
    // prepare rendering data of entts which have alpha clip + cull none

    const EntityID* ids = rsDataToRender_.enttsAlphaClipping_.ids_.data();
    const size numEntts = rsDataToRender_.enttsAlphaClipping_.ids_.size();

    if (numEntts == 0)
        return;

    Render::RenderDataStorage& storage = pRender->dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        pEnttMgr,
        storage.alphaClippedModelInstBuffer,
        storage.alphaClippedModelInstances);

    // compute how many vertices will we render
    for (const Render::Instance& inst : storage.alphaClippedModelInstances)
        pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void CGraphics::PrepBlendedInstancesForRender(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    // prepare rendering data of entts which have alpha clip + cull none

    const EntityID* ids = rsDataToRender_.enttsBlended_.ids_.data();
    const size numEntts = rsDataToRender_.enttsBlended_.ids_.size();

    if (numEntts == 0)
        return;

    Render::RenderDataStorage& storage = pRender->dataStorage_;

    prep_.PrepareEnttsDataForRendering(
        ids,
        numEntts,
        pEnttMgr,
        storage.blendedModelInstBuffer,
        storage.blendedModelInstances);

    // compute how many vertices will we render
    for (const Render::Instance& inst : storage.alphaClippedModelInstances)
        pSysState_->visibleVerticesCount += inst.numInstances * inst.GetNumVertices();
}

///////////////////////////////////////////////////////////

void CGraphics::ComputeFrustumCulling(
    SystemState& sysState,
    ECS::EntityMgr* pEnttMgr)
{
    ECS::EntityMgr& mgr = *pEnttMgr;
    ECS::RenderSystem& renderSys = mgr.renderSystem_;

    const cvector<EntityID>& enttsRenderable = renderSys.GetAllEnttsIDs();
    const size numRenderableEntts = enttsRenderable.size();
    size numVisEntts = 0;                                     // the number of currently visible entts

    if (numRenderableEntts == 0)
        return;

    static cvector<BoundingSphere> boundSpheres;                   // bounding sphere of the whole entity
    static cvector<index>          idxsToVisEntts(numRenderableEntts);
    static cvector<XMMATRIX>       enttsLocal(numRenderableEntts);

    // get arr of bounding spheres for each renderable entt
    mgr.boundingSystem_.GetBoundSpheres(
        enttsRenderable.data(),
        numRenderableEntts,
        boundSpheres);

#if 1
    // inverse world matrix of each renderable entt
    static cvector<XMMATRIX> invWorlds;  

    // get inverse world matrix of each renderable entt
    mgr.transformSystem_.GetInverseWorlds(
        enttsRenderable.data(),
        numRenderableEntts,
        invWorlds);

    const XMMATRIX invView = mgr.cameraSystem_.GetInverseView(currCameraID_);

    // compute local space matrices for frustum transformations
    for (index i = 0; i < numRenderableEntts; ++i)
        enttsLocal[i] = DirectX::XMMatrixMultiply(invView, invWorlds[i]);

    // go through each entity and define if it is visible
    for (index idx = 0; idx < numRenderableEntts; ++idx)
    {
        // transform the camera frustum from view space to the object's local space
        DirectX::BoundingFrustum LSpaceFrustum;
        frustums_[0].Transform(LSpaceFrustum, enttsLocal[idx]);

        idxsToVisEntts[numVisEntts] = idx;
        numVisEntts += LSpaceFrustum.Intersects(boundSpheres[idx]);
    }

#else
    static ECS::cvector<XMMATRIX> worlds;

    mgr.transformSystem_.GetWorlds(
        enttsRenderable.data(),
        numRenderableEntts,
        worlds);

    // offset of entity in local space relatively to the Origin
    for (index i = 0; i < numRenderableEntts; ++i)
    {
        // change radius
        boundSpheres[i].Radius *= worlds[i].r[0].m128_f32[0];

        // change position
        float* pos = worlds[i].r[3].m128_f32;
        boundSpheres[i].Center.x += pos[0];
        boundSpheres[i].Center.y += pos[1];
        boundSpheres[i].Center.z += pos[2];
    }

    // transform camera's frustum to world space
    DirectX::BoundingFrustum WSpaceFrustum;
    frustums_[0].Transform(WSpaceFrustum, mgr.cameraSystem_.GetInverseView(currCameraID_));

    // Load origin and orientation of the frustum.
    XMVECTOR vOrigin = XMLoadFloat3(&WSpaceFrustum.Origin);
    XMVECTOR vOrientation = XMLoadFloat4(&WSpaceFrustum.Orientation);

    // Create 6 planes (do it inline to encourage use of registers)
    XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, WSpaceFrustum.Near);
    NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
    NearPlane = XMPlaneNormalize(NearPlane);

    XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -WSpaceFrustum.Far);
    FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
    FarPlane = XMPlaneNormalize(FarPlane);

    XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -WSpaceFrustum.RightSlope, 0.0f);
    RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
    RightPlane = XMPlaneNormalize(RightPlane);

    XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, WSpaceFrustum.LeftSlope, 0.0f);
    LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
    LeftPlane = XMPlaneNormalize(LeftPlane);

    XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -WSpaceFrustum.TopSlope, 0.0f);
    TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
    TopPlane = XMPlaneNormalize(TopPlane);

    XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, WSpaceFrustum.BottomSlope, 0.0f);
    BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
    BottomPlane = XMPlaneNormalize(BottomPlane);


    // go through each entity and define if it is visible
    for (index idx = 0; idx < numRenderableEntts; ++idx)
    {
        const DirectX::ContainmentType type = boundSpheres[idx].ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
        idxsToVisEntts[numVisEntts] = idx;
        numVisEntts += ((type == INTERSECTS) | (type == CONTAINS));
    }
#endif

    // ------------------------------------------

    // store ids of visible entts
    cvector<EntityID>& visibleEntts = renderSys.GetAllVisibleEntts();
    visibleEntts.resize(numVisEntts);

    for (index i = 0; i < numVisEntts; ++i)
        visibleEntts[i] = enttsRenderable[idxsToVisEntts[i]];

    sysState.visibleObjectsCount = (u32)numVisEntts;
}

///////////////////////////////////////////////////////////

void CGraphics::ComputeFrustumCullingOfLightSources(
    SystemState& sysState,
    ECS::EntityMgr* pEnttMgr)
{
    // store IDs of light sources which are currently visible by camera frustum
    // (by visibility means the WHOLE area which is lit by this light source)

    using namespace DirectX;
    ECS::EntityMgr& mgr = *pEnttMgr;

    mgr.renderSystem_.ClearVisibleLightSources();
    cvector<EntityID>& visPointLights = mgr.renderSystem_.GetVisiblePointLights();

    // get all the point light sources which are in the visibility range
    const size numPointLights      = mgr.lightSystem_.GetNumPointLights();
    const EntityID* pointLightsIDs = mgr.lightSystem_.GetPointLights().ids.data();


    static cvector<XMMATRIX> invWorlds(numPointLights);
    static cvector<XMMATRIX> localSpaces(numPointLights);

    // get inverse world matrix of each point light source
    mgr.transformSystem_.GetInverseWorlds(pointLightsIDs, numPointLights, invWorlds);

    const XMMATRIX invView = mgr.cameraSystem_.GetInverseView(currCameraID_);

    // compute local space matrices for frustum transformations
    for (int i = 0; i < numPointLights; ++i)
        localSpaces[i] = DirectX::XMMatrixMultiply(invView, invWorlds[i]);

    visPointLights.resize(numPointLights);
    u32 numVisPointLights = 0;
    constexpr DirectX::BoundingSphere defaultBoundSphere({ 0,0,0 }, 1);

    // go through each point light source and define if it is visible
    for (int idx = 0; idx < numPointLights; ++idx)
    {
        // transform the camera frustum from view space to the point light local space
        BoundingFrustum LSpaceFrustum;
        frustums_[0].Transform(LSpaceFrustum, localSpaces[idx]);

        // if we see any part of bound sphere we store an index to related light source
        if (LSpaceFrustum.Intersects(defaultBoundSphere))
            visPointLights[numVisPointLights++] = pointLightsIDs[idx];
    }

    visPointLights.resize(numVisPointLights);

    // we'll use these values to render counts onto the screen
    sysState.numVisiblePointLights = numVisPointLights;
}

///////////////////////////////////////////////////////////

void CGraphics::UpdateShadersDataPerFrame(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    // Update shaders common data for this frame: 
    // viewProj matrix, camera position, light sources data, etc.

    Render::PerFrameData& perFrameData = pRender->perFrameData_;

    perFrameData.viewProj = DirectX::XMMatrixTranspose(viewProj_);
    perFrameData.cameraPos = pEnttMgr->transformSystem_.GetPosition(currCameraID_);

    SetupLightsForFrame(pEnttMgr, perFrameData);

    // update lighting data, camera pos, etc. for this frame
    pRender->UpdatePerFrame(pDeviceContext_, perFrameData);
}

///////////////////////////////////////////////////////////

void CGraphics::ClearRenderingDataBeforeFrame(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    // clear rendering data from the previous frame / instances set

    pRender->dataStorage_.Clear();
    rsDataToRender_.Clear();
}

///////////////////////////////////////////////////////////

// memory allocation and releasing
void* CGraphics::operator new(size_t i)
{
    if (void* ptr = _aligned_malloc(i, 16))
        return ptr;

    LogErr("can't allocate memory for this object");
    throw std::bad_alloc{};
}

///////////////////////////////////////////////////////////

void CGraphics::operator delete(void* ptr)
{
    _aligned_free(ptr);
}


// =================================================================================
// Rendering methods
// =================================================================================
void CGraphics::RenderHelper(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender)
{
    try
    {
        // prepare the sky textures: in different shaders we will sample the sky
        // texture pixels so we bind them only once at the beginning of the frame
        const SkyModel& sky = g_ModelMgr.GetSky();
        const TexID* skyTexIDs = sky.GetTexIDs();
        const int skyTexMaxNum = sky.GetMaxTexturesNum();

        // get shader resource views (textures) for the sky
        ID3D11DeviceContext* pContext = pDeviceContext_;
        g_TextureMgr.GetSRVsByTexIDs(skyTexIDs, skyTexMaxNum, texturesBuf_);
        pContext->PSSetShaderResources(0U, 1U, texturesBuf_.data());


        RenderEnttsDefault(pRender);
        RenderEnttsAlphaClipCullNone(pRender);
        RenderTerrainGeomip(pRender, pEnttMgr);
        //RenderTerrainQuadtree(pRender, pEnttMgr);
        RenderSkyDome(pRender, pEnttMgr);
        //RenderFoggedBillboards(pRender, pEnttMgr);

#if 0
        pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        const EntityID* visEntts = entityMgr_.renderSystem_.GetAllVisibleEntts().data();
        const size numVisEntts = entityMgr_.renderSystem_.GetVisibleEnttsCount();

        // check if we are able and need to render bounding line boxes
        if ((numVisEntts != 0) && (aabbShowMode_ != NONE))
            return;

        RenderBoundingLineBoxes(pRender, pEnttMgr, visEntts, numVisEntts);
        RenderBoundingLineSpheres();
        RenderEnttsBlended();
#endif
    }
    catch (const std::out_of_range& e)
    {
        LogErr(e.what());
        LogErr("there is no such a key to data");
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't render 3D entts onto the scene");
    }
}

///////////////////////////////////////////////////////////

void CGraphics::RenderModel(BasicModel& model, const DirectX::XMMATRIX& world)
{
    // for specific purposes:
    // just render a single asset/model at the center of the world
}

///////////////////////////////////////////////////////////

void InitMatIconFrameBuffer(
    ID3D11Device* pDevice,
    FrameBuffer& buf,
    const int iconWidth,
    const int iconHeight,
    const float nearZ,
    const float farZ)
{
    // init a single frame buffer which is used to render material big icon (for the editor's material browser)

    // setup params for the frame buffers
    FrameBufferSpecification frameBufSpec;
    frameBufSpec.width          = (UINT)iconWidth;
    frameBufSpec.height         = (UINT)iconHeight;
    frameBufSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    frameBufSpec.screenNear     = nearZ;
    frameBufSpec.screenDepth    = farZ;

    // TODO: if input params differ from the frame buffer's params we need to recreate the frame buffer according to new params
    
    buf.Initialize(pDevice, frameBufSpec);
}

///////////////////////////////////////////////////////////

void InitMatIconFrameBuffers(
    ID3D11Device* pDevice,
    cvector<FrameBuffer>& frameBuffers,
    const size numIcons,
    const int iconWidth,
    const int iconHeight,
    const float nearZ,
    const float farZ)
{
    // initialize frame buffers which will be used to render material icons (for material browser in the editor)

    // setup params for the frame buffers (we will use the same params for each)
    FrameBufferSpecification frameBufSpec;
    frameBufSpec.width          = (UINT)iconWidth;
    frameBufSpec.height         = (UINT)iconHeight;
    frameBufSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    frameBufSpec.screenNear     = nearZ;
    frameBufSpec.screenDepth    = farZ;

    // release memory from the previous set of frame buffers
    for (FrameBuffer& buf : frameBuffers)
        buf.Shutdown();

    // alloc memory for frame buffers and init each frame buffer
    frameBuffers.resize(numIcons);

    for (index i = 0; i < numIcons; ++i)
    {
        if (!frameBuffers[i].Initialize(pDevice, frameBufSpec))
        {
            LogErr(LOG, "can't initialize a frame buffer (idx: %d)", (int)i);
        }
    }
}

///////////////////////////////////////////////////////////

bool CGraphics::RenderBigMaterialIcon(
    const MaterialID matID,
    const int iconWidth,
    const int iconHeight,
    const float yRotationAngle,
    Render::CRender* pRender,
    ID3D11ShaderResourceView** outMaterialImg)
{
    if (!pRender)
    {
        LogErr("input ptr to render == nullptr");
        return false;
    }
    if (!outMaterialImg)
    {
        LogErr("input shader resource view == nullptr");
        return false;
    }

 
    D3DClass& d3d                 = GetD3DClass();
    ID3D11Device* pDevice         = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();
    const float nearZ             = d3d.GetNearZ();
    const float farZ              = d3d.GetFarZ();

    if (!materialBigIconFrameBuf_.IsInit())
        InitMatIconFrameBuffer(pDevice, materialBigIconFrameBuf_, iconWidth, iconHeight, nearZ, farZ);

    // get a sphere model
    const ModelID basicSphereID    = g_ModelMgr.GetModelIdByName("basic_sphere");
    BasicModel& sphere             = g_ModelMgr.GetModelByID(basicSphereID);
    const MeshGeometry& sphereMesh = sphere.meshes_;

    ID3D11Buffer* vb     = sphereMesh.vb_.Get();
    ID3D11Buffer* ib     = sphereMesh.ib_.Get();
    const int indexCount = (int)sphereMesh.ib_.GetIndexCount();
    const int vertexSize = (int)sphereMesh.vb_.GetStride();

    // change view*proj matrix so we will be able to render material icons properly
    const XMMATRIX world    = XMMatrixRotationY(yRotationAngle);
    const XMMATRIX view     = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj     = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    Render::MaterialIconShader& matIconShader = pRender->shadersContainer_.materialIconShader_;
    matIconShader.SetMatrix(pContext, world, view, proj);


    // prepare responsible frame buffer for rendering
    materialBigIconFrameBuf_.ClearBuffers(pContext, { 0,0,0,0 });
    materialBigIconFrameBuf_.Bind(pContext);

    // prepare material data and its textures
    const Material& mat = g_MaterialMgr.GetMaterialByID(matID);

    const Render::Material renderMat(
        DirectX::XMFLOAT4(&mat.ambient.x),
        DirectX::XMFLOAT4(&mat.diffuse.x),
        DirectX::XMFLOAT4(&mat.specular.x),
        DirectX::XMFLOAT4(&mat.reflect.x));

    cvector<ID3D11ShaderResourceView*> texSRVs;
    g_TextureMgr.GetSRVsByTexIDs(mat.textureIDs, NUM_TEXTURE_TYPES, texSRVs);

    // render material into responsible frame buffer
    matIconShader.PrepareRendering(pContext, vb, ib, vertexSize);
    matIconShader.Render(pContext, indexCount, texSRVs.data(), renderMat);

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->SetViewProj(pContext, DirectX::XMMatrixTranspose(viewProj_));

    // copy frame buffer texture into the input SRV
    *outMaterialImg = materialBigIconFrameBuf_.GetSRV();

    d3d.ResetBackBufferRenderTarget();
    d3d.ResetViewport();

    return true;
}

///////////////////////////////////////////////////////////

void CGraphics::RenderMaterialsIcons(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender,
    ID3D11ShaderResourceView** outArrShaderResourceViews,
    const size numIcons,
    const int iconWidth,
    const int iconHeight)
{
    // render material icon (just sphere model with particular material) into
    // frame buffer and store shader resource view into the input arr;
    //
    // NOTE: outArrShaderResourceViews.size() must == numShaderResourceViews

    if (!pRender)
    {
        LogErr("input ptr to render == nullptr");
        return;
    }
    if (!outArrShaderResourceViews)
    {
        LogErr("input arr of shader resource views == nullptr");
        return;
    }

    D3DClass& d3d                 = GetD3DClass();
    ID3D11Device* pDevice         = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();
    const float nearZ             = d3d.GetNearZ();
    const float farZ              = d3d.GetFarZ();

    InitMatIconFrameBuffers(pDevice, materialsFrameBuffers_, numIcons, iconWidth, iconHeight, nearZ, farZ);

    // get a sphere model
    const ModelID basicSphereID    = g_ModelMgr.GetModelIdByName("basic_sphere");
    BasicModel& sphere             = g_ModelMgr.GetModelByID(basicSphereID);
    const MeshGeometry& sphereMesh = sphere.meshes_;

    ID3D11Buffer* vb     = sphereMesh.vb_.Get();
    ID3D11Buffer* ib     = sphereMesh.ib_.Get();
    const int indexCount = (int)sphereMesh.ib_.GetIndexCount();
    const int vertexSize = (int)sphereMesh.vb_.GetStride();

    // change view*proj matrix so we will be able to render material icons properly
    const XMMATRIX world = XMMatrixRotationY(0.0f);
    const XMMATRIX view  = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj  = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    Render::MaterialIconShader& matIconShader = pRender->shadersContainer_.materialIconShader_;

    matIconShader.SetMatrix(pContext, world, view, proj);
    matIconShader.PrepareRendering(pContext, vb, ib, vertexSize);

    // render material by idx into responsible frame buffer
    for (int matIdx = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        buf.ClearBuffers(pContext, { 0,0,0,0 });
        buf.Bind(pContext);

        // prepare material data and its textures
        Material& mat = g_MaterialMgr.GetMaterialByID(matIdx);

        const Render::Material renderMat(
            XMFLOAT4(&mat.ambient.x),
            XMFLOAT4(&mat.diffuse.x),
            XMFLOAT4(&mat.specular.x),
            XMFLOAT4(&mat.reflect.x));

        cvector<ID3D11ShaderResourceView*> texSRVs;
        g_TextureMgr.GetSRVsByTexIDs(mat.textureIDs, NUM_TEXTURE_TYPES, texSRVs);
        
        matIconShader.Render(pContext, indexCount, texSRVs.data(), renderMat);

        ++matIdx;
    }

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->SetViewProj(pContext, DirectX::XMMatrixTranspose(viewProj_));

    // copy frame buffers textures into the input array of SRVs
    for (int i = 0; FrameBuffer& buf : materialsFrameBuffers_)
        outArrShaderResourceViews[i++] = buf.GetSRV();
}

///////////////////////////////////////////////////////////

void CGraphics::RenderEnttsDefault(Render::CRender* pRender)
{
    const Render::RenderDataStorage& storage = pRender->dataStorage_;

    // check if we have any instances to render
    if (storage.modelInstances.empty())
        return;


    // setup states before rendering
    if (pRender->shadersContainer_.debugShader_.GetDebugType() == Render::eDebugState::DBG_WIREFRAME)
    {
        RenderStates& renderStates = d3d_.GetRenderStates();
        renderStates.SetRS(pDeviceContext_, { FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
    }
    else
    {
        d3d_.GetRenderStates().ResetRS(pDeviceContext_);
        d3d_.GetRenderStates().ResetBS(pDeviceContext_);
    }

    pRender->UpdateInstancedBuffer(pDeviceContext_, storage.modelInstBuffer);

    pRender->RenderInstances(
        pDeviceContext_,
        Render::ShaderTypes::LIGHT,
        storage.modelInstances.data(),
        (int)storage.modelInstances.size());
}

///////////////////////////////////////////////////////////

void CGraphics::RenderEnttsAlphaClipCullNone(Render::CRender* pRender)
{
    // render all the visible entts with cull_none and alpha clipping;
    // (entts for instance: wire fence, bushes, leaves, etc.)

    const Render::RenderDataStorage& storage = pRender->dataStorage_;

    // check if we have any instances to render
    if (storage.alphaClippedModelInstances.empty())
        return;

    RenderStates& renderStates = d3d_.GetRenderStates();
    ID3D11DeviceContext* pContext = pDeviceContext_;


    // setup states before rendering
    if (pRender->shadersContainer_.debugShader_.GetDebugType() == Render::eDebugState::DBG_WIREFRAME)
    {
        renderStates.SetRS(pContext, { FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
    }
    else
    {
        renderStates.SetRS(pContext, { FILL_SOLID, CULL_NONE, FRONT_CLOCKWISE });
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
        pRender->SwitchAlphaClipping(pContext, true);
    }

    // load instances data and render them
    pRender->UpdateInstancedBuffer(pContext, storage.alphaClippedModelInstBuffer);

    pRender->RenderInstances(
        pContext,
        Render::ShaderTypes::LIGHT,
        storage.alphaClippedModelInstances.data(),
        (int)storage.alphaClippedModelInstances.size());

    // reset rendering pipeline
    renderStates.ResetRS(pContext);
    pRender->SwitchAlphaClipping(pContext, false);
}

///////////////////////////////////////////////////////////

void CGraphics::RenderEnttsBlended(Render::CRender* pRender)
{
    // render all the visible blended entts

    const Render::RenderDataStorage& storage = pRender->dataStorage_;

    // check if we have any instances to render
    if (storage.blendedModelInstances.empty())
        return;

    const ECS::EnttsBlended& blendData = rsDataToRender_.enttsBlended_;
    const ECS::eRenderState* blendStates = blendData.states_.data();
    const size numBlendStates = blendData.states_.size();
    const size* numInstancesPerBlendState = blendData.instanceCountPerBS_.data();
    const Render::InstBuffData& instBuffer = storage.blendedModelInstBuffer;

    // push data into the instanced buffer
    pRender->UpdateInstancedBuffer(pDeviceContext_, instBuffer);

    int instanceOffset = 0;

    // go through each blending state, turn it on and render blended entts with this state
    for (index bsIdx = 0; bsIdx < numBlendStates; ++bsIdx)
    {
        d3d_.TurnOnBlending(eRenderState(blendStates[bsIdx]));

        for (u32 instCount = 0; instCount < numInstancesPerBlendState[bsIdx]; ++instCount)
        {
            const Render::Instance* instance = &(storage.blendedModelInstances[instanceOffset]);
            pRender->RenderInstances(pDeviceContext_, Render::ShaderTypes::LIGHT, instance, 1);
            ++instanceOffset;
        }
    }
}

///////////////////////////////////////////////////////////

void CGraphics::RenderBoundingLineBoxes(
    Render::CRender* pRender,
    ECS::EntityMgr* pEnttMgr)
{
    Render::RenderDataStorage& storage           = pRender->dataStorage_;
    Render::InstBuffData& instancesBuffer        = storage.boundingLineBoxBuffer;
    cvector<Render::Instance>& instances = storage.boundingLineBoxInstances;

    // prepare the line box instance
    const int numInstances = 1;                // how many different line box models we have
    const ModelID lineBoxId = 1;
    BasicModel& lineBox = g_ModelMgr.GetModelByID(lineBoxId);
    
    // we will use only one type of model -- line box
    instances.resize(1);                         
    Render::Instance& instance = instances[0];
    prep_.PrepareInstanceData(lineBox, instance);


    // choose the bounding box show mode
    // (1: box around the while model, 2: box around each model's mesh)
    if (aabbShowMode_ == MODEL)
        prep_.PrepareEnttsBoundingLineBox(pEnttMgr, instance, instancesBuffer);
    else if (aabbShowMode_ == MESH)
        prep_.PrepareEnttsMeshesBoundingLineBox(pEnttMgr, instance, instancesBuffer);


    // render
    ID3D11DeviceContext* pContext = pDeviceContext_;
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    pRender->UpdateInstancedBuffer(pContext, instancesBuffer);
    pRender->RenderBoundingLineBoxes(pContext, &instance, numInstances);
}

///////////////////////////////////////////////////////////

void CGraphics::RenderBoundingLineSpheres()
{
    // render line bound sphere around each visible point light source

#if 0
    const std::vector<EntityID>& visPointLights = entityMgr_.renderSystem_.GetArrVisibleLightSources();
    const size numVisLightSources = std::ssize(visPointLights);

    if (numVisLightSources > 0)
    {
        Render::RenderDataStorage& storage = render_.dataStorage_;
        Render::InstBuffData& instancesBuffer = storage.boundingLineBoxBuffer_;
        std::vector<Render::Instance>& instances = storage.boundingLineBoxInstances_;

        // prepare instance data
        const int numInstances = 1;
        BasicModel& boundSphere = modelStorage_.GetModelByName("bound_sphere");

        instances.resize(1);
        Render::Instance& instance = instances[0];
        prep_.PrepareInstanceData(boundSphere, instance);


        // prepare instances buffer data
        instance.numInstances = numVisLightSources;
        instancesBuffer.Resize(instance.numInstances);

        // generate world matrix for each instance of bounding sphere 
        for (int i = 0; i < (int)numVisLightSources; ++i)
        {
            const int pointLightIdx = idxsToVisLightSources[i];
            const float scale = posAndRange[pointLightIdx].range;
            const XMFLOAT3& pos = posAndRange[pointLightIdx].position;
            instancesBuffer.worlds_[i] = DirectX::XMMatrixScaling(scale, scale, scale) * DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        }

        pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

        // render
        render_.UpdateInstancedBuffer(pDeviceContext_, instancesBuffer);
        render_.RenderBoundingLineBoxes(pDeviceContext_, &instance, numInstances);
    }
#endif
}

///////////////////////////////////////////////////////////

void CGraphics::RenderFoggedBillboards(
    Render::CRender* pRender,
    ECS::EntityMgr* pEnttMgr)
{
    // render billboard of entities which are fully fogged so we see only its silhouette

    const EntityID* foggedEntts = rsDataToRender_.enttsFogged_.ids_.data();
    const size numFoggedEntts = rsDataToRender_.enttsFogged_.ids_.size();

    // if we don't have any billboard to render we just go out
    if (numFoggedEntts == 0)
        return;

    d3d_.TurnOnBlending(ALPHA_TO_COVERAGE);

    RenderStates& renderStates = d3d_.GetRenderStates();
    renderStates.SetRS(pDeviceContext_, CULL_NONE);
    pRender->SwitchAlphaClipping(pDeviceContext_, true);

    //
    // prepare billboards data for rendering
    //
    std::vector<Render::Material> materials(numFoggedEntts);
    cvector<DirectX::XMFLOAT3> foggedPositions;
    std::vector<DirectX::XMFLOAT2> sizes(numFoggedEntts, { 30, 30 });

    pEnttMgr->transformSystem_.GetPositions(foggedEntts, numFoggedEntts, foggedPositions);

    pRender->shadersContainer_.billboardShader_.UpdateInstancedBuffer(
        pDeviceContext_,
        materials.data(),
        foggedPositions.data(),
        sizes.data(),
        (int)numFoggedEntts);


    //
    // render billboards instances
    //
    Render::Instance instance;
    instance.pVB = pGeomVB_;
    instance.pIB = pGeomIB_;
    instance.texSRVs = { g_TextureMgr.GetTexPtrByName("tree_billboard")->GetTextureResourceView() };
    instance.vertexStride = sizeof(TreePointSprite);

    pRender->shadersContainer_.billboardShader_.Render(pDeviceContext_, instance);

    // reset rendering pipeline
    renderStates.ResetRS(pDeviceContext_);
    pRender->SwitchAlphaClipping(pDeviceContext_, false);
   
}

///////////////////////////////////////////////////////////

void CGraphics::RenderSkyDome(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr)
{
    // check if we at least have a sky entity
    const EntityID skyEnttID = pEnttMgr->nameSystem_.GetIdByName("sky");

    // if we haven't any sky entity
    if (skyEnttID == INVALID_ENTITY_ID)
        return;

    const SkyModel& sky = g_ModelMgr.GetSky();
    Render::SkyInstance instance;

    instance.vertexStride = sky.GetVertexStride();
    instance.pVB          = sky.GetVertexBuffer();
    instance.pIB          = sky.GetIndexBuffer();
    instance.indexCount   = sky.GetNumIndices();
    instance.colorCenter  = sky.GetColorCenter();
    instance.colorApex    = sky.GetColorApex();


    // setup rendering pipeline before rendering of the sky dome
    ID3D11DeviceContext* pContext = pDeviceContext_;

    RenderStates& renderStates = d3d_.GetRenderStates();
    renderStates.SetRS(pContext, { FILL_SOLID, CULL_NONE, FRONT_COUNTER_CLOCKWISE });
    renderStates.ResetBS(pContext);
    renderStates.SetDSS(pContext, SKY_DOME, 1);


    // compute a worldViewProj matrix for the sky instance
    const XMFLOAT3 skyOffset     = pEnttMgr->transformSystem_.GetPosition(skyEnttID);
    const XMFLOAT3 eyePos        = pEnttMgr->cameraSystem_.GetPos(currCameraID_);
    const XMFLOAT3 translation   = skyOffset + eyePos;
    const XMMATRIX world         = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
    const XMMATRIX worldViewProj = DirectX::XMMatrixTranspose(world * viewProj_);

    pRender->RenderSkyDome(pContext, instance, worldViewProj);
}

//---------------------------------------------------------
// Desc:   render terrain onto the screen
// Args:   - pRender: a pointer to the renderer (look at Render module)
//         - pEnttMgr: a pointer to the ECS manager (look at ECS module)
//---------------------------------------------------------
void CGraphics::RenderTerrainGeomip(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr)
{
    const char* terrainName = "terrain_geomipmap";
    const EntityID terrainID = pEnttMgr->nameSystem_.GetIdByName(terrainName);

    // if we haven't any terrain entity
    if (terrainID == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "can't find terrain by name: %s", terrainName);
        return;
    }


    // prepare the terrain instance
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    Render::TerrainInstance instance;

    // prepare material
    const Material& mat = g_MaterialMgr.GetMaterialByID(terrain.materialID_);
    memcpy(&instance.material.ambient_.x,  &mat.ambient.x,  sizeof(float) * 4);
    memcpy(&instance.material.diffuse_.x,  &mat.diffuse.x,  sizeof(float) * 4);
    memcpy(&instance.material.specular_.x, &mat.specular.x, sizeof(float) * 4);
    memcpy(&instance.material.reflect_.x,  &mat.reflect.x,  sizeof(float) * 4);

    // prepare textures
    g_TextureMgr.GetSRVsByTexIDs(mat.textureIDs, NUM_TEXTURE_TYPES, texturesBuf_);
    memcpy(instance.textures, texturesBuf_.begin(), NUM_TEXTURE_TYPES * sizeof(ID3D11ShaderResourceView*));

#if TERRAIN_V1
    // vertex/index buffers data
    instance.vertexStride   = terrain.GetVertexStride();
    instance.pVB            = terrain.GetVertexBuffer();
    instance.pIB            = terrain.GetIndexBuffer();
    instance.numVertices    = terrain.verticesOffset_;
    instance.indexCount     = terrain.indicesOffset_;

    // for debugging
    //instance.wantDebug = terrain.wantDebug_;

    // setup rendering pipeline before rendering of the terrain
    RenderStates&        renderStates = d3d_.GetRenderStates();
    ID3D11DeviceContext* pContext     = pDeviceContext_;

    if (pRender->shadersContainer_.debugShader_.GetDebugType() == Render::eDebugState::DBG_WIREFRAME)
    {
        renderStates.SetRS(pDeviceContext_, { FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }
    else
    {
        renderStates.ResetRS(pContext);
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }


    pRender->shadersContainer_.terrainShader_.Render(pContext, instance);

    // compute how many vertices we already rendered
    pSysState_->visibleVerticesCount += (uint32_t)instance.numVertices;
    //pSysState_->visibleVerticesCount += terrain.vb_.GetVertexCount();
#else
    // vertex/index buffers data
    instance.vertexStride   = terrain.GetVertexStride();
    instance.pVB            = terrain.GetVertexBuffer();
    instance.pIB            = terrain.GetIndexBuffer();

    const TerrainLodMgr& lodMgr = terrain.GetLodMgr();
    const int terrainLen        = terrain.GetTerrainLength();
    const int numPatchesPerSide = terrain.GetNumPatchesPerSide();
    const int patchSize = lodMgr.GetPatchSize();
    //const int numPatchesPerSide = (terrainLen - 1) / (patchSize - 1);
    

    // setup rendering pipeline before rendering of the terrain
    RenderStates& renderStates = d3d_.GetRenderStates();
    ID3D11DeviceContext* pContext = pDeviceContext_;

    if (pRender->shadersContainer_.debugShader_.GetDebugType() == Render::eDebugState::DBG_WIREFRAME)
    {
        renderStates.SetRS(pDeviceContext_, { FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }
    else
    {
        renderStates.ResetRS(pContext);
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }

    Render::TerrainShader& shader = pRender->shadersContainer_.terrainShader_;
    shader.Prepare(pContext, instance);


    // 
    for (int patchZ = 0; patchZ < numPatchesPerSide; ++patchZ)
    {
        for (int patchX = 0; patchX < numPatchesPerSide; ++patchX)
        {
            const TerrainLodMgr::PatchLod& plod = lodMgr.GetPatchLodInfo(patchX, patchZ);

            terrain.GetLodInfoByPatch(plod, instance.baseIndex, instance.indexCount);

            const int z = patchZ * (patchSize - 1);
            const int x = patchX * (patchSize - 1);
            instance.baseVertex = (UINT)(z*terrainLen + x);

            shader.RenderPatch(pContext, instance);      
        }
    }
#endif



    //terrain.wantDebug_ = false;
}

//---------------------------------------------------------
// Desc:   render quadtree terrain onto the screen
// Args:   - pRender:  a ptr to the Render class from Render module
//         - pEnttMgr: a ptr to the Entity manager from ECS module
//---------------------------------------------------------
void CGraphics::RenderTerrainQuadtree(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr)
{
    const char* terrainName = "terrain_quadtree";
    const EntityID terrainID = pEnttMgr->nameSystem_.GetIdByName(terrainName);

    // if we haven't any terrain entity
    if (terrainID == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "can't find terrain by name: %s", terrainName);
        return;
    }


    // prepare the terrain instance
    TerrainQuadtree& terrain = g_ModelMgr.GetTerrainQuadtree();
    Render::TerrainInstance instance;

    // prepare material
    const Material& mat = g_MaterialMgr.GetMaterialByID(terrain.GetMaterialId());
    memcpy(&instance.material.ambient_.x,  &mat.ambient.x,  sizeof(float) * 4);
    memcpy(&instance.material.diffuse_.x,  &mat.diffuse.x,  sizeof(float) * 4);
    memcpy(&instance.material.specular_.x, &mat.specular.x, sizeof(float) * 4);
    memcpy(&instance.material.reflect_.x,  &mat.reflect.x,  sizeof(float) * 4);

    // prepare textures
    g_TextureMgr.GetSRVsByTexIDs(mat.textureIDs, NUM_TEXTURE_TYPES, texturesBuf_);
    memcpy(instance.textures, texturesBuf_.begin(), NUM_TEXTURE_TYPES * sizeof(ID3D11ShaderResourceView*));

    // prepare vertex/index buffers data
   
    instance.pVB          = terrain.GetVertexBuffer();
    instance.vertexStride = terrain.GetVertexStride();
    instance.pIB          = terrain.GetIndexBuffer();
    instance.numVertices  = terrain.GetQuadtreeNumVertices();
    instance.indexCount   = terrain.GetNumIndices();

    // for debugging
    //instance.wantDebug = terrain.wantDebug_;

    // setup rendering pipeline before rendering of the terrain
    RenderStates&        renderStates = d3d_.GetRenderStates();
    ID3D11DeviceContext* pContext     = pDeviceContext_;

    if (pRender->shadersContainer_.debugShader_.GetDebugType() == Render::eDebugState::DBG_WIREFRAME)
    {
        renderStates.SetRS(pDeviceContext_, { FILL_WIREFRAME, CULL_BACK, FRONT_CLOCKWISE });
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }
    else
    {
        renderStates.ResetRS(pContext);
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }

    pRender->shadersContainer_.terrainShader_.Render(pContext, instance);

    // compute how many vertices we already rendered
    pSysState_->visibleVerticesCount += (uint32_t)instance.numVertices;

    //terrain.wantDebug_ = false;
}

///////////////////////////////////////////////////////////

void CGraphics::SetupLightsForFrame(
    ECS::EntityMgr* pEnttMgr,
    Render::PerFrameData& outData)
{
    // convert light source data from the ECS into Render format
    // (they are the same so we simply need to copy data)

    const ECS::LightSystem& lightSys         = pEnttMgr->lightSystem_;
    const ECS::RenderSystem& renderSys       = pEnttMgr->renderSystem_;
    const ECS::TransformSystem& transformSys = pEnttMgr->transformSystem_;

    const ECS::DirLights&  dirLights  = lightSys.GetDirLights();
    const ECS::SpotLights& spotLights = lightSys.GetSpotLights();

    const size numDirLights   = dirLights.data.size();
    const size numSpotLights  = spotLights.data.size();

    const cvector<EntityID>& visPointLights = renderSys.GetVisiblePointLights();
    const size numVisPointLightSources      = visPointLights.size();


    outData.ResizeLightData((int)numDirLights, (int)numVisPointLightSources, (int)numSpotLights);

    // ----------------------------------------------------
    // prepare data of point lights

    if (numVisPointLightSources > 0)
    {
        lightSys.GetPointLightsData(
            visPointLights.data(),
            numVisPointLightSources,
            s_LightTmpData.pointLightsData,
            s_LightTmpData.pointLightsPositions);

        // store light properties, range, and attenuation
        for (index i = 0; i < numVisPointLightSources; ++i)
        {
            outData.pointLights[i].ambient  = s_LightTmpData.pointLightsData[i].ambient;
            outData.pointLights[i].diffuse  = s_LightTmpData.pointLightsData[i].diffuse;
            outData.pointLights[i].specular = s_LightTmpData.pointLightsData[i].specular;
            outData.pointLights[i].att      = s_LightTmpData.pointLightsData[i].att;
            outData.pointLights[i].range    = s_LightTmpData.pointLightsData[i].range;
        }

        // store positions
        for (index i = 0; i < numVisPointLightSources; ++i)
            outData.pointLights[i].position = s_LightTmpData.pointLightsPositions[i];
    }

    // ----------------------------------------------------
    // prepare data of directed lights

    for (index i = 0; i < numDirLights; ++i)
    {
        outData.dirLights[i].ambient  = dirLights.data[i].ambient;
        outData.dirLights[i].diffuse  = dirLights.data[i].diffuse;
        outData.dirLights[i].specular = dirLights.data[i].specular;
    }

    pEnttMgr->transformSystem_.GetDirections(
        dirLights.ids.data(),
        numDirLights,
        s_LightTmpData.dirLightsDirections);

    for (int i = 0; const XMVECTOR& dirQuat : s_LightTmpData.dirLightsDirections)
    {
        DirectX::XMStoreFloat3(&outData.dirLights[i].direction, dirQuat);
        ++i;
    }

    // ----------------------------------------------------
    // prepare data of spot lights

    lightSys.GetSpotLightsData(
        spotLights.ids.data(),
        numSpotLights,
        s_LightTmpData.spotLightsData,
        s_LightTmpData.spotLightsPositions,
        s_LightTmpData.spotLightsDirections);

    for (index i = 0; i < numSpotLights; ++i)
    {
        outData.spotLights[i].ambient  = s_LightTmpData.spotLightsData[i].ambient;
        outData.spotLights[i].diffuse  = s_LightTmpData.spotLightsData[i].diffuse;
        outData.spotLights[i].specular = s_LightTmpData.spotLightsData[i].specular;
        outData.spotLights[i].range    = s_LightTmpData.spotLightsData[i].range;
        outData.spotLights[i].spot     = s_LightTmpData.spotLightsData[i].spot;
        outData.spotLights[i].att      = s_LightTmpData.spotLightsData[i].att;
    }

    for (int i = 0; const XMFLOAT3& pos : s_LightTmpData.spotLightsPositions)
        outData.spotLights[i++].position = pos;

    for (int i = 0; const XMFLOAT3& dir : s_LightTmpData.spotLightsDirections)
        outData.spotLights[i++].direction = dir;
}

///////////////////////////////////////////////////////////

int CGraphics::TestEnttSelection(const int sx, const int sy, ECS::EntityMgr* pEnttMgr)
{
    // check if we have any entity by input screen coords;
    // 
    // in:   screen pixel coords
    // out:  0  - there is no entity, we didn't select any
    //       ID - we selected some entity so return its ID

    using namespace DirectX;

    const XMMATRIX& P       = pEnttMgr->cameraSystem_.GetProj(currCameraID_);
    const XMMATRIX& invView = pEnttMgr->cameraSystem_.GetInverseView(currCameraID_);

    // TODO: optimize it!
    const float xndc = (+2.0f * sx / d3d_.GetWindowWidth() - 1.0f);
    const float yndc = (-2.0f * sy / d3d_.GetWindowHeight() + 1.0f);

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
        const BasicModel& model = g_ModelMgr.GetModelByID(modelID);

        if (model.type_ == eModelType::Terrain)
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
        const std::string& name = pEnttMgr->nameSystem_.GetNameById(selectedEnttID);

        SetConsoleColor(YELLOW);
        LogMsg("picked entt (id: %ld; name: %s)", selectedEnttID, name.c_str());
        SetConsoleColor(RESET);
    }

    // return ID of the selected entt, or 0 if we didn't pick any
    return selectedEnttID;
}

///////////////////////////////////////////////////////////

void CGraphics::BuildGeometryBuffers()
{
    HRESULT hr = S_OK;
    TreePointSprite spriteVertex;

    spriteVertex.pos = { 0,0,0 };
    spriteVertex.size = { 30, 30 };

    //
    // Create VB
    //
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(TreePointSprite);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vbInitData;
    vbInitData.pSysMem = &spriteVertex;

    hr = pDevice_->CreateBuffer(&vbd, &vbInitData, &pGeomVB_);
    CAssert::NotFailed(hr, "can't create a vertex buffer for tree sprite");


    //
    // Create IB
    //
    UINT indices = 0;

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA ibInitData;
    ibInitData.pSysMem = &indices;

    hr = pDevice_->CreateBuffer(&ibd, &ibInitData, &pGeomIB_);
    CAssert::NotFailed(hr, "can't create an index buffer for tree sprite");
}

} // namespace Core
