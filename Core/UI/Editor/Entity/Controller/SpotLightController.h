// =================================================================================
// Filename:      SpotLightController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the spotlight entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../Model/LightEditorModel.h"


namespace UI
{

class SpotLightController
{
public:
	SpotLightController();

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const uint32_t enttID);

	void ExecuteCommand(const ICommand* pCmd, const uint32_t enttID);
	void UndoCommand   (const ICommand* pCmd, const uint32_t enttID);

	inline const ModelEntitySpotLight& GetModel() const { return spotLightModel_; }

private:
	void ExecChangeAmbient        (const uint32_t enttID, const ColorRGBA& ambient);
	void ExecChangeDiffuse        (const uint32_t enttID, const ColorRGBA& diffuse);
	void ExecChangeSpecular       (const uint32_t enttID, const ColorRGBA& specular);
	void ExecChangePosition       (const uint32_t enttID, const Vec3& pos);
	void ExecChangeDirectionByQuat(const uint32_t enttID, const Vec4& dirQuat);
	void ExecChangeRange          (const uint32_t enttID, const float range);
	void ExecChangeAttenuation    (const uint32_t enttID, const Vec3& attenuation);
	void ExecChangeSpotExponent   (const uint32_t enttID, const float spotExponent);

private:
	ModelEntitySpotLight spotLightModel_;
	IFacadeEngineToUI*   pFacade_ = nullptr;          // facade interface btw GUI and engine        
};

}