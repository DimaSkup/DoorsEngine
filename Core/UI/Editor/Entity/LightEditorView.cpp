// ====================================================================================
// Filename:      LightEditorView.cpp
// 
// Created:       01.01.25  by DimaSkup
// ====================================================================================
#include "LightEditorView.h"
#include "../../../Common/Assert.h"
#include "EntityEditorCommands.h"

#include <imgui.h>


namespace View
{

Light::Light(ViewListener* pListener) : pListener_(pListener)
{
	Assert::NotNullptr(pListener, "ptr to the view listener == nullptr");
}

///////////////////////////////////////////////////////////

void Light::Render(const Model::EntityPointLight* pData)
{
	// show editor fields to control the selected point light source

	using enum EntityEditorCmdType;

	
	ColorRGBA ambient;
	ColorRGBA diffuse;
	ColorRGBA specular;
	Vec3      position;
	Vec3      attenuation;
	float     range = 0;

	// make local copies of the current model data to use it in the fields
	pData->GetData(ambient, diffuse, specular, position, attenuation, range);


	// draw editor fields
	if (ImGui::DragFloat3("Position", position.xyz_, 0.005f, -FLT_MAX, +FLT_MAX))
	{
		pListener_->Execute(new CmdEntityChangeVec3(CHANGE_POINT_LIGHT_POSITION, position));
	}

	if (ImGui::ColorEdit4("Ambient color", ambient.rgba_))
	{
		pListener_->Execute(new CmdChangeColor(CHANGE_POINT_LIGHT_AMBIENT, ambient));
	}

	if (ImGui::ColorEdit4("Diffuse color", diffuse.rgba_))
	{
		pListener_->Execute(new CmdChangeColor(CHANGE_POINT_LIGHT_DIFFUSE, diffuse));
	}

	if (ImGui::ColorEdit4("Specular color", specular.rgba_))
	{
		pListener_->Execute(new CmdChangeColor(CHANGE_POINT_LIGHT_SPECULAR, specular));
	}

	if (ImGui::DragFloat3("Attenuation", attenuation.xyz_, 0.1f, 0.0f))
	{
		pListener_->Execute(new CmdEntityChangeVec3(CHANGE_POINT_LIGHT_ATTENUATION, attenuation));
	}

	if (ImGui::SliderFloat("Range", &range, 0.1f, 200))
	{
		pListener_->Execute(new CmdEntityChangeFloat(CHANGE_POINT_LIGHT_RANGE, range));
	}
}

///////////////////////////////////////////////////////////

}; // namespace View