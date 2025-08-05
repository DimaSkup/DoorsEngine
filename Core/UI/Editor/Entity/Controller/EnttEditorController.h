// =================================================================================
// Filename:       EnttEditorController.h
// Description:    1. handle all the entity editor events
//                 2. execute changes of the entity properties 
//                 3. store this change as a command into the events history stack
// Created:        01.01.25
// =================================================================================
#pragma once

#include <UICommon/StatesGUI.h>
#include <UICommon/IEditorController.h>
#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../View/EnttTransformView.h"
#include "../View/EnttLightView.h"
#include "../View/EnttParticlesView.h"

#include "../Model/SelectedEnttData.h"
#include "EnttTransformController.h"
#include "EnttDirLightController.h"
#include "EnttPointLightController.h"
#include "EnttSpotLightController.h"
#include "EnttParticlesController.h"


namespace UI
{

class EnttEditorController : public IEditorController
{
public:
    SelectedEnttData            selectedEnttData_;

private:
    // entities MVC views
    EnttTransformView           viewEnttTransform_;
    EnttLightView               viewEnttLight_;
    EnttParticlesView           viewEnttParticles_;

    // entities MVC controllers
    EnttTransformController     transformController_;
    EnttParticlesController     particlesController_;

    EnttDirLightController      dirLightController_;
    EnttPointLightController    pointLightController_;
    EnttSpotLightController     spotLightController_;

    IFacadeEngineToUI*          pFacade_ = nullptr;          // facade interface btw GUI and engine        
    StatesGUI*                  pStatesGUI_ = nullptr;

public:
    EnttEditorController(StatesGUI* pStatesGUI);

    void Initialize(IFacadeEngineToUI* pFacade);
    void Render();
    void SetSelectedEntt(const EntityID entityID);

    inline EntityID GetSelectedEnttID() const { return selectedEnttData_.id; }


    // methods for transformations with gizmo
    void UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world);

    // execute command and store this change into the events history
    virtual void ExecCmd(const ICommand* pCommand) override;

    // undo/alt_undo an event from the events history
    virtual void UndoCmd(const ICommand* pCommand, const EntityID entityID) override;
};

} // namespace UI
