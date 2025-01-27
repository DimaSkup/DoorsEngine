// =================================================================================
// Filename:   EditorPanels.cpp
// 
// Created:    08.01.25 by DimaSkup
// =================================================================================
#include "EditorPanels.h"

#include "../../../Common/Assert.h"
#include "../../../Common/log.h"

#include <vector>
#include <string>
#include <imgui.h>


EditorPanels::EditorPanels()
{
}


// =================================================================================
//                              public methods
// =================================================================================

void EditorPanels::Initialize(IFacadeEngineToUI* pFacade)
{
	Assert::NotNullptr(pFacade, "ptr to the facade interface == nullptr");
	pFacadeEngineToUI_ = pFacade;

	enttEditorController_.Initialize(pFacadeEngineToUI_);
	fogEditorController_.Initialize(pFacadeEngineToUI_);

	debugEditor_.Initialize(pFacadeEngineToUI_);
}

///////////////////////////////////////////////////////////

void EditorPanels::Render(
	SystemState& sysState,
	const ImGuiChildFlags childFlags,
	const ImGuiWindowFlags wndFlags)
{
	if (pFacadeEngineToUI_ == nullptr)
	{
		Log::Error("you have to initialize a ptr to the facade interface!");
		return;
	}

	RenderEntitiesListWnd(sysState);
	RenderPropertiesControllerWnd();
	RenderDebugPanel(sysState);
	RenderLogPanel();
}



// =================================================================================
//                              private methods
// =================================================================================

void EditorPanels::RenderLogPanel()
{
	if (ImGui::Begin("Log"))
	{
		// show logger messages
		for (std::string& logMsg : Log::GetLogMsgsList())
			ImGui::Text(logMsg.c_str());                    // print each log msg
	}
	ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderEntitiesListWnd(SystemState& sysState)
{
	// render editor elements which are responsible for rendering 
	// the scene hierarchy list, etc.

	if (ImGui::Begin("Entities List", &isEnttsListWndOpen_))
	{
		const uint32_t* pEnttsIDs = nullptr;
		int numEntts = 0;

		// get an ID of each entity on the scene
		pFacadeEngineToUI_->GetAllEnttsIDs(pEnttsIDs, numEntts);

		std::vector<std::string> enttsNames(numEntts);

		// ------ TODO: optimize ----------
		// 
		// get a name of each entity on the scene
		for (int i = 0; std::string& name : enttsNames)
			pFacadeEngineToUI_->GetEnttNameByID(pEnttsIDs[i++], name);

		// render selectable menu with entts names
		for (int i = 0; i < numEntts; ++i)
		{
			bool isSelected = pEnttsIDs[i] == enttEditorController_.GetSelectedEntt();

			if (ImGui::Selectable(enttsNames[i].c_str(), isSelected))
			{
				sysState.pickedEntt_ = pEnttsIDs[i];                 // set this ID into the system state
				enttEditorController_.SetSelectedEntt(sysState.pickedEntt_);  // and update the editor to show data of this entt
			}
		}
	}
	ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderDebugPanel(const SystemState& systemState)
{
	if (ImGui::Begin("Debug"))
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::SeparatorText("Common data:");

		ImGui::Text("mouse pos: %f %f", io.MousePos.x, io.MousePos.y);
		ImGui::Text("is clicked: %d", (int)ImGui::IsMouseDown(ImGuiMouseButton_Left));

		// show fps and frame time
		ImGui::Text("Fps:        %d", systemState.fps);
		ImGui::Text("Frame time: %f", systemState.frameTime);

		// show debug options
		if (ImGui::TreeNode("Show as Color:"))
		{
			debugEditor_.Draw();
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderPropertiesControllerWnd()
{
	// render editor elements which are responsible for editing of:
	// sky, scene fog, entities, etc.

	if (ImGui::Begin("Properties"), &isPropertiesWndOpen_)
	{
		if (enttEditorController_.IsSelectedAnyEntt())
			enttEditorController_.Render();
		

		// show fog editor
		if (ImGui::TreeNodeEx("FogEditor", ImGuiTreeNodeFlags_SpanFullWidth))
		{
			fogEditorController_.Draw();
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

///////////////////////////////////////////////////////////
