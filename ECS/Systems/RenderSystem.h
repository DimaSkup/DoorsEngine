// =================================================================================
// Filename:     RenderSystem.h
// Description:  an ECS system for execution of entities rendering
// 
// Created:      21.05.24
// =================================================================================
#pragma once

#include "../Components/Rendered.h"

namespace ECS
{

class RenderSystem
{
public:
    RenderSystem(Rendered* pRenderComponent);

    ~RenderSystem() {}

    void AddRecords(
        const EntityID* ids,
        const size numEntts);

    void RemoveRecords(const EntityID* ids, const size numEntts);


    // clear an arr of entities that were visible in the previous frame;
    // so we will be able to use it again for the current frame;
    inline void ClearVisibleEntts() { pRenderComponent_->visibleEnttsIDs.clear(); }

    // clear the array of visible light sources (were visible in the prev frame);
    // so we will be able to use it again for the current frame;
    inline void ClearVisibleLightSources() { pRenderComponent_->visiblePointLightsIDs.clear(); }


    // for debug/unit-test purposes
    inline const cvector<EntityID>& GetAllEnttsIDs()  const { return pRenderComponent_->ids; }
    inline cvector<EntityID>& GetVisiblePointLights() const { return pRenderComponent_->visiblePointLightsIDs; }
    

    inline void SetVisibleEntts(const cvector<EntityID>& inEntts)       { pRenderComponent_->visibleEnttsIDs = inEntts; }
    inline cvector<EntityID>& GetAllVisibleEntts()                const { return pRenderComponent_->visibleEnttsIDs; }
    inline size GetVisibleEnttsCount()                            const { return pRenderComponent_->visibleEnttsIDs.size(); }

private:
    Rendered* pRenderComponent_ = nullptr;
};

}
