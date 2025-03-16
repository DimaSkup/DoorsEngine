// ====================================================================================
// Filename:      LightEditorView.cpp
// 
// Created:       01.01.25  by DimaSkup
// ====================================================================================
#include "LightEditorView.h"

#include <CoreCommon/Assert.h>
#include <UICommon/EditorCommands.h>
#include <imgui.h>


namespace UI
{

ViewLight::ViewLight(IEditorController* pController) : pController_(pController)
{
	Core::Assert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

///////////////////////////////////////////////////////////

void ViewLight::Render(const ModelEntityDirLight& model)
{
    // show editor fields to control the selected directed light source

    using enum EditorCmdType;
    DirectedLightData data;

    // make local copies of the current model's data to use it in the fields
    model.GetData(data);

    // draw editor fields
    if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_AMBIENT, data.ambient);
        pController_->Execute(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_DIFFUSE, data.diffuse);
        pController_->Execute(&cmd);
    }

    if (ImGui::ColorEdit4("Specular", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_SPECULAR, data.specular);
        pController_->Execute(&cmd);
    }

    if (ImGui::DragFloat3("Direction", data.direction.xyz, 0.05f))
    {
        CmdChangeVec3 cmd(CHANGE_DIR_LIGHT_DIRECTION, data.direction);
        pController_->Execute(&cmd);
    }
}

///////////////////////////////////////////////////////////

void ViewLight::Render(const ModelEntityPointLight& model)
{
	// show editor fields to control the selected point light source

	using enum EditorCmdType;
	PointLightData data;

	// make local copies of the current model data to use it in the fields
	model.GetData(data);

	// draw editor fields
	if (ImGui::DragFloat3("Position", data.position.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
	{
		CmdChangeVec3 cmd(CHANGE_POINT_LIGHT_POSITION, data.position);
		pController_->Execute(&cmd);
	}

	if (ImGui::ColorEdit4("Ambient color", data.ambient.rgba))
	{
		CmdChangeColor cmd(CHANGE_POINT_LIGHT_AMBIENT, data.ambient);
		pController_->Execute(&cmd);
	}

	if (ImGui::ColorEdit4("Diffuse color", data.diffuse.rgba))
	{
		CmdChangeColor cmd(CHANGE_POINT_LIGHT_DIFFUSE, data.diffuse);
		pController_->Execute(&cmd);
	}

	if (ImGui::ColorEdit4("Specular color", data.specular.rgba))
	{
		CmdChangeColor cmd(CHANGE_POINT_LIGHT_SPECULAR, data.specular);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.001f, 0.0f))
	{
		CmdChangeVec3 cmd(CHANGE_POINT_LIGHT_ATTENUATION, data.attenuation);
		pController_->Execute(&cmd);
	}

	if (ImGui::SliderFloat("Range", &data.range, 0.1f, 200))
	{
		CmdChangeFloat cmd(CHANGE_POINT_LIGHT_RANGE, data.range);
		pController_->Execute(&cmd);
	}
}

///////////////////////////////////////////////////////////

void ViewLight::Render(const ModelEntitySpotLight& model)
{
	// show editor fields to control the selected spotlight source

	using enum EditorCmdType;
	SpotLightData data;

	// make local copies of the current model data to use it in the fields
	model.GetData(data);

	if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
	{
		CmdChangeColor cmd(CHANGE_SPOT_LIGHT_AMBIENT, data.ambient);
		pController_->Execute(&cmd);
	}

	if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
	{
		CmdChangeColor cmd(CHANGE_SPOT_LIGHT_DIFFUSE, data.diffuse);
		pController_->Execute(&cmd);
	}

	if (ImGui::ColorEdit4("Specular", data.specular.rgba))
	{
		CmdChangeColor cmd(CHANGE_SPOT_LIGHT_SPECULAR, data.specular);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat3("Position", data.position.xyz, 0.05f))
	{
		CmdChangeVec3 cmd(CHANGE_SPOT_LIGHT_POSITION, data.position);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat3("Direction", data.direction.xyz, 0.05f))
	{
		CmdChangeVec3 cmd(CHANGE_SPOT_LIGHT_DIRECTION, data.direction);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.05f))
	{
		CmdChangeVec3 cmd(CHANGE_SPOT_LIGHT_ATTENUATION, data.attenuation);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat("Range (distance)", &data.range))
	{
		CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_RANGE, data.range);
		pController_->Execute(&cmd);
	}

	if (ImGui::DragFloat("Exponent", &data.spotExp, 0.5f))
	{
		CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, data.spotExp);
		pController_->Execute(&cmd);
	}
}

///////////////////////////////////////////////////////////

}; // namespace UI
