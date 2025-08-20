// =================================================================================
// Filename:     UserInterface.cpp
// 
// Created:      25.05.23
// =================================================================================
#include <CoreCommon/pch.h>
#include "UserInterface.h"
#include "../Texture/TextureMgr.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>

#pragma warning(disable : 4996)

namespace UI
{

// --------------------------------------------------------
// init global instance of the EventsHistory container
// --------------------------------------------------------
EventsHistory g_EventsHistory;

// --------------------------------------------------------
// UI constructor/desctructor
// --------------------------------------------------------
UserInterface::UserInterface() : editorPanels_(&guiStates_)
{
}

UserInterface::~UserInterface()
{
    pFacadeEngineToUI_ = nullptr;
}


// =================================================================================
// Modification API
// =================================================================================
void UserInterface::Initialize(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    IFacadeEngineToUI* pFacadeEngineToUI,
    const std::string& fontDataFilePath,      // a path to file with data about this type of font
    const std::string& fontTextureFilePath,   // a path to texture file for this font
    const int wndWidth,
    const int wndHeight,
    const UINT videoCardMemory,
    const std::string& videoCardName)
{
    // initialize the graphics user interface (GUI)

    LogDbg(LOG, "initialization of the User Interface");

    try
    {
        CAssert::True(pFacadeEngineToUI,                                 "a ptr to the facade interface == nullptr");
        CAssert::True((wndWidth > 0) && (wndHeight > 0),                 "wrong window dimensions");
        CAssert::True((!videoCardName.empty()) && (videoCardMemory > 0), "wrong video card data");
        CAssert::True((!fontDataFilePath.empty()),                       "wrong path to font data file");
        CAssert::True((!fontTextureFilePath.empty()),                    "wrong path to font texture file");

        // initialize the window dimensions members for internal using
        windowWidth_ = wndWidth;
        windowHeight_ = wndHeight;

        // --------------------------------------------

        // initialize the first font object
        font1_.Initialize(pDevice, fontDataFilePath.c_str(), fontTextureFilePath.c_str());

        // initialize the editor parts and interfaces
        pFacadeEngineToUI_ = pFacadeEngineToUI;
        editorPanels_.Initialize(pFacadeEngineToUI_);

        // create text strings to show debug info onto the screen
        LoadDebugInfoStringFromFile(pDevice, videoCardName, videoCardMemory);

        textStorage_.InitDebugText(pDevice, font1_);


        LogDbg(LOG, "USER INTERFACE is initialized");
    }
    catch (EngineException& e)
    {
        LogErr(e, false);
        LogErr("can't initialize the UserInterface");
    }
}

///////////////////////////////////////////////////////////

void UserInterface::Update(
    ID3D11DeviceContext* pContext, 
    const Core::SystemState& systemState)
{
    // each frame we call this function for updating the UI
    try
    {
        pFacadeEngineToUI_->deltaTime = systemState.deltaTime;

        // update debug text only when we want to see it
        if (systemState.isShowDbgInfo)
            textStorage_.Update(pContext, font1_, systemState);
    }
    catch (EngineException & e)
    {
        LogErr(e);
        LogErr("can't update the GUI");
    }
}

///////////////////////////////////////////////////////////

void UserInterface::UndoEditorLastEvent()
{
    // move to the previous step in the events history;
    // 
    // for instance: if we moved some model at the new position then we
    //               can undo this event and place the model at the beginning position


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
SentenceID UserInterface::CreateConstStr(
    ID3D11Device* pDevice,
    const char* text,                         
    const POINT& drawAt)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "input text is empty");
        return 0;
    }

    try
    {
        const DirectX::XMFLOAT2 pos = ComputePosOnScreen(drawAt);
        return textStorage_.CreateConstSentence(pDevice, font1_, text, pos);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't create the sentence: %s", text);
        return 0;
    }
}

//---------------------------------------------------------
// Desc:  create a new GUI string by input data;
//        the content of this string is supposed to be changed from frame to frame;
//        you can also change its position on the screen;
//---------------------------------------------------------
SentenceID UserInterface::CreateDynamicStr(
    ID3D11Device* pDevice,
    const char* text,
    const POINT& drawAt,
    const int maxStrLen)
{
    try
    {
        CAssert::True(text && text[0] != '\0',           "input string is empty");
        CAssert::True(maxStrLen >= (size)(strlen(text)), "input max len of str is can't be lower than input text length");

        return textStorage_.CreateSentence(
            pDevice,
            font1_,
            text,
            maxStrLen,
            ComputePosOnScreen(drawAt),
            true);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't create a sentence: %s", text);
        LogErr(e);
        LogErr(g_String);
        return 0;
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

    constexpr int bufsize = 64;
    char buffer[bufsize]{ '\0' };
    char str[bufsize]{ '\0' };
    int posX = 0;
    int posY = 0;
    int maxStrSize = 0;

    while (fgets(buffer, bufsize, pFile))
    {
        // we read and create a const string (won't be changed during the runtime)
        if (strncmp(buffer, "const_str", 9) == 0)
        {
            if (EOF != sscanf(buffer, "%*s %s %d %d", str, &posX, &posY))
            {
                const DirectX::XMFLOAT2 drawAt = ComputePosOnScreen({ posX, posY });
                textStorage_.AddDebugConstSentence("const_str", str, drawAt.x, drawAt.y);
            }
            else
            {
                LogErr("the input doen't match the format string");
            }
        }
        // we read and create a dynamic string (will be changed during the runtime)
        else if (strncmp(buffer, "dynamic_str", 11) == 0)
        {
            if (EOF != sscanf(buffer, "%*s %s %d %d %d", str, &posX, &posY, &maxStrSize))
            {
                const DirectX::XMFLOAT2 drawAt = ComputePosOnScreen({ posX, posY });
                textStorage_.AddDebugDynamicSentence(str, "0", drawAt.x, drawAt.y);
            }
            else
            {
                LogErr("the input doen't match the format string");
            }
        }
    } // while

    fclose(pFile);

    // create some debug strings about the video card
    sprintf(g_String, "%d MB", videoCardMemory);

    const DirectX::XMFLOAT2 drawAt1 = ComputePosOnScreen({ 150, 10 });
    const DirectX::XMFLOAT2 drawAt2 = ComputePosOnScreen({ 150, 30 });

    textStorage_.AddDebugConstSentence("GPU_name", videoCardName.c_str(), drawAt1.x, drawAt1.y);
    textStorage_.AddDebugConstSentence("GPU_memory", g_String, drawAt2.x, drawAt2.y);
}


// ====================================================================================
// Rendering API
// ====================================================================================
void UserInterface::RenderGameUI(
    ID3D11DeviceContext* pContext,
    Render::FontShader& fontShader,
    Core::SystemState& systemState)
{
    // print onto the screen some debug info
    if (systemState.isShowDbgInfo)
        RenderDebugInfo(pContext, fontShader, systemState);
}

///////////////////////////////////////////////////////////

void UserInterface::RenderEditor(Core::SystemState& systemState)
{
    // Render the editor's panels and related stuff

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


    editorMainMenuBar_.RenderBar(guiStates_);

    // show window to control engine options
    if (guiStates_.showWndEngineOptions)
        editorMainMenuBar_.RenderWndEngineOptions(&guiStates_.showWndEngineOptions);

    editorPanels_.Render(systemState);
}

///////////////////////////////////////////////////////////

void UserInterface::RenderSceneWnd(Core::SystemState& sysState)
{
    // render the scene screen space window and gizmos (if we selected any entity)

    if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove))
    {
        // set this flags to true if mouse is currently over the wnd
        // so then we can use it to check if we clicked on the 
        // scene screen space or clicked on some editor panel
        isSceneWndHovered_ = ImGui::IsWindowHovered();

        //
        // Gizmos
        //
        EnttEditorController& controller = editorPanels_.enttEditorController_;
        uint32_t selectedEntt = GetSelectedEntt();
        
        // if any entt is selected and any gizmo operation is chosen
        if (selectedEntt && (guiStates_.gizmoOperation != -1))    
        {
            // is any gizmo manipulator hovered by a mouse
            guiStates_.isGizmoHovered = ImGuizmo::IsOver();

            // set rendering of the gizmos only in the screen window space:
            // to make gizmo be rendered behind editor panels BUT in this case the gizmo is inactive :(
            //ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());                

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(0, 0, (float)sysState.wndWidth_, (float)sysState.wndHeight_);

            const float* cameraView = sysState.cameraView.r->m128_f32;
            const float* cameraProj = sysState.cameraProj.r->m128_f32;

            // selected entity transformation using the gizmo
            DirectX::XMMATRIX world;
            float* rawWorld = nullptr;
      

            // handle directed and spot lights in a separate way for correct change
            // of its direction using gizmo
            const Vec3 pos = pFacadeEngineToUI_->GetEnttPosition(selectedEntt);

            world       = DirectX::XMMatrixIdentity();
            world.r[3]  = {pos.x, pos.y, pos.z, 1.0f};
            rawWorld    = world.r->m128_f32;


            if (guiStates_.useSnapping)
            {
                switch (guiStates_.gizmoOperation)
                {
                    case ImGuizmo::TRANSLATE:
                    {
                        guiStates_.snap = guiStates_.snapTranslation;
                        ImGui::InputFloat3("Translation Snap", &guiStates_.snap.x);
                        break;
                    }
                    case ImGuizmo::ROTATE:
                    {
                        guiStates_.snap = guiStates_.snapRotation;
                        ImGui::InputFloat("Angle Snap", &guiStates_.snap.x);
                        break;
                    }
                    case ImGuizmo::SCALE:
                    {
                        guiStates_.snap = guiStates_.snapScale;
                        ImGui::InputFloat("Scale Snap", &guiStates_.snap.x);
                        break;
                    }
                }
            }

            //ImGuizmo::MODE gizmoMode = (guiStates_.gizmoOperation_ == ImGuizmo::OPERATION::ROTATE) ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
            

            ImGuizmo::Manipulate(
                cameraView,
                cameraProj,
                ImGuizmo::OPERATION(guiStates_.gizmoOperation),
                ImGuizmo::MODE::WORLD,
                rawWorld,
                NULL,
                (guiStates_.useSnapping) ? &guiStates_.snap.x : NULL);

            // if we do some manipulations using guizmo
            if (ImGuizmo::IsUsingAny())
            {
                controller.UpdateSelectedEnttWorld(world);
            }
        }
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void UserInterface::RenderDebugInfo(
    ID3D11DeviceContext* pContext,
    Render::FontShader& fontShader,
    const Core::SystemState& sysState)
{
    // render engine/game stats as text onto the sceen
    // (is used when we in the game mode)

    // receive a font texture SRV 
    ID3D11ShaderResourceView* const* ppFontTexSRV = font1_.GetTextureResourceViewAddress();
    
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
    fontShader.Render(
        pContext,
        vertexBuffers,
        pIB,
        indexCounts,
        numSentences,
        sizeof(Core::VertexFont),
        ppFontTexSRV);
}


// ====================================================================================
//                                   Helpers
// ====================================================================================
DirectX::XMFLOAT2 UserInterface::ComputePosOnScreen(const POINT& drawAt)
{
    // in:  top left pos relatively to the top left corner of the screen
    // out: top left pos relatively to the screen center
    return
    {
        (float)(-(windowWidth_  >> 1) + drawAt.x),   // posX
        (float)(+(windowHeight_ >> 1) - drawAt.y),   // posY
    };
}

} // namespace UI
