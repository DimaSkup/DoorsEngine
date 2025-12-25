// =================================================================================
// Filename:      ui_entt_point_light_controller.h
// Description:   editor controller for execution/undoing of commands 
//                related to the point light entities
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <types.h>
#include "../Model/EnttLightData.h"


namespace UI
{
// forward declaration
class IFacadeEngineToUI;
class ICommand;


class EnttPointLightController
{
public:
	void LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id);

	void ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);
	void UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);

	inline const EnttPointLightData* GetData() const { return &data_; }

private:
	// execute change of entt's point light props
    void ExecChangeActivation (IFacadeEngineToUI* pFacade, const EntityID id, const float isActive);
	void ExecChangeAmbient    (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& ambient);
	void ExecChangeDiffuse    (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& diffuse);
	void ExecChangeSpecular   (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& specular);
	void ExecChangeRange      (IFacadeEngineToUI* pFacade, const EntityID id, const float range);
	void ExecChangeAttenuation(IFacadeEngineToUI* pFacade, const EntityID id, const Vec3& att);

private:
	EnttPointLightData data_;   // point light data to visualize in the editor
};

} // namespace UI
