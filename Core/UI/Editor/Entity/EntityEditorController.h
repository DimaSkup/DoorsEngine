// =================================================================================
// Filename:       EntityEditorController.h
// Description:    handle all the entity editor events and executes changes
// Created:        01.01.25
// =================================================================================
#pragma once

#include "../../UICommon/ViewListener.h"
#include "../../UICommon/ICommand.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "EntityEditorView.h"
#include "EntityEditorModel.h"


class EntityEditorController : public ViewListener
{
public:
	EntityEditorController() : enttView_(this) {}

	void Initialize(IFacadeEngineToUI* pFacade);
	void Update(const uint32_t entityID);
	void Render();

	virtual void Execute(ICommand* pCommand) override;

	inline uint32_t GetSelectedEntt() const { return enttModel_.GetEntityID(); }

private:
	Model::Entity enttModel_;
	View::Entity  enttView_;
	IFacadeEngineToUI* pFacade_ = nullptr;      // facade interface btw GUI and engine        

};