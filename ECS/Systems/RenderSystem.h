// =================================================================================
// Filename:     RenderSystem.h
// Description:  an ECS system for execution of entities rendering
// 
// Created:      21.05.24
// =================================================================================
#pragma once


#include "../Common/Types.h"
#include "../Components/Rendered.h"
#include <d3d11.h>

namespace ECS
{

class RenderSystem final
{
public:
    RenderSystem(Rendered* pRenderComponent);

    ~RenderSystem() {}

    void Serialize(std::ofstream& fout, u32& offset);
    void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecords(
        const EntityID* ids,
        const RenderInitParams* params,
        const size numEntts);

    void RemoveRecords(const EntityID* ids, const size numEntts);

    void GetRenderingDataOfEntts(
        const EntityID* ids,
        const size numEntts,
        cvector<ECS::RenderShaderType>& outShaderTypes);


    // clear an arr of entities that were visible in the previous frame;
    // so we will be able to use it again for the current frame;
    inline void ClearVisibleEntts() { pRenderComponent_->visibleEnttsIDs.clear(); }

    // clear the array of visible light sources (were visible in the prev frame);
    // so we will be able to use it again for the current frame;
    inline void ClearVisibleLightSources() { pRenderComponent_->visiblePointLightsIDs.clear(); }


    // for debug/unit-test purposes
    inline const cvector<EntityID>& GetAllEnttsIDs()              const { return pRenderComponent_->ids; }
    inline cvector<EntityID>& GetArrVisibleLightSources()         const { return pRenderComponent_->visiblePointLightsIDs; }

    inline void SetVisibleEntts(const cvector<EntityID>& inEntts)       { pRenderComponent_->visibleEnttsIDs = inEntts; }
    inline const cvector<EntityID>& GetAllVisibleEntts()          const { return pRenderComponent_->visibleEnttsIDs; }
    inline size GetVisibleEnttsCount()                            const { return pRenderComponent_->visibleEnttsIDs.size(); }

private:
    void GetShaderTypesOfEntts(
        const EntityID* ids,
        const size numEntts,
        cvector<ECS::RenderShaderType>& outShaderTypes);

private:
    Rendered* pRenderComponent_ = nullptr;
};

}
