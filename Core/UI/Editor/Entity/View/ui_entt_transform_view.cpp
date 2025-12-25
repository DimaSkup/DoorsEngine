// ====================================================================================
// Filename:      ui_entt_transform_view.cpp
// Created:       01.01.25
// ====================================================================================
#include <CoreCommon/pch.h>
#include "ui_entt_transform_view.h"       // MVC view
#include "../Model/EnttTransformData.h"   // MVC model
#include <UICommon/ieditor_controller.h>  // MVC controller
#include <UICommon/color.h>
#include <UICommon/editor_cmd.h>

#include <math/vec3.h>
#include <imgui.h>

#pragma warning (disable : 4996)


namespace UI
{

//---------------------------------------------------------
// Desc:  show editor's fields to control
//        the selected entity transformation properties
//---------------------------------------------------------
void EnttTransformView::Render(
    IEditorController* pController,
    const EnttTransformData* pData)
{
    if (!pController || !pData)
    {
        LogErr(LOG, "some input ptr == nullptr");
        return;
    }

    // make local copies of the current model data to use it in the fields
    Vec3 pos;
    Vec3 dir;
    Vec4 rotQuat;
    float scale;

    pData->GetData(pos, dir, rotQuat, scale);

    //
    // draw editor fields
    //
    if (ImGui::DragFloat3("Position", pos.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
    {
        CmdChangeVec3 cmd(CHANGE_ENTITY_POSITION, pos);
        pController->ExecCmd(&cmd);
    }

    ImGui::DragFloat3("Direction",     dir.xyz,      0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_NoInput);
    ImGui::DragFloat4("Rotation quat", rotQuat.xyzw, 1.0f,   0.0f,      0.0f,    "%.3f", ImGuiSliderFlags_NoInput);

    if (ImGui::DragFloat("Uniform scale", &scale, 0.001f))
    {
        CmdChangeFloat cmd(CHANGE_ENTITY_SCALE, scale);
        pController->ExecCmd(&cmd);
    }
}

} // namespace UI
