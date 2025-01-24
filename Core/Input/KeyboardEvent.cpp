////////////////////////////////////////////////////////////////////
// Filename: KeyboardEvent.cpp
// Revising: 05.10.22
////////////////////////////////////////////////////////////////////
#include "KeyboardEvent.h"

// generates an invalid keyboard event
KeyboardEvent::KeyboardEvent()
{
	type_ = EventType::Invalid;
	key_ = 0u;
}

// generates some particular event by passed type and key
KeyboardEvent::KeyboardEvent(const EventType type, const unsigned char key)
{
	type_ = type;
	key_ = key;
}