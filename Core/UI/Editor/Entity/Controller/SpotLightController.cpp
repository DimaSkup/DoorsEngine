// =================================================================================
// Filename:      SpotLightController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "SpotLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>

namespace UI
{

void SpotLightController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void SpotLightController::LoadEnttData(const EntityID id)
{
    ModelEntitySpotLight& model = spotLightModel_;  // MVC model

    if (!pFacade_->GetEnttSpotLightData(
        id,
        model.data_.ambient,
        model.data_.diffuse,
        model.data_.specular,
        model.data_.position,
        model.data_.range,
        model.data_.direction,
        model.data_.spotExp,
        model.data_.attenuation))
    {
        Core::Log::Error("can't load data of the spotlight entity by ID: " + std::to_string(id));
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
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
        case CHANGE_SPOT_LIGHT_POSITION:
        {
            ExecChangePosition(id, pCmd->GetVec3());
            break;
        }
        case CHANGE_SPOT_LIGHT_DIRECTION:
        {
            ExecChangeDirection(id, pCmd->GetVec3());
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

void SpotLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    // "undo" the change of spotlight entity by ID according to the input command;

    switch (pCmd->type_)
    {
        case CHANGE_SPOT_LIGHT_AMBIENT:
        {
            if (pFacade_->SetSpotLightAmbient(id, pCmd->GetColorRGBA()))
                spotLightModel_.data_.ambient = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_DIFFUSE:
        {
            if (pFacade_->SetSpotLightDiffuse(id, pCmd->GetColorRGBA()))
                spotLightModel_.data_.diffuse = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_SPECULAR:
        {
            if (pFacade_->SetSpotLightSpecular(id, pCmd->GetColorRGBA()))
                spotLightModel_.data_.specular = pCmd->GetColorRGBA();
            break;
        }
        case CHANGE_SPOT_LIGHT_POSITION:
        {
            if (pFacade_->SetSpotLightPos(id, pCmd->GetVec3()))
                spotLightModel_.data_.position = pCmd->GetVec3();
            break;
        }
        case CHANGE_SPOT_LIGHT_DIRECTION:
        {
            Core::Log::Error("IMPLEMENT ME!");
            break;
        }
        case CHANGE_SPOT_LIGHT_RANGE:
        {
            if (pFacade_->SetSpotLightRange(id, pCmd->GetFloat()))
                spotLightModel_.data_.range = pCmd->GetFloat();
            break;
        }
        case CHANGE_SPOT_LIGHT_ATTENUATION:
        {
            if (pFacade_->SetSpotLightAttenuation(id, pCmd->GetVec3()))
                spotLightModel_.data_.attenuation = pCmd->GetVec3();
            break;
        }
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:
        {
            if (pFacade_->SetSpotLightSpotExponent(id, pCmd->GetFloat()))
                spotLightModel_.data_.spotExp = pCmd->GetFloat();
            break;
        }
    }
}



// =================================================================================
// Private API: change spotlight properties
// =================================================================================

static std::string GenerateMsgForHistory(const EntityID id, const std::string& propertyName)
{
    return std::string(
        "changed " + propertyName +
        " of entt (type: spotlight; id: " + std::to_string(id) + ")");
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
    const ColorRGBA oldAmbient = pFacade_->GetSpotLightAmbient(id);

    if (pFacade_->SetSpotLightAmbient(id, ambient))
    {
        // update editor fields	
        spotLightModel_.data_.ambient = ambient;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_AMBIENT, oldAmbient),
            GenerateMsgForHistory(id, "ambient"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
    const ColorRGBA oldDiffuse = pFacade_->GetSpotLightDiffuse(id);

    if (pFacade_->SetSpotLightDiffuse(id, diffuse))
    {
        // update editor fields	
        spotLightModel_.data_.diffuse = diffuse;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_DIFFUSE, oldDiffuse),
            GenerateMsgForHistory(id, "diffuse"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
    const ColorRGBA oldSpecular = pFacade_->GetSpotLightSpecular(id);

    if (pFacade_->SetSpotLightSpecular(id, specular))
    {
        // update editor fields	
        spotLightModel_.data_.specular = specular;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeColor(CHANGE_SPOT_LIGHT_SPECULAR, oldSpecular),
            GenerateMsgForHistory(id, "specular"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangePosition(const EntityID id, const Vec3& pos)
{
    const Vec3 oldPos = pFacade_->GetSpotLightPos(id);

    if (pFacade_->SetSpotLightPos(id, pos))
    {
        // update editor fields	
        spotLightModel_.data_.position = pos;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_POSITION, oldPos),
            GenerateMsgForHistory(id, "position"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeDirection(const EntityID id, const Vec3& direction)
{
    const Vec3 oldDirection = pFacade_->GetSpotLightDirection(id);

    if (pFacade_->SetSpotLightDirection(id, direction))
    {
        // update editor fields
        spotLightModel_.data_.direction = direction;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_DIRECTION, oldDirection),
            GenerateMsgForHistory(id, "direction"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeRange(const EntityID id, const float range)
{
    // change how far can this spotlight lit

    const float oldRange = pFacade_->GetSpotLightRange(id);

    if (pFacade_->SetSpotLightRange(id, range))
    {
        // update editor fields
        spotLightModel_.data_.range = range;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeFloat(CHANGE_SPOT_LIGHT_RANGE, oldRange),
            GenerateMsgForHistory(id, "range"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeAttenuation(const EntityID id, const Vec3& att)
{
    const Vec3 oldAttenuation = pFacade_->GetSpotLightAttenuation(id);

    if (pFacade_->SetSpotLightAttenuation(id, att))
    {
        // update editor fields
        spotLightModel_.data_.attenuation = att;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeVec3(CHANGE_SPOT_LIGHT_ATTENUATION, oldAttenuation),
            GenerateMsgForHistory(id, "attenuation"),
            id);
    }
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeSpotExponent(const EntityID id, const float spotExponent)
{
    // change the light intensity fallof (for control the spotlight cone)

    const float oldSpotExponent = pFacade_->GetSpotLightSpotExponent(id);

    if (pFacade_->SetSpotLightSpotExponent(id, spotExponent))
    {
        // update editor fields
        spotLightModel_.data_.spotExp = spotExponent;

        // generate an "undo" command and store it into the history
        gEventsHistory.Push(
            CmdChangeFloat(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, oldSpotExponent),
            GenerateMsgForHistory(id, "spot exponent"),
            id);
    }
}

} // namespace UI
