// =================================================================================
// Filename:       SkyEditorController.cpp
// Created:        31.12.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "FogEditorController.h"


namespace UI
{

void FogEditorController::Initialize(IFacadeEngineToUI* pFacade)
{
	// the facade interface is used to contact with the rest of the engine
	CAssert::NotNullptr(pFacade, "ptr to the IFacadeEngineToUI interface == nullptr");
	pFacade_ = pFacade;


	// initialize the fog editor model  
	ColorRGB fogColor;
	float    fogStart = 5.0f;
	float    fogRange = 100.0f;
    bool     fogEnabled = true;

	if (pFacade_->GetFogData(fogColor, fogStart, fogRange, fogEnabled))
		fogModel_.Update(fogColor, fogStart, fogRange, fogEnabled);
	else
		LogErr("can't gather data for the fog editor :(");
}

///////////////////////////////////////////////////////////

void FogEditorController::Execute(const ICommand* pCmd)
{
    if (!pCmd)
    {
        LogErr("input ptr to command == nullptr");
        return;
    }

    if (!pFacade_)
    {
        LogErr("ptr to the facade interface == nullptr (you should init it first!)");
        return;
    }

	
	// execute changes according to the command type
	switch (pCmd->type_)
	{
        case CHANGE_FOG_ENABLED:
        {
            // enable/disable the scene fog
            const float oldFogState = fogModel_.GetEnabled();
            const bool newFogState = (bool)pCmd->GetFloat();

            if (pFacade_->SetFogEnabled(newFogState))
            {
                fogModel_.SetEnabled(newFogState);
                // TODO: store the command into the events history
            }
            break;
        }
		case CHANGE_FOG_COLOR:
		{
			const ColorRGB newFogColor = pCmd->GetColorRGB();

			if (pFacade_->SetFogColor(newFogColor))
			{
                fogModel_.SetColor(newFogColor);
				// TODO: store the command into the events history
			}
			break;
		}

		// distance where for starts
		case CHANGE_FOG_START:        
		{
			const float newFogStart = pCmd->GetFloat();

			if (pFacade_->SetFogStart(newFogStart))
			{
                fogModel_.SetStart(newFogStart);
				// TODO: store the command into the events history
			}
			break;
		}

		// distance after which the objects are fully fogged
		case CHANGE_FOG_RANGE:         
		{
			const float newFogRange = pCmd->GetFloat();

			if (pFacade_->SetFogRange(newFogRange))
			{
                fogModel_.SetRange(newFogRange);
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
