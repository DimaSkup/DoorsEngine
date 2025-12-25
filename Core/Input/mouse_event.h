#pragma once


struct MousePoint
{
    int x;
    int y;
};

class MouseEvent
{
public:
    enum EventType
    {
        LPress,
        LRelease,
        RPress,
        RRelease,
        MPress,
        MRelease,
        WheelUp,
        WheelDown,
        Move,
        RAW_MOVE,
        LeftDoubleClick,
        Invalid
    };

private:
    EventType type;
    int x;
    int y;

public:
    MouseEvent() :
        type(EventType::Invalid), x(0), y(0)
    {}

    MouseEvent(EventType type, int x, int y) :
        type(type), x(x), y(y)
    {}

    inline bool IsValid()           const { return type != EventType::Invalid; }
    inline EventType GetEventType() const { return type; }

    inline MousePoint GetPos()      const { return { x, y }; }
    inline int GetPosX()            const { return x; }
    inline int GetPosY()            const { return y; }
};
