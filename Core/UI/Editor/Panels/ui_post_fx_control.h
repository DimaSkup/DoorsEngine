/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_post_fx_control.h
    Desc:     editor's panels for post effects controlling

    Created:  07.11.2025 by DimaSkup
\**********************************************************************************/
#pragma once

namespace UI
{

class IFacadeEngineToUI;

//---------------------------------------------------------

class UI_PostFxControl
{
public:
    UI_PostFxControl();

    void Draw(IFacadeEngineToUI* pFacade);
};

} // namespace
