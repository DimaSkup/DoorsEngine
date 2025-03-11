// =================================================================================
// Filename:      SkyController.cpp
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#include "SkyController.h"

#include <UICommon/EventsHistory.h>
#include <UICommon/EditorCommands.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <format>


namespace UI
{
	
SkyController::SkyController()
{
}

///////////////////////////////////////////////////////////

void SkyController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the facade == nullptr");
	pFacade_ = pFacade;
}

///////////////////////////////////////////////////////////

void SkyController::LoadEnttData(const uint32_t skyEnttID)
{
	// load/reload data of currently selected entity by ID (which is actually the sky entt)
	ColorRGB center;
	ColorRGB apex;
	Vec3 offset;

	if (pFacade_->GetSkyData(skyEnttID, center, apex, offset))
	{
		skyModel_.SetColorCenter(center);
		skyModel_.SetColorApex(apex);
		skyModel_.SetSkyOffset(offset);
	}
	else
	{
		Core::Log::Error("can't gather data for the sky editor model for unknown reason");
	}
}


// =================================================================================
// private API: commands executors
// =================================================================================

void SkyController::ExecuteCommand(const ICommand* pCmd, const uint32_t enttID)
{
	// execute the sky changes according to the command type
	switch (pCmd->type_)
	{
		// change the sky horizon color
	case CHANGE_SKY_COLOR_CENTER:
	{
		const ColorRGB newColorCenter = pCmd->GetColorRGB();

		if (pFacade_->SetSkyColorCenter(newColorCenter))
		{
			skyModel_.SetColorCenter(newColorCenter);
			// TODO: store the command into the events history
		}
		break;
	}
	// change the sky top color
	case CHANGE_SKY_COLOR_APEX:
	{
		const ColorRGB newColorApex = pCmd->GetColorRGB();

		if (pFacade_->SetSkyColorApex(newColorApex))
		{
			skyModel_.SetColorApex(newColorApex);
			// TODO: store the command into the events history
		}
		break;
	}
	case CHANGE_SKY_OFFSET:
	{
		const Vec3 newOffset = pCmd->GetVec3();

		if (pFacade_->SetSkyOffset(newOffset))
		{
			skyModel_.SetSkyOffset(newOffset);
			// TODO: store the command into the events history
		}
		break;
	}
	default:
	{
		Core::Log::Error("Unknown command to execute: " + std::to_string(pCmd->type_));
	}
	}; // switch
}

} // namespace UI