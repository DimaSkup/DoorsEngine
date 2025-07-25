#include "Mouse.h"

void Mouse::OnLeftPressed(int x, int y)
{
	leftIsDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::LPress, x, y));
}

void Mouse::OnLeftReleased(int x, int y)
{
	leftIsDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::LRelease, x, y));
}

void Mouse::OnRightPressed(int x, int y)
{
	rightIsDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RPress, x, y));
}

void Mouse::OnRightReleased(int x, int y)
{
	rightIsDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RRelease, x, y));
}

void Mouse::OnMiddlePressed(int x, int y)
{
	mbuttonDown_ = true;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::MPress, x, y));
}

void Mouse::OnMiddleReleased(int x, int y)
{
	mbuttonDown_ = false;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::MRelease, x, y));
}

void Mouse::OnWheelUp(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelUp, x, y));
}

void Mouse::OnWheelDown(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelDown, x, y));
}

void Mouse::OnMouseMove(int x, int y)
{
	x_ = x;
	y_ = y;
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::Move, x, y));
}

// handles the relative changes of the mouse position
void Mouse::OnMouseMoveRaw(int x, int y)
{
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::RAW_MOVE, x, y));
}

void Mouse::OnLeftDoubleClick()
{
	// handle left button double clicking
	eventBuffer_.push(MouseEvent(MouseEvent::EventType::LeftDoubleClick, 0, 0));
}

MouseEvent Mouse::ReadEvent()
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

