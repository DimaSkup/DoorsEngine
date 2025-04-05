// ====================================================================================
// Filename:      EnttTransformView.cpp
// Created:       01.01.25
// ====================================================================================
#include "EnttTransformView.h"

#include <CoreCommon/Assert.h>
#include <UICommon/Color.h>
#include <UICommon/Vectors.h>
#include <UICommon/EditorCommands.h>

#include <imgui.h>


namespace UI
{

EnttTransformView::EnttTransformView(IEditorController* pController) :
    pController_(pController)
{
    Core::Assert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

///////////////////////////////////////////////////////////

void EnttTransformView::Render(const EnttTransformData& data)
{
    // show editor fields to control the selected entity model properties

    using enum eEditorCmdType;

    // make local copies of the current model data to use it in the fields
    Vec3     position = data.GetPosition();
    float    uniformScale = data.GetScale();
    
    //
    // draw editor fields
    //
    if (ImGui::DragFloat3("Position", position.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
    {
        CmdChangeVec3 cmd(CHANGE_ENTITY_POSITION, position);
        pController_->Execute(&cmd);
    }

    // ------------------------------------------

    std::string pitchInfo = std::to_string(data.GetPitchInDeg());
    std::string yawInfo   = std::to_string(data.GetYawInDeg());
    std::string rollInfo  = std::to_string(data.GetRollInDeg());

    ImGui::Text("Rotation in degrees (around axis)");
    ImGui::InputText("pitch", pitchInfo.data(), pitchInfo.size()+1, ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("yaw",   yawInfo.data(),   yawInfo.size()+1,   ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("roll",  rollInfo.data(),  rollInfo.size()+1,  ImGuiInputTextFlags_ReadOnly);

    // ------------------------------------------

    if (ImGui::SliderFloat("Uniform scale", &uniformScale, 0.1f, 20))
    {
        CmdChangeFloat cmd(CHANGE_ENTITY_SCALE, uniformScale);
        pController_->Execute(&cmd);
    }
}

} // namespace UI
