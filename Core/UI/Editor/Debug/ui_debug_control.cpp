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
    if (!pFacade)
    {
        LogErr(LOG, "ptr to the facade interface == nullptr");
        return;
    }

    //---------------------------------
    // enable/disable locking of frustum culling
    // (so in editor we see culling result from the editor's camera)
    ImGui::Separator();

    bool bLockFrustumCulling = pFacade->IsLockedFrustumCulling();

    if (ImGui::Checkbox("Lock frustum culling", &bLockFrustumCulling))
        pFacade->LockFrustumCulling(bLockFrustumCulling);

    //---------------------------------
    // enable/disable depth prepass
    ImGui::Separator();

    bool bEnableDepthPrepass = pFacade->IsEnabledDepthPrepass();

    if (ImGui::Checkbox("Enable depth prepass", &bEnableDepthPrepass))
        pFacade->EnableDepthPrepass(bEnableDepthPrepass);

    //---------------------------------
    // turn on/off fullscreen for the game mode
    ImGui::Separator();

    bool bFullscreenInGameMode = pFacade->IsFullscreenInGameMode();

    if (ImGui::Checkbox("Fullscreen in game mode", &bFullscreenInGameMode))
        pFacade->SetFullscreenInGameMode(bFullscreenInGameMode);

    //---------------------------------

    ImGui::Separator();

    ImGui::Text("Anti aliasing:");

    const uint8 currAA = pFacade->GetAntiAliasingType();
    bool enabledAA[3]{ false };
    enabledAA[currAA] = true;

    if (ImGui::Checkbox("NO AA", &enabledAA[0]))
        pFacade->SetAntiAliasingType(0);

    if (ImGui::Checkbox("FXAA", &enabledAA[1]))
        pFacade->SetAntiAliasingType(1);

    if (ImGui::Checkbox("MSAA", &enabledAA[2]))
        pFacade->SetAntiAliasingType(2);

    ImGui::Separator();

    // switch btw different rendering states
    ImGui::Text("Switch render debug type:");
    DrawRndDebugTypesSwitcher(pFacade);
}

} // namespace
