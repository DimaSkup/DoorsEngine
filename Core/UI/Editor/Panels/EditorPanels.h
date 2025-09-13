// =================================================================================
// Filename:     EditorPanels.h
// Description:  wrapper for rendering and handling the left panel of the editor
// 
// Created:      08.01.25 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IFacadeEngineToUI.h>
#include <UICommon/StatesGUI.h>
#include <CoreCommon/SystemState.h>

#include "../EditorPanelElement/ModelsAssetsList.h"
#include "../EditorPanelElement/TextureAssetsBrowser.h"
#include "../EditorPanelElement/MaterialAssetsBrowser.h"

#include "../Debug/DebugEditor.h"
#include "../Entity/Controller/EnttEditorController.h"
#include "../Fog/FogEditorController.h"
#include "../Terrain/Sky/SkyController.h"

namespace UI
{

class EditorPanels
{
public:
    EditorPanels();

    void Initialize(IFacadeEngineToUI* pFacade);

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

    DebugEditor          debugEditor_;
    EnttEditorController enttEditorController_;
    FogEditorController  fogEditorController_;
    SkyController        skyEditorController_;

private:
    IFacadeEngineToUI* pFacade_         = nullptr;

    TextureAssetsBrowser  texturesBrowser_;
    MaterialAssetsBrowser materialsBrowser_;
    ModelsAssetsList      modelsAssetsList_;
};

} // namespace UI
