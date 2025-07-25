////////////////////////////////////////////////////////////////////
// Filename:      Keyboard.cpp
// Revising:      06.11.22
////////////////////////////////////////////////////////////////////
#include "Keyboard.h"


//---------------------------------------------------------
// Desc:   initialize all key states to off (false)
//---------------------------------------------------------
Keyboard::Keyboard()
{
    memset(keyStates_, 0, sizeof(bool) * 256);  
}

//---------------------------------------------------------
// Desc:   store the keys which a currently pressed so in the next frame 
//         we will be able to prevent repeated handling of some events
//---------------------------------------------------------
void Keyboard::Update()
{
    keysPressedBefore_ = pressedKeys_;
}

//---------------------------------------------------------
// check if input key was pressed in the previous frame
//---------------------------------------------------------
bool Keyboard::WasPressedBefore(const unsigned char keycode)
{
    for (auto it : keysPressedBefore_)
    {
        if (it == keycode)
            return true;
    }

    return false;
}

//---------------------------------------------------------
// Desc::   put the key into the list of pressed keys if it hasn't been pressed before
//---------------------------------------------------------
void Keyboard::OnKeyPressed(const unsigned char keycode)
{
    keyStates_[keycode] = true;
    
    for (auto it : pressedKeys_)
    {
        if (it == keycode)
            return;
    }

    pressedKeys_.push_back(eKeyCodes(keycode));
}

//---------------------------------------------------------
// Desc:   remove the input keycode from the list of pressed keys
//---------------------------------------------------------
void Keyboard::OnKeyReleased(const unsigned char keycode)
{
    keyStates_[keycode] = false;

    for (auto it = pressedKeys_.begin(); it != pressedKeys_.end(); ++it)
    {
        if (*it == keycode)
        {
            pressedKeys_.erase(it);

            KeyboardEvent e(KeyboardEvent::EventType::Release, keycode);
            eventsReleased_.push_back(e);
            return;
        }
    }
}

//---------------------------------------------------------
// Desc:   return a code of released key from the list;
//         and remove this first item from the list;
//         if we haven't any released keys (events) we just return 0;
//---------------------------------------------------------
int Keyboard::ReadReleasedKey()
{
    if (eventsReleased_.empty())
    {
        return 0;
    }
    else
    {
        int code = eventsReleased_.begin()->GetKeyCode();
        eventsReleased_.pop_front();
        return code;
    }	
}
