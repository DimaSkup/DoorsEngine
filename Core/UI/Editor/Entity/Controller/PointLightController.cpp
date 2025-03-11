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

PointLightController::PointLightController()
{
}

///////////////////////////////////////////////////////////

void PointLightController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////
	
void PointLightController::LoadEnttData(const uint32_t enttID)
{
	// load/reload data of currently selected entity by ID (which is a point light source)
	ModelEntityPointLight& model = pointLightModel_;

	pFacade_->GetEnttPointLightData(
		enttID,
		model.data_.ambient,
		model.data_.diffuse,
		model.data_.specular,
		model.data_.position,
		model.data_.range,
		model.data_.attenuation);
}

///////////////////////////////////////////////////////////

void PointLightController::ExecuteCommand(const ICommand* pCmd,	const uint32_t enttID)
{
	switch (pCmd->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			ExecChangeAmbient(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			ExecChangeDiffuse(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			ExecChangeSpecular(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_POINT_LIGHT_POSITION:
		{
			ExecChangePos(enttID, pCmd->GetVec3());
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			ExecChangeRange(enttID, pCmd->GetFloat());
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			ExecChangeAttenuation(enttID, pCmd->GetVec3());
			break;
		}
	}
}

///////////////////////////////////////////////////////////

void PointLightController::UndoCommand(const ICommand* pCmd, const uint32_t enttID)
{
	// "undo" the change of point light entity by ID according to the input command;
	// we simple do the reverse command 
	// (for instance: we changed pos from <0,0,0> to <10, 10, 10> so when undo we
	// execute command to change pos by <-10,-10,-10>)

	switch (pCmd->type_)
	{
		case CHANGE_POINT_LIGHT_AMBIENT:
		{
			if (pFacade_->SetPointLightAmbient(enttID, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.data_.ambient = pCmd->GetColorRGBA();              // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_DIFFUSE:
		{
			if (pFacade_->SetPointLightDiffuse(enttID, pCmd->GetColorRGBA()))  // update entity
				pointLightModel_.data_.diffuse = pCmd->GetColorRGBA();              // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_SPECULAR:
		{
			if (pFacade_->SetPointLightSpecular(enttID, pCmd->GetColorRGBA())) // update entity
				pointLightModel_.data_.specular = pCmd->GetColorRGBA();             // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_POSITION:
		{
			if (pFacade_->SetPointLightPos(enttID, pCmd->GetVec3()))           // update entity
				pointLightModel_.data_.position = pCmd->GetVec3();                  // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_RANGE:
		{
			if (pFacade_->SetPointLightRange(enttID, pCmd->GetFloat()))        // update entity
				pointLightModel_.data_.range = pCmd->GetFloat();                    // update editor fields
			break;
		}
		case CHANGE_POINT_LIGHT_ATTENUATION:
		{
			if (pFacade_->SetPointLightAttenuation(enttID, pCmd->GetVec3()))   // update entity
				pointLightModel_.data_.attenuation = pCmd->GetVec3();               // update editor fields
			break;
		}
		default:
		{
			Core::Log::Error("unknown undo command for entity (point light): " + std::to_string(enttID));
			return;
		}
	}
}


// =================================================================================
// Private API: commands executors 
// (execute some change of point light source and store this event into history)
// =================================================================================

void PointLightController::ExecChangeAmbient(const uint32_t enttID, const ColorRGBA& ambient)
{
	const ColorRGBA& oldAmbient = pFacade_->GetPointLightAmbient(enttID);

	if (pFacade_->SetPointLightAmbient(enttID, ambient))
	{
		// update editor fields
		pointLightModel_.data_.ambient = ambient;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_AMBIENT, oldAmbient),
			std::format("changed ambient of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change ambient of entt (type: point light; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeDiffuse(const uint32_t enttID, const ColorRGBA& diffuse)
{
	const ColorRGBA& oldDiffuse = pFacade_->GetPointLightDiffuse(enttID);

	if (pFacade_->SetPointLightDiffuse(enttID, diffuse))
	{
		// update editor fields
		pointLightModel_.data_.diffuse = diffuse;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_DIFFUSE, oldDiffuse),
			std::format("changed diffuse of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change diffuse of entt (type: point light; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeSpecular(const uint32_t enttID, const ColorRGBA& specular)
{
	const ColorRGBA& oldSpecular = pFacade_->GetPointLightSpecular(enttID);

	if (pFacade_->SetPointLightSpecular(enttID, specular))
	{
		// update editor fields
		pointLightModel_.data_.specular = specular;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeColor(CHANGE_POINT_LIGHT_SPECULAR, oldSpecular),
			std::format("changed specular of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change specular of entt (type: point light; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangePos(const uint32_t enttID, const Vec3& pos)
{
	const Vec3 oldPos = pFacade_->GetPointLightPos(enttID);

	if (pFacade_->SetPointLightPos(enttID, pos))
	{
		// update editor fields	
		pointLightModel_.data_.position = pos;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_POINT_LIGHT_POSITION, oldPos),
			std::format("changed position of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change position of entt (type: point light; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeRange(const uint32_t enttID, const float range)
{
	const float oldRange = pFacade_->GetPointLightRange(enttID);

	if (pFacade_->SetPointLightRange(enttID, range))
	{
		// update editor fields
		pointLightModel_.data_.range = range;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeFloat(CHANGE_POINT_LIGHT_RANGE, oldRange),
			std::format("changed range of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change range of entt (type: point light; id: {})", enttID));
	}
}

///////////////////////////////////////////////////////////

void PointLightController::ExecChangeAttenuation(const uint32_t enttID, const Vec3& att)
{
	const Vec3 oldAttenuation = pFacade_->GetPointLightAttenuation(enttID);

	if (pFacade_->SetPointLightAttenuation(enttID, att))
	{
		// update editor fields
		pointLightModel_.data_.attenuation = att;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_POINT_LIGHT_ATTENUATION, oldAttenuation),
			std::format("changed attenuation of entt (type: point light; id: {})", enttID),
			enttID);
	}
	else
	{
		Core::Log::Error(std::format("can't change attenuation of entt (type: point light; id: {})", enttID));
	}
}

} // namespace UI