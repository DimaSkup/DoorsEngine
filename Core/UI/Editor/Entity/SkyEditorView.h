// ====================================================================================
// Filename:      SkyEditorView.h
// Description:   the View part of the SkyEditor MVC;
//                visualises data of the Model::Sky;
//                contains fields to change the Sky params;
// ====================================================================================
#pragma once

// engine common
#include "../../../Common/log.h"
#include "../../../Common/Assert.h"

// UI common
#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"

#include "EntityEditorCommands.h"
#include "SkyEditorModel.h"

#include <imgui.h>


namespace View
{

class Sky
{
private:
	ViewListener* pListener_ = nullptr;

public:
	Sky(ViewListener* pListener) : pListener_(pListener)
	{
		Assert::NotNullptr(pListener, "ptr to the view listener == nullptr");
	}

	// ----------------------------------------------------

	void Render(Model::Sky* pData)
	{
		using enum EntityEditorCmdType;
	
		ColorRGB colorCenter;
		ColorRGB colorApex;
		Vec3 offset;

		// make local copies of the current model data to use it in the fields
		pData->GetColorCenter(colorCenter);
		pData->GetColorApex(colorApex);
		pData->GetSkyOffset(offset);

		// draw editor fields
		if (ImGui::ColorEdit3("Center color", colorCenter.rgb_))
		{
			pListener_->Execute(new CmdChangeColor(CHANGE_SKY_COLOR_CENTER, colorCenter));
		}

		if (ImGui::ColorEdit3("Apex color", colorApex.rgb_))
		{
			pListener_->Execute(new CmdChangeColor(CHANGE_SKY_COLOR_APEX, colorApex));
		}

		if (ImGui::DragFloat3("Sky offset", offset.xyz_))
		{
			pListener_->Execute(new CmdEntityChangeVec3(CHANGE_SKY_OFFSET, offset));
		}
	}
};

}

