// ====================================================================================
// Filename:      EnttTransformView.h
// Description:   (MVC view) contains control fields for
//                change transformation of selected entity
// 
// Created:       01.01.25
// ====================================================================================
#pragma once

#include "../Model/EnttTransformData.h"


namespace UI
{
// forward declaration
class IEditorController;


class EnttTransformView
{
public:
    EnttTransformView(IEditorController* pController);

	void Render(const EnttTransformData& data);

private:
	IEditorController* pController_ = nullptr;
};

}
