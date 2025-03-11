// =================================================================================
// Filename:      SkyController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the sky entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../Model/SkyEditorModel.h"


namespace UI
{

class SkyController
{
public:
	SkyController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const uint32_t enttID);

	void ExecuteCommand(const ICommand* pCmd, const uint32_t enttID);
	void UndoCommand(const ICommand* pCmd, const uint32_t enttID) { assert(0 && "TODO"); };

	inline const ModelSky& GetModel() const { return skyModel_; }

private:
	void ExecChangeColorCenter   (const uint32_t enttID, const ColorRGB& color);
	void ExecChangeColorApex     (const uint32_t enttID, const ColorRGB& color);
	void ExecChangeVerticalOffset(const uint32_t enttID, const Vec3& offset);

private:
	ModelSky           skyModel_;               // MVC model
	IFacadeEngineToUI* pFacade_ = nullptr;      // facade interface btw GUI and engine
};

}