// =================================================================================
// Filename:      EnttTransformController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttTransformController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>

#pragma warning (disable : 4996)

namespace UI
{

EnttTransformController::EnttTransformController()
{
}

///////////////////////////////////////////////////////////

void EnttTransformController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    CAssert::True(pFacade != nullptr, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void EnttTransformController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
{
    // execute the entity changes according to the input command
    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        {
            ExecChangePosition(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_ENTITY_DIRECTION:
        {
            // change direction using quaternion (so get vec4)
            ExecChangeDirection(id, pCmd->GetVec4());
            break;
        }
        case CHANGE_ENTITY_SCALE:
        {
            ExecChangeUniformScale(id, pCmd->GetFloat());
            break;
        }
        default:
        {
            sprintf(g_String, "Unknown entt transform command to execute (cmd: %d, entt_id: %ld)", pCmd->type_, id);
            LogErr(g_String);
        }
    }
}

///////////////////////////////////////////////////////////

void EnttTransformController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    switch (pCmd->type_)
    {
        case CHANGE_ENTITY_POSITION:
        {
            UndoChangePosition(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_ENTITY_DIRECTION:
        {
            // undo change of direction using the inverse quaternion (so get vec4)
            UndoChangeDirection(id, pCmd->GetVec4());
            break;
        }
        case CHANGE_ENTITY_SCALE:
        {
            UndoChangeScale(id, pCmd->GetFloat());
            break;
        }
        default:
        {
            sprintf(g_String, "unknown transform undo command (cmd: %d; entt_id: %ld)", pCmd->type_, id);
            LogErr(g_String);
            return;
        }
    }
}

///////////////////////////////////////////////////////////

void EnttTransformController::LoadEnttData(const EntityID id)
{
    // load/reload data of currently selected entity by ID
    // so we will be able to see refreshed data in the editor
    Vec3 position;
    Vec3 direction;
    float uniScale = 0.0f;

    if (pFacade_->GetEnttTransformData(id, position, direction, uniScale))
        data_.SetData(position, direction, uniScale);
}


// =================================================================================
// Private API: commands executors
// =================================================================================

void EnttTransformController::ExecChangePosition(const EntityID id, const Vec3& newPos)
{
    // execute some command and store it into the events history

    Vec3 oldPos = pFacade_->GetEnttPosition(id);

    if (pFacade_->SetEnttPosition(id, newPos))
    {
        // update editor fields
        data_.position_ = newPos;

        // generate an "undo" command and store it into the history
        const CmdChangeVec3 undoCmd(CHANGE_ENTITY_POSITION, oldPos);
        const std::string msg = "changed posisition of entt (id: " + std::to_string(id) + ")";
        g_EventsHistory.Push(undoCmd, msg, id);
    }
    else
    {
        sprintf(g_String, "can't change position of entt: %ld", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EnttTransformController::ExecChangeDirection(const EntityID id, const Vec4& rotationQuat)
{
    // update direction for entity by ID and store this event into the history


    Vec3 oldDirection = pFacade_->GetEnttDirection(id);

    if (pFacade_->RotateEnttByQuat(id, rotationQuat))
    {
        // update editor fields
        data_.direction_ = pFacade_->GetEnttDirection(id);

        // generate an "undo" command and store it into the history
        const CmdChangeVec3 undoCmd(CHANGE_ENTITY_DIRECTION, oldDirection);
        sprintf(g_String, "changed direction of entt (id: %ld)", id);
        g_EventsHistory.Push(undoCmd, g_String, id);
    }
    else
    {
        sprintf(g_String, "can't change direction of entt: %ld", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EnttTransformController::ExecChangeUniformScale(const EntityID id, const float uniformScale)
{
    // update uniform scale for entity by ID and store this event into the history

    float oldUniformScale = pFacade_->GetEnttScale(id);

    if (pFacade_->SetEnttUniScale(id, uniformScale))
    {
        // update editor fields
        data_.uniformScale_ = uniformScale;

        // generate an "undo" command and store it into the history
        const CmdChangeFloat undoCmd(CHANGE_ENTITY_SCALE, oldUniformScale);
        sprintf(g_String, "changed uniform scale of entt (id: %ld)", id);
        g_EventsHistory.Push(undoCmd, g_String, id);
    }
    else
    {
        sprintf(g_String, "can't change scale of entt: %ld", id);
        LogErr(g_String);
    }
}


// =================================================================================
// Private API: Undo / alternative undo
// =================================================================================

void EnttTransformController::UndoChangePosition(const EntityID id, const Vec3& pos)
{
    // undo change of the entity model position
    // (execute the reverted command - just set the prev position)

    if (pFacade_->SetEnttPosition(id, pos))                     // update entity
        data_.position_ = pos;                                  // update editor fields
}

///////////////////////////////////////////////////////////

void EnttTransformController::UndoChangeDirection(const EntityID id, const Vec4& rotationQuat)
{
    if (pFacade_->RotateEnttByQuat(id, rotationQuat))           // update entity
        data_.direction_ = pFacade_->GetEnttDirection(id);      // update editor fields
}

///////////////////////////////////////////////////////////

void EnttTransformController::UndoChangeScale(const EntityID id, const float uniformScale)
{
    if (pFacade_->SetEnttUniScale(id, uniformScale))            // update entity
        data_.uniformScale_ = uniformScale;                     // update editor fields
}

} // namespace UI
