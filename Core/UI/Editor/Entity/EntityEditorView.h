// ====================================================================================
// Filename:      EntityEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Entity;
//                contains fields to change the Entity params;
// 
// Created:       01.01.25
// ====================================================================================
#pragma once


// engine common
#include "../../../Common/Assert.h"

// UI common
#include "../../UICommon/ViewListener.h"


#include "EntityEditorCommands.h"
#include "EntityEditorModel.h"


namespace View
{

class Entity
{
private:
	ViewListener* pListener_ = nullptr;

public:
	Entity(ViewListener* pListener) : pListener_(pListener)
	{
		Assert::NotNullptr(pListener, "ptr to the view listener == nullptr");
	}

	void Draw(
		const Model::Entity* pData,
		const float* cameraView,       // camera view matrix
		const float* cameraProj);      // camera projection matrix
};

}