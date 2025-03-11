// =================================================================================
// Filename:     LightEditorView.h
// Description:  editor control fields for different types of light sources
// 
// Created:      30.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IEditorController.h>
#include "../Model/LightEditorModel.h"


namespace UI
{

class ViewLight
{
private:
	IEditorController* pController_ = nullptr;

public:
	ViewLight(IEditorController* pController);

	void Render(const ModelEntityDirLight& data);
	void Render(const ModelEntityPointLight& data);
	void Render(const ModelEntitySpotLight& data);
};

}