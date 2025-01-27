// =================================================================================
// Filename:      Application.h
// Description:   the application main class where everything starts
// 
// Created:       19.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "Engine/Engine.h"
#include "Engine/Settings.h"
#include "Common/EngineException.h"

class Application
{
public:
	void Initialize()
	{
		eventHandler_.AddEventListener(&engine_);

		// set an event handler for the window container
		windowContainer_.SetEventHandler(&eventHandler_);

		// get main params for the window initialization
		const bool isFullScreen = settings_.GetBool("FULL_SCREEN");
		const std::string wndTitle = settings_.GetString("WINDOW_TITLE");
		const std::string wndClass = "MyWindowClass";
		const int wndWidth = settings_.GetInt("WINDOW_WIDTH");
		const int wndHeight = settings_.GetInt("WINDOW_HEIGHT");

		// init the main window
		bool result = windowContainer_.renderWindow_.Initialize(
			hInstance_,
			mainWnd_,
			isFullScreen,
			wndTitle,
			wndClass,
			wndWidth,
			wndHeight);
		Assert::True(result, "can't initialize the window");

		// explicitly init Windows Runtime and COM
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		Assert::NotFailed(hr, "ERROR: can't explicitly initialize Windows Runtime and COM");


		// init the engine
		result = engine_.Initialize(hInstance_, mainWnd_, settings_, wndTitle);
		Assert::True(result, "can't initialize the engine");
	}

	///////////////////////////////////////////////////////

	void Run()
	{
		// run the engine
		while (windowContainer_.renderWindow_.ProcessMessages(hInstance_, mainWnd_) == true)
		{
			if (!engine_.IsPaused())
			{
				engine_.Update();
				engine_.RenderFrame();
			}
			else
			{
				Sleep(100);
			}

			if (engine_.IsExit())
				break;
		}
	}

	///////////////////////////////////////////////////////

	void Close()
	{
		windowContainer_.renderWindow_.UnregisterWindowClass(hInstance_);
	}


private:
	HINSTANCE hInstance_ = GetModuleHandle(NULL);
	Log logger_;          // ATTENTION: put the declation of logger before all the others; this instance is necessary to create a logger text file

	Doors::Engine engine_;
	HWND mainWnd_;

	Settings settings_;
	EventHandler eventHandler_;
	Doors::WindowContainer  windowContainer_;
};