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

#include "../Debug/DebugEditor.h"
#include "../Entity/Controller/EditorController.h"
#include "../Fog/FogEditorController.h"


namespace UI
{

class EditorPanels
{
public:
	EditorPanels(StatesGUI* pStatesGUI);

	void Initialize(IFacadeEngineToUI* pFacade);

	void Render(Core::SystemState& sysState);

private:
	void RenderEntitiesListWnd(Core::SystemState& sysState);
	void RenderPropertiesControllerWnd();
	void RenderDebugPanel(const Core::SystemState& sysState);
	void RenderLogPanel();
	void RenderAssetsManager();
	void RenderEditorEventHistory();


public:
	DebugEditor          debugEditor_;
	EditorController     enttEditorController_;
	FogEditorController  fogEditorController_;

private:
	bool isEnttsListWndOpen_  = true;
	bool isPropertiesWndOpen_ = true;

	IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;
	StatesGUI*         pStatesGUI_        = nullptr;
};

} // namespace UI