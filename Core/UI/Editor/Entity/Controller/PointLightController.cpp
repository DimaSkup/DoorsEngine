// =================================================================================
// Filename:      PointLightController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "PointLightController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <format>


namespace UI
{

void PointLightController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////
	
void PointLightController::LoadEnttData(const EntityID id)
{
	// load/reload data of currently selected entity by ID (which is a point light source)
	ModelEntityPointLight& model = pointLightModel_;

	if (!pFacade_ || !pFacade_->GetEnttPointLightData(
		id,
		model.data_.ambient,
		model.data_.diffuse,
		model.data_.specular,
		model.data_.position,
		model.data_.range,
		model.data_.attenuation))
    {
        Core::Log::Error("can't load data of the point light entity by ID: " + std::to_string(id));
    }
}

///////////////////////////////////////////////////////////

void PointLightController::ExecuteCommand(const ICommand* pCmd,	const EntityID id)
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
		case CHANGE_POINT_LIGHT_POSITION:
		{
			ExecChangePos(id, pCmd->GetVec3());
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
            Core::Log::Error("unknown type of command: " + pCmd->type_);
        }
	}
}

///////////////////////////////////////////////////////////

void PointLightController::UndoCommand(const ICommand* pCmd, const EntityID id)
{
	// "undo" the change of point light entity by ID according to the input command;

	switch (pCmd->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			if (pFacade_->SetPointLightAmbient(id, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.data_.ambient = pCmd->GetColorRGBA();         // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			if (pFacade_->SetPointLightDiffuse(id, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.data_.diffuse = pCmd->GetColorRGBA();         // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			if (pFacade_->SetPointLightSpecular(id, pCmd->GetColorRGBA())) // update entity
				pointLightModel_.data_.specular = pCmd->GetColorRGBA();        // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_POSITION:
		{
			if (pFacade_->SetPointLightPos(id, pCmd->GetVec3()))           // update entity
				pointLightModel_.data_.position = pCmd->GetVec3();             // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			if (pFacade_->SetPointLightRange(id, pCmd->GetFloat()))        // update entity
				pointLightModel_.data_.range = pCmd->GetFloat();               // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			if (pFacade_->SetPointLightAttenuation(id, pCmd->GetVec3()))   // update entity
				pointLightModel_.data_.attenuation = pCmd->GetVec3();          // update editor fields
			break;
		}
		default:
		{
			Core::Log::Error("unknown undo command for entity (point light): " + std::to_string(id));
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

void PointLightController::ExecChangeAmbient(const EntityID id, const ColorRGBA& ambient)
{
	const ColorRGBA& oldAmbient = pFacade_->GetPointLightAmbient(id);

	if (pFacade_->SetPointLightAmbient(id, ambient))
	{
		// update editor fields
		pointLightModel_.data_.ambient = ambient;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_AMBIENT, oldAmbient),
            GenerateMsgForHistory(id, "ambient"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "ambient"));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeDiffuse(const EntityID id, const ColorRGBA& diffuse)
{
	const ColorRGBA& oldDiffuse = pFacade_->GetPointLightDiffuse(id);

	if (pFacade_->SetPointLightDiffuse(id, diffuse))
	{
		// update editor fields
		pointLightModel_.data_.diffuse = diffuse;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_DIFFUSE, oldDiffuse),
            GenerateMsgForHistory(id, "diffuse"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "diffuse"));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeSpecular(const EntityID id, const ColorRGBA& specular)
{
	const ColorRGBA& oldSpecular = pFacade_->GetPointLightSpecular(id);

	if (pFacade_->SetPointLightSpecular(id, specular))
	{
		// update editor fields
		pointLightModel_.data_.specular = specular;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_SPECULAR, oldSpecular),
            GenerateMsgForHistory(id, "specular"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "specular"));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangePos(const EntityID id, const Vec3& pos)
{
	const Vec3 oldPos = pFacade_->GetPointLightPos(id);

	if (pFacade_->SetPointLightPos(id, pos))
	{
		// update editor fields	
		pointLightModel_.data_.position = pos;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_POINT_LIGHT_POSITION, oldPos),
            GenerateMsgForHistory(id, "position"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "position"));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeRange(const EntityID id, const float range)
{
	const float oldRange = pFacade_->GetPointLightRange(id);

	if (pFacade_->SetPointLightRange(id, range))
	{
		// update editor fields
		pointLightModel_.data_.range = range;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeFloat(CHANGE_POINT_LIGHT_RANGE, oldRange),
            GenerateMsgForHistory(id, "range"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "range"));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeAttenuation(const EntityID id, const Vec3& att)
{
	const Vec3 oldAttenuation = pFacade_->GetPointLightAttenuation(id);

	if (pFacade_->SetPointLightAttenuation(id, att))
	{
		// update editor fields
		pointLightModel_.data_.attenuation = att;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_POINT_LIGHT_ATTENUATION, oldAttenuation),
            GenerateMsgForHistory(id, "attenuation"),
			id);
	}
	else
	{
        Core::Log::Error(GenerateErrMsg(id, "attenuation"));
	}
}

} // namespace UI
