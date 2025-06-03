#include "EventHandler.h"
#include <assert.h>

void EventHandler::AddEventListener(EventListener* eventListener)
{
	assert(eventListener != nullptr);
	pEventListener_ = eventListener;
}

///////////////////////////////////////////////////////////

void EventHandler::HandleEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool isResizing = false;

	switch (uMsg)
	{
		// --- keyboard messages --- //
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		{
			pEventListener_->EventKeyboard(hwnd, uMsg, wParam, lParam);
			return;
		}
		// --- mouse messages --- //
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN: case WM_LBUTTONUP:
		case WM_MBUTTONDOWN: case WM_MBUTTONUP:
		case WM_RBUTTONDOWN: case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_INPUT:
		case WM_LBUTTONDBLCLK:
		{
			pEventListener_->EventMouse(hwnd, uMsg, wParam, lParam);
			return;
		}
		// this message is sent when an application becomes activated or deactivated
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				// set that the application is paused
				pEventListener_->EventActivate(EventListener::APP_STATE::DEACTIVATED);
			}
			else
			{
				// set that the application is activated
				pEventListener_->EventActivate(EventListener::APP_STATE::ACTIVATED);
			}
			return;
		}
		// this message is sent when the used grabs the resize bars
		case WM_ENTERSIZEMOVE:
		{
			pEventListener_->EventActivate(EventListener::APP_STATE::DEACTIVATED);
			isResizing = true;
			return;
		}
		// this message is sent when the used releases the resize bars;
		// here we reset everything based on the new window dimensions;
		case WM_EXITSIZEMOVE:
		{
			pEventListener_->EventActivate(EventListener::APP_STATE::ACTIVATED);
			//pEventListener_->EventWindowResize(hwnd, uMsg, wParam, lParam);
			isResizing = false;
			return;
		}
		case WM_MOVE:
		{
			pEventListener_->EventWindowMove(hwnd, uMsg, wParam, lParam);
			return;
		}
		case WM_SIZE:
		{
			// is sent when a window has been resized to allow child windows to be resized
			pEventListener_->EventWindowResize(hwnd, uMsg, wParam, lParam);
			return;
		}
		case WM_SIZING:
		{
			// is sent while the user is resizing it to allow the size to be adjusted
			pEventListener_->EventWindowSizing(hwnd, uMsg, wParam, lParam);
			return;
		}
	}
}
