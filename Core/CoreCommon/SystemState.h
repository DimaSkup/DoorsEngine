// ====================================================================================
// Filename:     SystemState.h
// Description:  contains the information about the current state of 
//               the engine and its parts; we use this information
//               for camera control, for the debug output (onto the screen), etc.
// Revising:     25.11.22
// ====================================================================================
#pragma once

#include <DirectXMath.h>


namespace Core
{

class SystemState
{
public:
    bool isGameMode = false;
	bool isEditorMode = true;                // to define if we want to render the engine's GUI onto the screen
	bool isShowDbgInfo = false;              // show/hide debug text info in the game mode
	bool intersect = false;                  // the flag to define if we clicked on some model or not

	int mouseX = 0;                          // the mouse cursor X position in the window
	int mouseY = 0;                          // the mouse cursor Y position in the window
	int fps = 0;                             // framerate
	int cpu = 0;                             // cpu performance
	int wndWidth_  = 800;                    // current width of the main window 
	int wndHeight_ = 600;                    // current height of the main window
				
	uint32 pickedEnttID_            = 0;     // currently chosen entity (its ID)
    uint32 numDrawnTerrainPatches   = 0;     // how many terrain's patched did we render for this frame?
    uint32 numCulledTerrainPatches  = 0;     // how many patches was culled during frustum test

    uint32 numDrawnVertices         = 0;     // the number of rendered vertices for this frame
    uint32 numDrawnInstances        = 0;     // rendered entities instances
    uint32 numDrawCallsForInstances = 0;

    float deltaTime = 0.0f;                  // seconds per last frame
	float frameTime = 0.0f;                  // ms per last frame
    float updateTime = 0.0f;                 // duration time of the whole update process
    float renderTime = 0.0f;                 // duration time of the whole rendering process
	DirectX::XMFLOAT3 cameraPos;             // the current position of the currently main camera
	DirectX::XMFLOAT3 cameraDir;             // the current rotation of the currently main camera
	DirectX::XMMATRIX cameraView;            // view matrix of the currently main camera
	DirectX::XMMATRIX cameraProj;            // projection matrix of the currently main camera
};

} // namespace Core
