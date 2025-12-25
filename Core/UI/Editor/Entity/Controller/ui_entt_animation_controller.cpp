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
#include <CoreCommon/pch.h>
#include "ui_entt_animation_controller.h"
#include "../Model/EnttAnimationData.h"

#include <UICommon/icommand.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <UICommon/editor_cmd.h>

#pragma warning (disable : 4996)


namespace UI
{

EnttAnimationController::EnttAnimationController()
{
    if (pData_)
        return;

    pData_ = NEW EnttAnimationData();
    if (!pData_)
    {
        LogErr(LOG, "can't alloc memory for model (MVC) animation data");
        return;
    }
}

//---------------------------------------------------------

EnttAnimationController::~EnttAnimationController()
{
    SafeDelete(pData_);
}

//---------------------------------------------------------
// Desc:  load animations data related to entity by id
//---------------------------------------------------------
void EnttAnimationController::LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id)
{
    if (!pFacade)
    {
        LogErr(LOG, "facade ptr == nullptr (did you forget to init it?)");
        return;
    }

    const SkeletonID skeletonId = pFacade->GetEnttSkeletonId(id);

    // if there is no skeleton and animation for this entity...
    if (skeletonId == 0)
    {
        const char* name = pFacade->GetEnttNameById(id);
        LogErr(LOG, "no animation for entity (id: %" PRIu32 ", name: %s)", id, name);
        return;
    }

    pData_->skeletonId      = skeletonId;
    pData_->animationId     = pFacade->GetEnttAnimationId(id);
    pData_->currAnimTime    = pFacade->GetEnttCurrAnimTime(id);
    pData_->endAnimTime     = pFacade->GetEnttEndAnimTime(id);

    // get a list of all animations names related to this skeleton
    pData_->animNames = (cvector<AnimationName>*)pFacade->GetSkeletonAnimNames(skeletonId);

    // store a name of current skeleton
    const char* skeletonName = pFacade->GetSkeletonName(skeletonId);
    strncpy(pData_->skeletonName.name, skeletonName, MAX_LEN_SKELETON_NAME);

    // store a name of current animation
    const char* animName = pFacade->GetSkeletonAnimName(skeletonId, pData_->animationId);
    strncpy(pData_->currAnimName.name, animName, MAX_LEN_ANIMATION_NAME);
}

//---------------------------------------------------------
//---------------------------------------------------------
void EnttAnimationController::ExecCmd(
    IFacadeEngineToUI* pFacade,
    const ICommand* pCmd,
    const EntityID enttId)
{
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }
    if (!pCmd)
    {
        LogErr(LOG, "command interface ptr == nullptr");
        return;
    }


    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_ANIMATION_SKELETON:
            printf("change skeleton for entt: %u\n", enttId);
            break;

        case CHANGE_ENTITY_ANIMATION_TYPE:
        {
            const AnimationID newAnimId = (AnimationID)pCmd->GetFloat();
            if (pFacade->SetEnttSkeletonAnimation(enttId, newAnimId))
            {
                LoadEnttData(pFacade, enttId);
            }
            break;
        }
            printf("change animation type for entt: %u\n", enttId);

        default:
            LogErr(LOG, "unknown type of command (code: %d, entt_id: %" PRIu32 ")", pCmd->type_, enttId);
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void EnttAnimationController::UndoCmd(
    IFacadeEngineToUI* pFacade,
    const ICommand* pCmd,
    const EntityID id)
{
    assert(0 && "IMPLEMENT ME");
}

} // namespace
