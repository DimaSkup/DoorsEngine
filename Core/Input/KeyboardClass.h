// =================================================================================
// Filename:      KeyboardClass.h
// Description:   this class is responsible containing and 
//                handling keyboard events, containing the list of 
//                the pressed keys.
// Revising:      05.10.22
// =================================================================================
#pragma once

#include "inputcodes.h"
#include "KeyboardEvent.h"
#include <list>

class KeyboardClass
{
public:
	KeyboardClass();

	void Update();
	bool WasPressedBefore(const unsigned char keycode);
	
	inline bool IsPressed(const unsigned char keycode) const { return keyStates_[keycode]; }
	inline bool IsAnyPressed()                         const { return !pressedKeys_.empty(); }
	inline bool HasReleasedEvents()                    const { return !eventsReleased_.empty(); }

	//unsigned char ReadChar();
	//inline bool CharBufferIsEmpty()       const { return charBuffer_.empty(); }
	//inline void OnChar(const unsigned char key) { charBuffer_.push(key); }

	const std::list<eKeyCodes>& GetPressedKeysList() const { return pressedKeys_; }

	void OnKeyPressed (const unsigned char key);
	void OnKeyReleased(const unsigned char key);

	int ReadReleasedKey();

	inline void EnableAutoRepeatKeys()                       { autoRepeatKeys_ = true; }
	inline void DisableAutoRepeatKeys()                      { autoRepeatKeys_ = false; }
	inline void EnableAutoRepeatChars()                      { autoRepeatChars_ = true; }
	inline void DisableAutoRepeatChars()                     { autoRepeatChars_ = false; }
	inline bool IsKeysAutoRepeat()                     const { return autoRepeatKeys_; }
	inline bool IsCharsAutoRepeat()                    const { return autoRepeatChars_; }

private:
	bool autoRepeatKeys_  = false;
	bool autoRepeatChars_ = false;
	bool keyStates_[256];                           // an array of all the keys

	std::list<eKeyCodes> pressedKeys_;              // a list of currently pressed keys
	std::list<eKeyCodes> keysPressedBefore_;        // a list of keys which were pressed during the previous frame

	// a list of events which are expected to be handled
	// (NOTE: we used it only for handling of released keys
	// since pressed keys are stores in the pressedKeys_ list)
	std::list<KeyboardEvent> eventsReleased_;    

	//std::queue<unsigned char> charBuffer_;
};