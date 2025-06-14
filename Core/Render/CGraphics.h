// =================================================================================
// Filename:     CGraphics.h
// Description:  controls all the main parts of rendering process:
//               mainly update frame data and prepare data for the rendering
// Revising:     07.11.22
// =================================================================================
#pragma once


// engine stuff
#include <CoreCommon/SystemState.h>     // contains the current information about the engine
#include "../Engine/Settings.h"

// input devices events
#include "../Input/KeyboardEvent.h"
#include "../Input/MouseEvent.h"

#include "RenderDataPreparator.h"

// render stuff
#include "CRender.h"
#include "InitializeGraphics.h"        // for initialization of the graphics
#include "FrameBuffer.h"      // for rendering to some particular texture

// Entity-Component-System
#include "Entity/EntityMgr.h"

#include <map>
#include <DirectXCollision.h>

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
};

// --------------------------------------------------------

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
        const Settings& settings,
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
    void ComputeFrustumCullingOld           (SystemState& sysState, ECS::EntityMgr* pEnttMgr);

    void ComputeFrustumCullingOfLightSources(SystemState& sysState, ECS::EntityMgr* pEnttMgr);
    void ClearRenderingDataBeforeFrame      (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void Render3D                           (ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void RenderModel                        (BasicModel& model, const DirectX::XMMATRIX& world);

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

    inline void SetGameMode(bool enableGameMode)                    { isGameMode_ = enableGameMode; }
    inline void SetAABBShowMode(const AABBShowMode mode)            { aabbShowMode_ = mode; }
    inline void SetFullFogDist(const int dist)                      { if (dist > 0) fullFogDistance_ = dist; }  // after this distance we use only billboards (I hope for it)

    inline void     SetCurrentCamera(const EntityID cameraID)       { currCameraID_ = cameraID; }
    inline EntityID GetCurrentCamera()                        const { return currCameraID_; }

    // ---------------------------------------
    // INLINE GETTERS/SETTERS

    inline D3DClass&       GetD3DClass()                      { return d3d_; }

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
        const Settings& settings,
        ECS::EntityMgr* pEnttMgr,
        Render::CRender* pRender);

    void InitRenderModule(
        const Settings& settings,
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
    void RenderFoggedBillboards      (Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderMaterialSphere        (const int matIdx, Render::CRender* pRender);

    // ------------------------------------------

    // render bounding boxes of models/meshes/light_sources/etc.
    void RenderBoundingLineBoxes(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderBoundingLineSpheres();
    void RenderSkyDome(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderTerrain(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);

#if 0
    void UpdateInstanceBuffAndRenderInstances(
        ID3D11DeviceContext* pDeviceContext,
        const Render::ShaderTypes type,
        const Render::InstBuffData& instanceBuffData,
        const std::vector<Render::Instance>& instances);
#endif

    // ------------------------------------------

    void SetupLightsForFrame(ECS::EntityMgr* pEnttMgr, Render::PerFrameData& perFrameData);

public:
    DirectX::XMMATRIX WVO_            = DirectX::XMMatrixIdentity();  // main_world * baseView * ortho
    DirectX::XMMATRIX viewProj_       = DirectX::XMMatrixIdentity();  // view * projection

    DirectX::XMMATRIX worldMatrix_    = DirectX::XMMatrixIdentity();  // main_world
    DirectX::XMMATRIX baseViewMatrix_ = DirectX::XMMatrixIdentity();  // for UI rendering
    DirectX::XMMATRIX orthoMatrix_    = DirectX::XMMatrixIdentity();  // for UI rendering

    // for furstum culling and picking
    std::vector<DirectX::XMMATRIX> enttsLocalSpaces_;                 // local space of each currently visible entt
    std::vector<DirectX::BoundingOrientedBox> enttsBoundBoxes_;

    ID3D11Device*         pDevice_ = nullptr;
    ID3D11DeviceContext*  pDeviceContext_ = nullptr;
    SystemState*          pSysState_ = nullptr;                       // we got this ptr during init

    std::vector<DirectX::BoundingFrustum> frustums_;

    D3DClass              d3d_;
    RenderDataPreparator  prep_;
    FrameBuffer           frameBuffer_;                           // for rendering to some texture
    EntityID currCameraID_ = 0;
    
    // for rendering
    ECS::RenderStatesSystem::EnttsRenderStatesData rsDataToRender_;

    LightTempData lightTempData_;

    FrameBuffer                         materialBigIconFrameBuf_;
    cvector<FrameBuffer>                materialsFrameBuffers_;   // frame buffers which are used to render materials icons (for editor's material browser)
    cvector<ID3D11ShaderResourceView*>  texturesBuf_;             // to avoid reallocation each time we use this shared buffer

    // temp for geometry buffer testing
    void BuildGeometryBuffers();
    ID3D11Buffer* pGeomVB_ = nullptr;
    ID3D11Buffer* pGeomIB_ = nullptr;
    
    // different boolean flags             
    bool isWireframeMode_ = false;             // do we render everything is the WIREFRAME mode?
    bool isCullBackMode_ = true;               // do we cull back faces?
    bool isBeginCheck_ = false;                // a variable which is used to determine if the user has clicked on the screen or not
    bool isIntersect_ = false;                 // a flag to define if we clicked on some model or not
    bool isGameMode_ = false;

    AABBShowMode aabbShowMode_ = NONE;

    int fullFogDistance_    = 0;
};

} // namespace Core
