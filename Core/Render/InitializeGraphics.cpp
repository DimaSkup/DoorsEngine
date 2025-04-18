// =================================================================================
// Filename:     InitializeGraphics.cpp
// Description:  there are functions for initialization of DirectX
//               and graphics parts of the engine;
//
// Created:      02.12.22
// =================================================================================
#include "InitializeGraphics.h"


#include <CoreCommon/Assert.h>
#include <CoreCommon/MathHelper.h>
#include <CoreCommon/StrHelper.h>
#include "Common/LIB_Exception.h"    // ECS exception

#include "../Texture/TextureTypes.h"
#include "../Mesh/MaterialMgr.h"
#include "../Model/ModelExporter.h"
#include "../Model/ModelImporter.h"
#include "../Model/ModelLoader.h"
#include "../Model/ModelMath.h"
#include "../Model/ModelMgr.h"

#include "../Engine/ProjectSaver.h"

#include "InitGraphicsHelperDataTypes.h"

//#include "../Model/SkyModel.h"

#include <filesystem>
#include <shellapi.h>


using namespace DirectX;
namespace fs = std::filesystem;

namespace Core
{

InitializeGraphics::InitializeGraphics()
{
    LogMsg("initialize graphics stuff");
}


// =================================================================================
//                                PUBLIC FUNCTIONS
// =================================================================================
bool InitializeGraphics::InitializeDirectX(
    D3DClass& d3d,
    HWND hwnd,
    const Settings& settings)
{
    // THIS FUNC initializes the DirectX stuff 
    // (device, deviceContext, swapChain, rasterizerState, viewport, etc)

    try 
    {
        bool result = d3d.Initialize(
            hwnd,
            settings.GetBool("VSYNC_ENABLED"),
            settings.GetBool("FULL_SCREEN"),
            settings.GetBool("ENABLE_4X_MSAA"),
            settings.GetFloat("NEAR_Z"),
            settings.GetFloat("FAR_Z"));         // how far we can see

        Assert::True(result, "can't initialize the Direct3D");

        // setup the rasterizer state to default params
        d3d.SetRS({ eRenderState::CULL_BACK, eRenderState::FILL_SOLID });
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize DirectX");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeScene(
    const Settings& settings,
    D3DClass& d3d,
    ECS::EntityMgr& entityMgr)
    
{
    // THIS FUNC initializes some main elements of the scene:
    // models, light sources, textures

    try
    {
       
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the scene");

        return false;
    }

    return true;
} 

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeCameras(
    D3DClass& d3d,
    Camera& gameCamera,
    Camera& editorCamera,
    DirectX::XMMATRIX& baseViewMatrix,      // is used for 2D rendering
    ECS::EntityMgr& enttMgr,
    const Settings& settings)
{
    try
    {
        const SIZE windowedSize           = d3d.GetWindowedWndSize();
        const SIZE fullscreenSize         = d3d.GetFullscreenWndSize();
        const float windowedAspectRatio   = (float)windowedSize.cx   / (float)windowedSize.cy;
        const float fullScreenAspectRatio = (float)fullscreenSize.cx / (float)fullscreenSize.cy;

        const float nearZ    = settings.GetFloat("NEAR_Z");
        const float farZ     = settings.GetFloat("FAR_Z");
        const float fovInRad = settings.GetFloat("FOV_IN_RAD");         // field of view in radians

        const XMFLOAT3 editorCamPos = { -18, 1, -15 };
        const XMFLOAT3 gameCamPos = { 0, 2, -3 };

        // 1. initialize the editor camera
        editorCamera.SetProjection(fovInRad, windowedAspectRatio, nearZ, farZ);
        editorCamera.SetPosition(editorCamPos);

        // 2. initialize the game camera
        gameCamera.SetProjection(fovInRad, fullScreenAspectRatio, nearZ, farZ);
        gameCamera.SetPosition(gameCamPos);

        // initialize view matrices of the cameras
        editorCamera.UpdateViewMatrix();
        gameCamera.UpdateViewMatrix();

        // initialize a base view matrix with the camera
        // for 2D user interface rendering (in GAME mode)
        baseViewMatrix = gameCamera.View();

        editorCamera.SetFreeCamera(true);
        gameCamera.SetFreeCamera(true);

        const float sensitivity  = settings.GetFloat("CAMERA_SENSITIVITY"); // camera rotation speed
        const float camWalkSpeed = settings.GetFloat("CAMERA_WALK_SPEED");
        const float camRunSpeed  = settings.GetFloat("CAMERA_RUN_SPEED");

        editorCamera.SetWalkSpeed(camWalkSpeed);
        editorCamera.SetRunSpeed(camRunSpeed);
        editorCamera.SetSensitiviry(sensitivity);

        gameCamera.SetWalkSpeed(camWalkSpeed);
        gameCamera.SetRunSpeed(camRunSpeed);
        gameCamera.SetSensitiviry(sensitivity);

        // create and setup an editor camera entity
        EntityID editorCamID = enttMgr.CreateEntity();
        enttMgr.AddTransformComponent(editorCamID, editorCamPos);
        enttMgr.AddCameraComponent(editorCamID, editorCamera.View(), editorCamera.Proj());
        enttMgr.AddNameComponent(editorCamID, "editor_camera");

        // create and setup an editor camera entity
        EntityID gameCamID = enttMgr.CreateEntity();
        enttMgr.AddTransformComponent(gameCamID, gameCamPos);
        enttMgr.AddCameraComponent(gameCamID, gameCamera.View(), gameCamera.Proj());
        enttMgr.AddNameComponent(gameCamID, "game_camera");
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize the cameras objects");
        return false;
    }

    return true;
}

} // namespace Core
