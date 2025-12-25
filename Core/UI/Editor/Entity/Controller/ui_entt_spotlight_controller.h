// =================================================================================
// Filename:      ui_entt_spotlight_controller.h
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
    void LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id);

    void ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);
    void UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);

    inline const EnttSpotLightData* GetData() const { return &data_; }

private:
    void ExecChangeAmbient     (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& ambient);
    void ExecChangeDiffuse     (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& diffuse);
    void ExecChangeSpecular    (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& specular);
    void ExecChangeRange       (IFacadeEngineToUI* pFacade, const EntityID id, const float range);
    void ExecChangeAttenuation (IFacadeEngineToUI* pFacade, const EntityID id, const Vec3& attenuation);
    void ExecChangeSpotExponent(IFacadeEngineToUI* pFacade, const EntityID id, const float spotExponent);

private:
    EnttSpotLightData data_;   // spotlight data to visualize in the editor
};

}
