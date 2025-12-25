// =================================================================================
// Filename:      ui_entt_dir_light_controller.cpp
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_dir_light_controller.h"
#include <UICommon/events_history.h>
#include <UICommon/icommand.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/IFacadeEngineToUI.h>

#pragma warning (disable : 4996)


namespace UI
{

//---------------------------------------------------------
// Desc:  load/reload directed light data of currently selected entity by ID
//---------------------------------------------------------
void EnttDirLightController::LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id)
{
    if (!pFacade)
    {
        LogErr(LOG, "UI facade ptr == nullptr");
        return;
    }

    EnttDirLightData& L = data_;

    if (!pFacade->GetEnttDirectedLightData(id, L.ambient, L.diffuse, L.specular))
    {
        LogErr(LOG, "can't load data of the directed light for entity by ID: %" PRIu32, id);
    }
}

//---------------------------------------------------------
// Desc:  execute change of directed light property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttDirLightController::ExecCmd(
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
        case CHANGE_DIR_LIGHT_AMBIENT:
            ExecChangeAmbient(pFacade, id, pCmd->GetColorRGBA());
            break;
        
        case CHANGE_DIR_LIGHT_DIFFUSE:
            ExecChangeDiffuse(pFacade, id, pCmd->GetColorRGBA());
            break;

        case CHANGE_DIR_LIGHT_SPECULAR:
            ExecChangeSpecular(pFacade, id, pCmd->GetColorRGBA());
            break;

        default:
            LogErr(LOG, "unknown type of command: %d", pCmd->type_);
    }
}
//---------------------------------------------------------
// Desc:  undo change of directed light property for entity by ID
//        according to input command
//---------------------------------------------------------
void EnttDirLightController::UndoCmd(
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
        case CHANGE_DIR_LIGHT_AMBIENT:
        {
            if (pFacade->SetDirectedLightAmbient(id, pCmd->GetColorRGBA()))     // update entity
                data_.ambient = pCmd->GetColorRGBA();                           // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_DIFFUSE:
        {
            if (pFacade->SetDirectedLightDiffuse(id, pCmd->GetColorRGBA()))     // update entity
                data_.diffuse = pCmd->GetColorRGBA();                           // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            if (pFacade->SetDirectedLightSpecular(id, pCmd->GetColorRGBA()))    // update entity
                data_.specular = pCmd->GetColorRGBA();                          // update editor
            break;
        }

        default:
            LogErr(LOG, "unknown undo command for entity (directed light): %" PRIu32, id);
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

//---------------------------------------------------------
// 1. change "ambient" property for directed light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttDirLightController::ExecChangeAmbient(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& ambient)
{
    assert(pFacade);
    const ColorRGBA oldAmbient = pFacade->GetDirectedLightAmbient(id);

    if (!pFacade->SetDirectedLightAmbient(id, ambient))
    {
        LogErr(GenerateErrMsg(id, "ambient").c_str());
        return;
    }

    // generate an "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_DIR_LIGHT_AMBIENT, oldAmbient),
        GenerateMsgForHistory(id, "ambient"),
        id);

    data_.ambient = ambient;
}

//---------------------------------------------------------
// 1. change "diffuse" property for directed light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttDirLightController::ExecChangeDiffuse(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& diffuse)
{
    assert(pFacade);
    const ColorRGBA oldDiffuse = pFacade->GetDirectedLightDiffuse(id);

    if (!pFacade->SetDirectedLightDiffuse(id, diffuse))
    {
        LogErr(GenerateErrMsg(id, "diffuse").c_str());
        return;
    }
       
    // generate an "undo" cmd
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_DIR_LIGHT_DIFFUSE, oldDiffuse),
        GenerateMsgForHistory(id, "diffuse"),
        id);

    data_.diffuse = diffuse;
}

//---------------------------------------------------------
// 1. change "specular" property for directed light of entity by ID
// 2. generate an "undo" command and store it into the editor's history
// 3. update related editor's field
//---------------------------------------------------------
void EnttDirLightController::ExecChangeSpecular(
    IFacadeEngineToUI* pFacade,
    const EntityID id,
    const ColorRGBA& specular)
{
    assert(pFacade);
    const ColorRGBA oldSpecular = pFacade->GetDirectedLightSpecular(id);

    if (!pFacade->SetDirectedLightSpecular(id, specular))
    {
        LogErr(GenerateErrMsg(id, "specular").c_str());
        return;
    }

    // generate an "undo" command and store it into the history
    g_EventsHistory.Push(
        CmdChangeColor(CHANGE_DIR_LIGHT_SPECULAR, oldSpecular),
        GenerateMsgForHistory(id, "specular"),
        id);

    data_.specular = specular;
}

} // namespace UI


