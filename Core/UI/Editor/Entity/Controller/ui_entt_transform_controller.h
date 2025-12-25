// =================================================================================
// Filename:      ui_entt_transform_controller.h
// Description:   editor controller for execution/undoing of commands 
//                related to entities transformation (changing pos, dir, scale)
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include "../Model/EnttTransformData.h"   // data container


namespace UI
{
// forward declarations
class ICommand;
class IFacadeEngineToUI;


class EnttTransformController
{
public:
    EnttTransformController() {};

    void LoadEnttData(IFacadeEngineToUI* pFacade, const EntityID id);

    void ExecCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);
    void UndoCmd(IFacadeEngineToUI* pFacade, const ICommand* pCmd, const EntityID id);

    inline const EnttTransformData* GetData() const { return &data_; }

private:
    void ExecChangePos  (IFacadeEngineToUI* pFacade, const EntityID id, const Vec3& pos);
    void ExecChangeRot  (IFacadeEngineToUI* pFacade, const EntityID id, const Vec4& rotationQuat);
    void ExecChangeScale(IFacadeEngineToUI* pFacade, const EntityID id, const float scale);
    
    void UndoChangePos  (IFacadeEngineToUI* pFacade, const EntityID id, const Vec3& pos);
    void UndoChangeRot  (IFacadeEngineToUI* pFacade, const EntityID id, const Vec4& rotQuat);
    void UndoChangeScale(IFacadeEngineToUI* pFacade, const EntityID id, const float uniformScale);

private:
    EnttTransformData data_;
};

}
