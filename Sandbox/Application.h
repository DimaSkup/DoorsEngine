// =================================================================================
// Filename:      Application.h
// Description:   the application main class where everything starts
// 
// Created:       19.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "Engine/Engine.h"
#include "Engine/EngineConfigs.h"
#include <EngineException.h>
#include <FileSystemPaths.h>

#include "SceneInitializer.h"

// UI
#include "../UI/UICommon/IFacadeEngineToUI.h"
#include "../UI/UserInterface.h"
#include "../UI/UICommon/FacadeEngineToUI.h"

using namespace Core;


namespace Game
{

class Application
{
public:
    Application();
    ~Application();

    void Initialize();
    void Run();
    void Close();

    bool InitWindow();
    bool InitEngine();
    bool InitScene(ID3D11Device* pDevice, const EngineConfigs& cfg);
    bool InitRenderModule(ID3D11Device* pDevice, const EngineConfigs& cfg, Render::CRender* pRender);
    bool InitGUI(ID3D11Device* pDevice, const int wndWidth, const int wndHeight);


private:
    HINSTANCE hInstance_ = GetModuleHandle(NULL);

    Core::Engine      engine_;
    ECS::EntityMgr    entityMgr_;
    Render::CRender   render_;                                // rendering module
    UI::UserInterface userInterface_;       // UI/GUI: for work with the graphics user interface (GUI)

    UI::IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;  // a facade interface which are used by UI to contact with some engine's parts

    HWND                  mainHWND_;
    Core::EngineConfigs        settings_;
    EventHandler          eventHandler_;
    Core::WindowContainer wndContainer_;
    bool                  startInGameMode_ = false;
};

} // namespace Game
