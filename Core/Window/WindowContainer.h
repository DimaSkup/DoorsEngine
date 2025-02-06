#pragma once

#include "../Engine/EventHandler.h"
#include "RenderWindow.h"


namespace Doors
{

class WindowContainer
{
public:
	WindowContainer(HWND hwnd);
	~WindowContainer();

	void SetEventHandler(EventHandler* pEventHandler);

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static WindowContainer* Get() { return pWindowContainer_; };   // returns a pointer to the current WindowContainer instance


public: 
	static WindowContainer* pWindowContainer_;
	RenderWindow  renderWindow_;
	EventHandler* pEventHandler_ = nullptr;

private:
	const USHORT RID_MOUSE = 2;
	const USHORT RID_KEYBOARD = 6;
	DWORD oldKeyboardDelayTime = 0;
};

} // namespace Doors