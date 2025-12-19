////////////////////////////////////////////////////////////////////
// Filename:     inputmanager.h
// Description:  contains methods for calling handlers for input events
//
// Revising:     01.06.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "Mouse.h"
#include "Keyboard.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


class InputManager
{
public:
	LRESULT HandleKeyboardMessage(Keyboard & keyboard, const UINT &message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMouseMessage   (Mouse & mouse,       const UINT &message, WPARAM wParam, LPARAM lParam);
};


