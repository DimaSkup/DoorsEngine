// ====================================================================================
// Filename:      FogEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Fog;
//                contains fields to change the Fog params;
// 
// Created:       31.12.24
// ====================================================================================
#pragma once

// engine common
#include "../../../Common/log.h"
#include "../../../Common/Assert.h"

// UI common
#include "../../UICommon/ViewListener.h"
#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"

#include "FogEditorCommands.h"
#include "FogEditorModel.h"

#include <imgui.h>


namespace View
{

class Fog
{
private:
	ViewListener* pListener_ = nullptr;

public:
	Fog(ViewListener* pListener) : pListener_(pListener)
	{
		Assert::NotNullptr(pListener, "ptr to the view listener == nullptr");
	}

	void Draw(const Model::Fog* pData)
	{
		//
		// show the fog editor fields
		//

		using enum FogEditorCmdType;

		ColorRGB fogColor;
		float    fogStart;
		float    fogRange;

		// make local copies of the current model data to use it in the fields
		pData->GetData(fogColor, fogStart, fogRange);

		// draw editor fields
		if (ImGui::ColorEdit3("Fog color", fogColor.rgb_))
		{
			pListener_->Execute(new CmdFogChangeColor(CHANGE_FOG_COLOR, fogColor));
		}

		if (ImGui::DragFloat("Fog start", &fogStart))
		{
			pListener_->Execute(new CmdFogChangeFloat(CHANGE_FOG_START, fogStart));
		}

		if (ImGui::DragFloat("Fog range", &fogRange))
		{
			pListener_->Execute(new CmdFogChangeFloat(CHANGE_FOG_RANGE, fogRange));
		}
	}
};

}