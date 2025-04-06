// =================================================================================
// Filename:   EditorPanels.cpp
// 
// Created:    08.01.25 by DimaSkup
// =================================================================================
#include "EditorPanels.h"

#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <UICommon/EventsHistory.h>


#include <vector>
#include <string>
#include <imgui.h>


namespace UI
{

EditorPanels::EditorPanels(StatesGUI* pStatesGUI) :
    enttEditorController_(pStatesGUI),
    pStatesGUI_(pStatesGUI)
{
    Core::Assert::NotNullptr(pStatesGUI, "input ptr to the GUI states container == nullptr");
}


// =================================================================================
//                              public methods
// =================================================================================

void EditorPanels::Initialize(IFacadeEngineToUI* pFacade)
{
    Core::Assert::NotNullptr(pFacade, "ptr to the facade interface == nullptr");
    pFacadeEngineToUI_ = pFacade;

    enttEditorController_.Initialize(pFacadeEngineToUI_);
    fogEditorController_.Initialize(pFacadeEngineToUI_);

    debugEditor_.Initialize(pFacadeEngineToUI_);

    texAssetsBrowser_.Initialize(pFacadeEngineToUI_);
}

///////////////////////////////////////////////////////////

void EditorPanels::Render(Core::SystemState& sysState)
{
    if (pFacadeEngineToUI_ == nullptr)
    {
        Core::Log::Error("you have to initialize a ptr to the facade interface!");
        return;
    }

    RenderEntitiesListWnd(sysState);
    RenderPropertiesControllerWnd();
    RenderDebugPanel(sysState);
    RenderLogPanel();
    RenderAssetsManager();
    RenderTexturesBrowser();
    RenderEditorEventHistory();
}


// =================================================================================
//                              private methods
// =================================================================================

void EditorPanels::RenderLogPanel()
{
    // show logger messages

    if (ImGui::Begin("Log"))
    {	
        for (std::string& logMsg : Core::Log::GetLogMsgsList())
            ImGui::Text(logMsg.c_str());                    // print each log msg
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderAssetsManager()
{
    if (ImGui::Begin("Models", &isTexBrowserWndOpen_))
    {
        modelsAssetsList_.LoadModelsNamesList(pFacadeEngineToUI_);
        modelsAssetsList_.PrintModelsNamesList();
    }
    ImGui::End();
    
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderTexturesBrowser()
{
    

    ImGui::SetNextWindowSize(texAssetsBrowser_.GetWindowSize(), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Textures browser", &isTexBrowserWndOpen_, ImGuiWindowFlags_MenuBar))
    {
        texAssetsBrowser_.Render(pFacadeEngineToUI_);
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderEditorEventHistory()
{
    if (ImGui::Begin("Events history"))
    {
        const std::deque<HistoryItem>& eventsHistory = gEventsHistory.history_;
        int idx = 0;

        for (auto it = eventsHistory.begin(); it != eventsHistory.end(); ++it)
        {
            ImGui::Text("Event[%d]: %s", idx, it->msg_.c_str());
            ++idx;
        }
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderEntitiesListWnd(Core::SystemState& sysState)
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

        /*
        for (int i = 0; i < numEnttsTypes; ++i)
        {

        }
        if (isOpen = ImGui::CollapsingHeader("Spotlight Properties", ImGuiTreeNodeFlags_SpanFullWidth))
            viewLight_.Render(spotLightController_.GetModel());
            */

        // render selectable menu with entts names
        for (int i = 0; i < numEntts; ++i)
        {
            bool isSelected = pEnttsIDs[i] == enttEditorController_.GetSelectedEnttID();

            if (ImGui::Selectable(enttsNames[i].c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
            {
                sysState.pickedEntt_ = pEnttsIDs[i];                 // set this ID into the system state
                enttEditorController_.SetSelectedEntt(sysState.pickedEntt_);  // and update the editor to show data of this entt

                // if we do double click on the selectable item we move our camera
                // to this item in world and fix on in
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    pFacadeEngineToUI_->FocusCameraOnEntity(sysState.pickedEntt_);
                    pStatesGUI_->gizmoOperation_ = 7;   // ImGizmo::OPERATION::TRANSLATE
                    //Log::Print("double click on: " + enttsNames[i], eConsoleColor::YELLOW);
                }
            }
        }
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderDebugPanel(const Core::SystemState& systemState)
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

        ImGui::Text("Visible point lights: %d", systemState.numVisiblePointLights);

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
        // if we have selected any entity
        if (enttEditorController_.selectedEnttData_.IsSelectedAnyEntt())
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

} // namespace UI
