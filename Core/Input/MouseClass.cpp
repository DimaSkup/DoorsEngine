#include "MouseClass.h"

void MouseClass::OnLeftPressed(int x, int y)
{
	this->leftIsDown_ = true;
	MouseEvent me(MouseEvent::EventType::LPress, x, y);
	this->eventBuffer_.push(me);
}

void MouseClass::OnLeftReleased(int x, int y)
{
	this->leftIsDown_ = false;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::LRelease, x, y));
}

void MouseClass::OnRightPressed(int x, int y)
{
	this->rightIsDown_ = true;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::RPress, x, y));
}

void MouseClass::OnRightReleased(int x, int y)
{
	this->rightIsDown_ = false;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::RRelease, x, y));
}

void MouseClass::OnMiddlePressed(int x, int y)
{
	this->mbuttonDown_ = true;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::MPress, x, y));
}

void MouseClass::OnMiddleReleased(int x, int y)
{
	this->mbuttonDown_ = false;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::MRelease, x, y));
}

void MouseClass::OnWheelUp(int x, int y)
{
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelUp, x, y));
}

void MouseClass::OnWheelDown(int x, int y)
{
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::WheelDown, x, y));
}

void MouseClass::OnMouseMove(int x, int y)
{
	this->x = x;
	this->y = y;
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::Move, x, y));
}

// handles the relative changes of the mouse position
void MouseClass::OnMouseMoveRaw(int x, int y)
{
	this->eventBuffer_.push(MouseEvent(MouseEvent::EventType::RAW_MOVE, x, y));
}

bool MouseClass::IsLeftDown() const
{
	return this->leftIsDown_;
}

bool MouseClass::IsMiddleDown() const
{
	return this->mbuttonDown_;
}

bool MouseClass::IsRightDown() const
{
	return this->rightIsDown_;
}

int MouseClass::GetPosX() const
{
	return this->x;
}

int MouseClass::GetPosY() const
{
	return this->y;
}

MousePoint MouseClass::GetPos()
{
	return{ this->x, this->y };
}

bool MouseClass::EventBufferIsEmpty()
{
	return this->eventBuffer_.empty();
}

MouseEvent MouseClass::ReadEvent()
{
	if (this->eventBuffer_.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e = this->eventBuffer_.front(); // get first event from buffer
		this->eventBuffer_.pop();                  // remove first event from buffer
		return e;
	}

}

