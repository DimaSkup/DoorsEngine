// ====================================================================================
// Filename:      EntityEditorView.cpp
// Created:       01.01.25
// ====================================================================================
#include "EntityEditorView.h"

#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/MathHelper.h>

#include <UICommon/Color.h>
#include <UICommon/Vectors.h>
#include <UICommon/EditorCommands.h>

#include <imgui.h>


namespace UI
{

ViewEntityModel::ViewEntityModel(IEditorController* pController) :
	pController_(pController)
{
	Core::Assert::NotNullptr(pController, "ptr to the editor controller == nullptr");
}

///////////////////////////////////////////////////////////

void ViewEntityModel::Render(const ModelEntity& data)
{
	// show editor fields to control the selected entity model properties

	using enum EditorCmdType;

	// make local copies of the current model data to use it in the fields
	Vec3     position = data.GetPosition();
	float    uniformScale = data.GetScale();
	
	//
	// draw editor fields
	//
	if (ImGui::DragFloat3("Position", position.xyz, 0.005f, -FLT_MAX, +FLT_MAX))
	{
		pController_->Execute(new CmdChangeVec3(CHANGE_MODEL_POSITION, position));
	}

	std::string pitchInfo = std::to_string(data.GetPitchInDeg());
	std::string yawInfo   = std::to_string(data.GetYawInDeg());
	std::string rollInfo  = std::to_string(data.GetRollInDeg());

	ImGui::Text("Rotation in degrees (around axis)");
	ImGui::InputText("pitch", pitchInfo.data(), pitchInfo.size()+1, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputText("yaw",   yawInfo.data(),   yawInfo.size()+1,   ImGuiInputTextFlags_ReadOnly);
	ImGui::InputText("roll",  rollInfo.data(),  rollInfo.size()+1,  ImGuiInputTextFlags_ReadOnly);

	if (ImGui::SliderFloat("Uniform scale", &uniformScale, 0.1f, 20))
	{
		pController_->Execute(new CmdChangeFloat(CHANGE_MODEL_SCALE, uniformScale));
	}
}

} // namespace UI