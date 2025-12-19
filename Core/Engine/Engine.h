// =================================================================================
// Filename:      Engine.h
// Description:   this class is responsible for initialization and running
//                all the main parts of the engine.
// 
// Created:       05.10.22
// =================================================================================
#pragma once


#include <CoreCommon/system_state.h>
#include "event_listener.h"

// input
#include "../Input/inputmanager.h"
#include "../Input/keyboard.h"
#include "../Input/mouse.h"
//#include "../Input/inputcodes.h"

// cpu/times
//#include "../Timers/cpuclass.h"
#include "../Timers/game_timer.h"

// graphics stuff

#include "../Mesh/material_mgr.h"
#include "../Render/CGraphics.h"

// ui
#include "../ImGui/imgui_layer.h"
#include "../UI/user_interface.h"

// from the Render module
#include "Render/CRender.h"

// from the ECS module: Entity-Component-System
#include "Entity/EntityMgr.h"



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
    void Init(
        HINSTANCE hInstance,
        HWND hwnd,
        const EngineConfigs& cfg,
        const std::string& windowTitle);

    void BindRender(Render::CRender* pRender);
    void BindECS(ECS::EntityMgr* pEnttMgr);
    void BindUI(UI::UserInterface* pUI);
    bool BindBindlessTextures(const char* configPath);

    // update the state of the engine/game for the current frame
    void Update();

    void UpdateRenderTimingStat(SystemState& sysState);

    void CalculateFrameStats();            // measure the number of frames being rendered per second (FPS)
    void RenderFrame();                    // do all the rendering onto the screen
    void RenderInEditorMode();
    void RenderInGameMode();
    void RenderUI(UI::UserInterface* pUI, Render::CRender* pRender);

    void SwitchGpuMetricsCollection(const bool state);

    // switch game/editor mode (we will use this flag to define if we need to switch)
    inline void                 SwitchEngineMode()       { switchEngineMode_ = true; }

    inline bool                 IsGameMode()       const { return systemState_.isGameMode; }
    inline bool                 IsPaused()         const { return isPaused_; }
    inline bool                 IsExit()           const { return isExit_; }
    inline void                 DoExit()                 { isExit_ = true; }

    // inline getters
    inline HWND                 GetHWND()          const { return hwnd_; }
    inline HINSTANCE            GetInstance()      const { return hInstance_; }
    inline CGraphics&           GetGraphicsClass()       { return graphics_; }
    inline Render::D3DClass&    GetD3D()                 { return pRender_->GetD3D(); }

    inline GameTimer&           GetTimer()               { return timer_; }
    inline SystemState&         GetSystemState()         { return systemState_; }
    inline Keyboard&            GetKeyboard()            { return keyboard_; }
    inline Mouse&               GetMouse()               { return mouse_; }


    // event listener methods implementation
    virtual void EventActivate    (const APP_STATE state) override;
    virtual void EventWindowMove  (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventKeyboard    (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventMouse       (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void HandleEditorEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);
    void HandleEditorEventMouse   (UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);
    void HandleGameEventMouse     (UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);

private:
    void TurnOnEditorMode();
    void TurnOnGameMode();

    void CollectTimestamps(ID3D11DeviceContext* pContext);

private:
    HWND      hwnd_         = NULL;             // main window handle
    HINSTANCE hInstance_    = NULL;             // application instance handle

    bool      isPaused_     = false;            // defines if the engine/game is currently paused
    bool      isExit_       = false;            // are we going to exit?
    bool      isMinimized_  = false;            // is the window minimized?
    bool      isMaximized_  = true;             // is the window maximized?
    bool      isResizing_   = false;            // are we resizing the window?
    bool      switchEngineMode_ = false;        // a flag to define if we want to switch btw game/editor mode
    float     deltaTime_    = 0.0f;             // the time since the previous frame

    std::string windowTitle_{ "" };             // window title/caption

    SystemState         systemState_;           // contains different info about the state of the engine
    //CpuClass            cpu_;                   // cpu usage counter
    GameTimer           timer_;                 // used to keep track of the "delta-time" and game time

    InputManager        inputMgr_;
    Keyboard            keyboard_;              // represents a keyboard device
    Mouse               mouse_;                 // represents a mouse device
    CGraphics           graphics_;              // rendering system

    ImGuiLayer          imGuiLayer_;

    //KeyboardEvent      keyboardEvent_;       // the current keyboard event
    MouseEvent          mouseEvent_;          // the current mouse event
    //SoundClass          sound_;

    ECS::EntityMgr*     pEnttMgr_       = nullptr;
    UI::UserInterface*  pUserInterface_ = nullptr;
    Render::CRender*    pRender_        = nullptr;

    // window's zone width/height
    UINT windowWidth_   = 0;
    UINT windowHeight_  = 0;

    // client's zone width/height
    UINT clientWidth_   = 0;
    UINT clientHeight_ = 0;
};

//==================================================================================
// Global instance of the engine (I don't care)
//==================================================================================
//extern Engine g_Engine;

} // namespace Core
