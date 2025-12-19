/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: rnd_dbg_type_switcher.h
    Desc:     switching rendering debug type
              (is used for some editor's tools and for debugging of rendering stuff)

    Created:  20.11.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <enum_rnd_debug_type.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>

namespace UI
{

//---------------------------------------------------------
// Desc:  draw a list of radio buttons for switching rendering debug states
//---------------------------------------------------------
static void DrawRndDebugTypesSwitcher(IFacadeEngineToUI* pFacade)
{
    if (!pFacade)
        return;

    const char** items = g_RndDebugTypeNames;
    static int debugOption = 0;
    bool anyBtnWasPressed = false;

    for (int i = 0; i < (int)NUM_RENDER_DEBUG_TYPES; ++i)
        anyBtnWasPressed |= ImGui::RadioButton(items[i], &debugOption, i);

    if (anyBtnWasPressed)
        pFacade->SwitchDebugState(debugOption);
}

}
