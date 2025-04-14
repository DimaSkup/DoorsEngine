// =================================================================================
// Filename:       SkyEditorController.cpp
// Created:        31.12.24
// =================================================================================
#include "FogEditorController.h"

#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>


namespace UI
{

void FogEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	Core::Assert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
	pFacade_ = pFacade;


	// initialize the fog editor model  
	ColorRGB fogColor;
	float    fogStart;
	float    fogRange;

	if (pFacade_->GetFogData(fogColor, fogStart, fogRange))
		fogModel_.Update(fogColor, fogStart, fogRange);
	else
		Core::LogErr("can't gather data for the fog editor :(");
}

///////////////////////////////////////////////////////////

void FogEditorController::Execute(const ICommand* pCommand)
{
	if ((pCommand == nullptr) || (pFacade_ == nullptr))
	{
		Core::LogErr("ptr to command or facade interface == nullptr");
		return;
	}
	
	//using enum EditorCmdType;   // for not writing "FogEditorCmdType::" before each case
	ModelFog& data = fogModel_;

	// execute changes according to the command type
	switch (pCommand->type_)
	{
		case CHANGE_FOG_COLOR:
		{
			const ColorRGB newFogColor = pCommand->GetColorRGB();

			if (pFacade_->SetFogParams(newFogColor, data.GetStart(), data.GetRange()))
			{
				data.SetColor(newFogColor);
				// TODO: store the command into the events history
			}
			break;
		}

		// distance where for starts
		case CHANGE_FOG_START:        
		{
			const float newFogStart = pCommand->GetFloat();

			if (pFacade_->SetFogParams(data.GetColor(), newFogStart, data.GetRange()))
			{
				data.SetStart(newFogStart);
				// TODO: store the command into the events history
			}
			break;
		}

		// distance after which the objects are fully fogged
		case CHANGE_FOG_RANGE:         
		{
			const float newFogRange = pCommand->GetFloat();

			if (pFacade_->SetFogParams(data.GetColor(), data.GetStart(), newFogRange))
			{
				data.SetRange(newFogRange);
				// TODO: store the command into the events history
			}
			break;
		}
	}
}

///////////////////////////////////////////////////////////

void FogEditorController::Undo(const ICommand* pCommand, const uint32_t entityID)
{
	assert(0 && "TODO: implement it!");
}

} // namespace UI