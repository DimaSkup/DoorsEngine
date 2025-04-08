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

struct StatesGUI
{
    // flags to show windows
    bool showWndEngineOptions = false;
    bool showWndEnttCreation  = false;
    bool showWndEnttsList = true;
    bool showWndEnttProperties = true;


    // browsers stuff
    bool showWndModelsBrowser = false;
    bool showWndTexturesBrowser = false;
    bool showWndMaterialsBrowser = false;

    // gizmo stuff
    bool useSnapping          = false;   // use stride by some fixed value when we transform with guizmo or with fields
    bool isGizmoHovered = false;    // is any gizmo manipulator is hovered by a mouse
    bool isGizmoClicked = false;    // is any gizmo manipulator is clicked by a mouse

    DirectX::XMFLOAT3 snapTranslation = { 0.1f, 0.1f, 0.1f };
    DirectX::XMFLOAT3 snapRotation    = { 45, 45, 45 };
    DirectX::XMFLOAT3 snapScale       = { 0.1f, 0.1f, 0.1f };
    DirectX::XMFLOAT3 snap            = { 0,0,0 };


    int gizmoOperation = -1;       // none gizmo operation is chosen
    int gizmoMode      = 1;        // world model
    
};

} // namespace UI
