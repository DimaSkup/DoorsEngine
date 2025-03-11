////////////////////////////////////////////////////////////////////
// Filename:     inputmanager.h
// Description:  contains methods for calling handlers for input events
//
// Revising:     01.06.22
////////////////////////////////////////////////////////////////////
#pragma once

#include <CoreCommon/Log.h>
#include "MouseClass.h"
#include "KeyboardClass.h"

class InputManager
{
public:
	LRESULT HandleKeyboardMessage(KeyboardClass & keyboard, const UINT &message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMouseMessage(MouseClass & mouse, const UINT &message, WPARAM wParam, LPARAM lParam);
};


