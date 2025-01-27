// =================================================================================
// Filename:       EntityEditorController.h
// Description:    1. handle all the entity editor events
//                 2. execute changes of the entity properties 
//                 3. store this change as a command into the events history stack
// Created:        01.01.25
// =================================================================================
#pragma once

#include "../../UICommon/ViewListener.h"
#include "../../UICommon/ICommand.h"
#include "../../UICommon/IFacadeEngineToUI.h"

#include "EntityEditorView.h"
#include "EntityEditorModel.h"

#include "SkyEditorView.h"
#include "SkyEditorModel.h"


class EntityEditorController : public ViewListener
{
	enum SelectedEnttType
	{
		NONE,
		SKY,
		USUAL,      // may be camera, light src, model
	};

public:
	EntityEditorController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void Render();

	virtual void Execute(ICommand* pCommand) override;

	void SetSelectedEntt(const uint32_t entityID);
	inline uint32_t GetSelectedEntt() const { return enttModel_.GetSelectedEntityID(); }
	inline bool IsSelectedAnyEntt()   const { return isSelectedAnyEntt_; }

private:
	// data loaders
	void LoadSkyEnttData(const uint32_t skyEnttID);
	void LoadUsualEnttData(const uint32_t enttID);

	// commands executors
	void ExecuteUsualEnttCommand(ICommand* pCommand);
	void ExecuteSkyCommand(ICommand* pCommand);

private:
	View::Sky          skyView_;
	Model::Sky         skyModel_;
	Model::Entity      enttModel_;
	View::Entity       enttView_;
	IFacadeEngineToUI* pFacade_ = nullptr;          // facade interface btw GUI and engine        
	bool               isSelectedAnyEntt_ = false;
	SelectedEnttType   selectedEnttType_ = NONE;
};