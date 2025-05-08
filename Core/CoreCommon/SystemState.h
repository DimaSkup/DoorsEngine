// ====================================================================================
// Filename:     SystemState.h
// Description:  contains the information about the current state of 
//               the engine and its parts; we use this information
//               for camera control, for the debug output (onto the screen), etc.
// Revising:     25.11.22
// ====================================================================================
#pragma once

#include <DirectXMath.h>
#include <CoreCommon/Types.h>

namespace Core
{

class SystemState
{
public:
	bool isEditorMode = true;                // to define if we want to render the engine's GUI onto the screen
	bool isShowDbgInfo = false;              // show/hide debug text info in the game mode
	bool intersect = false;                  // the flag to define if we clicked on some model or not
	//bool needComputeFrustumCulling_ = true;  // the flag to define if we need to compute frustum culling for this frame (if camera doesn't move/rotate we have no need to compute it each frame)
	int mouseX = 0;                          // the mouse cursor X position in the window
	int mouseY = 0;                          // the mouse cursor Y position in the window
	int fps = 0;                             // framerate
	int cpu = 0;                             // cpu performance
	int wndWidth_  = 800;                    // current width of the main window 
	int wndHeight_ = 600;                    // current height of the main window
				
	u32 visibleVerticesCount = 0;            // the number of rendered vertices for this frame
	u32 visibleObjectsCount = 0;             // the number of rendered models for this frame
	u32 numVisiblePointLights = 0;
	u32 cellsDrawn = 0;                      // the number of rendered terrain cells
	u32 cellsCulled = 0;                     // the number of culled terrain cells
	u32 pickedEnttID_ = 0;                   // currently chosen entity (its ID)

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
