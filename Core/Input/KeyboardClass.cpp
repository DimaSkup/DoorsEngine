////////////////////////////////////////////////////////////////////
// Filename:      KeyboardClass.cpp
// Revising:      06.11.22
////////////////////////////////////////////////////////////////////
#include "KeyboardClass.h"


KeyboardClass::KeyboardClass()
{
	// initialize all key states to off (false)
	memset(keyStates_, 0, sizeof(bool) * 256);  
}

///////////////////////////////////////////////////////////

void KeyboardClass::Update()
{
	// store the keys which a currently pressed so in the next frame 
	// we will be able to prevent repeated handling of some events
	keysPressedBefore_ = pressedKeys_;
}

///////////////////////////////////////////////////////////

bool KeyboardClass::WasPressedBefore(const unsigned char keycode)
{
	// check if input key was pressed in the previous frame

	for (auto it = keysPressedBefore_.begin(); it != keysPressedBefore_.end(); ++it)
	{
		if (*it == keycode)
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////

#if 0

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
#endif

///////////////////////////////////////////////////////////

void KeyboardClass::OnKeyPressed(const unsigned char keycode)
{
	keyStates_[keycode] = true;

	// put the key into the list of pressed keys if it hasn't been pressed before
	for (auto it = pressedKeys_.begin(); it != pressedKeys_.end(); ++it)
	{
		if (*it == keycode)
			return;
	}

	pressedKeys_.push_back(eKeyCodes(keycode));
}

///////////////////////////////////////////////////////////

void KeyboardClass::OnKeyReleased(const unsigned char keycode)
{
	keyStates_[keycode] = false;

	// remove the input keycode from the list of pressed keys
	for (auto it = pressedKeys_.begin(); it != pressedKeys_.end(); ++it)
	{
		if (*it == keycode)
		{
			pressedKeys_.erase(it);
			return;
		}
	}
}