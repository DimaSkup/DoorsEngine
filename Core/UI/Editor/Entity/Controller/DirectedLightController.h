// =================================================================================
// Filename:      DirectedLightController.h
// Description:   editor controller for execution/undoing of commands 
//                related to the directed light entities
// 
// Created:       15.03.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>

#include "../Model/LightEditorModel.h"


namespace UI
{

class DirectedLightController
{
public:
    DirectedLightController() {};

    void Initialize(IFacadeEngineToUI* pFacade);
    void LoadEnttData(const EntityID id);

    void ExecuteCommand(const ICommand* pCmd, const EntityID id);
    void UndoCommand   (const ICommand* pCmd, const EntityID id);

    inline const ModelEntityDirLight& GetModel() const { return dirLightModel_; }

private:
    void ExecChangeAmbient        (const EntityID id, const ColorRGBA& ambient);
    void ExecChangeDiffuse        (const EntityID id, const ColorRGBA& diffuse);
    void ExecChangeSpecular       (const EntityID id, const ColorRGBA& specular);
    void ExecChangeDirection      (const EntityID id, const Vec3& direction);
    void ExecChangeDirectionByQuat(const EntityID id, const Vec4& dirQuat);

private:
    ModelEntityDirLight dirLightModel_;
    IFacadeEngineToUI* pFacade_ = nullptr;    // facade interface btw GUI and the engine
};




} // namespace UI
