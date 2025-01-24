// ====================================================================================
// Filename:   SkyEditorController.cpp
// Created:    
// ====================================================================================
#include "SkyEditorController.h"

#include "../../../Common/Assert.h"
#include "../../../Common/log.h"


void SkyEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
	pFacade_ = pFacade;

	// initialize the sky editor model data 
	ColorRGB center;
	ColorRGB apex;
	Vec3 offset;
	uint32_t skyID = -1;

	pFacade_->GetEnttIDByName("sky", skyID);

	// if there is no such an entity
	if (skyID == 0)
		return;

	if (pFacade_->GatherSkyData(center, apex, offset))
	{
		skyModel_.SetColorCenter(center);
		skyModel_.SetColorApex(apex);
		skyModel_.SetSkyOffset(offset);
	}
	else
	{
		Log::Error("can't gather data for the sky editor model for unknown reason");
	}
	
}

///////////////////////////////////////////////////////////

void SkyEditorController::Draw()
{ 
	uint32_t skyID = -1;

	pFacade_->GetEnttIDByName("sky", skyID);

	if (skyID != 0)
		skyView_.Draw(&skyModel_); 
}

void SkyEditorController::Execute(ICommand* pCommand)
{
	if ((pCommand == nullptr) || (pFacade_ == nullptr))
	{
		Log::Error("ptr to command or facade interface == nullptr");
		return;
	}

	// for not writing "SkyEditorCmdType::" before each enum
	using enum SkyEditorCmdType;

	// execute changes according to the command type
	switch (pCommand->type_)
	{
		// change the sky horizon color
		case CHANGE_COLOR_CENTER:
		{
			const ColorRGB newColorCenter = pCommand->GetColorRGB();

			if (pFacade_->SetSkyColorCenter(newColorCenter))
			{
				skyModel_.SetColorCenter(newColorCenter);
				// TODO: store the command into the events history
			}
			break;
		}
		// change the sky top color
		case CHANGE_COLOR_APEX:
		{
			const ColorRGB newColorApex = pCommand->GetColorRGB();

			if (pFacade_->SetSkyColorApex(newColorApex))
			{
				skyModel_.SetColorApex(newColorApex);
				// TODO: store the command into the events history
			}
			break;
		}
		case CHANGE_SKY_OFFSET:
		{
			const Vec3 newOffset = pCommand->GetVec3();

			if (pFacade_->SetSkyOffset(newOffset))
			{
				skyModel_.SetSkyOffset(newOffset);
				// TODO: store the command into the events history
			}
			break;
		}
	}
}
