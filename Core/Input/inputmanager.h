////////////////////////////////////////////////////////////////////
// Filename:     inputmanager.h
// Description:  contains methods for calling handlers for input events
//
// Revising:     01.06.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "MouseClass.h"
#include "KeyboardClass.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


class InputManager
{
public:
	LRESULT HandleKeyboardMessage(KeyboardClass & keyboard, const UINT &message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMouseMessage   (MouseClass & mouse,       const UINT &message, WPARAM wParam, LPARAM lParam);
};


