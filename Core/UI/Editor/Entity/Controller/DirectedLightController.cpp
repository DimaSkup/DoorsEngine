// =================================================================================
// Filename:      DirectedLightController.cpp
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#include "DirectedLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <format>

namespace UI
{

void DirectedLightController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void DirectedLightController::LoadEnttData(const EntityID id)
{
    // load/reload data of currently selected entity by ID (which is a directed light source)
    ModelEntityDirLight& model = dirLightModel_;

    if (!pFacade_ || !pFacade_->GetEnttDirectedLightData(
        id,
        model.data_.ambient,
        model.data_.diffuse,
        model.data_.specular,
        model.data_.direction))
    {
        Core::Log::Error("can't load data of the directed light entity by ID: " + std::to_string(id));
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
{
    switch (pCmd->type_)
    {
        case CHANGE_DIR_LIGHT_AMBIENT:
        {
            ExecChangeAmbient(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_DIR_LIGHT_DIFFUSE:
        {
            ExecChangeDiffuse(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            ExecChangeSpecular(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_DIR_LIGHT_DIRECTION:
        {
            // is used when we change direction using editor fields
            ExecChangeDirection(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_DIR_LIGHT_DIRECTION_BY_QUAT:
        {
            // is used when we change direction using gizmo
            ExecChangeDirectionByQuat(id, pCmd->GetVec4());
        }
        default:
        {
            Core::Log::Error("unknown type of command: " + pCmd->type_);
        }
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    // "undo" the change of directed light entity by ID according to the input command;

    switch (pCmd->type_)
    {
        case CHANGE_DIR_LIGHT_AMBIENT:
        {
            if (pFacade_->SetDirectedLightAmbient(id, pCmd->GetColorRGBA()))   // update entity
                dirLightModel_.data_.ambient = pCmd->GetColorRGBA();           // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_DIFFUSE:
        {
            if (pFacade_->SetDirectedLightDiffuse(id, pCmd->GetColorRGBA()))   // update entity
                dirLightModel_.data_.diffuse = pCmd->GetColorRGBA();           // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            if (pFacade_->SetDirectedLightSpecular(id, pCmd->GetColorRGBA()))  // update entity
                dirLightModel_.data_.specular = pCmd->GetColorRGBA();          // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_DIRECTION:
        {
            if (pFacade_->SetDirectedLightDirection(id, pCmd->GetVec3()))      // update entity
                dirLightModel_.data_.direction = pCmd->GetVec3();              // update editor
            break;
        }
        default:
        {
            Core::Log::Error("unknown undo command for entity (directed light): " + std::to_string(id));
            return;
        }
    }
}


// =================================================================================
// Private API: commands executors 
// (execute some change of directed light source and store this event into history)
// =================================================================================

static std::string GenerateMsgForHistory(const EntityID id, const std::string& propertyName)
{
    return "changed " + propertyName + " of entt (type: directed light; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

static std::string GenerateErrMsg(const EntityID id, const std::string& propertyName)
{
    return "can't change " + propertyName + " of entt (type: directed light; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
    const ColorRGBA oldAmbient = pFacade_->GetDirectedLightAmbient(id);

    if (pFacade_->SetDirectedLightAmbient(id, ambient))
    {
        // update editor fields
        dirLightModel_.data_.ambient = ambient;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_DIR_LIGHT_AMBIENT, oldAmbient),
            GenerateMsgForHistory(id, "ambient"),
            id);
    }
    else
    {
        Core::Log::Error(GenerateErrMsg(id, "ambient"));
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
    const ColorRGBA oldDiffuse = pFacade_->GetDirectedLightDiffuse(id);

    if (pFacade_->SetDirectedLightDiffuse(id, diffuse))
    {
        // update editor fields
        dirLightModel_.data_.diffuse = diffuse;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_DIR_LIGHT_DIFFUSE, oldDiffuse),
            GenerateMsgForHistory(id, "diffuse"),
            id);
    }
    else
    {
        Core::Log::Error(GenerateErrMsg(id, "diffuse"));
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
    const ColorRGBA oldSpecular = pFacade_->GetDirectedLightSpecular(id);

    if (pFacade_->SetDirectedLightSpecular(id, specular))
    {
        // update editor fields
        dirLightModel_.data_.specular = specular;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_DIR_LIGHT_SPECULAR, oldSpecular),
            GenerateMsgForHistory(id, "specular"),
            id);
    }
    else
    {
        Core::Log::Error(GenerateErrMsg(id, "specular"));
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecChangeDirection(const EntityID id, const Vec3& direction)
{
    const Vec3 oldDirection = pFacade_->GetDirectedLightDirection(id);

    if (pFacade_->SetDirectedLightDirection(id, direction))
    {
        // update editor fields
        dirLightModel_.SetDirection(direction);
        //dirLightModel_.data_.direction = direction;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_DIR_LIGHT_DIRECTION, oldDirection),
            GenerateMsgForHistory(id, "direction"),
            id);
    }
    else
    {
        Core::Log::Error(GenerateErrMsg(id, "direction"));
    }
}

///////////////////////////////////////////////////////////

void DirectedLightController::ExecChangeDirectionByQuat(const EntityID id, const Vec4& dirQuat)
{
    // rotate current direction vector of directed light source by input quaternion
    using namespace DirectX;

    const Vec3 origDirection = pFacade_->GetDirectedLightDirection(id);
    const XMVECTOR vec       = origDirection.ToXMVector();
    const XMVECTOR quat      = dirQuat.ToXMVector();
    const XMVECTOR invQuat   = XMQuaternionInverse(quat);

    // rotated_vec = inv_quat * vec * quat
    XMVECTOR newVec = XMQuaternionMultiply(invQuat, vec);
    newVec = XMQuaternionMultiply(newVec, quat);

    if (pFacade_->SetDirectedLightDirection(id, newVec))
    {
        // update editor fields
        dirLightModel_.SetDirection(newVec);

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_DIR_LIGHT_DIRECTION, origDirection),
            GenerateMsgForHistory(id, "direction"),
            id);
    }
    else
    {
        Core::Log::Error(GenerateErrMsg(id, "direction"));
    }
}

} // namespace UI


