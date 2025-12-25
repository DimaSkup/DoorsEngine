// =================================================================================
// Filename:       EnttEditorController.h
// Desc:           main controller over all the entity related MVC
//                 (model-view-controller)
// 
//                 1. handle all the entity editor events
//                 2. execute changes of the entity properties 
//                 3. store this change as a command into the events history stack
// Created:        01.01.25
// =================================================================================
#pragma once

#include <UICommon/ieditor_controller.h>

// model (data)
#include "Model/SelectedEnttData.h"

// view
#include "View/ui_entt_transform_view.h"
#include "View/ui_entt_light_view.h"
#include "View/EnttParticlesView.h"
#include "View/ui_entt_animation_view.h"

// controller
#include "Controller/ui_entt_transform_controller.h"
#include "Controller/ui_entt_dir_light_controller.h"
#include "Controller/ui_entt_point_light_controller.h"
#include "Controller/ui_entt_spotlight_controller.h"
#include "Controller/EnttParticlesController.h"
#include "Controller/ui_entt_animation_controller.h"


namespace UI
{

// forward declarations
class IFacadeEngineToUI;
class ICommand;


class EnttEditorController : public IEditorController
{
public:
    SelectedEnttData            selectedEnttData_;

private:
    // entities MVC views
    EnttTransformView           viewEnttTransform_;
    EnttLightView               viewEnttLight_;
    EnttParticlesView           viewEnttParticles_;
    EnttAnimationView           viewEnttAnimation_;

    // entities MVC controllers
    EnttTransformController     transformController_;
    EnttParticlesController     particlesController_;
    EnttAnimationController*    pAnimationController_ = nullptr;

    EnttDirLightController      dirLightController_;
    EnttPointLightController    pointLightController_;
    EnttSpotLightController     spotLightController_;

    IFacadeEngineToUI*          pFacade_ = nullptr;          // facade interface btw GUI and engine        

public:
    EnttEditorController();
    ~EnttEditorController();

    void Init(IFacadeEngineToUI* pFacade);
    void Render();
    void SetSelectedEntt(const EntityID entityId);

    inline EntityID GetSelectedEnttID() const { return selectedEnttData_.id; }


    // methods for transformations with gizmo
    void UpdateSelectedEnttWorld(const DirectX::XMMATRIX& world);

    // execute command and store this change into the events history
    virtual void ExecCmd(const ICommand* pCommand) override;

    // undo/alt_undo an event from the events history
    virtual void UndoCmd(const ICommand* pCommand, const EntityID entityID) override;
};

} // namespace UI
