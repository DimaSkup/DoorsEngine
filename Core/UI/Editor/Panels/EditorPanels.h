// =================================================================================
// Filename:     EditorPanels.h
// Description:  wrapper for rendering and handling the left panel of the editor
// 
// Created:      08.01.25 by DimaSkup
// =================================================================================
#pragma once

#include <CoreCommon/system_state.h>

#include "../EditorPanelElement/ui_models_browser.h"
#include "../EditorPanelElement/ui_textures_browser.h"
#include "../EditorPanelElement/ui_materials_browser.h"

#include "../Tools/model_screenshot.h"
#include "../Debug/ui_debug_control.h"
#include "../Entity/Controller/EnttEditorController.h"
#include "../Fog/FogEditorController.h"
#include "../Terrain/Sky/SkyController.h"

namespace UI
{
// forward declarations
class IFacadeEngineToUI;


class EditorPanels
{
public:
    EditorPanels();

    void Init(IFacadeEngineToUI* pFacade);
    void Render(Core::SystemState& sysState);

private:
    void RenderEntitiesListWnd(Core::SystemState& sysState);
    void RenderPropertiesControllerWnd();
    void RenderDebugPanel(const Core::SystemState& sysState);
    void RenderLogPanel();
    void RenderModelsBrowser();
    void RenderTexturesBrowser();
    void RenderMaterialsBrowser();
    void RenderEditorEventHistory();

    void RenderWndModelAssetsCreation(bool* pOpen);

public:
    DebugControl          debugControl_;
    EnttEditorController  enttEditorController_;
    FogEditorController   fogEditorController_;
    SkyController         skyEditorController_;

private:
    IFacadeEngineToUI*    pFacade_ = nullptr;

    UITexturesBrowser     texturesBrowser_;
    UIMaterialsBrowser    materialsBrowser_;
    UIModelsBrowser       modelsBrowser_;
    ModelScreenshot       modelScreenshot_;

    cvector<std::string>  enttsNames_;
};

} // namespace UI
