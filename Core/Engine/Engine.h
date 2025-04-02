// =================================================================================
// Filename:      Engine.h
// Description:   this class is responsible for initialization and running
//                all the main parts of the engine.
// 
// Created:       05.10.22
// =================================================================================
#pragma once


#include <CoreCommon/SystemState.h>
#include "EventListener.h"

#include "../ImGui/ImGuiLayer.h"
#include "../Sound/SoundClass.h"
#include "../Render/Framebuffer.h"

// input
#include "../Input/inputmanager.h"
#include "../Input/KeyboardClass.h"
#include "../Input/MouseClass.h"
#include "../Input/inputcodes.h"

// window module
#include "../Window/WindowContainer.h"

// cpu/times
#include "../Timers/cpuclass.h"
#include "../Timers/GameTimer.h"

// camera stuff
#include "../Camera/Camera.h"


// graphics stuff
#include "../Mesh/MaterialMgr.h"
#include "../Render/graphicsclass.h"

// UI
#include "../UI/UICommon/IFacadeEngineToUI.h"
#include "../UI/UserInterface.h"



namespace Core
{

class Engine : public EventListener
{
public:
	Engine();
	~Engine();

	// restrict a copying of this class instance
	Engine(const Engine& obj) = delete;
	Engine& operator=(const Engine& obj) = delete;


	// initializes the private members for the Engine class
	bool Initialize(
		HINSTANCE hInstance,
		HWND hwnd,
		const Settings& engineSettings,
		const std::string& windowTitle);

	bool InitializeGUI(D3DClass& d3d, const Settings& settings);

	//bool ProcessMessages();                // processes all the messages which we get from input devices
	void Update();                         // update the state of the engine/game for the current frame                   
	void CalculateFrameStats();            // measure the number of frames being rendered per second (FPS)
	void RenderFrame();                    // do all the rendering onto the screen
	void RenderUI();

	inline bool IsPaused()         const { return isPaused_; }
	inline bool IsExit()           const { return isExit_; }

	// access functions return a copy of the main window handle or app instance handle;
	inline HWND GetHWND()          const { return hwnd_; }
	inline HINSTANCE GetInstance() const { return hInstance_; }

	// event listener methods implementation
	virtual void EventActivate(const APP_STATE state) override;
	virtual void EventWindowMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void EventKeyboard(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void EventMouse(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void HandleEditorEventKeyboard();
	void HandleGameEventKeyboard();

	void SwitchFlashLight(const Camera& camera);
	void UpdateFlashLightPosition(const DirectX::XMFLOAT3& position);
	void UpdateFlashLightDirection(const DirectX::XMFLOAT3& direction);
	void HandleEditorEventMouse();
	void HandleGameEventMouse();

	void RenderModelIntoTexture(
		ID3D11DeviceContext* pContext,
		FrameBuffer& frameBuffer);

private:
	void TurnOnEditorMode();
	void TurnOnGameMode();

private:
	HWND      hwnd_ = NULL;                     // main window handle
	HINSTANCE hInstance_ = NULL;                // application instance handle

	bool      isPaused_ = false;                // defines if the engine/game is currently paused
	bool      isExit_ = false;                  // are we going to exit?
	bool      isMinimized_ = false;             // is the window minimized?
	bool      isMaximized_ = true;              // is the window maximized?
	bool      isResizing_ = false;              // are we resizing the window?
	float     deltaTime_ = 0.0f;                // the time since the previous frame

	std::string windowTitle_{ "" };             // window title/caption


	//Settings           settings_;            // settings container							   
	SystemState        systemState_;         // contains different info about the state of the engine
	CpuClass           cpu_;                 // cpu usage counter
	GameTimer          timer_;               // used to keep track of the "delta-time" and game time

	InputManager       inputMgr_;
	KeyboardClass      keyboard_;            // represents a keyboard device
	MouseClass         mouse_;               // represents a mouse device
	GraphicsClass      graphics_;            // rendering system

	ImGuiLayer         imGuiLayer_;
	UI::UserInterface  userInterface_;       // UI/GUI: for work with the graphics user interface (GUI)

	//KeyboardEvent      keyboardEvent_;       // the current keyboard event
	MouseEvent         mouseEvent_;          // the current mouse event
	SoundClass         sound_;

	FrameBuffer        frameBuffer_;         // we use this obj for rendering the scene into a texture and then we pass this texture into GUI, to show it in the editor

	UI::IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;  // a facade interface which are used by UI to contact with some engine's parts 

	// window's zone width/height
	UINT windowWidth_ = 0;
	UINT windowHeight_ = 0;

	// client's zone width/height
	UINT clientWidth_ = 0;
	UINT clientHeight_ = 0;

}; // class Engine

} // namespace Core
