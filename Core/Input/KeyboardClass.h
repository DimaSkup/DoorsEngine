////////////////////////////////////////////////////////////////////
// Filename:      KeyboardClass.h
// Description:   this class is responsible containing and 
//                handling keyboard events, generating of 
//                relevant events, containing the queue of 
//                the keyboard events.
// Revising:      05.10.22
////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include "KeyboardEvent.h"
#include <queue>


//////////////////////////////////
// Class name: KeyboardClass
//////////////////////////////////
class KeyboardClass
{
public:
	KeyboardClass();

	
	KeyboardEvent ReadKey();
	unsigned char ReadChar();

	inline bool KeyIsPressed(const unsigned char keycode) const { return keyStates_[keycode]; }
	inline bool KeyBufferIsEmpty() const { return keyBuffer_.empty(); }
	inline bool CharBufferIsEmpty() const { return charBuffer_.empty(); }

	void OnKeyPressed(const unsigned char key);
	void OnKeyReleased(const unsigned char key);

	inline void OnChar(const unsigned char key) { charBuffer_.push(key); }

	inline void EnableAutoRepeatKeys() { autoRepeatKeys_ = true; }
	inline void DisableAutoRepeatKeys() { autoRepeatKeys_ = false; }
	inline void EnableAutoRepeatChars() { autoRepeatChars_ = true; }
	inline void DisableAutoRepeatChars() { autoRepeatChars_ = false; }
	inline bool IsKeysAutoRepeat() const { return autoRepeatKeys_; }
	inline bool IsCharsAutoRepeat() const { return autoRepeatChars_; }

private:
	bool autoRepeatKeys_ = false;
	bool autoRepeatChars_ = false;
	bool keyStates_[256];     // an array of all the keys

	std::queue<KeyboardEvent> keyBuffer_;
	std::queue<unsigned char> charBuffer_;
};