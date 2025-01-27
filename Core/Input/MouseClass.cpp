#include "MouseClass.h"

void MouseClass::OnLeftPressed(int x, int y)
{
	leftIsDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::LPress, x, y));
}

void MouseClass::OnLeftReleased(int x, int y)
{
	leftIsDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::LRelease, x, y));
}

void MouseClass::OnRightPressed(int x, int y)
{
	rightIsDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RPress, x, y));
}

void MouseClass::OnRightReleased(int x, int y)
{
	rightIsDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RRelease, x, y));
}

void MouseClass::OnMiddlePressed(int x, int y)
{
	mbuttonDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::MPress, x, y));
}

void MouseClass::OnMiddleReleased(int x, int y)
{
	mbuttonDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::MRelease, x, y));
}

void MouseClass::OnWheelUp(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelUp, x, y));
}

void MouseClass::OnWheelDown(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelDown, x, y));
}

void MouseClass::OnMouseMove(int x, int y)
{
	x_ = x;
	y_ = y;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::Move, x, y));
}

// handles the relative changes of the mouse position
void MouseClass::OnMouseMoveRaw(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RAW_MOVE, x, y));
}

MouseEvent MouseClass::ReadEvent()
{
	if (!eventBuffer_.empty())
	{
		MouseEvent e = eventBuffer_.front(); // get first event from buffer
		eventBuffer_.pop();                  // remove first event from buffer
		return e;
	}
	else
	{
		return MouseEvent();
	}

}

