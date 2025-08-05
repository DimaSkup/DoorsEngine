// ====================================================================================
// Filename:      EnttTransformView.cpp
// Created:       01.01.25
// ====================================================================================
#include <CoreCommon/pch.h>
#include "EnttTransformView.h"
#include <UICommon/Color.h>
#include <UICommon/Vectors.h>
#include <UICommon/EditorCommands.h>
#include <imgui.h>

#pragma warning (disable : 4996)


namespace UI
{

EnttTransformView::EnttTransformView(IEditorController* pController) :
    pController_(pController)
{
    CAssert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

///////////////////////////////////////////////////////////

void EnttTransformView::Render(const EnttTransformData& data)
{
    // show editor fields to control the selected entity model properties

    // make local copies of the current model data to use it in the fields
    Vec3  position      = data.position_;
    Vec3  direction     = data.direction_;
    float uniformScale  = data.uniformScale_;
    
    //
    // draw editor fields
    //
    if (ImGui::DragFloat3("Position", position.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
    {
        CmdChangeVec3 cmd(CHANGE_ENTITY_POSITION, position);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("Direction", direction.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
    {
        CmdChangeVec3 cmd(CHANGE_ENTITY_DIRECTION, position);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::SliderFloat("Uniform scale", &uniformScale, 0.1f, 20))
    {
        CmdChangeFloat cmd(CHANGE_ENTITY_SCALE, uniformScale);
        pController_->ExecCmd(&cmd);
    }
}

} // namespace UI
