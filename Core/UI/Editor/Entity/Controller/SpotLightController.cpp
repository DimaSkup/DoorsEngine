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
#include <format>


namespace UI
{

SpotLightController::SpotLightController()
{
}

///////////////////////////////////////////////////////////

void SpotLightController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void SpotLightController::LoadEnttData(const uint32_t enttID)
{
	ModelEntitySpotLight& model = spotLightModel_;  // MVC model

	if (!pFacade_->GetEnttSpotLightData(
		enttID,
		model.data_.ambient,
		model.data_.diffuse,
		model.data_.specular,
		model.data_.position,
		model.data_.range,
		model.data_.direction,
		model.data_.spotExp,
		model.data_.attenuation))
	{
		Core::Log::Error("can't load data of the spotlight entity by ID: " + std::to_string(enttID));
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecuteCommand(const ICommand* pCmd, const uint32_t enttID)
{
	switch (pCmd->type_)
	{
		case CHANGE_SPOT_LIGHT_AMBIENT:
		{
			ExecChangeAmbient(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_SPOT_LIGHT_DIFFUSE:
		{
			ExecChangeDiffuse(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_SPOT_LIGHT_SPECULAR:
		{
			ExecChangeSpecular(enttID, pCmd->GetColorRGBA());
			break;
		}
		case CHANGE_SPOT_LIGHT_POSITION:
		{
			ExecChangePosition(enttID, pCmd->GetVec3());
			break;
		}
		case CHANGE_SPOT_LIGHT_DIRECTION:
		{
			ExecChangeDirectionByQuat(enttID, pCmd->GetVec4());
			break;
		}
		case CHANGE_SPOT_LIGHT_RANGE:           // how far spotlight can lit
		{
			ExecChangeRange(enttID, pCmd->GetFloat());
			break;
		}
		case CHANGE_SPOT_LIGHT_ATTENUATION:
		{
			ExecChangeAttenuation(enttID, pCmd->GetVec3());
			break;
		}
		case CHANGE_SPOT_LIGHT_SPOT_EXPONENT:   // light intensity fallof (for control the spotlight cone)
		{
			ExecChangeSpotExponent(enttID, pCmd->GetFloat());
			break;
		}
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::UndoCommand(const ICommand* pCmd, const uint32_t enttID)
{
	Core::Log::Error("IMPLEMENT ME");
}


// =================================================================================
// Private API: change spotlight properies
// =================================================================================
void SpotLightController::ExecChangeAmbient(const uint32_t enttID, const ColorRGBA& ambient)
{
	const ColorRGBA oldAmbient = pFacade_->GetSpotLightAmbient(enttID);

	if (pFacade_->SetSpotLightAmbient(enttID, ambient))
	{
		// update editor fields	
		spotLightModel_.data_.ambient = ambient;
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeDiffuse(const uint32_t enttID, const ColorRGBA& diffuse)
{
	const ColorRGBA oldDiffuse = pFacade_->GetSpotLightDiffuse(enttID);

	if (pFacade_->SetSpotLightDiffuse(enttID, diffuse))
	{
		// update editor fields	
		spotLightModel_.data_.diffuse = diffuse;
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeSpecular(const uint32_t enttID, const ColorRGBA& specular)
{
	const ColorRGBA oldSpecular = pFacade_->GetSpotLightSpecular(enttID);

	if (pFacade_->SetSpotLightSpecular(enttID, specular))
	{
		// update editor fields	
		spotLightModel_.data_.specular = specular;
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangePosition(const uint32_t enttID, const Vec3& pos)
{
	const Vec3 oldPos = pFacade_->GetSpotLightPos(enttID);

	if (pFacade_->SetSpotLightPos(enttID, pos))
	{
		// update editor fields	
		spotLightModel_.data_.position = pos;

		// generate an "undo" command and store it into the history
		gEventsHistory.Push(
			CmdChangeVec3(CHANGE_SPOT_LIGHT_POSITION, oldPos),
			std::format("changed position of entt (type: spot light; id: {})", enttID),
			enttID);
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeDirectionByQuat(const uint32_t enttID, const Vec4& quat)
{
	Core::Log::Error("FIXME");
	return;


	//const vec3 olddirquat = pfacade_->getspotlightdirectionvec(enttid);

	//if (pfacade_->setspotlightdirectionvec(enttid, quat))
	//{
	//	// update editor fields
	//	spotlightmodel_.setrotation(quat);

	//	// generate an "undo" command and store it into the history
	//	geventshistory.push(
	//		cmdchangevec3(change_spot_light_direction, olddirquat),
	//		std::format("changed direction of entt (type: spot light; id: {})", enttid),
	//		enttid);
	//}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeRange(const uint32_t enttID, const float range)
{
	const float oldRange = pFacade_->GetSpotLightRange(enttID);

	if (pFacade_->SetSpotLightRange(enttID, range))
	{
		// update editor fields
		spotLightModel_.data_.range = range;
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeAttenuation(const uint32_t enttID, const Vec3& attenuation)
{
	const Vec3 oldAttenuation = pFacade_->GetSpotLightAttenuation(enttID);

	if (pFacade_->SetSpotLightAttenuation(enttID, attenuation))
	{
		// update editor fields
		spotLightModel_.data_.attenuation = attenuation;
	}
}

///////////////////////////////////////////////////////////

void SpotLightController::ExecChangeSpotExponent(const uint32_t enttID, const float spotExponent)
{
	// change the light intensity fallof (for control the spotlight cone)

	const float oldSpotExponent = pFacade_->GetSpotLightSpotExponent(enttID);

	if (pFacade_->SetSpotLightSpotExponent(enttID, spotExponent))
	{
		// update editor fields
		spotLightModel_.data_.spotExp = spotExponent;
	}
}

} // namespace UI