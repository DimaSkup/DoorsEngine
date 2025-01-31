// ====================================================================================
// Filename:      EntityEditorView.cpp
// Created:       01.01.25
// ====================================================================================
#include "EntityEditorView.h"

#include "../../../Common/log.h"
#include "../../../Common/Assert.h"
#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"
#include "EntityEditorCommands.h"

#include <imgui.h>



namespace View
{

Entity::Entity(ViewListener* pListener) : pListener_(pListener)
{
	Assert::NotNullptr(pListener, "ptr to the view listener == nullptr");
}

///////////////////////////////////////////////////////////

void Entity::Render(const Model::Entity* pData)
{
	// show editor fields to control the selected entity model properties

	using enum EntityEditorCmdType;


	//Vec3     rotationStride;
	//constexpr float PIMUL2 = DirectX::XM_2PI;
	Vec3     position;
	Vec4     dirQuat;
	float    uniformScale;
	
	// make local copies of the current model data to use it in the fields
	pData->GetData(position, dirQuat, uniformScale);
	

	//
	// draw editor fields
	//
	if (ImGui::DragFloat3("Position (x,y,z)", position.xyz_, 0.005f, -FLT_MAX, +FLT_MAX))
	{
		pListener_->Execute(new CmdEntityChangeVec3(CHANGE_POSITION, position));
	}

#if 0 // FIXME
	if (ImGui::SliderFloat3("Rotation change in rad (x,y,z)", rotationStride.xyz_, -PIMUL2, +PIMUL2))
	{
		DirectX::XMVECTOR rotationQuat    = dirQuat.ToXMVector();
		DirectX::XMVECTOR rotateByQuat    = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotationStride.ToXMVector());
		DirectX::XMVECTOR newRotationQuat = DirectX::XMQuaternionMultiply(rotationQuat, rotateByQuat);

		pListener_->Execute(new CmdEntityChangeVec4(CHANGE_ROTATION, newRotationQuat));
	}
#endif

	if (ImGui::SliderFloat("Scale", &uniformScale, 0.001f, 20))
	{
		pListener_->Execute(new CmdEntityChangeFloat(CHANGE_SCALE, uniformScale));
	}
}

} // namespace View