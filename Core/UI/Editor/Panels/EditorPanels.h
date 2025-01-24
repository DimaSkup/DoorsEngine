// =================================================================================
// Filename:     EditorPanels.h
// Description:  wrapper for rendering and handling the left panel of the editor
// 
// Created:      08.01.25 by DimaSkup
// =================================================================================
#pragma once

#include "../../../Engine/SystemState.h"
#include "../../UICommon/StatesGUI.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "../Debug/DebugEditor.h"
#include "../Entity/EntityEditorController.h"
#include "../Sky/SkyEditorController.h"
#include "../Fog/FogEditorController.h"


class EditorPanels
{
public:
	EditorPanels();

	void Initialize(IFacadeEngineToUI* pFacade);

	void Render(
		SystemState& sysState,
		StatesGUI& guiStates,
		const ImGuiChildFlags childFlags,
		const ImGuiWindowFlags wndFlags);

	void Update(const SystemState& sysState);

private:
	void RenderEntitiesListWnd(SystemState& sysState);
	void RenderPropertiesControllerWnd();
	void RenderDebugPanel(const SystemState& sysState);
	void RenderLogPanel();

private:
	bool isEnttsListWndOpen_ = true;
	bool isPropertiesWndOpen_ = true;

	IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;

	DebugEditor             debugEditor_;
	EntityEditorController  enttEditorController_;
	SkyEditorController     skyEditorController_;
	FogEditorController     fogEditorController_;
};