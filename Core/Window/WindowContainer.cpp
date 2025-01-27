#include "WindowContainer.h"

#include "../Common/log.h"

#include <cassert>

// ImGui stuff
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
//#include "imgui_impl_dx11.h"


// forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace Doors
{

WindowContainer* WindowContainer::pWindowContainer_ = nullptr;


// initialize main devices handlers and setup its behaviour
WindowContainer::WindowContainer()
{
	Log::Debug();


	// we can have only one instance of the WindowContainer
	if (WindowContainer::pWindowContainer_ == nullptr)
	{
		WindowContainer::pWindowContainer_ = this;
		static bool raw_input_initialized = false;

		// try to register a mouse as a RAW INPUT device
		if (raw_input_initialized == false) 
		{
			RAWINPUTDEVICE rid;

			rid.usUsagePage = 0x01;     // mouse
			rid.usUsage     = 0x02;
			rid.dwFlags     = 0;        // use default flags
			rid.hwndTarget  = NULL;

			if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
			{
				Log::Error("can't register raw input devices");
				exit(-1);
			}

			raw_input_initialized = true;
		}
	}
	else
	{
		Log::Error("you can have only one instance of the WindowContainer");
		exit(-1);
	}
}

///////////////////////////////////////////////////////////

WindowContainer::~WindowContainer()
{
	Log::Debug();
}




// ==================================================================================
//                                PUBLIC METHODS
// ==================================================================================


///////////////////////////////////////////////////////////

void WindowContainer::SetEventHandler(EventHandler* pEventHandler)
{
	assert(pEventHandler != nullptr);
	pEventHandler_ = pEventHandler;
}

///////////////////////////////////////////////////////////

LRESULT CALLBACK WindowContainer::WindowProc(
	HWND hwnd, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam)
{
	// Main window procedure;
	// this function is a handler for the window messages

	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	static bool isMouseMoving = false;
	
	switch (uMsg)
	{
		case WM_ACTIVATE:             // handle activation or deactivation of the window
		{
			pEventHandler_->HandleEvent(hwnd, uMsg, wParam, lParam);
			return 0;
		}
		case WM_CLOSE:                // if we hit the "X" (close) button of the window
		{
			Log::Print("the window is closed");
			DestroyWindow(hwnd);
			return 0;
		}
		case WM_DESTROY:              // an event of the window destroyment
		{
			// close the engine entirely
			Log::Print("the window is destroyed");
			PostQuitMessage(0);
			return 0;
		}
		case WM_MENUCHAR:             // is sent when a menu is active and user presses a key that doesn't correspone to any mnemonic or accelerator key
		{
			// don't beep when we alt-enter
			return MAKELRESULT(0, MNC_CLOSE);
		}
		case WM_GETMINMAXINFO:        // catch this message to prevent the window from becoming too small
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;
		}

	}

	// handle window movement, window resizing, keyboard and mouse events, etc.
	pEventHandler_->HandleEvent(hwnd, uMsg, wParam, lParam);


	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	
} 

} // namespace Doors