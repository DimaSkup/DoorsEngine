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
	RenderAssetsManager();
}



// =================================================================================
//                              private methods
// =================================================================================

void EditorPanels::RenderLogPanel()
{
	// show logger messages

	if (ImGui::Begin("Log"))
	{	
		for (std::string& logMsg : Log::GetLogMsgsList())
			ImGui::Text(logMsg.c_str());                    // print each log msg
	}
	ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderAssetsManager()
{
	
	if (ImGui::Begin("Assets"))
	{
		int numAssets = pFacadeEngineToUI_->GetNumAssets();

		// if we have any assets in assets/models manager
		if (numAssets > 0)
		{
			std::vector<std::string> names(numAssets);
			pFacadeEngineToUI_->GetAssetsNamesList(names.data(), (int)names.size());

			for (const std::string& name : names)
				ImGui::Text(name.c_str());
		}
		
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

			if (ImGui::Selectable(enttsNames[i].c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
			{
				sysState.pickedEntt_ = pEnttsIDs[i];                 // set this ID into the system state
				enttEditorController_.SetSelectedEntt(sysState.pickedEntt_);  // and update the editor to show data of this entt

				// if we do double click on the selectable item we move our camera
				// to this item in world and fix on in
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					pFacadeEngineToUI_->PlaceCameraNearEntt(sysState.pickedEntt_);
					Log::Print("double click on: " + enttsNames[i], ConsoleColor::YELLOW);
				}
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


		const DirectX::XMFLOAT3& camPos = systemState.cameraPos;
		const DirectX::XMFLOAT3& camDir = systemState.cameraDir;
		ImGui::Text("Camera pos: %.2f %.2f %.2f", camPos.x, camPos.y, camPos.z);

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
