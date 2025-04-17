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

// from Render module
#include "CRender.h"



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
        const std::string& windowTitle,
        ECS::EntityMgr* pEnttMgr,
        UI::UserInterface* pUserInterface,
        Render::CRender* pRender);

    bool InitializeGUI(D3DClass& d3d, const Settings& settings);

    // update the state of the engine/game for the current frame
    void Update();                         

    void CalculateFrameStats();            // measure the number of frames being rendered per second (FPS)
    void RenderFrame();                    // do all the rendering onto the screen
    void RenderUI(UI::UserInterface* pUI, Render::CRender* pRender);

    inline bool IsPaused()                   const { return isPaused_; }
    inline bool IsExit()                     const { return isExit_; }

    // access functions return a copy of the main window handle or app instance handle;
    inline HWND           GetHWND()          const { return hwnd_; }
    inline HINSTANCE      GetInstance()      const { return hInstance_; }
    inline CGraphics& GetGraphicsClass()       { return graphics_; }
    inline GameTimer&     GetTimer()               { return timer_; }

    // event listener methods implementation
    virtual void EventActivate(const APP_STATE state) override;
    virtual void EventWindowMove  (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventKeyboard    (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    virtual void EventMouse       (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void HandleEditorEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);
    void HandleGameEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);

    void SwitchFlashLight(
        const Camera& camera,
        ECS::EntityMgr& mgr,
        Render::CRender& render);

    void UpdateFlashLightPosition(const DirectX::XMFLOAT3& position, ECS::EntityMgr* pEnttMgr);
    void UpdateFlashLightDirection(const DirectX::XMFLOAT3& direction, ECS::EntityMgr* pEnttMgr);
    void HandleEditorEventMouse(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);
    void HandleGameEventMouse(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr);

    void RenderMaterialsIcons(
        ID3D11ShaderResourceView** outArrShaderResourceViews,
        const size numShaderResourceViews,
        const int iconWidth,
        const int iconHeight);

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
    CGraphics      graphics_;            // rendering system

    ImGuiLayer         imGuiLayer_;

    //KeyboardEvent      keyboardEvent_;       // the current keyboard event
    MouseEvent         mouseEvent_;          // the current mouse event
    SoundClass         sound_;

    cvector<FrameBuffer> materialsFrameBuffers_;  // frame buffers which are used to render materials icons (for material browser)

    ECS::EntityMgr*    pEnttMgr_ = nullptr;
    UI::UserInterface* pUserInterface_ = nullptr;
    Render::CRender*   pRender_ = nullptr;

    // window's zone width/height
    UINT windowWidth_ = 0;
    UINT windowHeight_ = 0;

    // client's zone width/height
    UINT clientWidth_ = 0;
    UINT clientHeight_ = 0;

};

} // namespace Core
