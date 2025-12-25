/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_entt_animation_controller.h
    Desc:     editor controller (MVC pattern) for execution/undoing of commands
              related to changing of entity animations

    Created:  24.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>

namespace UI
{
// forward declarations
class EnttAnimationData;    // MVC model
class ICommand;
class IFacadeEngineToUI;


//---------------------------------------------------------

class EnttAnimationController
{
public:
    EnttAnimationController();
    ~EnttAnimationController();

    void LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id);

    void ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);
    void UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);

    inline const EnttAnimationData* GetData() const { return pData_; }

private:
    EnttAnimationData* pData_ = nullptr;
};

}
