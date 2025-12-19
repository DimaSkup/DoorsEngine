#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Core
{

class EventListener
{
public:
	enum APP_STATE
	{
		ACTIVATED,    // the app is currently running
		DEACTIVATED   // the app is currently paused
	};

public:
	virtual void EventActivate(const APP_STATE state) = 0;
	virtual void EventWindowMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void EventKeyboard(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void EventMouse(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

}
