// =================================================================================
// Filename:   EditorPanels.cpp
// 
// Created:    08.01.25 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_post_fx_control.h"
#include "EditorPanels.h"
#include <UICommon/gui_states.h>
#include <UICommon/events_history.h>
#include <imgui.h>
#include <UICommon/IFacadeEngineToUI.h>


namespace UI
{

EditorPanels::EditorPanels()
{
}

// =================================================================================
//                              public methods
// =================================================================================
void EditorPanels::Init(IFacadeEngineToUI* pFacade)
{
    CAssert::NotNullptr(pFacade, "ptr to the facade interface == nullptr");
    pFacade_ = pFacade;

    enttEditorController_.Init(pFacade_);
    fogEditorController_.Initialize(pFacade_);
    skyEditorController_.Initialize(pFacade_);

    texturesBrowser_.Init(pFacade_);
    materialsBrowser_.Init(pFacade_);

    modelScreenshot_.Init(pFacade_);
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

    if (g_GuiStates.showWndModelScreenshot)
    {
        bool open = g_GuiStates.showWndModelScreenshot;
        modelScreenshot_.Render(&open);
        g_GuiStates.showWndModelScreenshot = open;
    }
        
}

//---------------------------------------------------------
// Desc:   show logger messages in a separate window
//---------------------------------------------------------
void EditorPanels::RenderLogPanel()
{
    if (ImGui::Begin("Log"))
    {
        constexpr ImVec4 textColors[4] = {
            ImVec4(0, 1, 0, 1),             // green:  color for LOG_TYPE_MESSAGE
            ImVec4(1, 1, 1, 1),             // white:  color for LOG_TYPE_DEBUG
            ImVec4(1, 0, 0, 1),             // red:    color for LOG_TYPE_ERROR
            ImVec4(1, 1, 0, 1)              // yellow: color for LOG_TYPE_FORMATTED
        };

        for (int i = 0; i < GetNumLogMsgs(); ++i)
        {
            const eLogType logType = GetLogTypeByIdx(i);
            const char*    logMsg  = GetLogTextByIdx(i);

            ImGui::TextColored(textColors[logType], logMsg);
        }
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render a list of models assets into a separate window
//---------------------------------------------------------
void EditorPanels::RenderModelsBrowser()
{
    bool showWndTexturesBrowser = g_GuiStates.showWndTexturesBrowser;

    if (ImGui::Begin("Models browser", &showWndTexturesBrowser))
    {
        modelsBrowser_.LoadModelsNamesList(pFacade_);
        modelsBrowser_.PrintModelsNamesList();
    }
    ImGui::End();
    g_GuiStates.showWndTexturesBrowser = showWndTexturesBrowser;
}

//---------------------------------------------------------
// Desc:   render a list of textures icons into a separate window (textures browser);
//         in the browser we can add/remove/observe currently loaded textures
//---------------------------------------------------------
void EditorPanels::RenderTexturesBrowser()
{
    const float wndSizeX = texturesBrowser_.GetWndSizeX();
    const float wndSizeY = texturesBrowser_.GetWndSizeY();
    ImGui::SetNextWindowSize(ImVec2(wndSizeX, wndSizeY), ImGuiCond_FirstUseEver);

    bool showWndTexBrowser = g_GuiStates.showWndTexturesBrowser;

    if (ImGui::Begin("Textures browser", &showWndTexBrowser, ImGuiWindowFlags_MenuBar))
    {
        texturesBrowser_.Render(pFacade_, &showWndTexBrowser);
    }
    ImGui::End();
    g_GuiStates.showWndTexturesBrowser = showWndTexBrowser;
}

//---------------------------------------------------------
// Desc:   render a list of materials icons (spheres with material)
//         into a separate window (material browser)
//---------------------------------------------------------
void EditorPanels::RenderMaterialsBrowser()
{
    const float wndSizeX = materialsBrowser_.GetWndSizeX();
    const float wndSizeY = materialsBrowser_.GetWndSizeY();
    ImGui::SetNextWindowSize(ImVec2(wndSizeX, wndSizeY), ImGuiCond_FirstUseEver);

    bool showWndMatBrowser = g_GuiStates.showWndMaterialsBrowser;

    if (ImGui::Begin("Materials browser", &showWndMatBrowser, ImGuiWindowFlags_MenuBar))
    {
        materialsBrowser_.Render(pFacade_, &showWndMatBrowser);
    }
    ImGui::End();
    g_GuiStates.showWndMaterialsBrowser = showWndMatBrowser;
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
    bool showWndEnttsList = g_GuiStates.showWndEnttsList;

    if (ImGui::Begin("Entities List", &showWndEnttsList))
    {
        const cvector<EntityID>& enttsIds    = *pFacade_->GetAllEnttsIDs();
        const size               numAllEntts = enttsIds.size();
        
        // get a name of each entity on the scene
        enttsNames_.resize(numAllEntts);

        for (int i = 0; std::string& name : enttsNames_)
        {
            name = pFacade_->GetEnttNameById(enttsIds[i++]);
        }

        const EntityID currSelectedEnttId = enttEditorController_.GetSelectedEnttID();
        constexpr ImGuiSelectableFlags_ flags = ImGuiSelectableFlags_AllowDoubleClick;

        // render selectable menu with entts names
        for (index i = 0; i < numAllEntts; ++i)
        {
            const char* name = enttsNames_[i].c_str();
            bool isSelected  = (enttsIds[i] == currSelectedEnttId);
            
            if (ImGui::Selectable(name, isSelected, flags))
            {
                const EntityID id = enttsIds[i];
                sysState.pickedEnttID_ = id;                 // set this ID into the system state
                enttEditorController_.SetSelectedEntt(id);   // and update the editor to show data of this entt

                // if we do double click on the selectable item we move our camera
                // to this item in world and fix on it
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    pFacade_->FocusCameraOnEntity(id);
                    g_GuiStates.gizmoOperation = 7;   // ImGizmo::OPERATION::TRANSLATE
                }
            }
        }
    }
    ImGui::End();
    g_GuiStates.showWndEnttsList = showWndEnttsList;
}


//---------------------------------------------------------
// Desc:  a little helper for showing checkboxes to turn on/off debug shapes rendering
// NOTE:  I check fucking nothing (input args), so be careful
//---------------------------------------------------------
inline bool CheckboxDbgShape(
    IFacadeEngineToUI* pFacade,
    const char* msg,
    const eRenderDbgShape type,
    bool& flag)
{
    if (ImGui::Checkbox(msg, &flag))
    {
        pFacade->SwitchRenderDebugShape(type, flag);
        return true;
    }
    return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void RenderDbgShapesControl(IFacadeEngineToUI* pFacade)
{
    if (ImGui::TreeNode("Debug shapes"))
    {
        bool rndDbgShapes      = g_GuiStates.renderDbgShapes;
        bool rndDbgLines       = g_GuiStates.renderDbgLines;
        bool rndDbgCross       = g_GuiStates.renderDbgCross;
        bool rndDbgSphere      = g_GuiStates.renderDbgSphere;
        bool rndDbgCircle      = g_GuiStates.renderDbgCircle;

        bool rndDbgAxes        = g_GuiStates.renderDbgAxes;
        bool rndDbgTriangle    = g_GuiStates.renderDbgTriangle;
        bool rndDbgAABB        = g_GuiStates.renderDbgAABB;
        bool rndDbgOBB         = g_GuiStates.renderDbgOBB;
        bool rndDbgFrustum     = g_GuiStates.renderDbgFrustum;
        bool rndDbgTerrainAABB = g_GuiStates.renderDbgTerrainAABB;

        if (CheckboxDbgShape(pFacade, "Draw debug shapes", RENDER_DBG_SHAPES, rndDbgShapes))
            g_GuiStates.renderDbgShapes = rndDbgShapes;

        ImGui::Separator();

        if (CheckboxDbgShape(pFacade, "lines", RENDER_DBG_SHAPES_LINE, rndDbgLines))
            g_GuiStates.renderDbgLines = rndDbgShapes;

        if (CheckboxDbgShape(pFacade, "cross", RENDER_DBG_SHAPES_CROSS, rndDbgCross))
            g_GuiStates.renderDbgCross = rndDbgCross;

        if (CheckboxDbgShape(pFacade, "sphere", RENDER_DBG_SHAPES_SPHERE, rndDbgSphere))
            g_GuiStates.renderDbgSphere = rndDbgSphere;

        if (CheckboxDbgShape(pFacade, "circle", RENDER_DBG_SHAPES_CIRCLE, rndDbgCircle))
            g_GuiStates.renderDbgCircle = rndDbgCircle;

        if (CheckboxDbgShape(pFacade, "axes", RENDER_DBG_SHAPES_AXES, rndDbgAxes))
            g_GuiStates.renderDbgAxes = rndDbgAxes;

        if (CheckboxDbgShape(pFacade, "triangle", RENDER_DBG_SHAPES_TRIANGLE, rndDbgTriangle))
            g_GuiStates.renderDbgTriangle = rndDbgTriangle;

        if (CheckboxDbgShape(pFacade, "AABB", RENDER_DBG_SHAPES_AABB, rndDbgAABB))
            g_GuiStates.renderDbgAABB = rndDbgAABB;

        if (CheckboxDbgShape(pFacade, "OBB", RENDER_DBG_SHAPES_OBB, rndDbgOBB))
            g_GuiStates.renderDbgOBB = rndDbgOBB;

        if (CheckboxDbgShape(pFacade, "frustum", RENDER_DBG_SHAPES_FRUSTUM, rndDbgFrustum))
            g_GuiStates.renderDbgFrustum = rndDbgFrustum;

        if (CheckboxDbgShape(pFacade, "terrain AABB", RENDER_DBG_SHAPES_TERRAIN_AABB, rndDbgTerrainAABB))
            g_GuiStates.renderDbgTerrainAABB = rndDbgTerrainAABB;

        ImGui::TreePop();
    }
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
        ImGui::Text("Camera dir: %.2f %.2f %.2f", camDir.x, camDir.y, camDir.z);

        //-------------------------------------------------

        UI_PostFxControl postFxController;
        postFxController.Draw(pFacade_);

        //-------------------------------------------------

        // show debug options
        if (ImGui::TreeNode("Debug:"))
        {
            debugControl_.Draw(pFacade_);
            ImGui::TreePop();
        }

        RenderDbgShapesControl(pFacade_);
      
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:  a little helper for rendering weather control fields
// NOTE:  I check fucking nothing (input args), so be careful
//---------------------------------------------------------
inline void DragWeatherParam(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eWeatherParam param)
{
    float value = pFacade->GetWeatherParam(param);

    if (ImGui::DragFloat(text, &value, 0.01f))
    {
        pFacade->SetWeatherParam(param, value);
    }
}

//---------------------------------------------------------
// Desc:   render editor elements which are responsible for editing of:
//         sky, scene fog, entities, grass, etc.
//---------------------------------------------------------
void EditorPanels::RenderPropertiesControllerWnd()
{
    bool showWndEnttProperties = g_GuiStates.showWndEnttProperties;

    if (ImGui::Begin("Properties"), &showWndEnttProperties)
    {
        // if we have selected any entity
        if (enttEditorController_.selectedEnttData_.IsSelectedAnyEntt())
            enttEditorController_.Render();
        

        if (ImGui::TreeNodeEx("Weather control", ImGuiTreeNodeFlags_SpanFullWidth))
        {
            fogEditorController_.Draw();

            DragWeatherParam(pFacade_, "wind direction x", WEATHER_WIND_DIR_X);
            DragWeatherParam(pFacade_, "wind direction y", WEATHER_WIND_DIR_Y);
            DragWeatherParam(pFacade_, "wind direction z", WEATHER_WIND_DIR_Z);

            DragWeatherParam(pFacade_, "wind speed",       WEATHER_WIND_SPEED);
            DragWeatherParam(pFacade_, "wind strenght",    WEATHER_WIND_STRENGTH);
            DragWeatherParam(pFacade_, "wave amplitude",   WEATHER_WAVE_AMPLITUDE);
            DragWeatherParam(pFacade_, "turbulence",       WEATHER_TURBULENCE);
            DragWeatherParam(pFacade_, "gust decay",       WEATHER_GUST_DECAY);
            DragWeatherParam(pFacade_, "gust power",       WEATHER_GUST_POWER);
            DragWeatherParam(pFacade_, "wave frequency",   WEATHER_WAVE_FREQUENCY);
            DragWeatherParam(pFacade_, "bend scale",       WEATHER_BEND_SCALE);
            DragWeatherParam(pFacade_, "sway distance",    WEATHER_SWAY_DISTANCE);

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

                // LOD distance control
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
    g_GuiStates.showWndEnttProperties = showWndEnttProperties;
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
