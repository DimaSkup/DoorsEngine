// ====================================================================================
// Filename:    SkyEditorView.cpp
//
// Created:     20.02.25  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "SkyEditorView.h"

#include <UICommon/Color.h>
#include <UICommon/Vectors.h>
#include <UICommon/EditorCommands.h>

#include <imgui.h>


namespace UI
{

ViewSky::ViewSky(IEditorController* pController) : pController_(pController)
{
	CAssert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

// ----------------------------------------------------

void ViewSky::Render(const ModelSky& data)
{
	using enum eEditorCmdType;
	
	ColorRGB colorCenter;
	ColorRGB colorApex;
	Vec3 offset;

	// make local copies of the current model data to use it in the fields
	data.GetColorCenter(colorCenter);
	data.GetColorApex(colorApex);
	data.GetSkyOffset(offset);

	// draw editor fields
	if (ImGui::ColorEdit3("Center color", colorCenter.rgb))
	{
        CmdChangeColor cmd(CHANGE_SKY_COLOR_CENTER, colorCenter);
		pController_->ExecCmd(&cmd);
	}

	if (ImGui::ColorEdit3("Apex color", colorApex.rgb))
	{
        CmdChangeColor cmd(CHANGE_SKY_COLOR_APEX, colorApex);
		pController_->ExecCmd(&cmd);
	}

	if (ImGui::DragFloat3("Sky offset", offset.xyz))
	{
        CmdChangeVec3 cmd(CHANGE_SKY_OFFSET, offset);
		pController_->ExecCmd(&cmd);
	}
}

} // namespace UI
