// =================================================================================
// Filename:      SkyController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the sky entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IEditorController.h>
#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "SkyEditorModel.h"
#include "SkyEditorView.h"

namespace UI
{

class SkyController : public IEditorController
{
public:
	SkyController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadSkyEnttData();

    // execute command and store this change into the events history
    virtual void Execute(const ICommand* pCmd) override;

    // undo/alt_undo an event (related to sky) from the events history
    virtual void Undo(const ICommand* pCmd, const uint32 enttID) override;

	inline const ModelSky& GetModel() const { return skyModel_; }

    void Draw();


private:
	void ExecChangeColorCenter   (const uint32 enttID, const ColorRGB& color);
	void ExecChangeColorApex     (const uint32 enttID, const ColorRGB& color);
	void ExecChangeVerticalOffset(const uint32 enttID, const Vec3& offset);

private:
	ModelSky           skyModel_;               // MVC model
    ViewSky            skyView_;
	IFacadeEngineToUI* pFacade_ = nullptr;      // facade interface btw GUI and engine
};

}
