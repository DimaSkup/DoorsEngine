// =================================================================================
// Filename:     LightEditorView.h
// Description:  editor control fields for different types of light sources
// 
// Created:      30.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "../../UICommon/ViewListener.h"
#include "LightEditorModel.h"


namespace View
{


class Light
{
public:
	Light(ViewListener* pListener);

	void Render(const Model::EntityDirLight* pData);
	void Render(const Model::EntityPointLight* pData);
	void Render(const Model::EntitySpotLight* pData);

private:
	ViewListener* pListener_ = nullptr;
};



}