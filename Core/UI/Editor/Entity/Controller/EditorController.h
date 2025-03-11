// =================================================================================
// Filename:       EditorController.h
// Description:    1. handle all the entity editor events
//                 2. execute changes of the entity properties 
//                 3. store this change as a command into the events history stack
// Created:        01.01.25
// =================================================================================
#pragma once

#include <UICommon/StatesGUI.h>
#include <UICommon/IEditorController.h>
#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../View/EntityEditorView.h"
#include "../View/SkyEditorView.h"
#include "../View/LightEditorView.h"

#include "ModelController.h"
#include "SkyController.h"
#include "PointLightController.h"
#include "SpotLightController.h"


namespace UI
{

class EditorController : public IEditorController
{

public:
	EditorController(StatesGUI* pStatesGUI);

	void Initialize(IFacadeEngineToUI* pFacade);
	void Render();
	void SetSelectedEntt(const uint32_t entityID);


	// methods for transformations with gizmo
	void UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world);
	void TranslateSelectedEntt(const DirectX::XMVECTOR& translation);
	void RotateSelectedEntt(const DirectX::XMVECTOR& quat);
	void ScaleSelectedEntt(const float uniformScale);


	// execute command and store this change into the events history
	virtual void Execute(const ICommand* pCommand) override;

	// undo/alt_undo an event from the events history
	virtual void Undo(const ICommand* pCommand, const uint32_t entityID) override;

private:
	// entities MVC views
	ViewSky             viewSky_;
	ViewEntityModel     viewModel_;
	ViewLight           viewLight_;

	// entities MVC controllers
	ModelController      modelController_;
	SkyController        skyController_;
	PointLightController pointLightController_;
	SpotLightController  spotLightController_;

	IFacadeEngineToUI* pFacade_ = nullptr;          // facade interface btw GUI and engine        
	StatesGUI* pStatesGUI_ = nullptr;
};

} // namespace UI