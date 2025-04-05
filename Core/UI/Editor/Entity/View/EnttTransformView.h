// ====================================================================================
// Filename:      EnttTransformView.h
// Description:   MVC view for transformation control fields of entity
// 
// Created:       01.01.25
// ====================================================================================
#pragma once

#include <UICommon/IEditorController.h>
#include "../Model/EnttTransformData.h"


namespace UI
{

class EnttTransformView
{
public:
    EnttTransformView(IEditorController* pController);

	void Render(const EnttTransformData& data);

private:
	IEditorController* pController_ = nullptr;
};

}
