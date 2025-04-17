// =================================================================================
// Filename:      EnttPointLightController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "EnttPointLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>

using namespace Core;

namespace UI
{

void EnttPointLightController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////
	
void EnttPointLightController::LoadEnttData(const EntityID id)
{
	// load/reload data of currently selected entity by ID (which is a point light source)
	EnttPointLightData& model = pointLightModel_;

    if (pFacade_ == nullptr)
    {
        LogErr("ptr to the facade interface == nullptr");
        return;
    }

	if (!pFacade_->GetEnttPointLightData(
		id,
		model.ambient,
		model.diffuse,
		model.specular,
		model.attenuation,
		model.range))
    {
        sprintf(g_String, "can't load point light component data of entt ID: %ld", id);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecuteCommand(const ICommand* pCmd,	const EntityID id)
{
	switch (pCmd->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			ExecChangeAmbient(id, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			ExecChangeDiffuse(id, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			ExecChangeSpecular(id, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			ExecChangeRange(id, pCmd->GetFloat());
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			ExecChangeAttenuation(id, pCmd->GetVec3());
			break;
		}
        default:
        {
            Core::LogErr("unknown type of command: " + pCmd->type_);
        }
	}
}

///////////////////////////////////////////////////////////

void EnttPointLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
	// "undo" the change of point light entity by ID according to the input command;

	switch (pCmd->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			if (pFacade_->SetPointLightAmbient(id, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.ambient = pCmd->GetColorRGBA();           // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			if (pFacade_->SetPointLightDiffuse(id, pCmd->GetColorRGBA()))   // update entity
				pointLightModel_.diffuse = pCmd->GetColorRGBA();            // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			if (pFacade_->SetPointLightSpecular(id, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.specular = pCmd->GetColorRGBA();           // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			if (pFacade_->SetPointLightRange(id, pCmd->GetFloat()))         // update entity
				pointLightModel_.range = pCmd->GetFloat();                  // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			if (pFacade_->SetPointLightAttenuation(id, pCmd->GetVec3()))    // update entity
				pointLightModel_.attenuation = pCmd->GetVec3();             // update editor fields
			break;
		}
		default:
		{
			sprintf(g_String, "unknown undo command (point light) for entity: %ld", id);
            LogErr(g_String);
			return;
		}
	}
}


// =================================================================================
// Private API: commands executors 
// (execute some change of point light source and store this event into history)
// =================================================================================

static std::string GenerateMsgForHistory(const EntityID id, const std::string& propertyName)
{
    return "changed " + propertyName + " of entt (type: point light; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

static std::string GenerateErrMsg(const EntityID id, const std::string& propertyName)
{
    return "can't change " + propertyName + " of entt (type: point light; id: " + std::to_string(id) + ")";
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
	const ColorRGBA& oldAmbient = pFacade_->GetPointLightAmbient(id);

	if (pFacade_->SetPointLightAmbient(id, ambient))
	{
		// update editor fields
		pointLightModel_.ambient = ambient;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_AMBIENT, oldAmbient),
            GenerateMsgForHistory(id, "ambient"),
			id);
	}
	else
	{
        LogErr(GenerateErrMsg(id, "ambient").c_str());
	}
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
	const ColorRGBA& oldDiffuse = pFacade_->GetPointLightDiffuse(id);

	if (pFacade_->SetPointLightDiffuse(id, diffuse))
	{
		// update editor fields
		pointLightModel_.diffuse = diffuse;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_DIFFUSE, oldDiffuse),
            GenerateMsgForHistory(id, "diffuse"),
			id);
	}
	else
	{
        Core::LogErr(GenerateErrMsg(id, "diffuse").c_str());
	}
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
	const ColorRGBA& oldSpecular = pFacade_->GetPointLightSpecular(id);

	if (pFacade_->SetPointLightSpecular(id, specular))
	{
		// update editor fields
		pointLightModel_.specular = specular;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_SPECULAR, oldSpecular),
            GenerateMsgForHistory(id, "specular"),
			id);
	}
	else
	{
        LogErr(GenerateErrMsg(id, "specular").c_str());
	}
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecChangeRange(const EntityID id, const float range)
{
	const float oldRange = pFacade_->GetPointLightRange(id);

	if (pFacade_->SetPointLightRange(id, range))
	{
		// update editor fields
		pointLightModel_.range = range;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeFloat(CHANGE_POINT_LIGHT_RANGE, oldRange),
            GenerateMsgForHistory(id, "range"),
			id);
	}
	else
	{
        LogErr(GenerateErrMsg(id, "range").c_str());
	}
}

///////////////////////////////////////////////////////////

void EnttPointLightController::ExecChangeAttenuation(const EntityID id, const Vec3& att)
{
	const Vec3 oldAttenuation = pFacade_->GetPointLightAttenuation(id);

	if (pFacade_->SetPointLightAttenuation(id, att))
	{
		// update editor fields
		pointLightModel_.attenuation = att;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_POINT_LIGHT_ATTENUATION, oldAttenuation),
            GenerateMsgForHistory(id, "attenuation"),
			id);
	}
	else
	{
        LogErr(GenerateErrMsg(id, "attenuation").c_str());
	}
}

} // namespace UI
