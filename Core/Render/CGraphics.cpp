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
#include <Render/RenderStates.h>
#include <Shaders/Shader.h>

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
        //d3d_.SetRS({ eRenderState::CULL_BACK, eRenderState::FILL_SOLID });

        // after initialization of the DirectX we can use pointers to the device and device context
        d3d_.GetDeviceAndContext(pDevice_, pContext_);

        // initializer the textures global manager (container)
        g_TextureMgr.Initialize(pDevice_);
    
        // create frustums for frustum culling
        frustums_.push_back(DirectX::BoundingFrustum());
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

//---------------------------------------------------------
// Desc:   update all the graphics related stuff for this frame
//---------------------------------------------------------
void CGraphics::UpdateHelper(
    SystemState& sysState,
    const float deltaTime,
    const float totalGameTime,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    TerrainGeomip& terrain   = g_ModelMgr.GetTerrainGeomip();
    const EntityID currCamId = currCameraID_;

    // ---------------------------------------------
    // update the cameras states

    if (sysState.isGameMode)
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
            // make player's offset by Y-axis to be always over the terrain even
            // when jump from lower to higher position
            const float terrainHeight = terrain.GetScaledInterpolatedHeightAtPoint(playerPos.x, playerPos.z);
            const float offsetOverTerrain = 3;

            player.SetMinVerticalOffset(terrainHeight + offsetOverTerrain);
        }

        sysState.cameraPos = { playerPos.x, playerPos.y, playerPos.z };
    }

    // we aren't in the game mode (now is the editor mode)
    else
    {
        sysState.cameraPos = pEnttMgr->transformSystem_.GetPosition(currCamId);
    }

    sysState.cameraDir = pEnttMgr->transformSystem_.GetDirection(currCamId);

    // update camera's view matrix
    ECS::CameraSystem& camSys = pEnttMgr->cameraSystem_;

    camSys.UpdateView(currCamId);
    sysState.cameraView = camSys.GetView(currCamId);
    sysState.cameraProj = camSys.GetProj(currCamId);
    viewProj_           = sysState.cameraView * sysState.cameraProj;

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

    camParams.fov         = camSys.GetFovX(currCamId);
    camParams.aspectRatio = 1.0f / camSys.GetAspect(currCamId);
    camParams.nearZ       = camSys.GetNearZ(currCamId);
    camParams.farZ        = camSys.GetFarZ(currCamId);


    memcpy(camParams.view, sysState.cameraView.r[0].m128_f32, sizeof(float)*16);
    memcpy(camParams.proj, sysState.cameraProj.r[0].m128_f32, sizeof(float)*16);

    // update LOD and visibility for each terrain's patch
    terrain.Update(camParams);


    static XMFLOAT3 prevCamPos = { 0,0,0 };
    bool updateTerrain = false;

    if (prevCamPos != sysState.cameraPos)
    {
        updateTerrain = true;
        prevCamPos = sysState.cameraPos;
    }


    // perform frustum culling on all of our currently loaded entities
    ComputeFrustumCulling(sysState, pEnttMgr);

    // Update shaders common data for this frame
    UpdateShadersDataPerFrame(pEnttMgr, pRender, totalGameTime);

    // prepare all the visible entities data for rendering
    const cvector<EntityID>& visibleEntts = pEnttMgr->renderSystem_.GetAllVisibleEntts();

    // reset some counters
    pSysState_->numDrawnTerrainPatches = 0;
    pSysState_->numCulledTerrainPatches = 0;
    pSysState_->numDrawnVertices = 0;
    pSysState_->numDrawnInstances = 0;
    pSysState_->numDrawCallsForInstances = 0;

    // prepare data for each entity
    PrepareRenderInstances(pEnttMgr, pRender, sysState.cameraPos);
}

// --------------------------------------------------------
// Desc:   prepare rendering data of entts which have default render states
// --------------------------------------------------------
void CGraphics::PrepareRenderInstances(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender,
    const DirectX::XMFLOAT3& cameraPos)
{
    const cvector<EntityID>& visibleEntts = pEnttMgr->renderSystem_.GetAllVisibleEntts();

    const EntityID* enttsIds = visibleEntts.data();
    const size      numEntts = visibleEntts.size();

    if (numEntts == 0)
        return;

    Render::RenderDataStorage& storage = pRender->dataStorage_;

    // gather entts data for rendering
    prep_.PrepareEnttsDataForRendering(enttsIds, numEntts, cameraPos, pEnttMgr, storage);

    pRender->UpdateInstancedBuffer(pContext_, storage.instancesBuf);
}

// --------------------------------------------------------
// Desc:   compute fructum culling for each entity which has Rendering component
// --------------------------------------------------------
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

    // store ids of visible entts
    cvector<EntityID>& visibleEntts = renderSys.GetAllVisibleEntts();
    visibleEntts.resize(numVisEntts);

    for (index i = 0; i < numVisEntts; ++i)
        visibleEntts[i] = enttsRenderable[idxsToVisEntts[i]];

    // this number of entities (instances) will be rendered onto the screen
    sysState.numDrawnInstances = (u32)numVisEntts;
}

//---------------------------------------------------------
// Desc:   pdate shaders common data for this frame: 
//         viewProj matrix, camera position, light sources data, etc.
// Args:   - totalGameTime:  the time passed since the start of the application
//---------------------------------------------------------
void CGraphics::UpdateShadersDataPerFrame(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender,
    const float totalGameTime)
{
    Render::PerFrameData& data = pRender->perFrameData_;

    data.viewProj      = DirectX::XMMatrixTranspose(viewProj_);
    data.cameraPos     = pEnttMgr->transformSystem_.GetPosition(currCameraID_);
    data.totalGameTime = totalGameTime;

    SetupLightsForFrame(pEnttMgr, data);

    // update const buffers with new data
    pRender->UpdatePerFrame(pContext_, data);
}

//---------------------------------------------------------
// Desc:   clear rendering data from the previous frame / instances set
//---------------------------------------------------------
void CGraphics::ClearRenderingDataBeforeFrame(
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender)
{
    pRender->dataStorage_.Clear();
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
        case MAT_PROP_NO_RENDER_TARGET_WRITES:
            renderStates.SetBS(pContext, R_NO_RENDER_TARGET_WRITES);
            break;

        case MAT_PROP_BLEND_DISABLE:
            renderStates.SetBS(pContext, R_ALPHA_DISABLE);
            break;

        case MAT_PROP_BLEND_ENABLE:
            renderStates.SetBS(pContext, R_ALPHA_ENABLE);
            break;

        case MAT_PROP_ADDING:
            renderStates.SetBS(pContext, R_ADDING);
            break;

        case MAT_PROP_SUBTRACTING:
            renderStates.SetBS(pContext, R_SUBTRACTING);
            break;

        case MAT_PROP_MULTIPLYING:
            renderStates.SetBS(pContext, R_MULTIPLYING);
            break;

        case MAT_PROP_TRANSPARENCY:
            renderStates.SetBS(pContext, R_TRANSPARENCY);
            break;

        case MAT_PROP_ALPHA_TO_COVERAGE:
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
        case MAT_PROP_DEPTH_ENABLED:
            renderStates.SetDSS(pContext, R_DEPTH_ENABLED, 0);
            break;

        case MAT_PROP_DEPTH_DISABLED:
            renderStates.SetDSS(pContext, R_DEPTH_DISABLED, 0);
            break;

        case MAT_PROP_MARK_MIRROR:
            renderStates.SetDSS(pContext, R_MARK_MIRROR, 0);
            break;

        case MAT_PROP_DRAW_REFLECTION:
            renderStates.SetDSS(pContext, R_DRAW_REFLECTION, 0);
            break;

        case MAT_PROP_NO_DOUBLE_BLEND:
            renderStates.SetDSS(pContext, R_NO_DOUBLE_BLEND, 0);
            break;

        case MAT_PROP_SKY_DOME:
            renderStates.SetDSS(pContext, R_SKY_DOME, 0);
            break;
    }
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   - renderStatesBitfield:  bitfield with render states for this material
//         - texIdx:                identifiers to textures which will be bound
//---------------------------------------------------------
void CGraphics::BindMaterial(
    Render::CRender* pRender,
    const uint32 renderStatesBitfields,
    const TexID* texIds)
{
    // find texture resource views by input textures ids
    ID3D11ShaderResourceView* texViews[NUM_TEXTURE_TYPES]{ nullptr };
    g_TextureMgr.GetTexViewsByIds(texIds, NUM_TEXTURE_TYPES, texViews);

    BindMaterial(pRender, renderStatesBitfields, texViews);
}

//---------------------------------------------------------
// Desc:   setup rendering states according to input material params
// Args:   - renderStatesBitfield:  bitfield with render states for this material
//         - texViews:              textures to bind
//---------------------------------------------------------
void CGraphics::BindMaterial(
    Render::CRender* pRender,
    const uint32 renderStatesBitfield,
    ID3D11ShaderResourceView* const* texViews)
{
    if (!texViews)
    {
        LogErr(LOG, "input arr of textures IDs == nullptr");
        return;
    }

    // static bitfield of render states
    static uint32 prevBitfield = 0;
    ID3D11DeviceContext* pContext = pContext_;


    // bind textures of this material
    pContext->PSSetShaderResources(10U, NUM_TEXTURE_TYPES, texViews);

    // check if we need to switch render states
    if (prevBitfield == renderStatesBitfield)
        return;

 
    Render::RenderStates& renderStates = d3d_.GetRenderStates();
    uint32 prev = 0;
    uint32 curr = 0;


    // switch alpha clipping
    prev = prevBitfield & MAT_PROP_ALPHA_CLIPPING;
    curr = renderStatesBitfield & MAT_PROP_ALPHA_CLIPPING;

    if (prev != curr)
        pRender->SwitchAlphaClipping(pContext, curr);


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
// 
//---------------------------------------------------------
void CGraphics::RenderHelper(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender)
{
    try
    {
        RenderStates& renderStates    = d3d_.GetRenderStates();
        ID3D11DeviceContext* pContext = pContext_;
        UINT startInstanceLocation    = 0;
        RenderStat stat;

        // prepare the sky textures: in different shaders we will sample the sky
        // texture pixels so we bind them only once at the beginning of the frame
        const SkyModel&  sky      = g_ModelMgr.GetSky();
        const MaterialID skyMatId = sky.GetMaterialId();
        const Material&  skyMat   = g_MaterialMgr.GetMatById(skyMatId);

        // get shader resource views (textures) for the sky 
        TexID skyTexId = skyMat.texIds[TEX_TYPE_DIFFUSE];
        ID3D11ShaderResourceView* skySRV = g_TextureMgr.GetTexViewsById(skyTexId);
        pContext->PSSetShaderResources(0U, 1U, &skySRV);

        // bind a perlin noise texture for "fog movement"
        Texture* pTexPerlinNoise = g_TextureMgr.GetTexPtrByName("perlin_noise");
        ID3D11ShaderResourceView* pPerlinNoiseSRV = pTexPerlinNoise->GetTextureResourceView();
        pContext->PSSetShaderResources(1U, 1U, &pPerlinNoiseSRV);


        const Render::RenderDataStorage& storage = pRender->dataStorage_;

        // check if we have any instances to render
        if (storage.instancesBuf.GetSize() > 0)
        {
            // reset textures and render states so we will be able to setup it properly for rendering
            ID3D11ShaderResourceView* zeroTexViews[NUM_TEXTURE_TYPES]{ nullptr };
            BindMaterial(pRender, MAT_PROP_DEFAULT, zeroTexViews);

            // first of all we render masked and opaque geometry
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
            RenderGrass(pRender);

            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            RenderInstanceGroups(pContext, pRender, storage.masked, startInstanceLocation, stat);
            RenderInstanceGroups(pContext, pRender, storage.opaque, startInstanceLocation, stat);

           

            // reset the render states before rendering
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            RenderTerrainGeomip(pRender, pEnttMgr);

            // render sky
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            RenderSkyDome(pRender, pEnttMgr);

            // render billboards and particles
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
            RenderParticles(pRender, pEnttMgr);

            // after all we render blended geometry
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            RenderInstanceGroups(pContext, pRender, storage.blended, startInstanceLocation, stat);
            RenderInstanceGroups(pContext, pRender, storage.blendedTransparent, startInstanceLocation, stat);

            pSysState_->numDrawnVertices += stat.numDrawnVertices;
            pSysState_->numDrawnInstances += stat.numDrawnInstances;
            pSysState_->numDrawCallsForInstances += stat.numDrawCallsForInstances;
        }

        // reset render states before rendering of 2D/UI elements
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pRender->SwitchAlphaClipping(pContext, false);
        renderStates.ResetRS(pContext);
        renderStates.ResetBS(pContext);
        renderStates.ResetDSS(pContext);
    }
    catch (const std::out_of_range& e)
    {
        LogErr(e.what());
        LogErr(LOG, "there is no such a key to data");
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't render 3D entts onto the scene");
    }
}

//---------------------------------------------------------
// Desc:   init a single frame buffer which is used to render material big icon
//         (for the editor's material browser)
// Args:   - width, height:          frame buffer's dimensions
//---------------------------------------------------------
bool CGraphics::InitMatBigIconFrameBuf(const int width, const int height)
{
    // check input args
    if (width <= 0 && height <= 0)
    {
        LogErr(LOG, "input icon width or height is wrong, it must be > 0 (current w: %d, h: %d)", width, height);
        return false;
    }

    D3DClass& d3d               = GetD3DClass();
    ID3D11Device* pDevice       = d3d.GetDevice();
    const float nearZ           = d3d.GetNearZ();
    const float farZ            = d3d.GetFarZ();

    // setup params for the frame buffers
    FrameBufferSpecification frameBufSpec;
    frameBufSpec.width          = (UINT)width;
    frameBufSpec.height         = (UINT)height;
    frameBufSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    frameBufSpec.screenNear     = nearZ;
    frameBufSpec.screenDepth    = farZ;

    // TODO: if input params differ from the frame buffer's params we
    // need to recreate the frame buffer according to new params
    
    if (!materialBigIconFrameBuf_.Initialize(g_pDevice, frameBufSpec))
    {
        LogErr(LOG, "can't initialize material big icon");
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


    D3DClass& d3d               = GetD3DClass();
    ID3D11Device* pDevice       = d3d.GetDevice();
    const float nearZ           = d3d.GetNearZ();
    const float farZ            = d3d.GetFarZ();

    // setup params for the frame buffers (we will use the same params for each)
    FrameBufferSpecification frameBufSpec;
    frameBufSpec.width          = (UINT)width;
    frameBufSpec.height         = (UINT)height;
    frameBufSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    frameBufSpec.screenNear     = nearZ;
    frameBufSpec.screenDepth    = farZ;

    // release memory from the previous set of frame buffers
    for (FrameBuffer& buf : materialsFrameBuffers_)
        buf.Shutdown();

    // alloc memory for frame buffers and init them
    materialsFrameBuffers_.resize(numBuffers);
    outShaderResourceViews.resize(numBuffers, nullptr);

    for (index i = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        // try to init a frame buffer
        if (!buf.Initialize(pDevice, frameBufSpec))
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

///////////////////////////////////////////////////////////

bool CGraphics::RenderBigMaterialIcon(
    const MaterialID matID,
    const float yRotationAngle,
    Render::CRender* pRender,
    ID3D11ShaderResourceView** outMaterialImg)
{
    if (!pRender)
    {
        LogErr(LOG, "input ptr to render == nullptr");
        return false;
    }
    if (!outMaterialImg)
    {
        LogErr(LOG, "input shader resource view == nullptr");
        return false;
    }

    D3DClass&            d3d      = GetD3DClass();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();


    // get a sphere model
    const ModelID basicSphereID    = g_ModelMgr.GetModelIdByName("basic_sphere");
    BasicModel& sphere             = g_ModelMgr.GetModelById(basicSphereID);
    const MeshGeometry& sphereMesh = sphere.meshes_;

    ID3D11Buffer* vb      = sphereMesh.vb_.Get();
    ID3D11Buffer* ib      = sphereMesh.ib_.Get();
    const UINT indexCount = (UINT)sphereMesh.ib_.GetIndexCount();
    const UINT stride     = (UINT)sphereMesh.vb_.GetStride();
    const UINT offset     = 0;

    // change view*proj matrix so we will be able to render material icons properly
    const XMMATRIX world    = XMMatrixRotationY(yRotationAngle);
    const XMMATRIX view     = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj     = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);


    Render::Shader* pShader = pRender->shaderMgr_.GetShaderByName("MaterialIconShader");
    if (!pShader)
    {
        LogErr(LOG, "there is no shader by name: MaterialIconShader");
        return false;
    }

    // prepare IA for rendering, bind shaders, set matrices
    pRender->shaderMgr_.BindShader(pContext, pShader);
    pRender->SetWorldAndViewProj(pContext, world, view * proj);

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    pContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0U);


    // prepare responsible frame buffer for rendering
    materialBigIconFrameBuf_.ClearBuffers(pContext, { 0,0,0,0 });
    materialBigIconFrameBuf_.Bind(pContext);

    // prepare material data and its textures
    const Material& mat = g_MaterialMgr.GetMatById(matID);
    BindMaterial(pRender, mat.renderStates, mat.texIds);

    pRender->cbpsMaterial_.data =
    {
        XMFLOAT4(&mat.ambient.x),
        XMFLOAT4(&mat.diffuse.x),
        XMFLOAT4(&mat.specular.x),
        XMFLOAT4(&mat.reflect.x)
    };
    pRender->cbpsMaterial_.ApplyChanges(pContext);

    // render geometry with material into frame buffer
    pContext->DrawIndexed(indexCount, 0U, 0U);


    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->SetViewProj(pContext, DirectX::XMMatrixTranspose(viewProj_));

    // copy frame buffer texture into the input SRV
    *outMaterialImg = materialBigIconFrameBuf_.GetSRV();

    d3d.ResetBackBufferRenderTarget();
    d3d.ResetViewport();

    return true;
}

//-----------------------------------------------------------------------------
// Desc:   render material icons (just sphere models with particular material) into
//         frame buffer, so later we will use its shader resource views
//         for visualization of materials (in the editor)
//-----------------------------------------------------------------------------
bool CGraphics::RenderMaterialsIcons(Render::CRender* pRender)
{
    // check input args
    if (!pRender)
    {
        LogErr(LOG, "input ptr to render == nullptr");
        return false;
    }


    D3DClass& d3d                 = GetD3DClass();
    ID3D11Device* pDevice         = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();
    const float nearZ             = d3d.GetNearZ();
    const float farZ              = d3d.GetFarZ();

    // get a sphere model
    const ModelID basicSphereID    = g_ModelMgr.GetModelIdByName("basic_sphere");
    BasicModel& sphere             = g_ModelMgr.GetModelById(basicSphereID);
    const MeshGeometry& sphereMesh = sphere.meshes_;

    ID3D11Buffer* vb     = sphereMesh.vb_.Get();
    ID3D11Buffer* ib     = sphereMesh.ib_.Get();
    const UINT indexCount = (UINT)sphereMesh.ib_.GetIndexCount();
    const UINT vertexSize = (UINT)sphereMesh.vb_.GetStride();
    const UINT offset     = 0;

    // change view*proj matrix so we will be able to render material icons properly
    //const XMMATRIX world = XMMatrixRotationY(0.0f);
    const XMMATRIX view  = XMMatrixTranslation(0, 0, 1.1f);
    const XMMATRIX proj  = XMMatrixPerspectiveFovLH(1.0f, 1.0f, 0.1f, 100.0f);

    Render::Shader* pShader = pRender->shaderMgr_.GetShaderByName("MaterialIconShader");
    if (!pShader)
    {
        LogErr(LOG, "there is no shader by name: MaterialIconShader");
        return false;
    }

    // prepare IA for rendering, bind shaders, set matrices
    pRender->shaderMgr_.BindShader(pContext, pShader);
    pRender->SetWorldAndViewProj(pContext, DirectX::XMMatrixIdentity(), view * proj);

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pContext->IASetVertexBuffers(0, 1, &vb, &vertexSize, &offset);
    pContext->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0U);


    // render material by idx into responsible frame buffer
    for (int matIdx = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        // clear prev content of this frame buffer and bind it for rendering
        buf.ClearBuffers(pContext, { 0,0,0,0 });
        buf.Bind(pContext);

        // bind material data and its textures
        Material& mat = g_MaterialMgr.GetMatById(matIdx);
        BindMaterial(pRender, mat.renderStates, mat.texIds);

        pRender->cbpsMaterial_.data =
        {
            XMFLOAT4(&mat.ambient.x),
            XMFLOAT4(&mat.diffuse.x),
            XMFLOAT4(&mat.specular.x),
            XMFLOAT4(&mat.reflect.x)
        };
        pRender->cbpsMaterial_.ApplyChanges(pContext);

        // render geometry
        pContext->DrawIndexed(indexCount, 0U, 0U);

        ++matIdx;
    }

    // reset camera's viewProj to the previous one (it can be game or editor camera)
    pRender->SetViewProj(pContext, DirectX::XMMatrixTranspose(viewProj_));

    d3d.ResetBackBufferRenderTarget();
    d3d.ResetViewport();

    return true;
}

XMFLOAT3 s_CamPos;

//---------------------------------------------------------
// Desc:   sort input data elements by distance from the camera
// Args:   - camPos:  camera's current position
//         - data:    in/out container of data to sort
//---------------------------------------------------------
void SortByDistance(
    const XMFLOAT3& cameraPos,
    cvector<VertexGrass>& vertices)
{
    s_CamPos = cameraPos;

    // sort by square of distance to camera
    std::qsort(
        vertices.data(),
        vertices.size(),
        sizeof(VertexGrass),
        [](const void* x, const void* y)
        {
            const VertexGrass& v1 = *(VertexGrass*)(x);
            const VertexGrass& v2 = *(VertexGrass*)(y);

            const float sqrDistToCamera1 = SQR(v1.pos.x-s_CamPos.x) + SQR(v1.pos.y-s_CamPos.y) + SQR(v1.pos.z-s_CamPos.z);
            const float sqrDistToCamera2 = SQR(v2.pos.x-s_CamPos.x) + SQR(v2.pos.y-s_CamPos.y) + SQR(v2.pos.z-s_CamPos.z);

            if (sqrDistToCamera1 > sqrDistToCamera2)
                return -1;

            if (sqrDistToCamera1 < sqrDistToCamera2)
                return 1;

            return 0;
        });
}

//---------------------------------------------------------
//---------------------------------------------------------
void CGraphics::RenderGrass(Render::CRender* pRender)
{
    Shader* pShader = pRender->shaderMgr_.GetShaderByName("GrassShader");
    if (!pShader)
    {
        LogErr(LOG, "can't get a shader by name: GrassShader");
        return;
    }

    // bind shader to use it for rendering
    ID3D11DeviceContext* pContext = pContext_;
    pRender->shaderMgr_.BindShader(pContext, pShader);

    VertexBuffer<VertexGrass>& vb = g_ModelMgr.GetGrassVB();
    //cvector<VertexGrass>& grassVertices = g_ModelMgr.GetGrassVertices();
    //cvector<VertexGrass> nearGrass;

    //GetNearGrass(pSysState_->cameraPos, grassVertices, nearGrass);
    //SortByDistance(pSysState_->cameraPos, nearGrass);

    
    //vb.UpdateDynamic(pContext, nearGrass.data(), nearGrass.size());
    
    UINT stride = sizeof(VertexGrass);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);

    const Material& mat = g_MaterialMgr.GetMatByName("grass_0");
    BindMaterial(pRender, mat.renderStates, mat.texIds);


    RenderStates& renderStates = d3d_.GetRenderStates();
    //renderStates.SetBS(pContext, R_ALPHA_TO_COVERAGE);

    // render grass
    pContext->Draw(vb.GetVertexCount()/2, 0);


#if 0
    const Material& mat2 = g_MaterialMgr.GetMatByName("grass_1");
    BindMaterial(pRender, mat2.renderStates, mat2.texIds);

   

    pContext->VSSetShader(pShader2->GetVS()->GetShader(), nullptr, 0);
    pContext->IASetInputLayout(pShader2->GetVS()->GetInputLayout());
    pContext->GSSetShader(pShader2->GetGS()->GetShader(), nullptr, 0);
    pContext->PSSetShader(pShader2->GetPS()->GetShader(), nullptr, 0);

    pContext->Draw(500000, 1000000);
#endif
    //pSysState_->numDrawnVertices += nearGrass.size() *4;
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
    ID3D11DeviceContext* pContext,
    Render::CRender* pRender,
    const cvector<Render::InstanceBatch>& instanceBatches,
    UINT& startInstanceLocation,
    RenderStat& stat)
{
    const ShaderID shaderId = 0;

    for (const Render::InstanceBatch& batch : instanceBatches)
    {
        BindMaterial(pRender, batch.renderStates, batch.textures);

        pRender->RenderInstances(pContext, batch, startInstanceLocation);

        // stride idx in the instances buffer and accumulate render statistic
        startInstanceLocation  += (UINT)batch.numInstances;
        stat.numDrawnVertices  += batch.GetNumVertices();
        stat.numDrawnInstances += batch.numInstances;
        stat.numDrawCallsForInstances++;
    }
}

void SortParticlesByDistance(
    const XMFLOAT3& cameraPos,
    cvector<BillboardSprite>& vertices)
{
    s_CamPos = cameraPos;

    // sort by square of distance to camera
    std::qsort(
        vertices.data(),
        vertices.size(),
        sizeof(BillboardSprite),
        [](const void* x, const void* y)
        {
            const BillboardSprite& v1 = *(BillboardSprite*)(x);
            const BillboardSprite& v2 = *(BillboardSprite*)(y);

            const float sqrDist1ToCam = SQR(v1.pos.x - s_CamPos.x) + SQR(v1.pos.y - s_CamPos.y) + SQR(v1.pos.z - s_CamPos.z);
            const float sqrDist2ToCam = SQR(v2.pos.x - s_CamPos.x) + SQR(v2.pos.y - s_CamPos.y) + SQR(v2.pos.z - s_CamPos.z);

            if (sqrDist1ToCam > sqrDist2ToCam)
                return -1;

            if (sqrDist1ToCam < sqrDist2ToCam)
                return 1;

            return 0;
        });
}

//---------------------------------------------------------
// Desc:   render particles onto the screen
//---------------------------------------------------------
void CGraphics::RenderParticles(
    Render::CRender* pRender,
    ECS::EntityMgr* pEnttMgr)
{
    ECS::ParticleSystem& particleSys = pEnttMgr->particleSystem_;
    const ECS::ParticlesRenderData& particlesData = particleSys.GetParticlesToRender();

    // prepare updated particles data for rendering
    VertexBuffer<BillboardSprite>& vb = g_ModelMgr.GetBillboardsBuffer();
    const int numVertices             = (int)particlesData.particles.size();
    cvector<BillboardSprite> vertices(numVertices);

#if 0
    // convert particle data into vertices data for rendering
    for (int i = 0; const ECS::ParticleRenderInstance & particle : particlesData.particles)
    {
        vertices[i].pos          = particle.pos;
        vertices[i].translucency = particle.translucency;
        vertices[i].color        = particle.color;
        vertices[i].size         = particle.size;
        ++i;
    }
#else
    memcpy(vertices.data(), particlesData.particles.data(), sizeof(ECS::ParticleRenderInstance) * numVertices);

    //SortParticlesByDistance(pSysState_->cameraPos, vertices);
#endif

    // update the vertex buffer with updated particles data
    vb.UpdateDynamic(g_pContext, vertices.data(), numVertices);

    Shader* pShader = pRender->shaderMgr_.GetShaderByName("ParticleShader");
    if (!pShader)
    {
        LogErr(LOG, "can't get a shader by name: ParticleShader");
        return;
    }

    ID3D11DeviceContext* pContext = pContext_;

    //pRender->shaderMgr_.BindShader(pContext, pShader);
    //pRender->shaderMgr_.BindVertexBuffer(pContext, vb.Get(), sizeof(BillboardSprite), 0);

    pContext->VSSetShader(pShader->GetVS()->GetShader(), nullptr, 0);
    pContext->IASetInputLayout(pShader->GetVS()->GetInputLayout());
    pContext->GSSetShader(pShader->GetGS()->GetShader(), nullptr, 0);
    pContext->PSSetShader(pShader->GetPS()->GetShader(), nullptr, 0);

    UINT stride = sizeof(BillboardSprite);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);

    // go through emitters and render its particles
    const vsize numEmitters = particlesData.materialIds.size();

    for (int i = 0; i < (int)numEmitters; ++i)
    {
        // bind a material for particles
        const MaterialID matId = particlesData.materialIds[i];
        const Material& mat    = g_MaterialMgr.GetMatById(matId);
        BindMaterial(pRender, mat.renderStates, mat.texIds);

        // render particles subset
        pContext->Draw(particlesData.numInstances[i], particlesData.baseInstance[i]);
    }

    pSysState_->numDrawnVertices += numVertices;

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
    //instance.colorCenter  = sky.GetColorCenter();
    //instance.colorApex    = sky.GetColorApex();


    // bind sky material
    const MaterialID skyMatId = sky.GetMaterialId();
    const Material& skyMat    = g_MaterialMgr.GetMatById(skyMatId);
    BindMaterial(pRender, skyMat.renderStates, skyMat.texIds);

    // compute a worldViewProj matrix for the sky instance
    const XMFLOAT3 skyOffset     = pEnttMgr->transformSystem_.GetPosition(skyEnttID);
    const XMFLOAT3 eyePos        = pEnttMgr->cameraSystem_.GetPos(currCameraID_);
    const XMFLOAT3 translation   = skyOffset + eyePos;
    const XMMATRIX world         = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
    const XMMATRIX worldViewProj = DirectX::XMMatrixTranspose(world * viewProj_);

    pRender->RenderSkyDome(pContext_, instance, worldViewProj);
}

//---------------------------------------------------------
// Desc:   render terrain onto the screen
// Args:   - pRender: a pointer to the renderer (look at Render module)
//         - pEnttMgr: a pointer to the ECS manager (look at ECS module)
//---------------------------------------------------------
void CGraphics::RenderTerrainGeomip(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr)
{
#if 1
    // prepare the terrain instance
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    Render::TerrainInstance instance;

    // bind terrain material: switch render states and bind textures
    const Material& mat = g_MaterialMgr.GetMatById(terrain.materialID_);
    BindMaterial(pRender, mat.renderStates, mat.texIds);
 
    // vertex/index buffers data
    instance.vertexStride   = terrain.GetVertexStride();
    instance.pVB            = terrain.GetVertexBuffer();
    instance.pIB            = terrain.GetIndexBuffer();

    const TerrainLodMgr& lodMgr = terrain.GetLodMgr();
    const int terrainLen        = terrain.GetTerrainLength();
    const int numPatchesPerSide = terrain.GetNumPatchesPerSide();
    const int patchSize         = lodMgr.GetPatchSize();

    // setup rendering pipeline before rendering of the terrain
    ID3D11DeviceContext* pContext = pContext_;

    // bind terrain shader
    Render::Shader* pShader = pRender->shaderMgr_.GetShaderByName("TerrainShader");
    if (!pShader)
    {
        LogErr(LOG, "can't get a shader: there is no shader by name: TerrainShader");
        return;
    }
    pRender->shaderMgr_.BindShader(pContext, pShader);

    // bind vb/ib
    const UINT stride = instance.vertexStride;
    const UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &instance.pVB, &stride, &offset);
    pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0U);


    // setup material
    pRender->cbpsTerrainMaterial_.data.ambient  = XMFLOAT4(&mat.ambient.x);
    pRender->cbpsTerrainMaterial_.data.diffuse  = XMFLOAT4(&mat.diffuse.x);
    pRender->cbpsTerrainMaterial_.data.specular = XMFLOAT4(&mat.specular.x);
    pRender->cbpsTerrainMaterial_.data.reflect  = XMFLOAT4(&mat.reflect.x);

#if 0
    pRender->cbpsTerrainMaterial_.data.ambient  = instance.matColors.ambient;
    pRender->cbpsTerrainMaterial_.data.diffuse  = instance.matColors.diffuse;
    pRender->cbpsTerrainMaterial_.data.specular = instance.matColors.specular;
    pRender->cbpsTerrainMaterial_.data.reflect  = instance.matColors.reflect;
#endif
    pRender->cbpsTerrainMaterial_.ApplyChanges(pContext);
    //pContext->PSSetConstantBuffers(3, 1, pRender->cbpsTerrainMaterial_.GetAddressOf());


    int numDrawnTerrainPatches = 0;
    int numDrawnTerrainVertices = 0;

    // render each visible patch (sector) of the terrain
    for (const int idx : terrain.GetVisiblePatches())
    {
        const TerrainLodMgr::PatchLod& plod = lodMgr.GetPatchLodInfo(idx);

        terrain.GetLodInfoByPatch(plod, instance.baseIndex, instance.indexCount);

        const int patchZ = idx / numPatchesPerSide;
        const int patchX = idx % numPatchesPerSide;
        const int z = patchZ * (patchSize - 1);
        const int x = patchX * (patchSize - 1);
        instance.baseVertex = (UINT)(z*terrainLen + x);

        // draw a patch
        pContext->DrawIndexed(instance.indexCount, instance.baseIndex, instance.baseVertex);

        numDrawnTerrainPatches++;
        numDrawnTerrainVertices += (instance.indexCount / 3);
    }

    // gather some rendering statistic
    pSysState_->numDrawnTerrainPatches = numDrawnTerrainPatches;
    pSysState_->numCulledTerrainPatches = terrain.GetNumAllPatches() - numDrawnTerrainPatches;
    pSysState_->numDrawnVertices += numDrawnTerrainVertices;
#endif
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
    const ECS::PointLights& pointLights = lightSys.GetPointLights();
    const ECS::SpotLights& spotLights = lightSys.GetSpotLights();

    const size numDirLights   = dirLights.data.size();
    const size numPointLights = pointLights.data.size();
    const size numSpotLights  = spotLights.data.size();

    const cvector<EntityID>& visPointLights = renderSys.GetVisiblePointLights();
    const size numVisPointLightSources      = visPointLights.size();


    // prepare enough memory for lights buffer
    outData.ResizeLightData((int)numDirLights, (int)numPointLights, (int)numSpotLights);

    // ----------------------------------------------------
    // prepare point lights data 

    // store light properties, range, and attenuation
    for (index i = 0; i < numPointLights; ++i)
    {
        outData.pointLights[i].ambient = pointLights.data[i].ambient;
        outData.pointLights[i].diffuse = pointLights.data[i].diffuse;
        outData.pointLights[i].specular = pointLights.data[i].specular;
        outData.pointLights[i].att = pointLights.data[i].att;
        outData.pointLights[i].range = pointLights.data[i].range;
    }

    // store positions
    cvector<XMFLOAT3> pointLightsPositions;
    pEnttMgr->transformSystem_.GetPositions(pointLights.ids.data(), numPointLights, pointLightsPositions);

    for (index i = 0; i < numPointLights; ++i)
    {
        outData.pointLights[i].position = pointLightsPositions[i];
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

    if (numSpotLights > 0)
    {
        lightSys.GetSpotLightsData(
            spotLights.ids.data(),
            numSpotLights,
            s_LightTmpData.spotLightsData,
            s_LightTmpData.spotLightsPositions,
            s_LightTmpData.spotLightsDirections);

        for (index i = 0; i < numSpotLights; ++i)
        {
            outData.spotLights[i].ambient = s_LightTmpData.spotLightsData[i].ambient;
            outData.spotLights[i].diffuse = s_LightTmpData.spotLightsData[i].diffuse;
            outData.spotLights[i].specular = s_LightTmpData.spotLightsData[i].specular;
            outData.spotLights[i].range = s_LightTmpData.spotLightsData[i].range;
            outData.spotLights[i].spot = s_LightTmpData.spotLightsData[i].spot;
            outData.spotLights[i].att = s_LightTmpData.spotLightsData[i].att;
        }

        for (int i = 0; const XMFLOAT3 & pos : s_LightTmpData.spotLightsPositions)
            outData.spotLights[i++].position = pos;

        for (int i = 0; const XMFLOAT3 & dir : s_LightTmpData.spotLightsDirections)
            outData.spotLights[i++].direction = dir;
    }

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
        const BasicModel& model = g_ModelMgr.GetModelById(modelID);

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
        const char* name = pEnttMgr->nameSystem_.GetNameById(selectedEnttID);

        SetConsoleColor(YELLOW);
        LogMsg("picked entt (id: %" PRIu32 "; name: % s)", selectedEnttID, name);
        SetConsoleColor(RESET);
    }

    // return ID of the selected entt, or 0 if we didn't pick any
    return selectedEnttID;
}

} // namespace Core
