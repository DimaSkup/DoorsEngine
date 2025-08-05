// =================================================================================
// Filename:      EnttDirLightController.h
// Description:   editor controller (a part of MVC pattern)
//                for execution/undoing of commands 
//                related to the directed light of entities
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>
#include "../Model/EnttLightData.h"        // mvc model


namespace UI
{

class EnttDirLightController
{
public:
    EnttDirLightController() {};

    void Initialize(IFacadeEngineToUI* pFacade);
    void LoadEnttData(const EntityID id);

    void ExecuteCommand(const ICommand* pCmd, const EntityID id);
    void UndoCommand   (const ICommand* pCmd, const EntityID id);

    inline const EnttDirLightData& GetModel() const { return dirLightModel_; }

private:
    void ExecChangeAmbient        (const EntityID id, const ColorRGBA& ambient);
    void ExecChangeDiffuse        (const EntityID id, const ColorRGBA& diffuse);
    void ExecChangeSpecular       (const EntityID id, const ColorRGBA& specular);

private:
    EnttDirLightData   dirLightModel_;
    IFacadeEngineToUI* pFacade_ = nullptr;    // a facade interface btw GUI and the engine
};

} // namespace
