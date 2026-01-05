// =================================================================================
// Filename:      App.h
// Description:   the application main class where everything starts
// 
// Created:       19.01.25  by DimaSkup
// =================================================================================
#pragma once

// core stuff
#include "Engine/event_handler.h"
#include "Engine/engine.h"
#include "Engine/engine_configs.h"
#include "Window/window_container.h"
#include <EngineException.h>

// shared stuff
#include <FileSystemPaths.h>

// UI (also from Core)
#include "../UI/UICommon/IFacadeEngineToUI.h"
#include "../UI/UICommon/FacadeEngineToUI.h"
#include "../UI/user_interface.h"

#include "Game.h"


namespace Game
{

class App
{
public:
    App();
    ~App();

    void Init();
    void Run();
    void Update(const float deltaTime, const float gameTime);
    void Close();

    bool InitWindow();
    void InitEngine();
    bool InitRender(const Core::EngineConfigs& cfg);
    bool InitScene (ID3D11Device* pDevice, const Core::EngineConfigs& cfg);
    bool InitGUI   (ID3D11Device* pDevice, const int wndWidth, const int wndHeight);

private:
    HINSTANCE hInstance_ = GetModuleHandle(NULL);

    Game                   game_;
    Core::Engine           engine_;

    ECS::EntityMgr         entityMgr_;
    Render::CRender        render_;                                // rendering module
    UI::UserInterface      userInterface_;       // UI/GUI: for work with the graphics user interface (GUI)

    UI::IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;  // a facade interface which are used by UI to contact with some engine's parts

    HWND                   mainHWND_;
    Core::EngineConfigs    engineConfigs_;
    Core::EventHandler     eventHandler_;
    Core::WindowContainer  wndContainer_;
};

} // namespace Game
