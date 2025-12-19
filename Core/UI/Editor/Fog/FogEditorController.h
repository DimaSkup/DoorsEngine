// =================================================================================
// Filename:       FogEditorController.h
// Description:    handle all the fog editor events and executes changes
// Created:        31.12.24
// =================================================================================
#pragma once

#include "FogEditorView.h"
#include "FogEditorModel.h"
#include <UICommon/ieditor_controller.h>


namespace UI
{
// forward declaration
class IFacadeEngineToUI;
class ICommand;


class FogEditorController : public IEditorController
{
private:
	ModelFog fogModel_;
	ViewFog  fogView_;
	IFacadeEngineToUI* pFacade_ = nullptr;

public:
	FogEditorController() : fogView_(this) {}

	void Initialize(IFacadeEngineToUI* pFacade);

	// execute command and store this change into the events history
	virtual void ExecCmd(const ICommand* pCmd) override;

	// undo/alt_undo an event (related to fog) from the events history
	virtual void UndoCmd(const ICommand* pCmd, const EntityID id) override;


	inline void Draw() 
	{ 
		fogView_.Draw(&fogModel_);
	}
};

} // namespace UI
