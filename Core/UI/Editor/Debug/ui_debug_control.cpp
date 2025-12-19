/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_debug_control.cpp

    Created:  11.11.2025 by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "ui_debug_control.h"
#include <UICommon/IFacadeEngineToUI.h>
#include "rnd_dbg_type_switcher.h"
#include <imgui.h>


namespace UI
{

//---------------------------------------------------------
// Desc:  draw debug info and show fields to control the rendering 
//---------------------------------------------------------
void DebugControl::Draw(IFacadeEngineToUI* pFacade)
{
    if (pFacade == nullptr)
    {
        LogErr(LOG, "ptr to the facade interface == nullptr");
        return;
    }


    bool enableDepthPrepass = pFacade->IsEnabledDepthPrepass();

    if (ImGui::Checkbox("Enable depth prepass", &enableDepthPrepass))
    {
        pFacade->EnableDepthPrepass(enableDepthPrepass);
    }

    ImGui::Separator();

    ImGui::Text("Anti aliasing:");

    const uint8 currAA = pFacade->GetAntiAliasingType();
    const char* names[3] = { "NO AA", "FXAA", "MSAA" };
    bool enabled[3]{ false };
    enabled[currAA] = true;

    if (ImGui::Checkbox("NO AA", &enabled[0]))
        pFacade->SetAntiAliasingType(0);

    if (ImGui::Checkbox("FXAA", &enabled[1]))
        pFacade->SetAntiAliasingType(1);

    if (ImGui::Checkbox("MSAA", &enabled[2]))
        pFacade->SetAntiAliasingType(2);

    ImGui::Separator();

    // switch btw different rendering states
    ImGui::Text("Switch render debug type:");
    DrawRndDebugTypesSwitcher(pFacade);
}

} // namespace
