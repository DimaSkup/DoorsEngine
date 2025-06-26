#pragma once
#include <Types.h>

namespace ECS
{

enum eEventType
{
    EVENT_INVALID              = (1 << 0),

    // translate/rotate/scale entity
    EVENT_TRANSLATE            = (1 << 1),
    EVENT_ROTATE               = (1 << 2),
    EVENT_SCALE                = (1 << 3),

    EVENT_PLAYER_SHOOT         = (1 << 4),        // example: shoot with a gun or hit with a sword
    EVENT_PLAYER_RUN           = (1 << 5),        // set the player is running or not
    EVENT_PLAYER_JUMP          = (1 << 6),
    EVENT_PLAYER_MOVE_FORWARD  = (1 << 7),
    EVENT_PLAYER_MOVE_BACK     = (1 << 8),
    EVENT_PLAYER_MOVE_RIGHT    = (1 << 9),
    EVENT_PLAYER_MOVE_LEFT     = (1 << 10),
    EVENT_PLAYER_MOVE_UP       = (1 << 11),       // when free fly camera mode
    EVENT_PLAYER_MOVE_DOWN     = (1 << 12),       // when free fly camera mode
};

// =================================================================================
// Basic event
// =================================================================================
struct Event
{
    eEventType type;
    EntityID enttID;
    float x;
    float y;
    float z;
    float w;
};

// =================================================================================
// Concrete events
// =================================================================================
struct EventTranslate : public Event
{
    EventTranslate(const float _x, const float _y, const float _z)
    {
        type = EVENT_TRANSLATE;
        x = _x;
        y = _y;
        z = _z;
    }
};

struct EventPlayerRun : public Event
{
    EventPlayerRun(const float isRun)
    {
        type = EVENT_PLAYER_RUN;
        x = isRun;
    }
};

struct EventPlayerMove : public Event
{
    EventPlayerMove(const eEventType& eventType)
    {
        constexpr uint32 moveFlags =
            EVENT_PLAYER_JUMP |
            EVENT_PLAYER_MOVE_FORWARD |
            EVENT_PLAYER_MOVE_BACK |
            EVENT_PLAYER_MOVE_RIGHT |
            EVENT_PLAYER_MOVE_LEFT |
            EVENT_PLAYER_MOVE_UP |
            EVENT_PLAYER_MOVE_DOWN;

        // if we have some move flag we set type to input eventType
        type = (moveFlags & eventType) ? eventType : EVENT_INVALID;
    }
};

}

