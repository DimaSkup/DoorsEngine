////////////////////////////////////////////////////////////////////
// Filename:      KeyboardClass.cpp
// Revising:      06.11.22
////////////////////////////////////////////////////////////////////
#include "KeyboardClass.h"


KeyboardClass::KeyboardClass()
{
	for (size_t i = 0; i < 256; i++)
		keyStates_[i] = false;      // initialize all key states to off (false)
}

KeyboardEvent KeyboardClass::ReadKey()
{
	if (keyBuffer_.empty()) // if no keys to be read
	{
		return KeyboardEvent();   // return an empty keyboard event
	}
	else
	{
		KeyboardEvent e = keyBuffer_.front();  // get the first Keyboard Event from the queue
		keyBuffer_.pop();  // remove the first item from the queue
		return e; // return a keyboard event
	}
}

unsigned char KeyboardClass::ReadChar()
{
	if (charBuffer_.empty()) // if no characters to be read
	{
		return 0u; // return 0 (NULL char == '\0')
	}
	else
	{
		unsigned char e = charBuffer_.front();  // get the first character from the queue
		charBuffer_.pop(); // remove the first item from the queue
		return e; // return a character
	}
}



// -------------------------------- EVENTS ------------------------------------- //

void KeyboardClass::OnKeyPressed(const unsigned char key)
{
	keyStates_[key] = true;
	keyBuffer_.push(KeyboardEvent(KeyboardEvent::EventType::Press, key));
}

void KeyboardClass::OnKeyReleased(const unsigned char keycode)
{
	keyStates_[keycode] = false;
	keyBuffer_.push(KeyboardEvent(KeyboardEvent::EventType::Release, keycode));
}