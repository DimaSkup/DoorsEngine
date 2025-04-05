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

// camera
//#include "../Camera/BasicCamera.h"
#include "../Camera/Camera.h"

// render stuff
#include "Render.h"
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

class GraphicsClass final
{
public:
    enum AABBShowMode
    {
        NONE,    // doesn't show any AABB line boxes
        MODEL,   // render AABB line box around the whole model
        MESH,    // render AABB line box around each mesh of model
    };

public:
    GraphicsClass();
    ~GraphicsClass();

    // restrict a copying of this class instance
    GraphicsClass(const GraphicsClass& obj) = delete;
    GraphicsClass& operator=(const GraphicsClass& obj) = delete;

    // main functions
    bool Initialize(HWND hwnd, SystemState& sysState, const Settings& settings);
    void Shutdown();
    void Update(SystemState& sysState, const float dt, const float totalGameTime);

    // ------------------------------------
    // render related methods

    void ComputeFrustumCulling(SystemState& sysState);
    void ComputeFrustumCullingOfLightSources(SystemState& sysState);
    void ClearRenderingDataBeforeFrame();
    void Render3D();
    void RenderModel(BasicModel& model, const DirectX::XMMATRIX& world);

    // ----------------------------------

    // change render states using keyboard
    void ChangeModelFillMode();   
    void ChangeCullMode();
    void SwitchGameMode(bool enableGameMode);

    inline void SetAABBShowMode(const AABBShowMode mode) { aabbShowMode_ = mode; }

    // ---------------------------------------
    // INLINE GETTERS

    inline D3DClass&       GetD3DClass()                      { return d3d_; }
    inline Camera&         GetEditorCamera()                  { return editorCamera_; }
    inline Camera&         GetGameCamera()                    { return gameCamera_; }
    inline Render::Render& GetRender()                        { return render_; }

    // matrices getters
    inline const DirectX::XMMATRIX& GetWorldMatrix()    const { return worldMatrix_; }
    inline const DirectX::XMMATRIX& GetBaseViewMatrix() const { return baseViewMatrix_; }
    inline const DirectX::XMMATRIX& GetOrthoMatrix()    const { return orthoMatrix_; }

    // memory allocation (because we have some XM-data structures)
    void* operator new(std::size_t count);                              // a replaceable allocation function
    void* operator new(std::size_t count, const std::nothrow_t & tag);  // a replaceable non-throwing allocation function
    void* operator new(std::size_t count, void* ptr);                   // a non-allocating placement allocation function
    void operator delete(void* ptr);

    // check if we have any entity by these coords of the screen
    int TestEnttSelection(const int sx, const int sy);

    void UpdateCameraEntity(
        const std::string& cameraEnttName,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj);

private:
    bool InitHelper(HWND hwnd, SystemState& systemState, const Settings& settings);
    void UpdateHelper(SystemState& sysState, const float deltaTime, const float gameTime);
    void RenderHelper();

    // private updating API
    void UpdateShadersDataPerFrame();

    // ------------------------------------------
    // rendering data prepararion stage API

    void PrepBasicInstancesForRender(const EntityID* ids, const size numEntts);
    void PrepAlphaClippedInstancesForRender(const EntityID* ids, const size numEntts);
    void PrepBlendedInstancesForRender(const EntityID* ids, const size numEntts);

    // ------------------------------------------

    void RenderEnttsDefault();
    void RenderEnttsAlphaClipCullNone();
    void RenderEnttsBlended();
    void RenderBillboards();

    // ------------------------------------------

    // render bounding boxes of models/meshes/light_sources/etc.
    void RenderBoundingLineBoxes();
    void RenderBoundingLineSpheres();
    void RenderSkyDome();

    void UpdateInstanceBuffAndRenderInstances(
        ID3D11DeviceContext* pDeviceContext,
        const Render::ShaderTypes type,
        const Render::InstBuffData& instanceBuffData,
        const std::vector<Render::Instance>& instances);

    // ------------------------------------------

    void SetupLightsForFrame(
        const ECS::LightSystem& lightSys,
        Render::PerFrameData& perFrameData);

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

    ECS::EntityMgr        entityMgr_;


    std::vector<DirectX::BoundingFrustum> frustums_;

    D3DClass              d3d_;
    Render::Render        render_;                                // rendering module
    RenderDataPreparator  prep_;
    
    Camera*               pCurrCamera_ = nullptr;                 // a currently chosen camera
    Camera                gameCamera_;                            // for fullscreen mode
    Camera                editorCamera_;                          // editor's main camera

    FrameBuffer           frameBuffer_;                           // for rendering to some texture
    
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
