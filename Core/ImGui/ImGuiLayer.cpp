// =================================================================================
// Filename:     ImGuiLayer.cpp
// Description:  implementation of the ImGuiLayer functional
// 
// Created:      16.01.25  by DimaSkup
// =================================================================================
#include "ImGuiLayer.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/Assert.h>

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

namespace fs = std::filesystem;


namespace Core
{

ImGuiLayer::ImGuiLayer()
{
}

void ImGuiLayer::Initialize(
    HWND hwnd,
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext)
{
    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    pImGuiContext_ = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.WantCaptureMouse = true;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // setup fonts
    const std::string defaultFont = g_RelPathUIDataDir + "arial.ttf";

    // check if such font file exists
    const bool exist = fs::exists(defaultFont);
    Assert::True(exist, "there is no such font file: " + defaultFont);

    io.Fonts->AddFontFromFileTTF(defaultFont.c_str(), 16.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF(defaultFont.c_str(), 16.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();


    SetDefaultThemeColors();

    // setup ImGuizmo
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::Enable(true);
    ImGuizmo::AllowAxisFlip(false);

    ImGuizmo::Style& imGuizmoStyle = ImGuizmo::GetStyle();
    const float lineThickness = 6.0f;

    imGuizmoStyle.RotationLineThickness         = lineThickness;
    imGuizmoStyle.RotationOuterLineThickness    = lineThickness;
    imGuizmoStyle.TranslationLineThickness      = lineThickness;
    imGuizmoStyle.TranslationLineArrowSize      = 12;
    imGuizmoStyle.ScaleLineThickness            = lineThickness;
    imGuizmoStyle.ScaleLineCircleSize           = lineThickness;


    // setup platform/renderer backeds
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pDevice, pContext);

    RECT winRect;
    GetWindowRect(hwnd, &winRect);

    ImGuiViewport* pViewport = ImGui::GetMainViewport();

    ImVec2 wndPos = { 0,0 }; //{ (float)winRect.left, (float)winRect.top };
    ImVec2 wndSize = { (float)(winRect.right - winRect.left), (float)(winRect.bottom - winRect.top) };

    pViewport->Pos = wndPos;
    pViewport->WorkPos = wndPos;

    pViewport->Size = wndSize;
    pViewport->WorkSize = wndSize;
}

///////////////////////////////////////////////////////////

void ImGuiLayer::Shutdown()
{
    // cleanup Dear ImGui
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

///////////////////////////////////////////////////////////

void ImGuiLayer::Begin()
{
    // start the Dear ImGui frame

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();


    //
    // ImGui docking
    //
    static bool isDockspaceOpen = true;
    static bool optFullscreen = true;
    static bool optPadding = false;
    static ImGuiDockNodeFlags_ dockspaceFlags = ImGuiDockNodeFlags_None;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    static ImGuiDockNode* pSceneNode = nullptr;

    // setup the dockspace wnd style
    ImGuiWindowFlags wndFlags = 0;
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    wndFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    wndFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
    {
        wndFlags |= ImGuiWindowFlags_NoTitleBar;
    }


    // push this style since we want to disable padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Root", &isDockspaceOpen, wndFlags);
    ImGui::PopStyleVar(3);

    // DockSpace 
    // NOTE: it is supposed that docking is always enalbed: (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == true

    ImGuiID dockspaceId = ImGui::GetID("Root");
    ImGui::DockSpace(dockspaceId, viewport->WorkSize, dockspaceFlags);

    // setup docked windows positions and sizes
    static auto firstTime = true;
    if (firstTime)
    {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

        // split main dockspace into separate dock windows
        ImGuiID dockIdRight            = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.2f, nullptr, &dockspaceId);
        ImGuiID dockIdRightBottomHalf  = ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Down, 0.5f, nullptr, &dockIdRight);
        ImGuiID dockIdBottom           = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.3f, nullptr, &dockspaceId);
        ImGuiID dockIdLeft             = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.2f, nullptr, &dockspaceId);
        ImGuiID dockIdScene            = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.89f, nullptr, &dockspaceId);

        // register dock windows and relate them to specific window names
        ImGui::DockBuilderDockWindow("Debug", dockIdRight);
        ImGui::DockBuilderDockWindow("Properties", dockIdRightBottomHalf);
        ImGui::DockBuilderDockWindow("Log", dockIdBottom);
        ImGui::DockBuilderDockWindow("Entities List", dockIdLeft);
        ImGui::DockBuilderDockWindow("Scene", dockIdScene);
        ImGui::DockBuilderDockWindow("Run scene", dockspaceId);
        ImGui::DockBuilderDockWindow("Models", dockIdBottom);
        ImGui::DockBuilderDockWindow("Textures browser", dockIdBottom);
        ImGui::DockBuilderDockWindow("Events history", dockIdBottom);

        pSceneNode = ImGui::DockBuilderGetNode(dockIdScene);

        ImGui::DockBuilderFinish(dockspaceId);
    }
}

///////////////////////////////////////////////////////////

void ImGuiLayer::End()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

///////////////////////////////////////////////////////////

void ImGuiLayer::SetDarkThemeColors()
{
    //
    // TODO
    //
    assert(0 && "TODO: IMPELEMNT IT");
}

///////////////////////////////////////////////////////////

ImVec4 operator-(const ImVec4& v1, float s)
{
    return ImVec4{ v1.x - s, v1.y - s, v1.z - s, 1.0f };
}

///////////////////////////////////////////////////////////

void ImGuiLayer::SetDefaultThemeColors()
{
    // setup GUI colors

    ImVec4* colors = ImGui::GetStyle().Colors;

    // main bg colors
    constexpr ImVec4 black      = { 0,0,0,1 };
    constexpr ImVec4 white      = { 1,1,1,1 };
    constexpr ImVec4 mainColor  = { 0.16f, 0.15f, 0.19f, 1.0f };

    // we need to store this value for switching btw opaque and transparent editor background
    mainBgColor_ = mainColor;

    // frame colors
    constexpr ImVec4 frameBgColor   = { 0.21f, 0.20f, 0.24f, 1.0f };
    constexpr ImVec4 frameBgHovered = { 0.26f, 0.25f, 0.29f, 1.0f };


    colors[ImGuiCol_Text] = white;

    colors[ImGuiCol_WindowBg]       = { 0,0,0,0 };
    colors[ImGuiCol_ChildBg]        = mainColor;
    colors[ImGuiCol_PopupBg]        = mainColor;
    colors[ImGuiCol_FrameBg]        = frameBgColor;
    colors[ImGuiCol_FrameBgHovered] = frameBgHovered;

    colors[ImGuiCol_TitleBg]        = mainColor;
    colors[ImGuiCol_TitleBgActive]  = frameBgColor;

    colors[ImGuiCol_Tab]            = mainColor;
    colors[ImGuiCol_TabSelected]    = { 0.31f, 0.30f, 0.34f, 1.0f };
    colors[ImGuiCol_TabSelectedOverline] = frameBgHovered;

    colors[ImGuiCol_Border]         = black;
    colors[ImGuiCol_Button]         = { 0.27f, 0.20f, 0.30f, 1.0f };

    colors[ImGuiCol_TabHovered]        = { 0.21f, 0.20f, 0.24f, 1.0f };
    colors[ImGuiCol_TabDimmedSelected] = { 0.26f, 0.25f, 0.29f, 1.0f };
}

} // namespace Core
