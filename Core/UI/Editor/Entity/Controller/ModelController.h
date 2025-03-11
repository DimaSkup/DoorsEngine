// =================================================================================
// Filename:      ModelController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the model entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../Model/EntityEditorModel.h"


namespace UI
{

class ModelController
{
public:
	ModelController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const uint32_t enttID);

	void ExecuteCommand(const ICommand* pCmd, const uint32_t enttID);
	void UndoCommand   (const ICommand* pCmd, const uint32_t enttID);

	inline const ModelEntity& GetModel() const { return enttModel_; }

private:
	void ExecChangePosition    (const uint32_t enttID, const Vec3& pos);
	void ExecChangeRotationQuat(const uint32_t enttID, const Vec4& quat);
	void ExecChangeUniformScale(const uint32_t enttID, const float scale);
	
	void UndoChangePosition    (const uint32_t enttID, const Vec3& pos);
	void UndoChangeRotation    (const uint32_t enttID, const Vec4& rotQuat);
	void UndoChangeScale       (const uint32_t enttID, const float uniformScale);

private:
	ModelEntity        enttModel_;
	IFacadeEngineToUI* pFacade_ = nullptr;    // facade interface btw GUI and engine        
};

}