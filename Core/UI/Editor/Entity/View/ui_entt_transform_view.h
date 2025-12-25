// ====================================================================================
// Filename:      ui_entt_transform_view.h
// Description:   (MVC view) contains control fields for
//                change transformation of selected entity
// 
// Created:       01.01.25
// ====================================================================================
#pragma once

namespace UI
{
// forward declaration
class IEditorController;
class EnttTransformData;


class EnttTransformView
{
public:
	void Render(IEditorController* pController, const EnttTransformData* pData);
};

}
