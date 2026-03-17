// =================================================================================
// Filename:     ImGuiLayer.h
// Description:  a little layer btw ImGui and the Doors Engine
// 
// Created:      16.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <d3d11.h>
#include <imgui.h>


namespace Core
{

class ImGuiLayer
{
public:
	ImGuiLayer();

	bool Init(HWND hwnd, ID3D11Device* pDevice, ID3D11DeviceContext* pCtx);
	void Shutdown();

	void Begin();
	void End();

	void SetDarkThemeColors();
	void SetDefaultThemeColors();

	inline ImVec4        GetBgColor() const { return mainBgColor_; }
	inline ImGuiContext* GetContext()       { return pImGuiContext_; }

private:
	// we use this color to set ImGui background color when rendering
	// because after rendering we have to reset ImGui windowBgColor
	ImVec4        mainBgColor_ = { 0,0,0,1 };  
	ImGuiContext* pImGuiContext_ = nullptr;
};

} // namespace Core
