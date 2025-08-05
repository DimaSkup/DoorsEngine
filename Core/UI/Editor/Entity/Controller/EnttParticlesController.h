// =================================================================================
// Filename:      EnttParticlesController.h
// Description:   editor controller (a part of MVC pattern)
//                for execution/undoing of commands 
//                related to the particles of entity
// 
// Created:       04.08.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>
#include "../Model/EnttParticlesModel.h"   // mvc model


namespace UI
{

class EnttParticlesController
{
public:
    EnttParticlesController() {};

    void Initialize(IFacadeEngineToUI* pFacade);

    void ExecCmd(const ICommand* pCmd, const EntityID id);
    void UndoCmd(const ICommand* pCmd, const EntityID id);

    inline const EnttParticlesModel& GetModel() const { return model_; }

private:
    EnttParticlesModel model_;
    IFacadeEngineToUI* pFacade_ = nullptr;    // a facade interface btw GUI and the engine
};

} // namespace
