#pragma once

#include <windows.h>
#include "EventListener.h"

class EventHandler
{
public:
	EventHandler() {}

	void AddEventListener(EventListener* eventListener);

	void HandleEvent(HWND hwnd,	UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	EventListener* pEventListener_ = nullptr;
};
