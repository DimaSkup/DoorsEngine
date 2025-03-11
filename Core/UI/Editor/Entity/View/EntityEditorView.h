// ====================================================================================
// Filename:      EntityEditorView.h
// Description:   MVC view for that entities which are model 
//                (cubes, trees, houses, etc.)
// 
// Created:       01.01.25
// ====================================================================================
#pragma once

#include <UICommon/IEditorController.h>
#include "../Model/EntityEditorModel.h"


namespace UI
{

class ViewEntityModel
{
public:
	ViewEntityModel(IEditorController* pController);

	void Render(const ModelEntity& data);

private:
	IEditorController* pController_ = nullptr;
};

}