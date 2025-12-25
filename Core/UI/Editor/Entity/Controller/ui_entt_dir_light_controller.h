// =================================================================================
// Filename:      ui_entt_dir_light_controller.h
// Description:   editor controller (a part of MVC pattern)
//                for execution/undoing of commands 
//                related to the directed light of entities
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#pragma once

#include <types.h>
#include "../Model/EnttLightData.h"        // mvc model


namespace UI
{
// forward declaration
class IFacadeEngineToUI;
class ICommand;

class EnttDirLightController
{
public:
    void LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id);

    void ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);
    void UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);

    inline const EnttDirLightData* GetData() const { return &data_; }

private:
    void ExecChangeAmbient (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& ambient);
    void ExecChangeDiffuse (IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& diffuse);
    void ExecChangeSpecular(IFacadeEngineToUI* pFacade, const EntityID id, const ColorRGBA& specular);

private:
    EnttDirLightData data_;
};

} // namespace
