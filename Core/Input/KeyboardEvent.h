////////////////////////////////////////////////////////////////////
// Filename:      KeyboardEvent.h
// Description:   this class is responsible for containing state 
//                of some particular keyboard event
// Revising:      05.10.22
////////////////////////////////////////////////////////////////////
#pragma once


//////////////////////////////////
// Class name: KeyboardEvent
//////////////////////////////////
class KeyboardEvent
{
public:
	enum EventType
	{
		Press,    // pressing of key
		Release,  // releasing of key
		Invalid   // event is invalid
	};

	KeyboardEvent();  // generates an invalid event
	KeyboardEvent(const EventType type, const unsigned char key);  // generates some particular event by passed type and key

	inline bool IsPress()   const { return type_ == EventType::Press; }
	inline bool IsRelease() const { return type_ == EventType::Release; }
	inline bool IsValid()   const { return type_ != EventType::Invalid; }
	inline unsigned char GetKeyCode() const { return key_; }

private:
	EventType type_;
	unsigned char key_;
};