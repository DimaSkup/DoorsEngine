// =================================================================================
// Filename:     EnttLightView.h
// Description:  editor control fields for different types of light sources
// 
// Created:      30.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IEditorController.h>
#include "../Model/EnttLightData.h"


namespace UI
{

class EnttLightView
{
private:
	IEditorController* pController_ = nullptr;

public:
    EnttLightView(IEditorController* pController);

	void Render(const EnttDirLightData& data);
	void Render(const EnttPointLightData& data);
	void Render(const EnttSpotLightData& data);
};

}
