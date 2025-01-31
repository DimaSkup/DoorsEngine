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

#include "LightEditorView.h"
#include "LightEditorModel.h"


class EntityEditorController : public ViewListener
{
	enum SelectedEnttType
	{
		NONE,
		SKY,
		DIRECTED_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT,
		CAMERA,
		MODEL,
	};

public:
	EntityEditorController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void Render();

	void UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world);

	virtual void Execute(ICommand* pCommand) override;

	void SetSelectedEntt(const uint32_t entityID);

	inline uint32_t GetSelectedEntt()       const { return selectedEnttID_; }
	inline bool IsSelectedAnyEntt()         const { return isSelectedAnyEntt_; }
	inline bool IsSelectedEnttLightSource() const { return (selectedEnttType_ == DIRECTED_LIGHT) || (selectedEnttType_ == POINT_LIGHT) || (selectedEnttType_ == SPOT_LIGHT); }

private:
	// data loaders
	void LoadSkyEnttData(const uint32_t skyEnttID);
	void LoadUsualEnttData(const uint32_t enttID);
	void LoadLightEnttData(const uint32_t enttID);

	// commands executors
	void ExecuteUsualEnttCommand(ICommand* pCommand);
	void ExecuteSkyCommand(ICommand* pCommand);
	void ExecutePointLightCommand(ICommand* pCommand);

private:
	View::Sky                skyView_;
	View::Entity             enttView_;
	View::Light              lightView_;

	Model::Sky               skyModel_;
	Model::Entity            enttModel_;
	Model::EntityDirLight    dirLightModel_;
	Model::EntityPointLight  pointLightModel_;
	Model::EntitySpotLight   spotLightModel_;


	uint32_t           selectedEnttID_ = 0;
	IFacadeEngineToUI* pFacade_ = nullptr;          // facade interface btw GUI and engine        
	bool               isSelectedAnyEntt_ = false;
	SelectedEnttType   selectedEnttType_ = NONE;
};