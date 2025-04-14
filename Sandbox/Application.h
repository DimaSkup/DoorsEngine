// =================================================================================
// Filename:      Application.h
// Description:   the application main class where everything starts
// 
// Created:       19.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "Engine/Engine.h"
#include "Engine/Settings.h"
#include <CoreCommon/EngineException.h>
#include <CoreCommon/FileSystemPaths.h>

// UI
#include "../UI/UICommon/IFacadeEngineToUI.h"
#include "../UI/UserInterface.h"
#include "../UI/UICommon/FacadeEngineToUI.h"

using namespace Core;


class Application
{
public:
    Application() : windowContainer_(mainWnd_)
    {
    }

    ~Application()
    {
        delete pFacadeEngineToUI_;
    }

    void Initialize()
    {
        // ATTENTION: put the declation of logger before all the others; this instance is necessary to create a logger text file
        Core::InitLogger();

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
        Core::Assert::True(result, "can't initialize the window");

        // explicitly init Windows Runtime and COM
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        Core::Assert::NotFailed(hr, "ERROR: can't explicitly initialize Windows Runtime and COM");


        // init the engine
        result = engine_.Initialize(
            hInstance_,
            mainWnd_,
            settings_,
            wndTitle,
            &entityMgr_,
            &userInterface_,
            &render_);
        Core::Assert::True(result, "can't initialize the engine");

        Core::D3DClass&      d3d = engine_.GetGraphicsClass().GetD3DClass();
        ID3D11Device*        pDevice = d3d.GetDevice();
        ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

        // create a facade btw the UI and the engine parts
        pFacadeEngineToUI_ = new UI::FacadeEngineToUI(
            pContext,
            &render_,
            &entityMgr_,
            &engine_.GetGraphicsClass().GetEditorCamera());

        // initialize the main UserInterface class
        InitializeGUI(pDevice, pContext, d3d.GetWindowWidth(), d3d.GetWindowHeight());

        // create a str with duration time of the engine initialization process
        char buf[32] {'\0'};
        sprintf(buf, "Engine init time: %s s", engine_.GetTimer().GetGameTime());
        userInterface_.CreateConstStr(pDevice, buf, { 10, 325 });

    }

    /////////////////////////////////////////////////

    bool InitializeGUI(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        int wndWidth,
        int wndHeight)
    {
        // this function initializes the GUI of the game/engine (interface elements, text, etc.);

        Core::LogMsgf(" ");
        Core::LogMsgf("%s----------------------------------------------------------", YELLOW);
        Core::LogMsgf("%s                   INITIALIZATION: GUI                    ", YELLOW);
        Core::LogMsgf("%s----------------------------------------------------------", YELLOW);

        try
        {

            std::string fontDataFullFilePath;
            fontDataFullFilePath += g_RelPathUIDataDir;
            fontDataFullFilePath += settings_.GetString("FONT_DATA_FILE_PATH");

            std::string fontTexFullFilePath;
            fontTexFullFilePath += g_RelPathUIDataDir;
            fontTexFullFilePath += settings_.GetString("FONT_TEXTURE_FILE_PATH");

            char videoCardName[128]{ '\0' };
            int videoCardMemory = 0;

            D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();

            d3d.GetVideoCardInfo(videoCardName, 128, videoCardMemory);

            // initialize the user interface
            userInterface_.Initialize(
                d3d.GetDevice(),
                d3d.GetDeviceContext(),
                pFacadeEngineToUI_,
                fontDataFullFilePath,
                fontTexFullFilePath,
                d3d.GetWindowWidth(),
                d3d.GetWindowHeight(),
                videoCardMemory,
                videoCardName);

            return true;
        }
        catch (Core::EngineException& e)
        {
            Core::LogErr(e, true);
            return false;
        }
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
        Core::CloseLogger();
    }


private:
    HINSTANCE hInstance_ = GetModuleHandle(NULL);

    Core::Engine      engine_;
    ECS::EntityMgr    entityMgr_;
    Render::CRender   render_;                                // rendering module
    UI::UserInterface userInterface_;       // UI/GUI: for work with the graphics user interface (GUI)

    UI::IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;  // a facade interface which are used by UI to contact with some engine's parts 


    HWND mainWnd_;

    Core::Settings settings_;
    EventHandler eventHandler_;
    Core::WindowContainer  windowContainer_;
};
