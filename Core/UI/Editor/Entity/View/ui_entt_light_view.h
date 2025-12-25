// =================================================================================
// Filename:     ui_entt_light_view.h
// Description:  editor's fields for control of different types of light sources
//               (a part of MVC pattern)
// 
// Created:      30.01.25  by DimaSkup
// =================================================================================
#pragma once

namespace UI
{
// forward declaration
class IEditorController;
class EnttDirLightData;
class EnttPointLightData;
class EnttSpotLightData;

class EnttLightView
{
public:
	void Render(IEditorController* pController, const EnttDirLightData*   pData);
    void Render(IEditorController* pController, const EnttPointLightData* pData);
	void Render(IEditorController* pController, const EnttSpotLightData*  pData);
};

}
