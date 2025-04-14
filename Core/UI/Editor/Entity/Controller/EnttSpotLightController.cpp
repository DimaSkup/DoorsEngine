// =================================================================================
// Filename:      EnttSpotLightController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "EnttSpotLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>

using namespace Core;

namespace UI
{

void EnttSpotLightController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::LoadEnttData(const EntityID id)
{
    EnttSpotLightData& model = spotLightModel_;  // MVC model

    if (pFacade_ == nullptr)
    {
        LogErr("ptr to facade interface == nullptr");
        return;
    }

    if (!pFacade_->GetEnttSpotLightData(
        id,
        model.ambient,
        model.diffuse,
        model.specular,
        model.attenuation,
        model.range,
        model.spotExp))
    {
        sprintf(g_String, "can't load spotlight data of the entity by ID: %ld", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
{
    switch (pCmd->type_)
    {
        case CHANGE_SPOT_LIGHT_AMBIENT:
        {
            ExecChangeAmbient(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        {
            ExecChangeDiffuse(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_SPOT_LIGHT_SPECULAR:
        {
            ExecChangeSpecular(id, pCmd->GetColorRGBA());
            break;
        }
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
        {
            ExecChangeRange(id, pCmd->GetFloat());
            break;
        }
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        {
            ExecChangeAttenuation(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
        {
            ExecChangeSpotExponent(id, pCmd->GetFloat());
            break;
        }
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    // "undo" the change of spotlight entity by ID according to the input command;

    switch (pCmd->type_)
    {
        case CHANGE_SPOT_LIGHT_AMBIENT:
        {
            if (pFacade_->SetSpotLightAmbient(id, pCmd->GetColorRGBA()))
                spotLightModel_.ambient = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        {
            if (pFacade_->SetSpotLightDiffuse(id, pCmd->GetColorRGBA()))
                spotLightModel_.diffuse = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_SPECULAR:
        {
            if (pFacade_->SetSpotLightSpecular(id, pCmd->GetColorRGBA()))
                spotLightModel_.specular = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_RANGE:
        {
            if (pFacade_->SetSpotLightRange(id, pCmd->GetFloat()))
                spotLightModel_.range = pCmd->GetFloat();
            break;
        }
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        {
            if (pFacade_->SetSpotLightAttenuation(id, pCmd->GetVec3()))
                spotLightModel_.attenuation = pCmd->GetVec3();
            break;
        }
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:
        {
            if (pFacade_->SetSpotLightSpotExponent(id, pCmd->GetFloat()))
                spotLightModel_.spotExp = pCmd->GetFloat();
            break;
        }
    }
}


// =================================================================================
// Private API: change spotlight properties
// =================================================================================

static std::string GenerateMsgForHistory(const EntityID id, const std::string& propertyName)
{
    return "changed " + propertyName + " of entt (type: spotlight; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

static std::string GenerateErrMsg(const EntityID id, const std::string& propertyName)
{
    return "can't change " + propertyName + " of entt (type: spotlight; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
    const ColorRGBA oldAmbient = pFacade_->GetSpotLightAmbient(id);

    if (pFacade_->SetSpotLightAmbient(id, ambient))
    {
        // update editor fields	
        spotLightModel_.ambient = ambient;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_AMBIENT, oldAmbient),
            GenerateMsgForHistory(id, "ambient"),
            id);
    }
    else
    {
        LogErr(GenerateErrMsg(id, "ambient").c_str());
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
    const ColorRGBA oldDiffuse = pFacade_->GetSpotLightDiffuse(id);

    if (pFacade_->SetSpotLightDiffuse(id, diffuse))
    {
        // update editor fields	
        spotLightModel_.diffuse = diffuse;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_DIFFUSE, oldDiffuse),
            GenerateMsgForHistory(id, "diffuse"),
            id);
    }
    else
    {
        LogErr(GenerateErrMsg(id, "diffuse").c_str());
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
    const ColorRGBA oldSpecular = pFacade_->GetSpotLightSpecular(id);

    if (pFacade_->SetSpotLightSpecular(id, specular))
    {
        // update editor fields	
        spotLightModel_.specular = specular;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_SPECULAR, oldSpecular),
            GenerateMsgForHistory(id, "specular"),
            id);
    }
    else
    {
        LogErr(GenerateErrMsg(id, "specular").c_str());
    }
}

#if 0

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeDirection(const EntityID id, const Vec3& direction)
{
    const Vec3 oldDirection = pFacade_->GetSpotLightDirection(id);

    if (pFacade_->SetSpotLightDirection(id, direction))
    {
        // update editor fields
        spotLightModel_.SetDirection(direction);

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_DIRECTION, oldDirection),
            GenerateMsgForHistory(id, "direction"),
            id);
    }
    else
    {
        Core::LogErr(GenerateErrMsg(id, "direction"));
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeDirectionByQuat(const EntityID id, const Vec4& dirQuat)
{
    // rotate current direction vector of directed light source by input quaternion
    using namespace DirectX;

    const Vec3 origDirection = pFacade_->GetSpotLightDirection(id);
    const XMVECTOR vec       = origDirection.ToXMVector();
    const XMVECTOR quat      = dirQuat.ToXMVector();
    const XMVECTOR invQuat   = XMQuaternionInverse(quat);

    // rotated_vec = inv_quat * vec * quat;
    XMVECTOR newVec = DirectX::XMQuaternionMultiply(invQuat, vec);
    newVec = DirectX::XMQuaternionMultiply(newVec, quat);

    if (pFacade_->SetSpotLightDirection(id, newVec))
    {
        // update editor fields
        spotLightModel_.SetDirection(newVec);

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_DIRECTION, origDirection),
            GenerateMsgForHistory(id, "direction"),
            id);
    }
    else
    {
        Core::LogErr(GenerateErrMsg(id, "direction"));
    }
}
#endif

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeRange(const EntityID id, const float range)
{
    // change how far can this spotlight lit

    const float oldRange = pFacade_->GetSpotLightRange(id);

    if (pFacade_->SetSpotLightRange(id, range))
    {
        // update editor fields
        spotLightModel_.range = range;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeFloat(CHANGE_SPOT_LIGHT_RANGE, oldRange),
            GenerateMsgForHistory(id, "range"),
            id);
    }
    else
    {
        LogErr(GenerateErrMsg(id, "range").c_str());
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeAttenuation(const EntityID id, const Vec3& att)
{
    // change params for control the spotlight lit distance

    const Vec3 oldAttenuation = pFacade_->GetSpotLightAttenuation(id);

    if (pFacade_->SetSpotLightAttenuation(id, att))
    {
        // update editor fields
        spotLightModel_.attenuation = att;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_ATTENUATION, oldAttenuation),
            GenerateMsgForHistory(id, "attenuation"),
            id);
    }
}

///////////////////////////////////////////////////////////

void EnttSpotLightController::ExecChangeSpotExponent(const EntityID id, const float spotExponent)
{
    // change the light intensity fallof (for control the spotlight cone)

    const float oldSpotExponent = pFacade_->GetSpotLightSpotExponent(id);

    if (pFacade_->SetSpotLightSpotExponent(id, spotExponent))
    {
        // update editor fields
        spotLightModel_.spotExp = spotExponent;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeFloat(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, oldSpotExponent),
            GenerateMsgForHistory(id, "spot exponent"),
            id);
    }
    else
    {
        LogErr(GenerateErrMsg(id, "spot exponent").c_str());
    }
}

} // namespace UI
