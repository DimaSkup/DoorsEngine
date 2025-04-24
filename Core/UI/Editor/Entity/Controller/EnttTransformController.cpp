// =================================================================================
// Filename:      EnttTransformController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "EnttTransformController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>

using namespace Core;

namespace UI
{

EnttTransformController::EnttTransformController()
{
}

///////////////////////////////////////////////////////////

void EnttTransformController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Assert::True(pFacade != nullptr, "ptr to the facade == nullptr");
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
            ExecChangeDirection(id, pCmd->GetVec3());
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
            UndoChangeDirection(id, pCmd->GetVec3());
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
        gEventsHistory.Push(undoCmd, msg, id);
    }
    else
    {
        sprintf(g_String, "can't change position of entt: %ld", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EnttTransformController::ExecChangeDirection(const EntityID id, const Vec3& direction)
{
    // update direction for entity by ID and store this event into the history

#if 0
    Vec4 oldDirectionQuat = pFacade_->GetEnttDirectionQuat(id);

    if (pFacade_->SetEnttDirectionQuat(id, dirQuat))
    {
        // update editor fields
        data_.SetDirection(dirQuat);

        // generate an "undo" command and store it into the history
        const CmdChangeVec4 undoCmd(CHANGE_ENTITY_DIRECTION, oldDirectionQuat);
        const std::string msg = "changed direction of entt (id: " + std::to_string(id) + ")";
        gEventsHistory.Push(undoCmd, msg, id);
    }
    else
    {
        Core::LogErr("can't change direction of entt; id: " + std::to_string(id));
    }
#elif 1
    Vec3 oldDirection = pFacade_->GetEnttDirection(id);

    if (pFacade_->SetEnttDirection(id, direction))
    {
        // update editor fields
        data_.direction_ = direction;

        // generate an "undo" command and store it into the history
        const CmdChangeVec3 undoCmd(CHANGE_ENTITY_DIRECTION, oldDirection);
        sprintf(g_String, "changed direction of entt (id: %ld)", id);
        gEventsHistory.Push(undoCmd, g_String, id);
    }
    else
    {
        sprintf(g_String, "can't change direction of entt: %ld", id);
        LogErr(g_String);
    }

#elif 0

    using namespace DirectX;

    const Vec4 origDirection = pFacade_->GetEnttDirectionQuat(id);
    const XMVECTOR vec       = origDirection.ToXMVector();
    const XMVECTOR quat      = dirQuat.ToXMVector();
    const XMVECTOR invQuat   = XMQuaternionInverse(quat);

    // rotated_vec = inv_quat * vec * quat;
    XMVECTOR newVec = DirectX::XMQuaternionMultiply(invQuat, vec);
    newVec = DirectX::XMQuaternionMultiply(newVec, quat);

    if (pFacade_->SetEnttDirectionQuat(id, newVec))
    {
        // update editor fields
        data_.SetDirection(newVec);
    }
    else
    {
        Core::LogErr("can't change direction of entt; id: " + std::to_string(id));
    }

#endif
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
        gEventsHistory.Push(undoCmd, g_String, id);
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

    if (pFacade_->SetEnttPosition(id, pos))                // update entity
        data_.position_ = pos;                             // update editor fields
}

///////////////////////////////////////////////////////////

void EnttTransformController::UndoChangeDirection(const EntityID id, const Vec3& dir)
{
    if (pFacade_->SetEnttDirection(id, dir))               // update entity
        data_.direction_ = dir;                            // update editor fields
}

///////////////////////////////////////////////////////////

void EnttTransformController::UndoChangeScale(const EntityID id, const float uniformScale)
{
    if (pFacade_->SetEnttUniScale(id, uniformScale))       // update entity
        data_.uniformScale_ = uniformScale;                // update editor fields
}

} // namespace UI
