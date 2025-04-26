// =================================================================================
// Filename:      EnttTransformController.h
// Description:   editor controller for execution/undoing of commands 
//                related to entities transformation (changing pos, dir, scale)
// 
// Created:       20.02.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/ICommand.h>
#include <UICommon/IFacadeEngineToUI.h>
#include "../Model/EnttTransformData.h"   // data container


namespace UI
{

class EnttTransformController
{
public:
    EnttTransformController();

    void Initialize(IFacadeEngineToUI* pFacade);
    void LoadEnttData(const EntityID id);

    void ExecuteCommand(const ICommand* pCmd, const EntityID id);
    void UndoCommand   (const ICommand* pCmd, const EntityID id);

    inline const EnttTransformData& GetModel() const { return data_; }

private:
    void ExecChangePosition    (const EntityID id, const Vec3& pos);
    void ExecChangeDirection   (const EntityID id, const Vec4& rotationQuat);
    void ExecChangeUniformScale(const EntityID id, const float scale);
    
    void UndoChangePosition    (const EntityID id, const Vec3& pos);
    void UndoChangeDirection   (const EntityID id, const Vec4& rotationQuat);
    void UndoChangeScale       (const EntityID id, const float uniformScale);

private:
    EnttTransformData  data_;
    IFacadeEngineToUI* pFacade_ = nullptr;    // facade interface btw GUI and engine        
};

}
