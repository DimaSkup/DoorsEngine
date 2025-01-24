// =================================================================================
// Filename:      Camera.h
// Description:   is needed to calculate and maintain  the position of 
//                the ENGINE'S EDITOR camera. Handles different 
//                movement changes.
//
// Revising:      25.11.22
// =================================================================================
#pragma once

#include "BasicCamera.h"


class Camera : public BasicCamera
{
public:
	Camera();
	~Camera();

	// handles the camera changes accodring to the input from the keyboard
	void HandleKeyboardEvents(const float deltaTime); 

	// handles the changing of the camera rotation
	void HandleMouseMovement(
		const int mouseX_delta, 
		const int mouseY_delta,
		const float deltaTime);          
};
