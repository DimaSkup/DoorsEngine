// ====================================================================================
// Filename:      ui_entt_light_view.cpp
// 
// Created:       01.01.25  by DimaSkup
// ====================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_light_view.h"
#include "../Model/EnttLightData.h"
#include <UICommon/ieditor_controller.h>
#include <UICommon/editor_cmd.h>
#include <imgui.h>


namespace UI
{

//---------------------------------------------------------
// Desc:  show editor's fields to control directed light properties
//        of the currently selected entity
//---------------------------------------------------------
void EnttLightView::Render(
    IEditorController* pController,
    const EnttDirLightData* pData)
{
    if (!pController || !pData)
    {
        LogErr(LOG, "some input ptr == nullptr");
        return;
    }

    // make local copies of the current model's data to use it in the fields
    EnttDirLightData data = pData->GetData();

    //
    // draw editor fields
    //
    if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_AMBIENT, data.ambient);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_DIFFUSE, data.diffuse);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_DIR_LIGHT_SPECULAR, data.specular);
        pController->ExecCmd(&cmd);
    }
}

//---------------------------------------------------------
// Desc:  show editor's fields to control point light properties
//        of the currently selected entity
//---------------------------------------------------------
void EnttLightView::Render(
    IEditorController* pController,
    const EnttPointLightData* pData)
{
    if (!pController || !pData)
    {
        LogErr(LOG, "some input ptr == nullptr");
        return;
    }

    // make local copies of the current model data to use it in the fields
    EnttPointLightData data = pData->GetData();

    //
    // draw editor fields
    //
    if (ImGui::Checkbox("Active", &data.isActive))
    {
        CmdChangeFloat cmd(CHANGE_POINT_LIGHT_ACTIVATION, data.isActive);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Ambient color", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_AMBIENT, data.ambient);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse color", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_DIFFUSE, data.diffuse);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular color", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_POINT_LIGHT_SPECULAR, data.specular);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.001f, 0.0f))
    {
        CmdChangeVec3 cmd(CHANGE_POINT_LIGHT_ATTENUATION, data.attenuation);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::SliderFloat("Range", &data.range, 0.1f, 200))
    {
        CmdChangeFloat cmd(CHANGE_POINT_LIGHT_RANGE, data.range);
        pController->ExecCmd(&cmd);
    }
}

//---------------------------------------------------------
// Desc:  show editor's fields to control spotlight properties
//        of the currently selected entity
//---------------------------------------------------------
void EnttLightView::Render(
    IEditorController* pController,
    const EnttSpotLightData* pData)
{
    if (!pController || !pData)
    {
        LogErr(LOG, "some input ptr == nullptr");
        return;
    }

    // make local copies of the current model data to use it in the fields
    EnttSpotLightData data = pData->GetData();

    //
    // draw editor fields
    //
    if (ImGui::ColorEdit4("Ambient", data.ambient.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_AMBIENT, data.ambient);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Diffuse", data.diffuse.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_DIFFUSE, data.diffuse);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit4("Specular", data.specular.rgba))
    {
        CmdChangeColor cmd(CHANGE_SPOT_LIGHT_SPECULAR, data.specular);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("Attenuation", data.attenuation.xyz, 0.05f))
    {
        CmdChangeVec3 cmd(CHANGE_SPOT_LIGHT_ATTENUATION, data.attenuation);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Range (distance)", &data.range))
    {
        CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_RANGE, data.range);
        pController->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Exponent", &data.spotExp, 0.5f))
    {
        CmdChangeFloat cmd(CHANGE_SPOT_LIGHT_SPOT_EXPONENT, data.spotExp);
        pController->ExecCmd(&cmd);
    }
}

}; // namespace UI
