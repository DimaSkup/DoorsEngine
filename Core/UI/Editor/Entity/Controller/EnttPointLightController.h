// =================================================================================
// Filename:      EnttPointLightController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the point light entities
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


class EnttPointLightController
{
public:
    EnttPointLightController() {};

	void Initialize(IFacadeEngineToUI* pFacade);
	void LoadEnttData(const EntityID id);

	void ExecuteCommand(const ICommand* pCmd, const EntityID id);
	void UndoCommand   (const ICommand* pCmd, const EntityID id);

	inline const EnttPointLightData& GetModel() const { return pointLightModel_; }

private:
	// change entt point light props
    void ExecChangeActivation (const EntityID id, const float isActive);
	void ExecChangeAmbient    (const EntityID id, const ColorRGBA& ambient);
	void ExecChangeDiffuse    (const EntityID id, const ColorRGBA& diffuse);
	void ExecChangeSpecular   (const EntityID id, const ColorRGBA& specular);
	void ExecChangeRange      (const EntityID id, const float range);
	void ExecChangeAttenuation(const EntityID id, const Vec3& att);

private:
	EnttPointLightData pointLightModel_;
	IFacadeEngineToUI* pFacade_ = nullptr;   // facade interface btw GUI and engine

};

} // namespace UI
