/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_r_states_controller.h
    Desc:     editor's functional for runtime modification of material's render states

    Created:  20.01.2026 by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>


namespace UI
{
//-------------------------------------
// forward declarations (pointer use only)
//-------------------------------------
class IFacadeEngineToUI;

class UIRenderStatesController
{
public:
    UIRenderStatesController(IFacadeEngineToUI* pFacade);

    // render states switching/editing
    void DrawRasterParamsControl        (const RsID rsId);
    void DrawBlendParamsControl         (const BsID bsId);
    void DrawDepthStencilParamsControl  (const DssID dssId);

    bool DrawRasterStatesSelectors      (const RsID rsId,   MaterialID matId);
    bool DrawBlendStatesSelectors       (const BsID bsId,   MaterialID matId);
    bool DrawDepthStencilStatesSelectors(const DssID dssId, MaterialID matId);

private:
    IFacadeEngineToUI* pFacade_ = nullptr;
};

} // namespace
