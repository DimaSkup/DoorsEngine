// =================================================================================
// Filename:     CGraphics.h
// Description:  controls all the main parts of rendering process:
//               mainly update frame data and prepare data for the rendering
// Revising:     07.11.22
// =================================================================================
#pragma once

#include <post_fx_enum.h>
#include <camera_params.h>
#include <geometry/frustum.h>

// engine stuff
#include <CoreCommon/system_state.h>     // contains the current information about the engine
#include "../Engine/engine_configs.h"

// render stuff
#include "../Mesh/material.h"
#include "../Mesh/material_mgr.h"
#include "Render/d3dclass.h"            // from Render module
#include "Render/CRender.h"             // from Render module

#include "r_data_preparator.h"
#include "frame_buffer.h"               // for rendering to some particular texture

// ECS
#include "Entity/EntityMgr.h"

#include <DirectXCollision.h>

//-----------------------------------------------------
// forward declaration (pointer use only)
//-----------------------------------------------------
class Matrix;
class Frustum;

namespace Core
{

//-----------------------------------------------------
// forward declaration (pointer use only)
//-----------------------------------------------------
class Model;

//-----------------------------------------------------

class CGraphics
{
public:

    //
    // internal structures + enums
    //

    //-----------------------------------------------------
    // geometry types of instances (entities);
    // this enum is used for frame stat gathering
    //-----------------------------------------------------
    enum eGeomType
    {
        GEOM_TYPE_ALL,           // whole frame
        GEOM_TYPE_ENTTS,         // all the entts on scene
        GEOM_TYPE_MASKED,        // masked: grass/trees/bushes/wireframe gates/etc.
        GEOM_TYPE_OPAQUE,        // opaque geometry
        GEOM_TYPE_BLENDED,       // blended geometry
        GEOM_TYPE_TRANSPARENT,   // transparent geometry
        GEOM_TYPE_GRASS,         
        GEOM_TYPE_TERRAIN,
        GEOM_TYPE_PARTICLE,
        GEOM_TYPE_SKY,           // sky sphere/box/dome
        GEOM_TYPE_SKY_PLANE,     // clouds

        NUM_GEOM_TYPES,
    };

    //-----------------------------------------------------
    // a container for rendering statistic for the current frame
    //-----------------------------------------------------
    struct RenderStat
    {
        uint32 numDrawnVerts    [NUM_GEOM_TYPES]{ 0 };    // drawn vertices
        uint32 numDrawnTris     [NUM_GEOM_TYPES]{ 0 };    // drawn triangles
        uint32 numDrawnInstances[NUM_GEOM_TYPES]{ 0 };
        uint32 numDrawCalls     [NUM_GEOM_TYPES]{ 0 };
    };

    //---------------------------------------------------------
    // model preview parameters (for model editor, or model screenshot tool)
    //---------------------------------------------------------
    enum eModelPreviewParams
    {
        MODEL_ID,                    // which model to render
        MODEL_POS_X,
        MODEL_POS_Y,
        MODEL_POS_Z,
        MODEL_ROT_X,
        MODEL_ROT_Y,
        MODEL_ROT_Z,
        MODEL_SCALE,                 // uniform scale

        CAMERA_POS_X,
        CAMERA_POS_Y,
        CAMERA_POS_Z,
        CAMERA_ROT_X,
        CAMERA_ROT_Y,
        CAMERA_ROT_Z,

        FRAME_BUF_WIDTH,
        FRAME_BUF_HEIGHT,

        ORTHO_MATRIX_VIEW_HEIGHT,
        USE_ORTHO_MATRIX,            // flag: use ortho matrix or not

        BG_COLOR_R,
        BG_COLOR_G,
        BG_COLOR_B,

        NUM_MODEL_PREVIEW_PARAMS,
    };

    //---------------------------------------------------------
    // params for model previewer or models screenshot tool
    //---------------------------------------------------------
    struct ModelPreviewRenderParams
    {
        // currently selected model for rendering
        ModelID modelId = 0;

        // model transformation
        Vec3 modelPos = { 0,0,0 };
        Vec3 modelRot = { 0,0,0 };
        float modelScale = 1.0f;

        // camera transformation
        Vec3 camPos = { 0,0,-3 };
        Vec3 camRot = { 0,0,0 };

        // frame buffer params (and output image as well)
        int frameBufWidth  = 480;
        int frameBufHeight = 320;
        Vec3 bgColor       = { 0.5f, 0.5f, 0.5f };

        // ortho matrix params
        bool useOrthoMatrix = false;
        float orthoViewHeight = 1.0f;  // height of the frustum at the near clipping plane

    };

    //-----------------------------------------------------

public:
    CGraphics();
    ~CGraphics();

    // restrict a copying of this class instance
    CGraphics(const CGraphics& obj) = delete;
    CGraphics& operator=(const CGraphics& obj) = delete;


    void Init(HWND hwnd, SystemState& sysState);
    void Shutdown();

    void Update(const float dt, const float totalGameTime);

    void BindRender(Render::CRender* pRender);
    void BindECS   (ECS::EntityMgr* pEnttMgr);

    //---------------------------------
    // collision:  ray/entities tests
    //---------------------------------
    bool TestRayIntersectEntts(
        const DirectX::XMVECTOR& rayOrigin,
        const DirectX::XMVECTOR& rayDir,
        IntersectionData& outData) const;

    int  TestEnttSelection (const int sx, const int sy);


    //---------------------------------
    // render related methods
    //---------------------------------
    void GetWorldFrustum          (const EntityID camId, Frustum& outFrustum, CameraParams& params);
    void FrustumCullingEntts      (const Frustum& worldFrustum);
    void FrustumCullingParticles  (const Frustum& worldFrustum);
    void FrustumCullingPointLights(const Frustum& worldFrustum);

    void ClearRenderingDataBeforeFrame();
    void Render3D();
    void RenderDecals();

    void LockFrustumCulling(const bool onOff);
    bool IsLockedFrustumCulling(void) const;


    //---------------------------------
    // material binding
    //---------------------------------
    void BindMaterial      (const Material& mat);
    void BindMaterialById  (const MaterialID matId);

    void BindMaterial(
        const bool alphaClip,
        const RsID rsId,
        const BsID bsId,
        const DssID dssId,
        const TexID* texIds);


    //---------------------------------
    // rendering into frame buffers
    //---------------------------------
    bool InitMatBigIconFrameBuf(const uint width, const uint height);

    bool InitMatIconFrameBuffers(
        const size numBuffers,
        const uint32 width,
        const uint32 height,
        cvector<ID3D11ShaderResourceView*>& outShaderResourceViews);

    bool RenderBigMaterialIcon(
        const MaterialID matID,
        const float yRotationAngle,
        ID3D11ShaderResourceView** outMaterialImg);

    bool RenderMaterialsIcons();

    //---------------------------------
    // inline getters/setters
    //---------------------------------
    inline void SetAntiAliasingType(const uint8 type)               { GetD3D().SetAntiAliasingType(type); }
    inline uint8 GetAntiAliasingType()                        const { return (uint8)(GetD3D().GetAntiAliasingType()); }

    inline void     SetActiveCamera(const EntityID id)              { currCameraId_ = id; }
    inline EntityID GetActiveCamera()                         const { return currCameraId_; }
   
    inline void EnableDepthPrepass(const bool onOff)                { enableDepthPrepass_ = onOff; }
    inline bool IsEnabledDepthPrepass()                       const { return enableDepthPrepass_; }

    inline void EnableDepthVisualize(const bool onOff)              { visualizeDepth_ = onOff;}
    inline bool IsEnabledDepthVisualize()                     const { return visualizeDepth_; }

    inline bool IsEnabledPostFxPass()                         const { return enablePostFXs_ && (numPostFxsInQueue_ != 0); }

    //---------------------------------
    // post effects
    //---------------------------------
    inline void EnablePostFxs(const bool onOff)                     { enablePostFXs_ = onOff; }     // turn on/off using of post effects
    inline bool IsPostFxsEnabled()                            const { return enablePostFXs_; }      // is using of post effects enabled?
    inline const ePostFxType* GetPostFxsQueue()               const { return postFxsQueue_; }       // return a ptr to queue of currently used post effects
    inline uint8 GetNumUsedPostFxs()                          const { return numPostFxsInQueue_; }  // return a number of currenly used post effects

    bool IsPostFxEnabled(const ePostFxType type) const;
    void PushPostFx  (const ePostFxType type);
    void RemovePostFx(const ePostFxType type);
    void RemovePostFx(const uint8 orderNum);


    //---------------------------------
    // model preview configuration (for model editor, or model screenshot tool)
    //---------------------------------
    bool InitModelFrameBuf(const uint width, const uint height);
    bool RenderModelIntoFrameBuf();

    void  SetModelPreviewParam(const uint8 param, const float val);
    float GetModelPreviewParam(const uint8 param) const;

    ID3D11ShaderResourceView* GetModelFrameBufView() const;

    void SwitchQuadTree();

private:

    inline Render::D3DClass&    GetD3D()     const { return pRender_->GetD3D(); }
    inline ID3D11Device*        GetDevice()  const { return pRender_->GetDevice(); }
    inline ID3D11DeviceContext* GetContext() const { return pRender_->GetContext(); }

    //---------------------------------
    // updating state
    //---------------------------------
    void UpdateParticlesVB        (void);
    void UpdateShadersDataPerFrame(const float dt, const float gameTime);
    void AddFrustumToRender       (const EntityID camId);
    void PrepareRenderInstances   (const DirectX::XMFLOAT3& cameraPos);
    void SetupLightsForFrame      (Render::PerFrameData& perFrameData);

    void ResetRenderStats       (void);
    void UpdatePlayerPos        (void);
    void UpdateCamera           (void);
    void GatherCameraParams     (const EntityID camId, CameraParams& outParams);
    void Push2dSpritesToRender  (void);

    void AddDebugShapesToRender (void);

    //---------------------------------
    // rendering stages
    //---------------------------------
    void ResetBeforeRendering();
    void ResetRenderStatesToDefault();
    void DepthPrepass();
    void ColorLightPass();
    void PostFxPass();

    //---------------------------------
    // rendering stage: depth pre-pass
    //---------------------------------
    void TerrainDepthPrepass();

    void DepthPrepassInstanceGroup(
        const cvector<Render::InstanceBatch>& instanceBatches,
        const eGeomType geomType,
        UINT& startInstanceLocation);

    //---------------------------------
    // rendering stage: color pass
    //---------------------------------
    void RenderSkinnedModel(const EntityID enttId);
    void RenderGrass();
    void RenderGrassField(const index fieldIdx);

    void RenderInstanceGroups(
        const cvector<Render::InstanceBatch>& instanceBatches,
        const eGeomType geomType,
        UINT& startInstanceLocation);

    void RenderParticles();
    void RenderSkyClouds();
    void RenderSkyDome();

    void RenderTerrain();
    void RenderPlayerWeapon();
    void RenderDebugShapes();

    //---------------------------------
    // collision helpers:  ray/entities tests
    //---------------------------------
    void RayEnttTest(
        const EntityID enttId,
        const DirectX::XMVECTOR& rayOrigW,
        const DirectX::XMVECTOR& rayDirW,
        float& tmin,
        IntersectionData& outData,
        DirectX::XMVECTOR& outRayOrigL,
        DirectX::XMVECTOR& outRayDirL) const;

    bool RayModelTest(
        const Model* pModel,
        const DirectX::XMVECTOR& rayOrigin,
        const DirectX::XMVECTOR& rayDir,
        float& tmin,
        uint& intersectedTriangleIdx) const;

    void GatherIntersectionData(
        const DirectX::XMVECTOR rayOrigL,
        const DirectX::XMVECTOR rayDirL,
        const DirectX::XMVECTOR rayOrigW,
        const DirectX::XMVECTOR rayDirW,
        const float t,
        IntersectionData& outData) const;

public:
    RenderStat rndStat_;

    DirectX::XMMATRIX       WVO_            = DirectX::XMMatrixIdentity();  // main_world * baseView * ortho
    DirectX::XMMATRIX       viewProj_       = DirectX::XMMatrixIdentity();  // view * projection

    DirectX::XMMATRIX       worldMatrix_    = DirectX::XMMatrixIdentity();  // main_world
    DirectX::XMMATRIX       baseViewMatrix_ = DirectX::XMMatrixIdentity();  // for UI rendering
    DirectX::XMMATRIX       orthoMatrix_    = DirectX::XMMatrixIdentity();  // for UI rendering

    SystemState*            pSysState_      = nullptr;                                

    RenderDataPreparator    prep_;
    FrameBuffer             frameBuffer_;                         // for rendering to some texture
    EntityID                currCameraId_   = 0;
    
    FrameBuffer                         materialBigIconFrameBuf_;
    FrameBuffer*                        pModelFrameBuf_ = nullptr;
    cvector<FrameBuffer>                materialsFrameBuffers_;   // frame buffers which are used to render materials icons (for editor's material browser)
    cvector<ID3D11ShaderResourceView*>  texturesBuf_;             // to avoid reallocation each time we use this shared buffer

    Render::CRender*        pRender_  = nullptr;
    ECS::EntityMgr*         pEnttMgr_ = nullptr;

private:
    bool enableDepthPrepass_  = false;
    bool visualizeDepth_      = false;
    bool enablePostFXs_       = false;
    bool bUseQuadTree_        = false;
    bool bLockFrustumCull_    = false;

    float gameTime_ = 0;

    ePostFxType postFxsQueue_[MAX_NUM_POST_EFFECTS];
    uint8 numPostFxsInQueue_ = 0;

    // for model preview
    ModelPreviewRenderParams modelPreviewRndParams_;
};


//==================================================================================
// inline functions
//==================================================================================

//---------------------------------------------------------
// Desc:   clear rendering data from the previous frame / instances set
//---------------------------------------------------------
inline void CGraphics::ClearRenderingDataBeforeFrame()
{
    pRender_->dataStorage_.Clear();
}

//---------------------------------------------------------
// Desc:  bind input material (textures + render states) 
//---------------------------------------------------------
inline void CGraphics::BindMaterial(const Material& mat)
{
    BindMaterial(mat.alphaClip, mat.rsId, mat.bsId, mat.dssId, mat.texIds);
}

//---------------------------------------------------------
// Desc:  bind a material (textures + render states) by input ID
//---------------------------------------------------------
inline void CGraphics::BindMaterialById(const MaterialID matId)
{
    const Material& mat = g_MaterialMgr.GetMatById(matId);
    BindMaterial(mat.alphaClip, mat.rsId, mat.bsId, mat.dssId, mat.texIds);
}

//---------------------------------------------------------
// Desc:  reset textures and render states so we will be able
//        to setup it properly for rendering
//---------------------------------------------------------
inline void CGraphics::ResetBeforeRendering()
{
    BindMaterialById(0);
}

//---------------------------------------------------------
// Desc:  texture surface of model's preview
//        (from the model viewer or models screenshot tool)
//---------------------------------------------------------
inline ID3D11ShaderResourceView* CGraphics::GetModelFrameBufView() const
{
    if (pModelFrameBuf_)
        return pModelFrameBuf_->GetSRV();

    return nullptr;
}

//---------------------------------------------------------
// enabled/disable using a quad tree for entities culling
//---------------------------------------------------------
inline void CGraphics::SwitchQuadTree()
{
    bUseQuadTree_ = !bUseQuadTree_;
    if (bUseQuadTree_)
        printf("switch to: QUAD TREE\n");
    else
        printf("switch to: BASIC CULL\n");
}

//---------------------------------------------------------
// 1. turn on/off locking of the frustum culling for the scene camera
// 2. is frustum culling is locked?
//---------------------------------------------------------
inline void CGraphics::LockFrustumCulling(const bool onOff)
{
    bLockFrustumCull_ = onOff;
}

inline bool CGraphics::IsLockedFrustumCulling(void) const
{
    return bLockFrustumCull_;
}

} // namespace Core
