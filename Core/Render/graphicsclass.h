// =================================================================================
// Filename:     graphicsclass.h
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


// mesh, models, game objects and related stuff
//#include "../Model/ModelsCreator.h"
#include "RenderDataPreparator.h"
//#include "../Model/ModelStorage.h"

// physics / interaction with user
//#include "../Physics/IntersectionWithGameObjects.h"

// render stuff
#include "CRender.h"
#include "InitializeGraphics.h"        // for initialization of the graphics
#include "FrameBuffer.h"      // for rendering to some particular texture

// Entity-Component-System
#include "Entity/EntityMgr.h"

#include <string>
#include <map>
#include <memory>
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

    void ComputeFrustumCulling(SystemState& sysState, ECS::EntityMgr* pEnttMgr);
    void ComputeFrustumCullingOfLightSources(SystemState& sysState, ECS::EntityMgr* pEnttMgr);
    void ClearRenderingDataBeforeFrame(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void Render3D(ECS::EntityMgr* pEnttMgr, Render::CRender* pRender);
    void RenderModel(BasicModel& model, const DirectX::XMMATRIX& world);

    // ----------------------------------

    // change render states using keyboard
    void ChangeModelFillMode();   
    void ChangeCullMode();

    inline void SetGameMode(bool enableGameMode) { isGameMode_ = enableGameMode; }
    inline void SetCurrentCamera(const EntityID cameraID) { currCameraID_ = cameraID; }
    inline void SetAABBShowMode(const AABBShowMode mode) { aabbShowMode_ = mode; }

    // ---------------------------------------
    // INLINE GETTERS/SETTERS

    inline D3DClass&       GetD3DClass()                      { return d3d_; }

    // matrices getters
    inline const DirectX::XMMATRIX& GetWorldMatrix()    const { return worldMatrix_; }
    inline const DirectX::XMMATRIX& GetBaseViewMatrix() const { return baseViewMatrix_; }
    inline const DirectX::XMMATRIX& GetOrthoMatrix()    const { return orthoMatrix_; }

    inline void SetBaseViewMatrix(const DirectX::XMMATRIX& view) { baseViewMatrix_ = view; }

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

    // ------------------------------------------

    // render bounding boxes of models/meshes/light_sources/etc.
    void RenderBoundingLineBoxes(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);
    void RenderBoundingLineSpheres();
    void RenderSkyDome(Render::CRender* pRender, ECS::EntityMgr* pEnttMgr);

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
    ECS::EntityID currCameraID_ = 0;
    
    // for rendering
    ECS::RenderStatesSystem::EnttsRenderStatesData rsDataToRender_;


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
    int fullFogDistanceSqr_ = 0;
};

} // namespace Core
