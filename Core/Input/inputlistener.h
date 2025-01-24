///////////////////////////////////////////////////////////////////////////////
// Filename:     inputlistener.h
// Description:  contains structures for input data and abstract class 
//               for inherited classes which will contain handlers for input
//
// Revising:     01.06.22
///////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include "inputcodes.h"


//////////////////////////////////
// EVENTS STRUCTURES
//////////////////////////////////

// contains the current mouse position
struct MouseMoveEvent
{
	MouseMoveEvent(int posX, int posY) :
		x(posX), y(posY) {}

	int x;
	int y;
};

// contains the mouse button code 
struct MouseClickEvent : public MouseMoveEvent
{
	MouseClickEvent(const eMouseKeyCodes& btn, int x, int y) :
		MouseMoveEvent(x, y), code(btn) {}

	const eMouseKeyCodes code;  // the code of the mouse button
};

// contains the direction of mouse wheel rotation
struct MouseWheelEvent : public MouseMoveEvent
{
	MouseWheelEvent(short nWheel, int x, int y) :
		MouseMoveEvent(x, y), wheel(nWheel) {}

	short wheel; // the direction code of mouse wheel rotation
};

// contains the keybutton code and its Unicode code
struct KeyButtonEvent
{
	KeyButtonEvent(const eKeyCodes& kc, wchar_t wch) :
		code(kc), wchar(wch) {}

	const eKeyCodes code;
	const wchar_t wchar;
};


//////////////////////////////////
// Class name: InputListener
//////////////////////////////////
class InputListener
{
public:
	// handler for mouse moving
	virtual void MouseMove(const MouseMoveEvent& arg) {}

	// handler for mouse wheel rotation
	virtual bool MouseWheel(const MouseWheelEvent& arg)    { return false; }

	// handlers for mouse buttons events
	virtual bool MousePressed(const MouseClickEvent& arg)  { return false; }
	virtual bool MouseReleased(const MouseClickEvent& arg) { return false; }

	// handlers for keyboard buttons events
	virtual bool KeyPressed(const KeyButtonEvent& arg)     { return false; }
	virtual bool KeyReleased(const KeyButtonEvent& arg)    { return false; }
};