// =================================================================================
// Filename:     CGraphics.h
// Description:  controls all the main parts of rendering process:
//               mainly update frame data and prepare data for the rendering
// Revising:     07.11.22
// =================================================================================
#pragma once


// engine stuff
#include <CoreCommon/SystemState.h>     // contains the current information about the engine
#include "../Engine/EngineConfigs.h"

// input devices events
#include "../Input/KeyboardEvent.h"
#include "../Input/MouseEvent.h"

// render stuff
#include "d3dclass.h"
#include "RenderDataPreparator.h"
#include "CRender.h"
#include "FrameBuffer.h"      // for rendering to some particular texture

// Entity-Component-System
#include "Entity/EntityMgr.h"

#include <DirectXCollision.h>

namespace Core
{

class CGraphics
{

public:
    enum AABBShowMode
    {
        NONE,    // doesn't show any AABB line boxes
        MODEL,   // render AABB line box around the whole model
        MESH,    // render AABB line box around each mesh of model
    };

public:
    CGraphics();
    ~CGraphics();

    // restrict a copying of this class instance
    CGraphics(const CGraphics& obj) = delete;
    CGraphics& operator=(const CGraphics& obj) = delete;


    bool Initialize(
        HWND hwnd,
        SystemState& sysState,
        const EngineConfigs& cfg,
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender);

    void Shutdown();

    void Update(
        SystemState& sysState,
        const float dt,
        const float totalGameTime,
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender);

    // ------------------------------------
    // render related methods

    void ComputeFrustumCulling              (SystemState& sysState, ECS::EntityMgr* pEnttMgr);

    void ComputeFrustumCullingOfLightSources(SystemState& sysState, ECS::EntityMgr* pEnttMgr);
    void ClearRenderingDataBeforeFrame      (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void Render3D                           (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);

    void BindMaterial(const Material& mat, Render::CRender* pRender);

    bool RenderBigMaterialIcon(
        const MaterialID matID,
        const int iconWidth,
        const int iconHeight,
        const float yRotationAngle,
        Render::CRender* pRender,
        ID3D11ShaderResourceView** outMaterialImg);

    void RenderMaterialsIcons(
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender,
        ID3D11ShaderResourceView** outArrShaderResourceViews,
        const size numIcons,
        const int iconWidth,
        const int iconHeight);


    // ----------------------------------

    
    inline void SetAABBShowMode(const AABBShowMode mode)            { aabbShowMode_ = mode; }

    inline void     SetCurrentCamera(const EntityID cameraID)       { currCameraID_ = cameraID; }
    inline EntityID GetCurrentCamera()                        const { return currCameraID_; }

    // ---------------------------------------
    // INLINE GETTERS/SETTERS

    inline D3DClass& GetD3DClass() { return d3d_; }

    // ---------------------------------------

    // memory allocation (because we have some XM-data structures)
    void* operator new(std::size_t count);                              // a replaceable allocation function
    void* operator new(std::size_t count, const std::nothrow_t & tag);  // a replaceable non-throwing allocation function
    void* operator new(std::size_t count, void* ptr);                   // a non-allocating placement allocation function
    void operator delete(void* ptr);

    // check if we have any entity by these coords of the screen
    int TestEnttSelection(const int sx, const int sy, ECS::EntityMgr* pEnttMgr);

private:

    bool InitHelper(
        HWND hwnd,
        SystemState& systemState,
        const EngineConfigs& settings,
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender);

    void InitRenderModule(
        const EngineConfigs& settings,
        Render::CRender* pRender);

    void UpdateHelper(
        SystemState& sysState,
        const float deltaTime,
        const float gameTime,
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender);

    void RenderHelper(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
  
    void UpdateShadersDataPerFrame(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);

    // ------------------------------------------
    // rendering data prepararion stage API

    void PrepBasicInstancesForRender        (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void PrepAlphaClippedInstancesForRender (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void PrepBlendedInstancesForRender      (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);

    // ------------------------------------------

    void RenderEnttsDefault          (Render::CRender* pRender);
    void RenderEnttsAlphaClipCullNone(Render::CRender* pRender);
    void RenderEnttsBlended          (Render::CRender* pRender);
    void RenderBillboards            (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderMaterialSphere        (const int matIdx, Render::CRender* pRender);

    // ------------------------------------------

    // render bounding boxes of models/meshes/light_sources/etc.
    void RenderBoundingLineBoxes  (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderBoundingLineSpheres();
    void RenderSkyDome            (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderTerrainGeomip      (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderTerrainQuadtree    (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);

    // ------------------------------------------

    void SetupLightsForFrame(ECS::EntityMgr* pEnttMgr, Render::PerFrameData& perFrameData);

public:
    DirectX::XMMATRIX WVO_            = DirectX::XMMatrixIdentity();  // main_world * baseView * ortho
    DirectX::XMMATRIX viewProj_       = DirectX::XMMatrixIdentity();  // view * projection

    DirectX::XMMATRIX worldMatrix_    = DirectX::XMMatrixIdentity();  // main_world
    DirectX::XMMATRIX baseViewMatrix_ = DirectX::XMMatrixIdentity();  // for UI rendering
    DirectX::XMMATRIX orthoMatrix_    = DirectX::XMMatrixIdentity();  // for UI rendering

    // for furstum culling and picking
    ID3D11Device*         pDevice_ = nullptr;
    ID3D11DeviceContext*  pContext_ = nullptr;
    SystemState*          pSysState_ = nullptr;                       // we got this ptr during init

    cvector<DirectX::BoundingFrustum> frustums_;

    D3DClass              d3d_;
    RenderDataPreparator  prep_;
    FrameBuffer           frameBuffer_;                           // for rendering to some texture
    EntityID              currCameraID_ = 0;
    
    // for rendering
    ECS::RenderStatesSystem::EnttsRenderStatesData rsDataToRender_;

    FrameBuffer                         materialBigIconFrameBuf_;
    cvector<FrameBuffer>                materialsFrameBuffers_;   // frame buffers which are used to render materials icons (for editor's material browser)
    cvector<ID3D11ShaderResourceView*>  texturesBuf_;             // to avoid reallocation each time we use this shared buffer

    AABBShowMode aabbShowMode_ = NONE;
};

} // namespace Core
