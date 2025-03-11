// =================================================================================
// Filename:      PointLightController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the point light entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../Model/LightEditorModel.h"


namespace UI
{

class PointLightController
{
public:
	PointLightController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const uint32_t enttID);

	void ExecuteCommand(const ICommand* pCmd, const uint32_t enttID);
	void UndoCommand   (const ICommand* pCmd, const uint32_t enttID);

	inline const ModelEntityPointLight& GetModel() const { return pointLightModel_; }

private:
	// change entt point light props
	void ExecChangeAmbient    (const uint32_t enttID, const ColorRGBA& ambient);
	void ExecChangeDiffuse    (const uint32_t enttID, const ColorRGBA& diffuse);
	void ExecChangeSpecular   (const uint32_t enttID, const ColorRGBA& specular);
	void ExecChangePos        (const uint32_t enttID, const Vec3& pos);
	void ExecChangeRange      (const uint32_t enttID, const float range);
	void ExecChangeAttenuation(const uint32_t enttID, const Vec3& att);

private:
	ModelEntityPointLight pointLightModel_;
	IFacadeEngineToUI*    pFacade_ = nullptr;   // facade interface btw GUI and engine

};

} // namespace UI