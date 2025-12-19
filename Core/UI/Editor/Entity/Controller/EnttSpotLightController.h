// =================================================================================
// Filename:      EnttSpotLightController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the spotlight entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include "../Model/EnttLightData.h"


namespace UI
{
// forward declaration
class IFacadeEngineToUI;
class ICommand;


class EnttSpotLightController
{
public:
    EnttSpotLightController() {};

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const EntityID id);

	void ExecuteCommand(const ICommand* pCmd, const EntityID id);
	void UndoCommand   (const ICommand* pCmd, const EntityID id);

	inline const EnttSpotLightData& GetModel() const { return spotLightModel_; }

private:
	void ExecChangeAmbient        (const EntityID id, const ColorRGBA& ambient);
	void ExecChangeDiffuse        (const EntityID id, const ColorRGBA& diffuse);
	void ExecChangeSpecular       (const EntityID id, const ColorRGBA& specular);
	void ExecChangeRange          (const EntityID id, const float range);
	void ExecChangeAttenuation    (const EntityID id, const Vec3& attenuation);
	void ExecChangeSpotExponent   (const EntityID id, const float spotExponent);

private:
	EnttSpotLightData spotLightModel_;
	IFacadeEngineToUI*   pFacade_ = nullptr;   // facade interface btw GUI and the engine        
};

}
