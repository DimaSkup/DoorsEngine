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

EditorPanels::EditorPanels(StatesGUI* pStatesGUI) :
    enttEditorController_(pStatesGUI),
    pStatesGUI_(pStatesGUI)
{
    CAssert::NotNullptr(pStatesGUI, "input ptr to the GUI states container == nullptr");
}


// =================================================================================
//                              public methods
// =================================================================================
void EditorPanels::Initialize(IFacadeEngineToUI* pFacade)
{
    CAssert::NotNullptr(pFacade, "ptr to the facade interface == nullptr");
    pFacadeEngineToUI_ = pFacade;

    enttEditorController_.Initialize(pFacadeEngineToUI_);
    fogEditorController_.Initialize(pFacadeEngineToUI_);
    skyEditorController_.Initialize(pFacadeEngineToUI_);

    debugEditor_.Initialize(pFacadeEngineToUI_);

    texturesBrowser_.Initialize(pFacadeEngineToUI_);
    materialsBrowser_.Initialize(pFacadeEngineToUI_);
}

///////////////////////////////////////////////////////////

void EditorPanels::Render(Core::SystemState& sysState)
{
    if (pFacadeEngineToUI_ == nullptr)
    {
        LogErr("you have to initialize a ptr to the facade interface!");
        return;
    }

    RenderEntitiesListWnd(sysState);
    RenderPropertiesControllerWnd();
    RenderDebugPanel(sysState);
    RenderLogPanel();
    RenderEditorEventHistory();

    if (pStatesGUI_->showWndModelsBrowser)
        RenderModelsBrowser();
    if (pStatesGUI_->showWndTexturesBrowser)
        RenderTexturesBrowser();

    if (pStatesGUI_->showWndMaterialsBrowser)
        RenderMaterialsBrowser();

    
    //if (pStatesGUI_->showWnd)
    //RenderWndModelAssetsCreation()

    // show modal window for entity creation
    if (pStatesGUI_->showWndEnttCreation)
    {
        RenderWndEntityCreation(&pStatesGUI_->showWndEnttCreation, pFacadeEngineToUI_);
    }
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
    if (ImGui::Begin("Models browser", &pStatesGUI_->showWndTexturesBrowser))
    {
        modelsAssetsList_.LoadModelsNamesList(pFacadeEngineToUI_);
        modelsAssetsList_.PrintModelsNamesList();
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderTexturesBrowser()
{
    // render the textures browser: in browser we can add/remove/observe currently loaded textures

    ImGui::SetNextWindowSize(texturesBrowser_.GetBrowserWndSize(), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Textures browser", &pStatesGUI_->showWndTexturesBrowser, ImGuiWindowFlags_MenuBar))
    {
        // render the textures icons (visualize the texture)
        texturesBrowser_.Render(pFacadeEngineToUI_, &pStatesGUI_->showWndTexturesBrowser);

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

    if (ImGui::Begin("Materials browser", &pStatesGUI_->showWndMaterialsBrowser, ImGuiWindowFlags_MenuBar))
    {
        materialsBrowser_.Render(pFacadeEngineToUI_, &pStatesGUI_->showWndMaterialsBrowser);

        if (materialsBrowser_.WasMaterialChanged())
        {
            // TODO: this is temporal to simply test functional (for changing texture of the entity)
            //const MaterialID currSelectedMat = materialsBrowser_.GetSelectedMaterialID();
            //const EntityID currSelectedEntt = enttEditorController_.GetSelectedEnttID();
            //pFacadeEngineToUI_->SetEnttMaterial(currSelectedEntt, 0, currSelectedMat);
        }
        //for (ID3D11ShaderResourceView* pIconTex : pFacade->pMaterialIcons_)
        //    ImGui::Image((ImTextureID)pIconTex, { 200, 200 });
    }
    ImGui::End();    
}

///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////

void EditorPanels::RenderEntitiesListWnd(Core::SystemState& sysState)
{
    // render editor elements which are responsible for rendering 
    // the scene hierarchy list, etc.

    if (ImGui::Begin("Entities List", &pStatesGUI_->showWndEnttsList))
    {
        const uint32_t* pEnttsIDs = nullptr;
        int numEntts = 0;

        // get an ID of each entity on the scene
        pFacadeEngineToUI_->GetAllEnttsIDs(pEnttsIDs, numEntts);



        cvector<std::string> enttsNames(numEntts);

        // ------ TODO: optimize ----------
        // 
        // get a name of each entity on the scene
        for (int i = 0; std::string& name : enttsNames)
        {
            pFacadeEngineToUI_->GetEnttNameById(pEnttsIDs[i++], name);
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
                    pFacadeEngineToUI_->FocusCameraOnEntity(id);
                    pStatesGUI_->gizmoOperation = 7;   // ImGizmo::OPERATION::TRANSLATE
                    //LogMsg("double click on: " + enttsNames[i], eConsoleColor::YELLOW);
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

    if (ImGui::Begin("Properties"), &pStatesGUI_->showWndEnttProperties)
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

        if (ImGui::TreeNodeEx("SkyEditor", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            skyEditorController_.Draw();
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void EditorPanels::RenderWndModelAssetsCreation(bool* pOpen)
{
    // render a modal window for loading/importing/generation of assets

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

///////////////////////////////////////////////////////////

void EditorPanels::RenderWndEntityCreation(bool* pOpen, IFacadeEngineToUI* pFacade)
{
    if (*pOpen)
    {
        if (pEnttCreatorWnd_ == nullptr)
        {
            pEnttCreatorWnd_ = new EntityCreatorWnd;

            SetConsoleColor(YELLOW);
            LogMsg(LOG, "Entity creator window is allocated");
            SetConsoleColor(RESET);
        }

        pEnttCreatorWnd_->RenderCreationWindow(pOpen, pFacade);
    }

    if (*pOpen == false)
    {
        if (pEnttCreatorWnd_ != nullptr)
        {
            delete pEnttCreatorWnd_;
            pEnttCreatorWnd_ = nullptr;
        }

        SetConsoleColor(YELLOW);
        LogMsg(LOG, "Entity creator window is DEallocated");
        SetConsoleColor(RESET);
    }
}


} // namespace UI
