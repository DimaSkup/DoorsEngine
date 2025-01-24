// =================================================================================
// Filename:       FogEditorController.h
// Description:    handle all the fog editor events and executes changes
// Created:        31.12.24
// =================================================================================
#pragma once

#include "../../UICommon/ViewListener.h"
#include "../../UICommon/ICommand.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "FogEditorView.h"
#include "FogEditorModel.h"


class FogEditorController : public ViewListener
{
private:
	Model::Fog fogModel_;
	View::Fog  fogView_;
	IFacadeEngineToUI* pFacade_ = nullptr;

public:
	FogEditorController() : fogView_(this) {}

	void Initialize(IFacadeEngineToUI* pFacade);
	virtual void Execute(ICommand* pCommand) override;

	inline void Draw() { fogView_.Draw(&fogModel_); }
};