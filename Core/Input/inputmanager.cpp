////////////////////////////////////////////////////////////////////
// Filename: inputmanager.cpp
// Revising: 06.10.22
////////////////////////////////////////////////////////////////////
#include "inputmanager.h"


LRESULT InputManager::HandleKeyboardMessage(
	KeyboardClass & keyboard, 
	const UINT & message, 
	WPARAM wParam, 
	LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			if (keyboard.IsKeysAutoRepeat())
			{
				keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
			else
			{
				// if the key hasn't been pressed before
				if (!(lParam & 0x40000000))  
				{
					keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
				}
			}
			
			return 0;
		}
		case WM_KEYUP:
		{
			keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
			return 0;
		}
#if 0
		case WM_CHAR:
		{
			keyboard.OnChar(static_cast<unsigned char>(wParam));

			if (keyboard.IsCharsAutoRepeat())
			{
				keyboard.OnChar(static_cast<unsigned char>(wParam));
			}
			else
			{
				// if the key hasn't been pressed before
				if (!(lParam & 0x40000000))
				{
					keyboard.OnChar(static_cast<unsigned char>(wParam));
				}
			}
		
			return 0;
		}
#endif
	} // switch

	return 0;
}

///////////////////////////////////////////////////////////

LRESULT InputManager::HandleMouseMessage(
	MouseClass & mouse, 
	const UINT& uMsg, 
	WPARAM wParam,
	LPARAM lParam)
{
	static bool isMouseMoving = false;

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);

	switch (uMsg)
	{
		case WM_MOUSEMOVE:
		{
			mouse.OnMouseMove(x, y);
			isMouseMoving = true;
			return 0;
		}
		case WM_LBUTTONDOWN: 
		{
			mouse.OnLeftPressed(x, y);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			mouse.OnRightPressed(x, y);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			mouse.OnMiddlePressed(x, y);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			mouse.OnLeftReleased(x, y);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			mouse.OnRightReleased(x, y);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			mouse.OnMiddleReleased(x, y);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				mouse.OnWheelUp(x, y);
			}
			else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			{
				mouse.OnWheelDown(x, y);
			}
			return 0;
		}
		case WM_LBUTTONDBLCLK:
		{
			mouse.OnLeftDoubleClick();
			return 0;
		}
		// --- raw input --- //
		case WM_INPUT:
		{
			if (isMouseMoving == true)
			{
				UINT dataSize = 0;
				void* ptrToLParam = &lParam;
				HRAWINPUT* ptrHRawInput = static_cast<HRAWINPUT*>(ptrToLParam); // convert the lParam structure to HRAWINPUT

				GetRawInputData(*ptrHRawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

				// if we got some data about a raw input
				// NOTE: it is supposed that dataSize <= 48
				if (dataSize > 0 && dataSize < 49) 
				{
					BYTE rawData[48];
					
					if (GetRawInputData(*ptrHRawInput, RID_INPUT, rawData, &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
					{
						//void* ptrRawDataGetToVoid = rawData;
						RAWINPUT* raw = static_cast<RAWINPUT*>((void*)rawData);
						if (raw->header.dwType == RIM_TYPEMOUSE)
						{
							// set how much the mouse position changed from the previous one
							mouse.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
							isMouseMoving = false;
						}
					}
				}
			}
			else
			{
				mouse.OnMouseMoveRaw(0, 0);
			}

			return 0;

		} // case WM_INPUT
	} // switch

	return 0;
}
