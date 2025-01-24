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
		Invalid
	};

private:
	EventType type;
	int x;
	int y;

public:
	MouseEvent();
	MouseEvent(const EventType type, const int x, const int y);

	inline bool IsValid() const { return type != EventType::Invalid; }
	inline EventType GetType() const { return type; }
	inline MousePoint GetPos() const { return { x, y }; }

	inline int GetPosX() const { return x; }
	inline int GetPosY() const { return y; }
};