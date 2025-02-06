#pragma once

#include "MouseEvent.h"
#include <queue>

class MouseClass
{
public:
	void OnLeftPressed(int x, int y);
	void OnLeftReleased(int x, int y);
	void OnRightPressed(int x, int y);
	void OnRightReleased(int x, int y);
	void OnMiddlePressed(int x, int y);
	void OnMiddleReleased(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y); // handles the relative changes of the mouse position
	void OnLeftDoubleClick();

	inline bool IsLeftDown()         const { return leftIsDown_; };
	inline bool IsMiddleDown()       const { return mbuttonDown_; };
	inline bool IsRightDown()        const { return rightIsDown_; };

	inline int GetPosX()             const { return x_; };
	inline int GetPosY()             const { return y_; };
	inline MousePoint GetPos()       const { return { x_, y_ }; }

	inline bool EventBufferIsEmpty() const { return eventBuffer_.empty(); };
	MouseEvent ReadEvent();

private:
	std::queue<MouseEvent> eventBuffer_;
	bool leftIsDown_  = false;
	bool rightIsDown_ = false;
	bool mbuttonDown_ = false;
	int x_            = 0;
	int y_            = 0;
};