// =================================================================================
// Filename:     StatesGUI.h
// Description:  GUI states container
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <cstdint>
#include <DirectXMath.h>

namespace UI
{

class StatesGUI
{
public:
	StatesGUI() {}


public:
	// flags to show windows
	bool showWndEngineOptions_ = false;
	bool showWndAssetsControl_ = false;
	bool showWndEnttCreation_  = false;

	bool useSnapping_          = false;   // use stride by some fixed value when we transform with guizmo or with fields
	DirectX::XMFLOAT3 snapTranslation_ = { 0.1f, 0.1f, 0.1f };
	DirectX::XMFLOAT3 snapRotation_    = { 45, 45, 45 };
	DirectX::XMFLOAT3 snapScale_       = { 0.1f, 0.1f, 0.1f };
	DirectX::XMFLOAT3 snap_            = { 0,0,0 };


	int gizmoOperation_ = -1;       // none gizmo operation is chosen
	int gizmoMode_      = 1;        // world model
	int isGizmoHovered_ = false;    // is any gizmo manipulator is hovered by a mouse
	int isGizmoClicked_ = false;    // is any gizmo manipulator is clicked by a mouse
};

} // namespace UI
