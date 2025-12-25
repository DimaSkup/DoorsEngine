// =================================================================================
// Filename:      EnttTransformController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_transform_controller.h"

#include <UICommon/events_history.h>
#include <UICommon/editor_cmd.h>

#include <UICommon/icommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#pragma warning (disable : 4996)


namespace UI
{

//---------------------------------------------------------
// Desc:  execute the entity changes according to the input command
//---------------------------------------------------------
void EnttTransformController::ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id)
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
        case CHANGE_ENTITY_POSITION:
            ExecChangePos(pFacade, id, pCmd->GetVec3());
            break;

        case CHANGE_ENTITY_ROTATION:
            // rotate entt using quaternion (so get vec4)
            ExecChangeRot(pFacade, id, pCmd->GetVec4());
            break;

        case CHANGE_ENTITY_SCALE:
            ExecChangeScale(pFacade, id, pCmd->GetFloat());
            break;

        default:
            LogErr(LOG, "Unknown entt transform command (cmd: %d, entt_id: %" PRIu32 ")", pCmd->type_, id);
    }
}

//---------------------------------------------------------
// Desc:  execute "undo" command according to input type for entity by id
//---------------------------------------------------------
void EnttTransformController::UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id)
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
        case CHANGE_ENTITY_POSITION:
            UndoChangePos(pFacade, id, pCmd->GetVec3());
            break;

        case CHANGE_ENTITY_ROTATION:
            // undo change of rotation using the inverse quaternion (so get vec4)
            UndoChangeRot(pFacade, id, pCmd->GetVec4());
            break;

        case CHANGE_ENTITY_SCALE:
            UndoChangeScale(pFacade, id, pCmd->GetFloat());
            break;

        default:
            LogErr(LOG, "unknown transform undo command (cmd: %d; entt_id: %" PRIu32 ")", pCmd->type_, id);
    }
}

//---------------------------------------------------------
// Desc:  load/reload data of currently selected entity by ID
//        so we will be able to see refreshed data in the editor
//---------------------------------------------------------
void EnttTransformController::LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id)
{
    if (!pFacade)
    {
        LogErr(LOG, "facade ptr == nullptr (did you forget to init it?)");
        return;
    }

    Vec3 position;
    Vec3 direction;
    Vec4 rotQuat;
    float uniScale = 0.0f;

    if (pFacade->GetEnttTransformData(id, position, direction, rotQuat, uniScale))
        data_.SetData(position, direction, rotQuat, uniScale);
}


// =================================================================================
// Private API: commands executors
// =================================================================================

//---------------------------------------------------------
// 1. update position of input entity by id
// 2. generate an "undo" command and store it into the editor's history
// 3. update editor's field
//---------------------------------------------------------
void EnttTransformController::ExecChangePos(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec3& newPos)
{
    if (!pFacade->SetEnttPosition(id, newPos))
    {
        LogErr(LOG, "can't change position of entt: %" PRIu32, id);
        return;
    }

    const Vec3 oldPos = data_.position_;
    const CmdChangeVec3 undoCmd(CHANGE_ENTITY_POSITION, oldPos);
    sprintf(g_String, "changed pos of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);

    // update editor's field
    data_.position_ = newPos;
}

//---------------------------------------------------------
// 1. update rotation of input entity by id
// 2. update editor fields
// 3. generate an "undo" command and store it into the editor's history
//---------------------------------------------------------
void EnttTransformController::ExecChangeRot(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec4& rotQuat)
{
    assert(pFacade);

    if (!pFacade->RotateEnttByQuat(id, rotQuat))
    {
        LogErr(LOG, "can't change rotation of entt: %" PRIu32, id);
        return;
    }

    // update editor's fields
    data_.direction_ = pFacade->GetEnttDirection(id);
    data_.rotQuat_   = pFacade->GetEnttRotQuat(id);

    // generate "undo"
    const DirectX::XMVECTOR xmInvQuat = DirectX::XMQuaternionInverse({ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });

    const Vec4          invQuat(xmInvQuat.m128_f32);
    const CmdChangeVec4 undoCmd(CHANGE_ENTITY_ROTATION, invQuat);
    sprintf(g_String, "changed rotation of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);
}

//---------------------------------------------------------
// 1. update uniform scale of input entity by id
// 2. update editor fields
// 3. generate an "undo" command and store it into the editor's history
//---------------------------------------------------------
void EnttTransformController::ExecChangeScale(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float scale)
{
    assert(pFacade);

    if (!pFacade->SetEnttUniScale(id, scale))
    {
        LogErr(LOG, "can't change scale of entt: %" PRIu32, id);
        return;
    }

    // generate "undo"
    const float oldScale = data_.uniformScale_;
    const CmdChangeFloat undoCmd(CHANGE_ENTITY_SCALE, oldScale);
    sprintf(g_String, "changed scale of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);

    // update editor's field
    data_.uniformScale_ = scale;
}


// =================================================================================
// Private API: Undo / alternative undo
// =================================================================================

//---------------------------------------------------------
// Desc:  undo change of the entity position
//        (execute the reverted command - just set the prev position)
//---------------------------------------------------------
void EnttTransformController::UndoChangePos(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec3& pos)
{
    assert(pFacade);

    if (pFacade->SetEnttPosition(id, pos))                     // update entity
        data_.position_ = pos;                                  // update editor fields
}

//---------------------------------------------------------
// Desc:  undo change of the entity rotation using the inverse
//        of the original rotation quaternion
//---------------------------------------------------------
void EnttTransformController::UndoChangeRot(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec4& invRotQuat)
{
    assert(pFacade);

    if (pFacade->RotateEnttByQuat(id, invRotQuat))             // update entity
    {
        data_.direction_ = pFacade->GetEnttDirection(id);      // update editor fields
        data_.rotQuat_   = pFacade->GetEnttRotQuat(id);
    }
}

//---------------------------------------------------------
// Desc:  undo change of the entity uniform scale
//---------------------------------------------------------
void EnttTransformController::UndoChangeScale(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float scale)
{
    assert(pFacade);

    if (pFacade->SetEnttUniScale(id, scale))                   // update entity
        data_.uniformScale_ = scale;                            // update editor fields
}

} // namespace UI
