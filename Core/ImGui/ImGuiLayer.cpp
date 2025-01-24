// =================================================================================
// Filename:     ImGuiLayer.cpp
// Description:  implementation of the ImGuiLayer functional
// 
// Created:      16.01.25  by DimaSkup
// =================================================================================
#include "ImGuiLayer.h"

#include "../Common/FileSystemPaths.h"

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>


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
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.WantCaptureMouse = true;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// setup fonts
	const std::string defaultFont = g_DataDir + "ui/arial.ttf";
	io.Fonts->AddFontFromFileTTF(defaultFont.c_str(), 16.0f);
	io.FontDefault = io.Fonts->AddFontFromFileTTF(defaultFont.c_str(), 16.0f);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();


	SetDefaultThemeColors();

	// setup ImGuizmo
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::Enable(true);

	ImGuizmo::Style& imGuizmoStyle = ImGuizmo::GetStyle();
	imGuizmoStyle.RotationLineThickness = 6;
	imGuizmoStyle.RotationOuterLineThickness = 6;
	imGuizmoStyle.TranslationLineThickness = 6;
	imGuizmoStyle.TranslationLineArrowSize = 9;
	imGuizmoStyle.ScaleLineThickness = 6;
	imGuizmoStyle.ScaleLineCircleSize = 9;

	

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
	/*
	// start the Dear ImGui frame
	ImGuiIO& io = ImGui::GetIO();

	io.MousePos.x = mouse_.GetPosX();
	io.MousePos.y = mouse_.GetPosY();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	*/

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();





}

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

	// text
	float colorText_[3] = { 1.0f, 1.0f, 1.0f };

	// background
	float colorWindowBg_[3] = { 0.23f, 0.23f, 0.23f };
	float colorChildBg_[3] = { 0.23f, 0.23f, 0.23f };
	float colorPopupBg_[3] = { 0.23f, 0.23f, 0.23f };
	float colorFrameBg_[3] = { 0.43f, 0.43f, 0.43f };  // Background of checkbox, radio button, plot, slider, text input
	float colorFrameBgHovered_[3] = { 0.33f, 0.33f, 0.33f };
	float colorTitleBg_[3] = { 0.13f, 0.13f, 0.13f };  // title background of the main window

	float colorBorder_[3] = { 0.0f, 0.0f, 0.0f };

}

///////////////////////////////////////////////////////////

ImVec4 operator-(const ImVec4& v1, float s)
{
	return ImVec4{ v1.x - s, v1.y - s, v1.z - s, 1.0f };
}

void ImGuiLayer::SetDefaultThemeColors()
{
	// setup GUI colors

	ImVec4* colors = ImGui::GetStyle().Colors;
	

	// main bg colors
	const ImVec4 black = { 0,0,0,1 };
	const ImVec4 white = { 1,1,1,1 };
	const ImVec4 mainColor = { 0.0f, 0.35f, 0.268f, 1.0f };
	mainBgColor_ = mainColor;

	// frame colors
	const ImVec4 frameBgColor = { 0.007f, 0.450f, 0.368f, 1.0f };
	const ImVec4 frameBgHovered = { 0.057f, 0.50f, 0.418f, 1.0f };


	colors[ImGuiCol_Text] = white;

	colors[ImGuiCol_WindowBg] = { 0,0,0,0 };
	colors[ImGuiCol_ChildBg]  = mainColor;
	colors[ImGuiCol_PopupBg]  = mainColor;
	colors[ImGuiCol_FrameBg]  = frameBgColor;
	colors[ImGuiCol_FrameBgHovered] = frameBgHovered;

	colors[ImGuiCol_TitleBg] = mainColor;
	colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBg];

	colors[ImGuiCol_Tab] = mainColor;
	colors[ImGuiCol_TabSelected] = frameBgHovered;
	colors[ImGuiCol_TabSelectedOverline] = frameBgHovered;

	colors[ImGuiCol_TabDimmedSelected] = mainColor;

	colors[ImGuiCol_Border] = black;

	colors[ImGuiCol_TabHovered]        = { 0.21f, 0.50f, 0.44f, 1.0f };
	colors[ImGuiCol_TabDimmedSelected] = { 0.16f, 0.45f, 0.39f, 1.0f };
}

///////////////////////////////////////////////////////////