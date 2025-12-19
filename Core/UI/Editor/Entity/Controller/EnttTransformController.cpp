// =================================================================================
// Filename:      EnttTransformController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttTransformController.h"

#include <UICommon/events_history.h>
#include <UICommon/editor_cmd.h>

#pragma warning (disable : 4996)


namespace UI
{

EnttTransformController::EnttTransformController()
{
}

//---------------------------------------------------------
// Desc:  init ptr to the facade interface is used to contact with the rest of the engine
//---------------------------------------------------------
void EnttTransformController::Initialize(IFacadeEngineToUI* pFacade)
{
    CAssert::True(pFacade != nullptr, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

//---------------------------------------------------------
// Desc:  execute the entity changes according to the input command
//---------------------------------------------------------
void EnttTransformController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
{
    
    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        {
            ExecChangePosition(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_ENTITY_ROTATION:
        {
            // rotate entt using quaternion (so get vec4)
            ExecChangeRotation(id, pCmd->GetVec4());
            break;
        }
        case CHANGE_ENTITY_SCALE:
        {
            ExecChangeUniformScale(id, pCmd->GetFloat());
            break;
        }
        default:
        {
            LogErr(LOG, "Unknown entt transform command to execute (cmd: %d, entt_id: %" PRIu32 ")", pCmd->type_, id);
        }
    }
}

//---------------------------------------------------------
// Desc:  execute "undo" command according to input type for entity by id
//---------------------------------------------------------
void EnttTransformController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        {
            UndoChangePosition(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_ENTITY_ROTATION:
        {
            // undo change of rotation using the inverse quaternion (so get vec4)
            UndoChangeRotation(id, pCmd->GetVec4());
            break;
        }
        case CHANGE_ENTITY_SCALE:
        {
            UndoChangeScale(id, pCmd->GetFloat());
            break;
        }
        default:
        {
            LogErr(LOG, "unknown transform undo command (cmd: %d; entt_id: %" PRIu32 ")", pCmd->type_, id);
        }
    }
}

//---------------------------------------------------------
// Desc:  load/reload data of currently selected entity by ID
//        so we will be able to see refreshed data in the editor
//---------------------------------------------------------
void EnttTransformController::LoadEnttData(const EntityID id)
{
    Vec3 position;
    Vec3 direction;
    Vec4 rotQuat;
    float uniScale = 0.0f;

    if (pFacade_->GetEnttTransformData(id, position, direction, rotQuat, uniScale))
        data_.SetData(position, direction, rotQuat, uniScale);
}


// =================================================================================
// Private API: commands executors
// =================================================================================

//---------------------------------------------------------
// 1. update position of input entity by id
// 2. update editor fields
// 3. generate an "undo" command and store it into the editor's history
//---------------------------------------------------------
void EnttTransformController::ExecChangePosition(const EntityID id, const Vec3& newPos)
{
    const Vec3 oldPos = pFacade_->GetEnttPosition(id);

    if (!pFacade_->SetEnttPosition(id, newPos))
    {
        LogErr(LOG, "can't change position of entt: %" PRIu32, id);
        return;
    }

    // update editor fields
    data_.position_ = newPos;

    // generate "undo"
    const CmdChangeVec3 undoCmd(CHANGE_ENTITY_POSITION, oldPos);
    sprintf(g_String, "changed pos of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);
}

//---------------------------------------------------------
// 1. update rotation of input entity by id
// 2. update editor fields
// 3. generate an "undo" command and store it into the editor's history
//---------------------------------------------------------
void EnttTransformController::ExecChangeRotation(const EntityID id, const Vec4& rotQuat)
{
    using namespace DirectX;

    if (!pFacade_->RotateEnttByQuat(id, rotQuat))
    {
        LogErr(LOG, "can't change rotation of entt: %" PRIu32, id);
        return;
    }

    // update editor fields
    data_.direction_ = pFacade_->GetEnttDirection(id);
    data_.rotQuat_   = pFacade_->GetEnttRotQuat(id);

    

    // generate "undo"
    const XMVECTOR      xmInvQuat = XMQuaternionInverse({ rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w });
    const Vec4          invQuat(xmInvQuat.m128_f32);
    printf("invQuat: %f %f %f %f\n", invQuat.x, invQuat.y, invQuat.z, invQuat.w);

    const CmdChangeVec4 undoCmd(CHANGE_ENTITY_ROTATION, invQuat);
    sprintf(g_String, "changed rotation of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);
}

//---------------------------------------------------------
// 1. update uniform scale of input entity by id
// 2. update editor fields
// 3. generate an "undo" command and store it into the editor's history
//---------------------------------------------------------
void EnttTransformController::ExecChangeUniformScale(const EntityID id, const float uniformScale)
{
    const float oldScale = pFacade_->GetEnttScale(id);

    if (!pFacade_->SetEnttUniScale(id, uniformScale))
    {
        LogErr(LOG, "can't change scale of entt: %" PRIu32, id);
        return;
    }

    // update editor fields
    data_.uniformScale_ = uniformScale;

    // generate "undo"
    const CmdChangeFloat undoCmd(CHANGE_ENTITY_SCALE, oldScale);
    sprintf(g_String, "changed scale of entt (id: %" PRIu32 ")", id);
    g_EventsHistory.Push(undoCmd, g_String, id);
}


// =================================================================================
// Private API: Undo / alternative undo
// =================================================================================

//---------------------------------------------------------
// Desc:  undo change of the entity position
//        (execute the reverted command - just set the prev position)
//---------------------------------------------------------
void EnttTransformController::UndoChangePosition(const EntityID id, const Vec3& pos)
{
    if (pFacade_->SetEnttPosition(id, pos))                     // update entity
        data_.position_ = pos;                                  // update editor fields
}

//---------------------------------------------------------
// Desc:  undo change of the entity rotation using the inverse
//        of the original rotation quaternion
//---------------------------------------------------------
void EnttTransformController::UndoChangeRotation(const EntityID id, const Vec4& invRotQuat)
{
    printf("invQuat: %f %f %f %f\n", invRotQuat.x, invRotQuat.y, invRotQuat.z, invRotQuat.w);


    if (pFacade_->RotateEnttByQuat(id, invRotQuat))             // update entity
    {
        data_.direction_ = pFacade_->GetEnttDirection(id);      // update editor fields
        data_.rotQuat_   = pFacade_->GetEnttRotQuat(id);
    }
}

//---------------------------------------------------------
// Desc:  undo change of the entity uniform scale
//---------------------------------------------------------
void EnttTransformController::UndoChangeScale(const EntityID id, const float scale)
{
    if (pFacade_->SetEnttUniScale(id, scale))                   // update entity
        data_.uniformScale_ = scale;                            // update editor fields
}

} // namespace UI
