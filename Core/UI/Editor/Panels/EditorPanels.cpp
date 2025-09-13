// =================================================================================
// Filename:   EditorPanels.cpp
// 
// Created:    08.01.25 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EditorPanels.h"
#include <UICommon/EventsHistory.h>
#include <imgui.h>


namespace UI
{

EditorPanels::EditorPanels()
{
}


// =================================================================================
//                              public methods
// =================================================================================
void EditorPanels::Initialize(IFacadeEngineToUI* pFacade)
{
    CAssert::NotNullptr(pFacade, "ptr to the facade interface == nullptr");
    pFacade_ = pFacade;

    enttEditorController_.Initialize(pFacade_);
    fogEditorController_.Initialize(pFacade_);
    skyEditorController_.Initialize(pFacade_);

    debugEditor_.Initialize(pFacade_);

    texturesBrowser_.Initialize(pFacade_);
    materialsBrowser_.Initialize(pFacade_);
}

///////////////////////////////////////////////////////////

void EditorPanels::Render(Core::SystemState& sysState)
{
    if (pFacade_ == nullptr)
    {
        LogErr(LOG, "you have to initialize a ptr to the facade interface!");
        return;
    }

    RenderEntitiesListWnd(sysState);
    RenderPropertiesControllerWnd();
    RenderDebugPanel(sysState);
    RenderLogPanel();
    RenderEditorEventHistory();

    if (g_GuiStates.showWndModelsBrowser)
        RenderModelsBrowser();
    if (g_GuiStates.showWndTexturesBrowser)
        RenderTexturesBrowser();

    if (g_GuiStates.showWndMaterialsBrowser)
        RenderMaterialsBrowser();
}



// =================================================================================
//                              private methods
// =================================================================================
void EditorPanels::RenderLogPanel()
{
    // show logger messages

    if (ImGui::Begin("Log"))
    {
        const LogStorage* pLogStorage = GetLogStorage();

        constexpr ImVec4 textColors[4] = {
            ImVec4(0, 1, 0, 1),             // color for LOG_TYPE_MESSAGE
            ImVec4(1, 1, 1, 1),             // color for LOG_TYPE_DEBUG
            ImVec4(1, 0, 0, 1),             // color for LOG_TYPE_ERROR
            ImVec4(1, 1, 0, 1)              // color for LOG_TYPE_FORMATTED
        };

        for (int i = 0; i < pLogStorage->numLogs; ++i)
        {
            const LogMessage& log = pLogStorage->logs[i];
            ImGui::TextColored(textColors[log.type], log.msg);
        }
            
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderModelsBrowser()
{
    if (ImGui::Begin("Models browser", &g_GuiStates.showWndTexturesBrowser))
    {
        modelsAssetsList_.LoadModelsNamesList(pFacade_);
        modelsAssetsList_.PrintModelsNamesList();
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderTexturesBrowser()
{
    // render the textures browser: in browser we can add/remove/observe currently loaded textures

    ImGui::SetNextWindowSize(texturesBrowser_.GetBrowserWndSize(), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Textures browser", &g_GuiStates.showWndTexturesBrowser, ImGuiWindowFlags_MenuBar))
    {
        // render the textures icons (visualize the texture)
        texturesBrowser_.Render(pFacade_, &g_GuiStates.showWndTexturesBrowser);

        if (texturesBrowser_.TexWasChanged())
        {
            
        }
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderMaterialsBrowser()
{
    ImGui::SetNextWindowSize(texturesBrowser_.GetBrowserWndSize(), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Materials browser", &g_GuiStates.showWndMaterialsBrowser, ImGuiWindowFlags_MenuBar))
    {
        materialsBrowser_.Render(pFacade_, &g_GuiStates.showWndMaterialsBrowser);
    }
    ImGui::End();    
}

//---------------------------------------------------------
// Desc:   render a list of editor events  (changing position of entities,
//         changing terrain geometry, and other such stuff)
//---------------------------------------------------------
void EditorPanels::RenderEditorEventHistory()
{
    if (ImGui::Begin("Events history"))
    {
        const std::deque<HistoryItem>& eventsHistory = g_EventsHistory.history_;
        int idx = 0;

        for (auto it = eventsHistory.begin(); it != eventsHistory.end(); ++it)
        {
            ImGui::Text("Event[%d]: %s", idx, it->msg_.c_str());
            ++idx;
        }
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render editor elements which are responsible for rendering 
//         the scene hierarchy list, etc.
//---------------------------------------------------------
void EditorPanels::RenderEntitiesListWnd(Core::SystemState& sysState)
{

    if (ImGui::Begin("Entities List", &g_GuiStates.showWndEnttsList))
    {
        const uint32* pEnttsIDs = nullptr;
        int numEntts = 0;

        // get an ID of each entity on the scene
        pFacade_->GetAllEnttsIDs(pEnttsIDs, numEntts);



        cvector<std::string> enttsNames(numEntts);

        // ------ TODO: optimize ----------
        // 
        // get a name of each entity on the scene
        for (int i = 0; std::string& name : enttsNames)
        {
            pFacade_->GetEnttNameById(pEnttsIDs[i++], name);
        }


        const EntityID currSelectedEnttID = enttEditorController_.GetSelectedEnttID();

        // render selectable menu with entts names
        for (int i = 0; i < numEntts; ++i)
        {
            bool isSelected = (pEnttsIDs[i] == currSelectedEnttID);

            if (ImGui::Selectable(enttsNames[i].c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick))
            {
                const EntityID id = pEnttsIDs[i];
                sysState.pickedEnttID_ = id;                 // set this ID into the system state
                enttEditorController_.SetSelectedEntt(id);   // and update the editor to show data of this entt

                // if we do double click on the selectable item we move our camera
                // to this item in world and fix on in
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    pFacade_->FocusCameraOnEntity(id);
                    g_GuiStates.gizmoOperation = 7;   // ImGizmo::OPERATION::TRANSLATE
                }
            }
        }
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render a panel with debug info and some fields for debugging 
//---------------------------------------------------------
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

        // show debug options
        if (ImGui::TreeNode("Show as Color:"))
        {
            debugEditor_.Draw();
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render editor elements which are responsible for editing of:
//         sky, scene fog, entities, grass, etc.
//---------------------------------------------------------
void EditorPanels::RenderPropertiesControllerWnd()
{
    if (ImGui::Begin("Properties"), &g_GuiStates.showWndEnttProperties)
    {
        // if we have selected any entity
        if (enttEditorController_.selectedEnttData_.IsSelectedAnyEntt())
            enttEditorController_.Render();
        

        if (ImGui::TreeNodeEx("FogEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            fogEditorController_.Draw();
            ImGui::TreePop();
        }


        if (ImGui::TreeNodeEx("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            skyEditorController_.Draw();
            ImGui::TreePop();
        }


        if (ImGui::TreeNodeEx("GrassEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            float distFullSize = pFacade_->GetGrassDistFullSize();
            float distVisible  = pFacade_->GetGrassDistVisible();

            // setup a distance from camera where grass is in its full size
            if (ImGui::DragFloat("dist full size", &distFullSize, 1.0f, 0.0f, distVisible))
            {
                if (distFullSize >= 0 && distFullSize <= distVisible)
                    pFacade_->SetGrassDistFullSize(distFullSize);
            }

            // setup distance from the camera after which we don't see any grass
            // ALSO from distFullSize to distVisible grass instances are linearly getting smaller until it won't dissapear
            if (ImGui::DragFloat("dist visible", &distVisible, 1.0f, distFullSize))
            {
                if (distVisible >= 0 && distVisible >= distFullSize)
                    pFacade_->SetGrassDistVisible(distVisible);
            }

            ImGui::TreePop();
        }


        if (ImGui::TreeNodeEx("TerrainEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            int numMaxLOD = pFacade_->GetTerrainNumMaxLOD();

            for (int i = 0; i <= numMaxLOD; ++i)
            {
                int dist = pFacade_->GetTerrainDistanceToLOD(i);

                ImGui::Text("LOD %d:  ", i);
                ImGui::SameLine();
                ImGui::PushID(i);
                if (ImGui::DragInt("distance", &dist, 1.0f, 0))
                {
                    pFacade_->SetTerrainDistanceToLOD(i, dist);
                }
                ImGui::PopID();
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render a modal window for loading/importing/generation of assets
//---------------------------------------------------------
void EditorPanels::RenderWndModelAssetsCreation(bool* pOpen)
{
    ImGuiViewport* pViewport = ImGui::GetMainViewport();

    const ImVec2 wndSize     = { 0.5f * pViewport->Size.x, 0.5f * pViewport->Size.y };
    const ImVec2 halfWndSize = { 0.5f * wndSize.x, 0.5f * wndSize.y };
    const ImVec2 midPoint    = wndSize;
    const ImVec2 pos         = { midPoint.x - halfWndSize.x, midPoint.y - halfWndSize.y };

    ImGui::SetNextWindowSize(wndSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Once);

    if (ImGui::Begin("Assets creator", pOpen))
    {
        if (ImGui::BeginTabBar("##TabBarAssetsControl"))
        {
            if (ImGui::BeginTabItem("Load"))
            {
                ImGui::Text("Load a new asset from the engine internal format ");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Import"))
            {
                ImGui::Text("Import a new asset from the external format");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Create"))
            {
                ImGui::Text("Create (generate) a new asset");
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace UI
