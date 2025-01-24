// =================================================================================
// Filename:       SkyEditorController.h
// Description:    handle all the sky editor events and executes changes
// Created:        
// =================================================================================
#pragma once

#include "../../UICommon/ViewListener.h"
#include "../../UICommon/ICommand.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "SkyEditorView.h"
#include "SkyEditorModel.h"


class SkyEditorController : public ViewListener
{
private:
	Model::Sky skyModel_;
	View::Sky  skyView_;
	IFacadeEngineToUI* pFacade_ = nullptr;

public:
	SkyEditorController() : skyView_(this) {}

	void Initialize(IFacadeEngineToUI* pFacade);
	void Draw();

	virtual void Execute(ICommand* pCommand) override;
};