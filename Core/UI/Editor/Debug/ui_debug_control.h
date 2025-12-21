/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_debug_control.h

    Desc:     editor parts to control the debugging:
              switching rendering states, turn on/off showing of
              the normals, binormals, bounding boxes,
              switching the wireframe or fill mode, etc.

    Created:  01.01.25 by DimaSkup
\**********************************************************************************/
#pragma once


namespace UI
{

// forward declaration
class IFacadeEngineToUI;


class DebugControl
{
public:
    void Draw(IFacadeEngineToUI* pFacade);
   
};

} // namespace UI
