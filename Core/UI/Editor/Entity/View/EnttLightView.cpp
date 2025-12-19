// ====================================================================================
// Filename:      EnttLightView.cpp
// 
// Created:       01.01.25  by DimaSkup
// ====================================================================================
#include <CoreCommon/pch.h>
#include "EnttLightView.h"

#include <UICommon/ieditor_controller.h>
#include <UICommon/editor_cmd.h>
#include <imgui.h>


namespace UI
{

EnttLightView::EnttLightView(IEditorController* pController) : pController_(pController)
{
    CAssert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

///////////////////////////////////////////////////////////

void EnttLightView::Render(const EnttDirLightData& model)
{
    // show editor fields to control the selected directed light source

    // make local copies of the current model's data to use it in the fields
    EnttDirLightData data = model.GetData();

    //
    // draw editor fields
    //
    if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_AMBIENT, data.ambient);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_DIFFUSE, data.diffuse);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_SPECULAR, data.specular);
        pController_->ExecCmd(&cmd);
    }
}

///////////////////////////////////////////////////////////

void EnttLightView::Render(const EnttPointLightData& model)
{
    // show editor fields to control the selected point light source

    // make local copies of the current model data to use it in the fields
    EnttPointLightData data = model.GetData();

    //
    // draw editor fields
    //
    if (ImGui::Checkbox("Active", &data.isActive))
    {
        CmdChangeFloat cmd(CHANGE_POINT_LIGHT_ACTIVATION, data.isActive);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Ambient color", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_AMBIENT, data.ambient);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse color", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_DIFFUSE, data.diffuse);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular color", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_SPECULAR, data.specular);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.001f, 0.0f))
    {
        CmdChangeVec3 cmd(CHANGE_POINT_LIGHT_ATTENUATION, data.attenuation);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::SliderFloat("Range", &data.range, 0.1f, 200))
    {
        CmdChangeFloat cmd(CHANGE_POINT_LIGHT_RANGE, data.range);
        pController_->ExecCmd(&cmd);
    }
}

///////////////////////////////////////////////////////////

void EnttLightView::Render(const EnttSpotLightData& model)
{
    // show editor fields to control the selected spotlight source

    // make local copies of the current model data to use it in the fields
    EnttSpotLightData data = model.GetData();

    //
    // draw editor fields
    //
    if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_AMBIENT, data.ambient);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_DIFFUSE, data.diffuse);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_SPECULAR, data.specular);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.05f))
    {
        CmdChangeVec3 cmd(CHANGE_SPOT_LIGHT_ATTENUATION, data.attenuation);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Range (distance)", &data.range))
    {
        CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_RANGE, data.range);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Exponent", &data.spotExp, 0.5f))
    {
        CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, data.spotExp);
        pController_->ExecCmd(&cmd);
    }
}

///////////////////////////////////////////////////////////

}; // namespace UI
