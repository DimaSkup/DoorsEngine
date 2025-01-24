// ====================================================================================
// Filename:      EntityEditorView.cpp
// Created:       01.01.25
// ====================================================================================
#include "EntityEditorView.h"

#include "../../../Common/log.h"

#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"

#include <imgui.h>



namespace View
{


void Entity::Draw(
	const Model::Entity* pData,
	const float* cameraView,     // camera view matrix
	const float* cameraProj)     // camera projection matrix
{
	//
	// show the entity editor fields
	//

	using enum EntityEditorCmdType;

	uint32_t entityID;
	Vec3     position;
	Vec4     dirQuat;
	Vec3     rotationStride;
	float    uniformScale;
	//float    angles[3];
	//DirectX::XMVECTOR axis;
	constexpr float PIMUL2 = DirectX::XM_2PI;

	// make local copies of the current model data to use it in the fields
	entityID = pData->GetEntityID();
	pData->GetTransformation(position, dirQuat, uniformScale);
	
	// ------------------------------------------

	// draw editor fields
	ImGui::Text("EntityID: %d", entityID);

	if (ImGui::DragFloat3("Position (x,y,z)", position.xyz_, 0.005f, -FLT_MAX, +FLT_MAX))
	{
		pListener_->Execute(new CmdEntityChangeVec3(CHANGE_POSITION, position));
	}

	if (ImGui::SliderFloat3("Rotation change in rad (x,y,z)", rotationStride.xyz_, -PIMUL2, +PIMUL2))
	{
		DirectX::XMVECTOR rotationQuat    = dirQuat.GetXMVector();
		DirectX::XMVECTOR rotateByQuat    = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotationStride.GetXMVector());
		DirectX::XMVECTOR newRotationQuat = DirectX::XMQuaternionMultiply(rotationQuat, rotateByQuat);
		DirectX::XMFLOAT4 q;
		DirectX::XMStoreFloat4(&q, newRotationQuat);
		pListener_->Execute(new CmdEntityChangeVec4(CHANGE_ROTATION, {q.x, q.y, q.z, q.w}));
	}

	if (ImGui::SliderFloat("Scale", &uniformScale, 0.001f, 20))
	{
		pListener_->Execute(new CmdEntityChangeFloat(CHANGE_SCALE, uniformScale));
	}
}

} // namespace View