#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "EventListener.h"

class EventHandler
{
public:
	EventHandler() {}

	void AddEventListener(EventListener* eventListener);
    void DetachAllEventListeners();

	void HandleEvent(HWND hwnd,	UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	EventListener* pEventListener_ = nullptr;
};
