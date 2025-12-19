// =================================================================================
// Filename:     StatesGUI.h
// Description:  editor GUI states container
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <types.h>

namespace UI
{

struct StatesGUI
{
    // flags to show windows
    uint32 showWndEngineOptions         : 1 = false;
    uint32 showWndEnttCreation          : 1 = false;
    uint32 showWndEnttsList             : 1 = true;
    uint32 showWndEnttProperties        : 1 = true;
    uint32 showWndImportModels          : 1 = false;
    uint32 showWndImportSound           : 1 = false;
    uint32 showWndModelScreenshot       : 1 = false;

    uint32 showWndModelsBrowser         : 1 = false;
    uint32 showWndTexturesBrowser       : 1 = true;
    uint32 showWndMaterialsBrowser      : 1 = true;

    // browsers stuff
    uint32 needUpdateMatBrowserIcons    : 1 = false;  
    uint32 needUpdateTexBrowserIcons    : 1 = false;  

    // gizmo stuff
    uint32 useSnapping                  : 1 = false;    // use stride by some fixed value when we transform with guizmo or with fields
    uint32 isGizmoHovered               : 1 = false;    // is any gizmo manipulator is hovered by a mouse
    uint32 isGizmoClicked               : 1 = false;    // is any gizmo manipulator is clicked by a mouse

    // debug shapes (for collision: visualization of AABBs, OBBs, spheres, etc.)
    uint32 renderDbgShapes              : 1 = false;

    uint32 renderDbgLines               : 1 = true;
    uint32 renderDbgCross               : 1 = true;
    uint32 renderDbgSphere              : 1 = true;
    uint32 renderDbgCircle              : 1 = true;
    uint32 renderDbgAxes                : 1 = true;
    uint32 renderDbgTriangle            : 1 = true;
    uint32 renderDbgAABB                : 1 = true;
    uint32 renderDbgOBB                 : 1 = true;
    uint32 renderDbgFrustum             : 1 = true;
    uint32 renderDbgTerrainAABB         : 1 = true;


    float snapTranslationX = 0.1f;
    float snapTranslationY = 0.1f;
    float snapTranslationZ = 0.1f;

    // rotation in degrees
    uint8 snapRotationX = 45;
    uint8 snapRotationY = 45;
    uint8 snapRotationZ = 45;

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


//=========================================================
// specific helper structures
//=========================================================

// store here your data from "model import" window
struct ImportModelWndData
{
    char modelName[MAX_LEN_MODEL_NAME]{ '\0' };
    char filePath[1024]{L'\0'};
};

} // namespace UI
