// ====================================================================================
// Filename:      FogEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Fog;
//                contains fields to change the Fog params;
// 
// Created:       31.12.24
// ====================================================================================
#pragma once

#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>

#include <UICommon/IEditorController.h>
#include <UICommon/Color.h>
#include <UICommon/Vectors.h>
#include <UICommon/EditorCommands.h>

#include "FogEditorModel.h"
#include <imgui.h>


namespace UI
{

class ViewFog
{
private:
	IEditorController* pController_ = nullptr;

public:
	ViewFog(IEditorController* pController) : pController_(pController)
	{
		Core::Assert::NotNullptr(pController, "ptr to the view listener == nullptr");
	}

	void Draw(const ModelFog* pData)
	{
		//
		// show the fog editor fields
		//

		using enum eEditorCmdType;

		ColorRGB fogColor;
		float    fogStart;
		float    fogRange;

		// make local copies of the current model data to use it in the fields
		pData->GetData(fogColor, fogStart, fogRange);

		// draw editor fields
		if (ImGui::ColorEdit3("Fog color", fogColor.rgb))
		{
			pController_->Execute(new CmdChangeColor(CHANGE_FOG_COLOR, fogColor));
		}

		if (ImGui::DragFloat("Fog start", &fogStart))
		{
			pController_->Execute(new CmdChangeFloat(CHANGE_FOG_START, fogStart));
		}

		if (ImGui::DragFloat("Fog range", &fogRange))
		{
			pController_->Execute(new CmdChangeFloat(CHANGE_FOG_RANGE, fogRange));
		}
	}
};

} // namespace UI
