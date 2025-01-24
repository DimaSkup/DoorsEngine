#include "MouseEvent.h"

MouseEvent::MouseEvent()
	:
	type(EventType::Invalid),
	x(0),
	y(0)
{}

MouseEvent::MouseEvent(EventType type, int x, int y)
	:
	type(type),
	x(x),
	y(y)
{}