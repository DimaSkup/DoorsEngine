// =================================================================================
// Filename:     StatesGUI.h
// Description:  GUI states container
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#pragma once

namespace UI
{

struct StatesGUI
{
    // flags to show windows
    bool showWndEngineOptions       = false;
    bool showWndEnttCreation        = false;
    bool showWndEnttsList           = true;
    bool showWndEnttProperties      = true;

    // browsers stuff
    bool showWndModelsBrowser       = false;
    bool showWndTexturesBrowser     = true;
    bool showWndMaterialsBrowser    = true;

    bool needUpdateMatBrowserIcons  = false;        // do we want to update icons in the materials browser?
    bool needUpdateTexBrowserIcons  = false;        // do we want to update icons in the textures browser?

    // gizmo stuff
    bool useSnapping                = false;        // use stride by some fixed value when we transform with guizmo or with fields
    bool isGizmoHovered             = false;        // is any gizmo manipulator is hovered by a mouse
    bool isGizmoClicked             = false;        // is any gizmo manipulator is clicked by a mouse

    float snapTranslationX = 0.1f;
    float snapTranslationY = 0.1f;
    float snapTranslationZ = 0.1f;

    // rotation in degrees
    float snapRotationX = 45;
    float snapRotationY = 45;
    float snapRotationZ = 45;

    float snapScaleX = 0.1f;
    float snapScaleY = 0.1f;
    float snapScaleZ = 0.1f;

    // current snapping values (are changed when we select any gizmo type: translation, rotation, scale)
    float snapX = 0;
    float snapY = 0;
    float snapZ = 0;

    int gizmoOperation = -1;       // none gizmo operation is chosen
    int gizmoMode      = 1;        // world model
};

//=========================================================
// GLOBAL instance of the GUI states container
//=========================================================
extern StatesGUI g_GuiStates;

} // namespace UI
