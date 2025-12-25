// =================================================================================
// Filename:      ui_entt_point_light_controller.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_point_light_controller.h"

#include <UICommon/icommand.h>
#include <UICommon/events_history.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/IFacadeEngineToUI.h>


#pragma warning (disable : 4996)

namespace UI
{

//---------------------------------------------------------
// Desc:  load/reload point light data of currently selected entity by ID
//---------------------------------------------------------
void EnttPointLightController::LoadEnttData(
    IFacadeEngineToUI* pFacade,
    const EntityID id)
{
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }

    EnttPointLightData& L = data_;

    if (!pFacade->GetEnttPointLightData(
        id,
        L.ambient,
        L.diffuse,
        L.specular,
        L.attenuation,
        L.range,
        L.isActive))
    {
        LogErr(LOG, "can't load point light data of entt ID: %" PRIu32, id);
    }
}

//---------------------------------------------------------
// Desc:  execute change of point light property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttPointLightController::ExecCmd(
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
        case CHANGE_POINT_LIGHT_ACTIVATION:
            ExecChangeActivation(pFacade, id, pCmd->GetFloat());
            break;

        case CHANGE_POINT_LIGHT_AMBIENT:
            ExecChangeAmbient(pFacade, id, pCmd->GetColorRGBA());
            break;

        case CHANGE_POINT_LIGHT_DIFFUSE:
            ExecChangeDiffuse(pFacade, id, pCmd->GetColorRGBA());
            break;

        case CHANGE_POINT_LIGHT_SPECULAR:
            ExecChangeSpecular(pFacade, id, pCmd->GetColorRGBA());
            break;

        case CHANGE_POINT_LIGHT_RANGE:
            ExecChangeRange(pFacade, id, pCmd->GetFloat());
            break;

        case CHANGE_POINT_LIGHT_ATTENUATION:
            ExecChangeAttenuation(pFacade, id, pCmd->GetVec3());
            break;

        default:
            LogErr(LOG, "unknown type of command: %d", pCmd->type_);
    }
}

//---------------------------------------------------------
// Desc:  undo change of point light property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttPointLightController::UndoCmd(
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
        case CHANGE_POINT_LIGHT_ACTIVATION:
        {
            LogErr(LOG, "you forgot to implement me :(");
            return;
        }
        case CHANGE_POINT_LIGHT_AMBIENT:
        {
            if (pFacade->SetPointLightAmbient(id, pCmd->GetColorRGBA()))    // update entity
                data_.ambient = pCmd->GetColorRGBA();                       // update editor fields
            break;
        }
        case CHANGE_POINT_LIGHT_DIFFUSE:
        {
            if (pFacade->SetPointLightDiffuse(id, pCmd->GetColorRGBA()))    // update entity
                data_.diffuse = pCmd->GetColorRGBA();                       // update editor fields
            break;
        }
        case CHANGE_POINT_LIGHT_SPECULAR:
        {
            if (pFacade->SetPointLightSpecular(id, pCmd->GetColorRGBA()))   // update entity
                data_.specular = pCmd->GetColorRGBA();                      // update editor fields
            break;
        }
        case CHANGE_POINT_LIGHT_RANGE:
        {
            if (pFacade->SetPointLightRange(id, pCmd->GetFloat()))          // update entity
                data_.range = pCmd->GetFloat();                             // update editor fields
            break;
        }
        case CHANGE_POINT_LIGHT_ATTENUATION:
        {
            if (pFacade->SetPointLightAttenuation(id, pCmd->GetVec3()))     // update entity
                data_.attenuation = pCmd->GetVec3();                        // update editor fields
            break;
        }
        default:
        {
            LogErr(LOG, "unknown undo command (%d) for entity: %" PRIu32, pCmd->type_, id);
        }
    }
}


// =================================================================================
// Private API: commands executors 
// (execute some change of point light source and store this event into history)
// =================================================================================

static std::string GenerateMsgForHistory(const EntityID id, const std::string& propertyName)
{
    return "changed '" + propertyName + "' of entt (type: point light; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

static std::string GenerateErrMsg(const EntityID id, const std::string& propertyName)
{
    return "can't change '" + propertyName + "' of entt (type: point light; id: " + std::to_string(id) + ")";
}

//---------------------------------------------------------
// 1. change "activation" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeActivation(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float isActive)
{
    assert(pFacade);
    const float oldActivation = pFacade->GetLightIsActive(id);

    if (!pFacade->SetLightActive(id, isActive))
    {
        LogErr(LOG, GenerateErrMsg(id, "activation").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeFloat(CHANGE_POINT_LIGHT_ACTIVATION, oldActivation),
        GenerateMsgForHistory(id, "activation"),
        id);

    data_.isActive = (bool)isActive;
}

//---------------------------------------------------------
// 1. change "ambient" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeAmbient(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& ambient)
{
    assert(pFacade);
    const ColorRGBA& oldAmbient = pFacade->GetPointLightAmbient(id);

    if (!pFacade->SetPointLightAmbient(id, ambient))
    {
        LogErr(LOG, GenerateErrMsg(id, "ambient").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_POINT_LIGHT_AMBIENT, oldAmbient),
        GenerateMsgForHistory(id, "ambient"),
        id);

    data_.ambient = ambient;
}

//---------------------------------------------------------
// 1. change "diffuse" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeDiffuse(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& diffuse)
{
    assert(pFacade);
    const ColorRGBA& oldDiffuse = pFacade->GetPointLightDiffuse(id);

    if (!pFacade->SetPointLightDiffuse(id, diffuse))
    {
        LogErr(LOG, GenerateErrMsg(id, "diffuse").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_POINT_LIGHT_DIFFUSE, oldDiffuse),
        GenerateMsgForHistory(id, "diffuse"),
        id);

    data_.diffuse = diffuse;
}

//---------------------------------------------------------
// 1. change "specular" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeSpecular(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& specular)
{
    assert(pFacade);
    const ColorRGBA& oldSpecular = pFacade->GetPointLightSpecular(id);

    if (!pFacade->SetPointLightSpecular(id, specular))
    {
        LogErr(LOG, GenerateErrMsg(id, "specular").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_POINT_LIGHT_SPECULAR, oldSpecular),
        GenerateMsgForHistory(id, "specular"),
        id);

    data_.specular = specular;
}

//---------------------------------------------------------
// 1. change "range" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeRange(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const float range)
{
    assert(pFacade);
    const float oldRange = pFacade->GetPointLightRange(id);

    if (!pFacade->SetPointLightRange(id, range))
    {
        LogErr(LOG, GenerateErrMsg(id, "range").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeFloat(CHANGE_POINT_LIGHT_RANGE, oldRange),
        GenerateMsgForHistory(id, "range"),
        id);

    data_.range = range;
}

//---------------------------------------------------------
// 1. change "attenuation" property for point light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttPointLightController::ExecChangeAttenuation(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const Vec3& att)
{
    assert(pFacade);
    const Vec3 oldAttenuation = pFacade->GetPointLightAttenuation(id);

    if (!pFacade->SetPointLightAttenuation(id, att))
    {
        LogErr(LOG, GenerateErrMsg(id, "attenuation").c_str());
        return;
    }

    // "undo" cmd
    g_EventsHistory.Push(
        CmdChangeVec3(CHANGE_POINT_LIGHT_ATTENUATION, oldAttenuation),
        GenerateMsgForHistory(id, "attenuation"),
        id);

    data_.attenuation = att;
}

} // namespace UI
