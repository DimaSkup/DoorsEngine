// =================================================================================
// Filename:     UserInterface.cpp
// 
// Created:      25.05.23
// =================================================================================
#include <CoreCommon/pch.h>
#include "user_interface.h"
#include <UICommon/IFacadeEngineToUI.h>

#include <win_file_dialog.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>

#pragma warning(disable : 4996)

namespace UI
{

// --------------------------------------------------------
// UI constructor/desctructor
// --------------------------------------------------------
UserInterface::UserInterface()
{
}

UserInterface::~UserInterface()
{
    // a ptr to facade is created outside of the UI so we just set ptr to NULL
    pFacadeEngineToUI_ = nullptr;

    SafeDelete(pImportModelWndData_);
}


// =================================================================================
// Modification API
// =================================================================================

//---------------------------------------------------------
// Args:   - dbgFontDataFilepath:  a path to datafile about debug font
//         - dbgFontTexFilepath:   a path to texture file for debug font
//---------------------------------------------------------
void UserInterface::Initialize(
    ID3D11Device* pDevice,
    IFacadeEngineToUI* pFacadeEngineToUI,
    const char* dbgFontDataFilepath,      
    const char* dbgFontTexFilepath,
    const char* gameFontDataFilepath,
    const char* gameFontTexFilepath,
    const int wndWidth,
    const int wndHeight,
    const UINT videoCardMemory,
    const std::string& videoCardName)
{
    LogDbg(LOG, "initialization of the User Interface");

    try
    {
        CAssert::True(pFacadeEngineToUI,                        "a ptr to the facade interface == nullptr");
        CAssert::True((wndWidth > 0) && (wndHeight > 0),        "wrong window dimensions");
        CAssert::True(!videoCardName.empty(),                   "video card name is empty");
        CAssert::True(!StrHelper::IsEmpty(dbgFontDataFilepath), "path to debug font data file is empty");
        CAssert::True(!StrHelper::IsEmpty(dbgFontTexFilepath),  "path to debug font texture file is empty");
        CAssert::True(!StrHelper::IsEmpty(dbgFontDataFilepath), "path to game font data file is empty");
        CAssert::True(!StrHelper::IsEmpty(dbgFontTexFilepath),  "path to game font texture file is empty");

        // initialize the window dimensions members for internal using
        windowWidth_ = wndWidth;
        windowHeight_ = wndHeight;

        // --------------------------------------------

        // initialize the first font object
        dbgFont01_.Initialize(pDevice, dbgFontDataFilepath, dbgFontTexFilepath);
        gameFont01_.Initialize(pDevice, gameFontDataFilepath, gameFontTexFilepath);

        // initialize the editor parts and interfaces
        pFacadeEngineToUI_ = pFacadeEngineToUI;
        editorPanels_.Init(pFacadeEngineToUI_);

        // create text strings to show debug info onto the screen
        LoadDebugInfoStringFromFile(pDevice, videoCardName, videoCardMemory);

        textStorage_.Init(pDevice, &dbgFont01_);
        

        LogDbg(LOG, "USER INTERFACE is initialized");
    }
    catch (EngineException& e)
    {
        LogErr(e, false);
        LogErr(LOG, "can't initialize the UserInterface");
    }
}

//---------------------------------------------------------
// Desc:   update the UI states
//---------------------------------------------------------
void UserInterface::Update(
    ID3D11DeviceContext* pContext, 
    const Core::SystemState& systemState)
{
    try
    {
        pFacadeEngineToUI_->deltaTime = systemState.deltaTime;

        // update debug text only when we want to see it
        if (systemState.isShowDbgInfo)
            textStorage_.Update(pContext, systemState);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't update the GUI");
    }
}

//---------------------------------------------------------
// Desc:   check call this func we move to the previous step in the events history;
// 
//         for instance: if we moved some entity at the new position then we
//                       can undo this event and place the entt at the beginning pos
//---------------------------------------------------------
void UserInterface::UndoEditorLastEvent()
{
    if (g_EventsHistory.HasHistory())
    {
        HistoryItem historyItem = g_EventsHistory.Undo();

        editorPanels_.enttEditorController_.UndoCmd(
            &historyItem.cmd_,
            historyItem.entityID_);
    }
}

//---------------------------------------------------------
// Desc:  create a new GUI string by input data;
//        the content of this string won't be changed;
//        you can only change its position on the screen;
// Args:  - text:   text content of the string
//        - drawAt: upper left corner of sentence (x and y position on the screen)
// Ret:   identifier of created sentence
//---------------------------------------------------------
SentenceID UserInterface::AddConstStr(
    ID3D11Device* pDevice,
    const char* text,                         
    const POINT& drawAt)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "input text is empty");
        return 0;
    }

    const DirectX::XMFLOAT2 pos = ComputePosOnScreen(drawAt);
    SentenceID id = textStorage_.AddDebugConstStr(text, pos.x, pos.y);

    if (id == 0)
        LogErr(LOG, "can't create a const sentence: %s", text);

    return id;
}

//---------------------------------------------------------
// Desc:  create a new GUI string by input data;
//        the content of this string is supposed to be changed from frame to frame;
//        you can also change its position on the screen;
// Args:  - key:         semantic key for this debug string
//        - text:        initial text content
//        - drawAt:      upper-left corner position of string
//        - maxStrLen:   max possible length for this dynamic string
//---------------------------------------------------------
SentenceID UserInterface::AddDynamicStr(
    const char* key,
    const char* text,
    const POINT& drawAt,
    const uint maxStrLen)
{
    // check input args
    if (StrHelper::IsEmpty(text))
    {
        LogErr(LOG, "input str is empty");
        return 0;
    }

    if (maxStrLen < (int)strlen(text))
    {
        LogErr(LOG, "input max length (%d) of str is can't be lower than input text length (%d)", (int)maxStrLen, (int)strlen(text));
        LogErr(LOG, "can't create a sentence: %s", text);
        return 0;
    }

    DirectX::XMFLOAT2 drawPos = ComputePosOnScreen(drawAt);

    SentenceID id = textStorage_.AddDebugDynamicStr(
        key,
        text,
        maxStrLen,
        drawPos.x,
        drawPos.y);

    if (id == 0)
        LogErr(LOG, "can't create a dynamic sentence: %s", text);

    return id;
}

//---------------------------------------------------------

void UserInterface::HandleDebugConstStr(const char* buffer)
{
    constexpr int bufsize = 64;
    char str[bufsize]{ '\0' };
    int posX = 0;
    int posY = 0;

    if (EOF != sscanf(buffer, "%*s %s %d %d", str, &posX, &posY))
    {
        const DirectX::XMFLOAT2 drawAt = ComputePosOnScreen({ posX, posY });
        textStorage_.AddDebugConstStr(str, drawAt.x, drawAt.y);
    }
    else
    {
        LogErr(LOG, "the input doen't match the format string");
    }
}

//---------------------------------------------------------

void UserInterface::HandleDebugDynamicStr(const char* buffer)
{
    constexpr int bufsize = 64;
    char str[bufsize]{ '\0' };
    int posX = 0;
    int posY = 0;
    int maxStrSize = 0;

    if (EOF != sscanf(buffer, "%*s %s %d %d %d", str, &posX, &posY, &maxStrSize))
    {
        const DirectX::XMFLOAT2 drawAt = ComputePosOnScreen({ posX, posY });
        textStorage_.AddDebugDynamicStr(str, "0", (uint)maxStrSize, drawAt.x, drawAt.y);
    }
    else
    {
        LogErr(LOG, "the input doen't match the format string");
    }
}

//---------------------------------------------------------
// Desc:   load debug info strings params from the file and create these strings;
//         and create some strings manually
//---------------------------------------------------------
void UserInterface::LoadDebugInfoStringFromFile(
    ID3D11Device* pDevice,
    const std::string& videoCardName,
    const int videoCardMemory)
{
    // generate a path to the file
    char filePath[256]{ '\0' };
    strcat(filePath, g_RelPathUIDataDir);
    strcat(filePath, "ui_debug_info_strings.txt");


    FILE* pFile = fopen(filePath, "r+");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for reading debug strings: %s", filePath);
        return;
    }


    char buffer[64]{ '\0' };
    
    while (fgets(buffer, 64, pFile))
    {
        // read and create a const string (won't be changed during the runtime)
        if (strncmp(buffer, "const_str", 9) == 0)
            HandleDebugConstStr(buffer);

        // read and create a dynamic string (will be changed during the runtime)
        else if (strncmp(buffer, "dynamic_str", 11) == 0)
            HandleDebugDynamicStr(buffer);

        else if (strcmp(buffer, "\n") == 0)
            continue;

        else
            LogErr(LOG, "unknown debug string type (str: %s)", buffer);
    }

    fclose(pFile);


    // create some debug strings about the video card
    sprintf(g_String, "%d MB", videoCardMemory);

    const DirectX::XMFLOAT2 drawAt1 = ComputePosOnScreen({ 150, 10 });
    const DirectX::XMFLOAT2 drawAt2 = ComputePosOnScreen({ 150, 30 });

    textStorage_.AddDebugConstStr(videoCardName.c_str(), drawAt1.x, drawAt1.y);
    textStorage_.AddDebugConstStr(g_String, drawAt2.x, drawAt2.y);
}


// ====================================================================================
// Rendering API
// ====================================================================================
void UserInterface::RenderGameUI(
    ID3D11DeviceContext* pContext,
    Render::CRender& render,
    Core::SystemState& systemState)
{
    // print onto the screen some debug info
    if (systemState.isShowDbgInfo)
        RenderDebugInfo(render, systemState);
}

//---------------------------------------------------------
// Desc:   render the editor's panels and related stuff
//---------------------------------------------------------
void UserInterface::RenderEditor(Core::SystemState& systemState)
{
    // "Run scene" docked window
    if (ImGui::Begin("Run scene"))
    {
        // hide the tab bar of this window
        if (ImGui::IsWindowDocked())
        {
            ImGuiWindow* pWnd = ImGui::FindWindowByName("Run scene");
            if (pWnd)
            {
                ImGuiDockNode* pNode = pWnd->DockNode;
                if (pNode && (!pNode->IsHiddenTabBar()))
                {
                    pNode->WantHiddenTabBarToggle = true;
                }
            }
        }

        // show the button which is used to run the game mode
        ImGui::Button("Run", { 50, 30 });
    }
    ImGui::End();


    editorMainMenuBar_.RenderBar();

    // show window for models importing
    if (g_GuiStates.showWndImportModels)
    {
        // always center this window when appearing
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        const ImVec2 size   = ImGui::GetMainViewport()->Size / 3;
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

        bool showWndImportModels = g_GuiStates.showWndImportModels;

        if (ImGui::Begin("Model Import", &showWndImportModels))
        {
            // alloc memory for import related data
            if (pImportModelWndData_ == nullptr)
            {
                pImportModelWndData_ = NEW ImportModelWndData();
                assert(pImportModelWndData_ != nullptr);
                printf("mem is alloc\n");
            }


            ImGui::InputText(
                "Model name",
                pImportModelWndData_->modelName,
                MAX_LEN_MODEL_NAME);

            if (ImGui::Button("Select file"))
            {
                std::string pathToModel;
                DialogWndFileOpen(pathToModel);
                strncpy(pImportModelWndData_->filePath, pathToModel.c_str(), 1024);
            }

            ImGui::SameLine();

            // if we have any model selected
            if (pImportModelWndData_->filePath[0] != '\0')
            {
                ImGui::Text("%s", pImportModelWndData_->filePath);
            }

            ImGui::NewLine();

            if (ImGui::Button("Import"))
            {
                char* name = pImportModelWndData_->modelName;
                char* path = pImportModelWndData_->filePath;

                printf("import model:\n");
                printf("name: %s\n", name);
                printf("path: %s\n", path);

                if (!pFacadeEngineToUI_->ImportModelFromFile(path, name))
                {
                    LogErr(LOG, "can't import a model from file: %s", path);
                }
            }
        }
        ImGui::End();
        g_GuiStates.showWndImportModels = showWndImportModels;

        // after closing the window
        if (!showWndImportModels)
        {
            SafeDelete(pImportModelWndData_);
            assert(pImportModelWndData_ == nullptr);
            printf("Wnd is closed (Import model)\n");
        }
    }

    // show window to control engine options
    if (g_GuiStates.showWndEngineOptions)
    {
        bool showWndEngineOptions = g_GuiStates.showWndEngineOptions;
        editorMainMenuBar_.RenderWndEngineOptions(&showWndEngineOptions);
        g_GuiStates.showWndEngineOptions = showWndEngineOptions;
    }

    editorPanels_.Render(systemState);
}

//---------------------------------------------------------
// render the scene screen space window and gizmos (if we selected any entity)
//---------------------------------------------------------
void UserInterface::RenderSceneWnd(Core::SystemState& sysState)
{
    if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove))
    {
        isSceneWndHovered_ = ImGui::IsWindowHovered();

        // gizmo stuff
        const EntityID selectedEntt       = GetSelectedEntt();
        const bool anyOperationIsSelected = (g_GuiStates.gizmoOperation != -1);
        
        if (selectedEntt && anyOperationIsSelected)
            RenderAndHandleGizmo(selectedEntt, sysState);
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   render gizmo manipulators according to chosen manipulation type
//         and handle manipulations over selected entt if we execute any
//---------------------------------------------------------
void UserInterface::RenderAndHandleGizmo(
    const EntityID selectedEntt,
    Core::SystemState& sysState)
{
    // set rendering of the gizmos only in the screen window space:
    // to make gizmo be rendered behind editor panels BUT in this case the gizmo is inactive :(
    //ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());                

    g_GuiStates.isGizmoHovered = ImGuizmo::IsOver();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(0, 0, (float)sysState.wndWidth_, (float)sysState.wndHeight_);
    

    const Vec3  p = pFacadeEngineToUI_->GetEnttPosition(selectedEntt);
    const float s = pFacadeEngineToUI_->GetEnttScale(selectedEntt);

    DirectX::XMMATRIX world =
    {
        s,      0,    0,   0,
        0,      s,    0,   0,
        0,      0,    s,   0,
        p.x,  p.y,  p.z,   1
    };

    float*         rawWorld = world.r->m128_f32;
    const float* cameraView = sysState.cameraView.r->m128_f32;
    const float* cameraProj = sysState.cameraProj.r->m128_f32;


    // setup current snapping params (if we turn on snapping)
    if (g_GuiStates.useSnapping)
    {
        switch (g_GuiStates.gizmoOperation)
        {
            case ImGuizmo::TRANSLATE:
            {
                g_GuiStates.snapX = g_GuiStates.snapTranslationX;
                g_GuiStates.snapY = g_GuiStates.snapTranslationY;
                g_GuiStates.snapZ = g_GuiStates.snapTranslationZ;
                ImGui::InputFloat3("Translation Snap", &g_GuiStates.snapX);
                break;
            }
            case ImGuizmo::ROTATE:
            {
                g_GuiStates.snapX = g_GuiStates.snapRotationX;
                g_GuiStates.snapY = g_GuiStates.snapRotationY;
                g_GuiStates.snapZ = g_GuiStates.snapRotationZ;
                ImGui::InputFloat("Angle Snap", &g_GuiStates.snapX);
                break;
            }
            case ImGuizmo::SCALE:
            {
                g_GuiStates.snapX = g_GuiStates.snapScaleX;
                g_GuiStates.snapY = g_GuiStates.snapScaleY;
                g_GuiStates.snapZ = g_GuiStates.snapScaleZ;
                ImGui::InputFloat("Scale Snap", &g_GuiStates.snapX);
                break;
            }
        }
    }

    //ImGuizmo::MODE gizmoMode = (guiStates_.gizmoOperation_ == ImGuizmo::OPERATION::ROTATE) ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;

    ImGuizmo::Manipulate(
        cameraView,
        cameraProj,
        ImGuizmo::OPERATION(g_GuiStates.gizmoOperation),
        ImGuizmo::MODE::WORLD,
        rawWorld,
        NULL,
        (g_GuiStates.useSnapping) ? &g_GuiStates.snapX : NULL);

    if (ImGuizmo::IsUsingAny())
        editorPanels_.enttEditorController_.UpdateSelectedEnttWorld(world);
}
 
//---------------------------------------------------------
// Desc:   render engine/game stats as text onto the sceen when hit F3 key
//         (is used only when we in the game mode)
//---------------------------------------------------------
void UserInterface::RenderDebugInfo(
    Render::CRender& render,
    const Core::SystemState& sysState)
{
    // prepare buffers for rendering
    ID3D11Buffer*  vertexBuffers[2]{ nullptr };
    ID3D11Buffer*  pIB = nullptr;         // IB is common for both vertex buffers
    u32            indexCounts[2]{ 0 };
    constexpr size numSentences = 2;      // we have only two text buffers for the whole debug text: one for const sentences, one for dynamic sentences

    textStorage_.GetRenderingData(
        &vertexBuffers[0],                // vb: debug const text
        &vertexBuffers[1],                // vb: debug dynamic text
        &pIB,
        indexCounts[0],                   // actual index count for debug const text
        indexCounts[1]);                  // actual index count for debug dynamic text

    // render
    render.RenderFont(
        vertexBuffers,
        pIB,
        indexCounts,
        numSentences,
        sizeof(Core::VertexFont));
}


// ====================================================================================
//                                   Helpers
// ====================================================================================

//---------------------------------------------------------
// in:  top left pos relatively to the top left corner of the screen
// out: top left pos relatively to the screen center
//---------------------------------------------------------
DirectX::XMFLOAT2 UserInterface::ComputePosOnScreen(const POINT& drawAt)
{
    return
    {
        (float)(-(windowWidth_  >> 1) + drawAt.x),   // posX
        (float)(+(windowHeight_ >> 1) - drawAt.y),   // posY
    };
}

} // namespace UI
