// ====================================================================================
// Filename:     SystemState.h
// Description:  contains the information about the current state of 
//               the engine and its parts; we use this information
//               for camera control, for the debug output (onto the screen), etc.
// Revising:     25.11.22
// ====================================================================================
#pragma once

#include <types.h>
#include <DirectXMath.h>


namespace Core
{

enum eRenderTimingType
{
    RND_TIME_RESET,
    RND_TIME_DEPTH_PREPASS,
    RND_TIME_GRASS,
    RND_TIME_MASKED,
    RND_TIME_OPAQUE,
    RND_TIME_SKINNED_MODELS,
    RND_TIME_TERRAIN,
    RND_TIME_SKY,
    RND_TIME_SKY_PLANE,
    RND_TIME_BLENDED,
    RND_TIME_TRANSPARENT,
    RND_TIME_PARTICLE,
    RND_TIME_WEAPON,
    RND_TIME_DBG_SHAPES,
    RND_TIME_POST_FX,        // duration of the post-effects computation
    RND_TIME_3D_SCENE,       // duration of the whole 3d scene rendering
    RND_TIME_UI,
    RND_TIME_FULL_FRAME,     // duration time of the whole rendering process


    MAX_RND_TIMINGS
};

//---------------------------------------------------------

struct SystemState
{
    bool isGameMode = false;
    bool isEditorMode = true;                // to define if we want to render the engine's GUI onto the screen
    bool isShowDbgInfo = false;              // show/hide debug text info in the game mode
    bool intersect = false;                  // the flag to define if we clicked on some model or not
    bool collectGpuMetrics = false;          // do we want to gather GPU metrics or not?

    int mouseX = 0;                          // the mouse cursor X position in the window
    int mouseY = 0;                          // the mouse cursor Y position in the window
    int fps = 0;                             // framerate
    int cpu = 0;                             // cpu performance
    int wndWidth_  = 800;                    // current width of the main window 
    int wndHeight_ = 600;                    // current height of the main window
                
    uint32 pickedEnttID_            = 0;     // currently chosen entity (its ID)
   
    //-----------------------------------------------------
    // this frame render stats
    //-----------------------------------------------------
    uint32 numDrawnTerrainPatches   = 0;        // how many terrain's patched did we render for this frame?
    uint32 numCulledTerrainPatches  = 0;        // how many patches was culled during frustum test
    uint32 numVisiblePointLights    = 0;
    uint32 numVisibleSpotlights     = 0;

    uint32 numDrawnAllVerts = 0;                // the number of all rendered vertices
    uint32 numDrawnAllTris = 0;                 // the number of all rendered triangles

    uint32 numDrawnEnttsInstances   = 0;        // the number of rendered entities instances
    uint32 numDrawCallsEnttsInstances = 0;      // the number of draw calls for all entities

    float deltaTime = 0.0f;                     // seconds per last frame
    float frameTime = 0.0f;                     // ms per last frame

    float updateTime        = 0;                // duration time of the whole update process
    float updateTimeAvg     = 0;                // (ms) average time of update for last 0.5 sec
    float updateTimeGame    = 0;                // duration (ms) of game updating
    float updateTimeEngine  = 0;                // duration (ms) of engine updating

    float msRenderTimings   [MAX_RND_TIMINGS]{ 0.0f };
    float msRenderTimingsAvg[MAX_RND_TIMINGS]{ 0.0f };

    //-----------------------------------------------------

    DirectX::XMFLOAT3 cameraPos;             // the current position of the currently main camera
    DirectX::XMFLOAT3 cameraDir;             // the current rotation of the currently main camera
    DirectX::XMMATRIX cameraView;            // view matrix of the currently main camera
    DirectX::XMMATRIX cameraProj;            // projection matrix of the currently main camera
};

} // namespace Core
