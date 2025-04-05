// =================================================================================
// Filename:      EnttDirLightController.cpp
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#include "EnttDirLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>

namespace UI
{

void EnttDirLightController::Initialize(IFacadeEngineToUI* pFacade)
{
    // the facade interface is used to contact with the rest of the engine
    Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
    pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void EnttDirLightController::LoadEnttData(const EntityID id)
{
    // load/reload data of currently selected entity by ID (which is a directed light source)
    EnttDirLightData& model = dirLightModel_;

    if (!pFacade_ || !pFacade_->GetEnttDirectedLightData(
        id,
        model.ambient,
        model.diffuse,
        model.specular))
    {
        Core::Log::Error("can't load data of the directed light entity by ID: " + std::to_string(id));
    }
}

///////////////////////////////////////////////////////////

void EnttDirLightController::ExecuteCommand(const ICommand* pCmd, const EntityID id)
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
        default:
        {
            Core::Log::Error("unknown type of command: " + pCmd->type_);
        }
    }
}

///////////////////////////////////////////////////////////

void EnttDirLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
    // "undo" the change of directed light entity by ID according to the input command;

    switch (pCmd->type_)
    {
        case CHANGE_DIR_LIGHT_AMBIENT:
        {
            if (pFacade_->SetDirectedLightAmbient(id, pCmd->GetColorRGBA()))    // update entity
                dirLightModel_.ambient = pCmd->GetColorRGBA();                  // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_DIFFUSE:
        {
            if (pFacade_->SetDirectedLightDiffuse(id, pCmd->GetColorRGBA()))    // update entity
                dirLightModel_.diffuse = pCmd->GetColorRGBA();                  // update editor
            break;
        }
        case CHANGE_DIR_LIGHT_SPECULAR:
        {
            if (pFacade_->SetDirectedLightSpecular(id, pCmd->GetColorRGBA()))   // update entity
                dirLightModel_.specular = pCmd->GetColorRGBA();                 // update editor
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

void EnttDirLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
    const ColorRGBA oldAmbient = pFacade_->GetDirectedLightAmbient(id);

    if (pFacade_->SetDirectedLightAmbient(id, ambient))
    {
        // update editor fields
        dirLightModel_.ambient = ambient;

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

void EnttDirLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
    const ColorRGBA oldDiffuse = pFacade_->GetDirectedLightDiffuse(id);

    if (pFacade_->SetDirectedLightDiffuse(id, diffuse))
    {
        // update editor fields
        dirLightModel_.diffuse = diffuse;

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

void EnttDirLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
    const ColorRGBA oldSpecular = pFacade_->GetDirectedLightSpecular(id);

    if (pFacade_->SetDirectedLightSpecular(id, specular))
    {
        // update editor fields
        dirLightModel_.specular = specular;

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

} // namespace UI


