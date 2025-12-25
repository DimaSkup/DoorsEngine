// =================================================================================
// Filename:      ui_entt_spotlight_controller.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_spotlight_controller.h"

#include <UICommon/events_history.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/icommand.h>
#include <UICommon/IFacadeEngineToUI.h>


#pragma warning (disable : 4996)

namespace UI
{

//---------------------------------------------------------
// Desc:  load/reload spotlight data of currently selected entity by ID
//---------------------------------------------------------
void EnttSpotLightController::LoadEnttData(
    IFacadeEngineToUI* pFacade,
    const EntityID id)
{
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }

    EnttSpotLightData& L = data_;

    if (!pFacade->GetEnttSpotLightData(
        id,
        L.ambient,
        L.diffuse,
        L.specular,
        L.attenuation,
        L.range,
        L.spotExp))
    {
        LogErr(LOG, "can't load spotlight data of the entity by ID: %" PRIu32, id);
    }
}

//---------------------------------------------------------
// Desc:  execute change of spotlight property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttSpotLightController::ExecCmd(
    IFacadeEngineToUI* pFacade,
    const ICommand* pCmd,
    const EntityID id)
{
    // check to prevent fuck up
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }
    if (!pCmd)
    {
        LogErr(LOG, "command ptr == nullptr");
        return;
    }


    switch (pCmd->type_)
    {
        case CHANGE_SPOT_LIGHT_AMBIENT:
            ExecChangeAmbient(pFacade, id, pCmd->GetColorRGBA());
            break;
        
        case CHANGE_SPOT_LIGHT_DIFFUSE:
            ExecChangeDiffuse(pFacade, id, pCmd->GetColorRGBA());
            break;
        
        case CHANGE_SPOT_LIGHT_SPECULAR:
            ExecChangeSpecular(pFacade, id, pCmd->GetColorRGBA());
            break;
        
        case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
            ExecChangeRange(pFacade, id, pCmd->GetFloat());
            break;
        
        case CHANGE_SPOT_LIGHT_ATTENUATION:
            ExecChangeAttenuation(pFacade, id, pCmd->GetVec3());
            break;
        
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
            ExecChangeSpotExponent(pFacade, id, pCmd->GetFloat());
            break;

        default:
            LogErr(LOG, "unknown type of command: %d", pCmd->type_);
    }
}

//---------------------------------------------------------
// Desc:  undo change of point light property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttSpotLightController::UndoCmd(
    IFacadeEngineToUI* pFacade,
    const ICommand* pCmd,
    const EntityID id)
{
    // check to prevent fuck up
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }
    if (!pCmd)
    {
        LogErr(LOG, "command ptr == nullptr");
        return;
    }


    switch (pCmd->type_)
    {
        case CHANGE_SPOT_LIGHT_AMBIENT:
            if (pFacade->SetSpotLightAmbient(id, pCmd->GetColorRGBA()))
                data_.ambient = pCmd->GetColorRGBA();
            break;
        
        case CHANGE_SPOT_LIGHT_DIFFUSE:
            if (pFacade->SetSpotLightDiffuse(id, pCmd->GetColorRGBA()))
                data_.diffuse = pCmd->GetColorRGBA();
            break;
        
        case CHANGE_SPOT_LIGHT_SPECULAR:
            if (pFacade->SetSpotLightSpecular(id, pCmd->GetColorRGBA()))
                data_.specular = pCmd->GetColorRGBA();
            break;
        
        case CHANGE_SPOT_LIGHT_RANGE:
            if (pFacade->SetSpotLightRange(id, pCmd->GetFloat()))
                data_.range = pCmd->GetFloat();
            break;
        
        case CHANGE_SPOT_LIGHT_ATTENUATION:
            if (pFacade->SetSpotLightAttenuation(id, pCmd->GetVec3()))
                data_.attenuation = pCmd->GetVec3();
            break;
        
        case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:
            if (pFacade->SetSpotLightSpotExponent(id, pCmd->GetFloat()))
                data_.spotExp = pCmd->GetFloat();
            break;
        
        default:
            LogErr(LOG, "unknown undo command (%d) for entity: %" PRIu32, pCmd->type_, id);
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

//---------------------------------------------------------
// 1. change "ambient" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeAmbient(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& ambient)
{
    assert(pFacade);
    const ColorRGBA oldAmbient = pFacade->GetSpotLightAmbient(id);

    if (!pFacade->SetSpotLightAmbient(id, ambient))
    {
        LogErr(GenerateErrMsg(id, "ambient").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_SPOT_LIGHT_AMBIENT, oldAmbient),
        GenerateMsgForHistory(id, "ambient"),
        id);

    data_.ambient = ambient;
}

//---------------------------------------------------------
// 1. change "diffuse" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeDiffuse(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& diffuse)
{
    assert(pFacade);
    const ColorRGBA oldDiffuse = pFacade->GetSpotLightDiffuse(id);

    if (!pFacade->SetSpotLightDiffuse(id, diffuse))
    {
        LogErr(GenerateErrMsg(id, "diffuse").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_SPOT_LIGHT_DIFFUSE, oldDiffuse),
        GenerateMsgForHistory(id, "diffuse"),
        id);

    data_.diffuse = diffuse;
}

//---------------------------------------------------------
// 1. change "specular" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeSpecular(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& specular)
{
    assert(pFacade);
    const ColorRGBA oldSpecular = pFacade->GetSpotLightSpecular(id);

    if (!pFacade->SetSpotLightSpecular(id, specular))
    {
        LogErr(GenerateErrMsg(id, "specular").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_SPOT_LIGHT_SPECULAR, oldSpecular),
        GenerateMsgForHistory(id, "specular"),
        id);

    data_.specular = specular;
}

#if 0

//---------------------------------------------------------
// 1. change "direction" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeDirection( IFacadeEngineToUI* pFacade,
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec3& direction)
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

//---------------------------------------------------------
// 1. change "direction" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeDirectionByQuat(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec4& dirQuat)
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

//---------------------------------------------------------
// 1. change "range" property for spotlight of entity by ID
//    (how far a spotlight can lit)
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeRange(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float range)
{
    assert(pFacade);
    const float oldRange = pFacade->GetSpotLightRange(id);

    if (!pFacade->SetSpotLightRange(id, range))
    {
        LogErr(GenerateErrMsg(id, "range").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeFloat(CHANGE_SPOT_LIGHT_RANGE, oldRange),
        GenerateMsgForHistory(id, "range"),
        id);

    data_.range = range;
}

//---------------------------------------------------------
// 1. change "attenuation" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeAttenuation(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec3& att)
{
    assert(pFacade);
    const Vec3 oldAttenuation = pFacade->GetSpotLightAttenuation(id);

    if (!pFacade->SetSpotLightAttenuation(id, att))
    {
        LogErr(GenerateErrMsg(id, "attenuation").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeVec3(CHANGE_SPOT_LIGHT_ATTENUATION, oldAttenuation),
        GenerateMsgForHistory(id, "attenuation"),
        id);

    data_.attenuation = att;
}

//---------------------------------------------------------
// 1. change "fallof / spot_exponent" property for spotlight of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttSpotLightController::ExecChangeSpotExponent(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float spotExponent)
{
    assert(pFacade);
    const float oldSpotExponent = pFacade->GetSpotLightSpotExponent(id);

    if (!pFacade->SetSpotLightSpotExponent(id, spotExponent))
    {
        LogErr(GenerateErrMsg(id, "spot exponent (fallof)").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeFloat(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, oldSpotExponent),
        GenerateMsgForHistory(id, "spot exponent"),
        id);

    data_.spotExp = spotExponent;
}

} // namespace UI
