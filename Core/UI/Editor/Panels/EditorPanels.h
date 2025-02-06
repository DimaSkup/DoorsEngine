// =================================================================================
// Filename:     EditorPanels.h
// Description:  wrapper for rendering and handling the left panel of the editor
// 
// Created:      08.01.25 by DimaSkup
// =================================================================================
#pragma once

#include "../../../Engine/SystemState.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "../Debug/DebugEditor.h"
#include "../Entity/EntityEditorController.h"
#include "../Fog/FogEditorController.h"


class EditorPanels
{
public:
	EditorPanels();

	void Initialize(IFacadeEngineToUI* pFacade);

	void Render(
		SystemState& sysState,
		const ImGuiChildFlags childFlags,
		const ImGuiWindowFlags wndFlags);

	inline void SetSelectedEntt(const uint32_t entityID) { enttEditorController_.SetSelectedEntt(entityID); }
	inline uint32_t GetSelectedEntt() const              { return enttEditorController_.GetSelectedEntt(); }

private:
	void RenderEntitiesListWnd(SystemState& sysState);
	void RenderPropertiesControllerWnd();
	void RenderDebugPanel(const SystemState& sysState);
	void RenderLogPanel();
	void RenderAssetsManager();


public:
	DebugEditor             debugEditor_;
	EntityEditorController  enttEditorController_;
	FogEditorController     fogEditorController_;

private:
	bool isEnttsListWndOpen_ = true;
	bool isPropertiesWndOpen_ = true;

	IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;
};