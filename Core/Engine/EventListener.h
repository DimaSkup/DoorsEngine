#pragma once

#include <windows.h>

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