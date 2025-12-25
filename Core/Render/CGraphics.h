// =================================================================================
// Filename:     CGraphics.h
// Description:  controls all the main parts of rendering process:
//               mainly update frame data and prepare data for the rendering
// Revising:     07.11.22
// =================================================================================
#pragma once

#include <post_fx_enum.h>
#include <geometry/frustum.h>

// engine stuff
#include <CoreCommon/system_state.h>     // contains the current information about the engine
#include "../Engine/engine_configs.h"

// render stuff
#include "Render/d3dclass.h"            // from Render module
#include "Render/CRender.h"             // from Render module

#include "rnd_data_preparator.h"
#include "frame_buffer.h"               // for rendering to some particular texture

// ECS
#include "Entity/EntityMgr.h"

#include <DirectXCollision.h>


namespace Core
{

struct Material;

//-----------------------------------------------------

class CGraphics
{

public:
    enum eTextureSlots
    {
        TEX_SLOT_DEPTH       = 10,     // resolved (non-MSAA) texture with depth values
        TEX_SLOT_DEPTH_MSAA  = 11,     // 4xMSAA texture with depth values
        TEX_SLOT_POST_FX_SRC = 12,     // source texture for our post process pass
    };

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

    // check if we have any entity by these coords of the screen
    void GetRayIntersectionPoint(const int sx, const int sy);
    int TestEnttSelection       (const int sx, const int sy);


    // ------------------------------------
    // render related methods

    void FrustumCullingEntts      (SystemState& sysState);
    void FrustumCullingParticles  (SystemState& sysState, const Frustum& worldFrustum);
    void FrustumCullingPointLights(SystemState& sysState, const Frustum& worldFrustum);

    void ClearRenderingDataBeforeFrame();
    void Render3D();
    
    void BindMaterial      (const Material& mat);
    void BindMaterialById  (const MaterialID matId);
    void BindMaterialByName(const char* matName);

    void BindMaterial(
        const uint32 renderStatesBitfields,
        const TexID* texIds);

    void BindMaterial(
        const uint32 renderStatesBitfield,
        ID3D11ShaderResourceView* const* texViews);

    
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


    // ---------------------------------------
    // INLINE GETTERS/SETTERS

    inline void SetAntiAliasingType(const uint8 type)               { GetD3D().SetAntiAliasingType(type); }
    inline uint8 GetAntiAliasingType()                        const { return (uint8)(GetD3D().GetAntiAliasingType()); }

    inline void     SetCurrentCamera(const EntityID cameraID)       { currCameraID_ = cameraID; }
    inline EntityID GetCurrentCamera()                        const { return currCameraID_; }
   
    inline void EnableDepthPrepass(const bool state)                { enableDepthPrepass_ = state; }
    inline bool IsEnabledDepthPrepass()                       const { return enableDepthPrepass_; }

    inline void EnableDepthVisualize(const bool state)              { visualizeDepth_ = state;}
    inline bool IsEnabledDepthVisualize()                     const { return visualizeDepth_; }

    inline bool IsEnabledPostFxPass()                         const { return enablePostFXs_ && (numPostFxsInQueue_ != 0); }

    //-----------------------------------------------------
    // post effects
    //-----------------------------------------------------
    inline void EnablePostFxs(const bool state)                     { enablePostFXs_ = state; }     // turn on/off using of post effects
    inline bool IsPostFxsEnabled()                            const { return enablePostFXs_; }      // is using of post effects enabled?
    inline const ePostFxType* GetPostFxsQueue()               const { return postFxsQueue_; }       // return a ptr to queue of currently used post effects
    inline uint8 GetNumUsedPostFxs()                          const { return numPostFxsInQueue_; }  // return a number of currenly used post effects

    void PushPostFx  (const ePostFxType type);
    void RemovePostFx(const uint8 orderNum);


    //-------------------------------------------
    // model preview configuration (for model editor, or model screenshot tool)
    //-------------------------------------------
    bool InitModelFrameBuf(const uint width, const uint height);
    bool RenderModelIntoFrameBuf();

    void  SetModelPreviewParam(const uint8 param, const float val);
    float GetModelPreviewParam(const uint8 param) const;

    inline ID3D11ShaderResourceView* GetModelFrameBufView() const
    {
        if (pModelFrameBuf_ != nullptr)
            return pModelFrameBuf_->GetSRV();

        return nullptr;
    }

    inline void IncreaseCurrentBoneId()
    {
        static uint boneId = 0;

        boneId++;
        boneId %= 35;
        pRender_->UpdateCbDebug(boneId);
        printf("current bone id: %u\n", boneId);
    }

    int currAnimIdx_ = 0;

    inline void IncreaseCurrAnimIdx()
    {
        currAnimIdx_++;
        currAnimIdx_ %= 22;
    }


private:

    inline Render::D3DClass&    GetD3D()     const { return pRender_->GetD3D(); }
    inline ID3D11Device*        GetDevice()  const { return pRender_->GetDevice(); }
    inline ID3D11DeviceContext* GetContext() const { return pRender_->GetContext(); }

    // --- updating state
    void UpdateHelper             (const float deltaTime, const float gameTime);
    void UpdateParticlesVB        ();
    void UpdateShadersDataPerFrame(const float deltaTime, const float gameTime);
    void AddFrustumToRender       (const EntityID camId);
    void PrepareRenderInstances   (const DirectX::XMFLOAT3& cameraPos);
    void SetupLightsForFrame      (Render::PerFrameData& perFrameData);
    
    // --- rendering stages
    void ResetBeforeRendering();
    void ResetRenderStatesToDefault();
    void DepthPrepass();
    void ColorLightPass();
    void PostFxPass();

    // --- rendering stage: depth pre-pass
    void TerrainDepthPrepass();

    void DepthPrepassInstanceGroup(
        const cvector<Render::InstanceBatch>& instanceBatches,
        const eGeomType geomType,
        UINT& startInstanceLocation);

    // --- rendering stage: color pass
    void RenderSkinnedModel(const EntityID enttId);
    void RenderGrass();

    void RenderInstanceGroups(
        const cvector<Render::InstanceBatch>& instanceBatches,
        const eGeomType geomType,
        UINT& startInstanceLocation);

    void RenderParticles();
    void RenderSkyClouds();
    void RenderSkyDome();

    void RenderTerrainGeomip();
    void RenderPlayerWeapon();
    void RenderDebugShapes();
    void VisualizeDepthBuffer();
    void RenderPostFxOnePass();
    void RenderPostFxMultiplePass();


public:
    RenderStat rndStat_;

    DirectX::XMMATRIX WVO_            = DirectX::XMMatrixIdentity();  // main_world * baseView * ortho
    DirectX::XMMATRIX viewProj_       = DirectX::XMMatrixIdentity();  // view * projection

    DirectX::XMMATRIX worldMatrix_    = DirectX::XMMatrixIdentity();  // main_world
    DirectX::XMMATRIX baseViewMatrix_ = DirectX::XMMatrixIdentity();  // for UI rendering
    DirectX::XMMATRIX orthoMatrix_    = DirectX::XMMatrixIdentity();  // for UI rendering

    SystemState* pSysState_ = nullptr;                                

    cvector<DirectX::BoundingFrustum> frustums_;
    
    RenderDataPreparator  prep_;
    FrameBuffer           frameBuffer_;                           // for rendering to some texture
    EntityID              currCameraID_ = 0;
    
    FrameBuffer                         materialBigIconFrameBuf_;
    FrameBuffer*                        pModelFrameBuf_ = nullptr;
    cvector<FrameBuffer>                materialsFrameBuffers_;   // frame buffers which are used to render materials icons (for editor's material browser)
    cvector<ID3D11ShaderResourceView*>  texturesBuf_;             // to avoid reallocation each time we use this shared buffer

    Render::CRender* pRender_ = nullptr;
    ECS::EntityMgr* pEnttMgr_ = nullptr;

private:
    bool enableDepthPrepass_  = false;
    bool visualizeDepth_      = false;
    bool enablePostFXs_       = false;

    float gameTime_ = 0;

    ePostFxType postFxsQueue_[MAX_NUM_POST_EFFECTS];
    uint8 numPostFxsInQueue_ = 0;

    // for model preview
    ModelPreviewRenderParams modelPreviewRndParams_;
};

} // namespace Core
