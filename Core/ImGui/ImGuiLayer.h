// =================================================================================
// Filename:     ImGuiLayer.h
// Description:  a little layer btw ImGui and the Doors Engine
// 
// Created:      16.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <d3d11.h>
#include <imgui.h>

class ImGuiLayer
{
public:
	ImGuiLayer();

	void Initialize(HWND hwnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Shutdown();

	void Begin();
	void End();

	void SetDarkThemeColors();
	void SetDefaultThemeColors();

	ImVec4 GetBackgroundColor() const { return mainBgColor_; }

private:
	// we use this color to set ImGui background color when rendering
	// because after rendering we have to reset ImGui windowBgColor
	ImVec4 mainBgColor_ = { 0,0,0,1 };  
};