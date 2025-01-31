// ====================================================================================
// Filename:      EntityEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Entity;
//                contains fields to change the Entity params;
// 
// Created:       01.01.25
// ====================================================================================
#pragma once


#include "../../UICommon/ViewListener.h"
#include "EntityEditorModel.h"


namespace View
{

class Entity
{
private:
	ViewListener* pListener_ = nullptr;

public:
	Entity(ViewListener* pListener);

	void Render(const Model::Entity* pData);
};

}