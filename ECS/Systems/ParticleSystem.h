/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ParticleSystem.h
    Desc:     ECS system for work with particle emitters

\**********************************************************************************/
#pragma once

#include <geometry/rect3d.h>
#include "TransformSystem.h"
#include "BoundingSystem.h"
#include "../Components/ParticleEmitter.h"

namespace ECS
{

class ParticleSystem
{
public:
    ParticleSystem(
        ParticleEmitter* pParticleComponent,
        TransformSystem* pTransformSys,
        BoundingSystem* pBoundSys);

    ~ParticleSystem() {}

    //-----------------------------------------------------

    void                      AddEmitter     (const EntityID id);
    void                      Update         (const float dt);

    ParticlesRenderData&      GetParticlesToRender();
    const cvector<Particle>&  GetParticlesOfEmitter(const EntityID id);

    const cvector<EntityID>&  GetAllEmitters() const;
    const EmitterData&        GetEmitterData(const EntityID id) const;
    EmitterData&              GetEmitterData(const EntityID id);

    const DirectX::XMFLOAT3   GetEmitterPos      (const EntityID id) const;
    Rect3d                    GetEmitterLocalAABB(const EntityID id) const;
    Rect3d                    GetEmitterWorldAABB(const EntityID id) const;

    void                      PushNewParticles(const EntityID id, const uint number);

    bool IsActive       (const EntityID id) const;

    void SetSpawnRate   (const EntityID id, const uint spawnRate);
    void SetMaterialId  (const EntityID id, const MaterialID matId);
    void SetLife        (const EntityID id, const float lifeMs);
    void SetMass        (const EntityID id, const float mass);
    void SetSize        (const EntityID id, const float sz);
    void SetColor       (const EntityID id, const float r, const float g, const float b);
    void SetFriction    (const EntityID id, const float friction);
    void SetExternForces(const EntityID id, const float x, const float y, const float z);

    void ResetNumSpawnedParticles(const EntityID id);

private:
    index GetEmitterIdx(const EntityID id) const;

    void  CreateNewParticles(const float dt);

    void SetupNewParticles(
        const EntityID emitterId,
        const EmitterData& initData,
        Particle* particles,
        const uint numParticles);

public:
    // visible emitters
    cvector<EntityID>   visEmitters_;

private:
    ParticleEmitter*    pParticleComponent_ = nullptr;
    TransformSystem*    pTransformSys_      = nullptr;
    BoundingSystem*     pBoundingSys_       = nullptr;

    ParticlesRenderData renderData_;
};

//==================================================================================
// inline functions
//==================================================================================
inline index ParticleSystem::GetEmitterIdx(const EntityID id) const
{
    const index idx = pParticleComponent_->ids.get_idx(id);

    if (pParticleComponent_->ids.is_valid_index(idx))
        return idx;

    return 0;
}

inline const EmitterData& ParticleSystem::GetEmitterData(const EntityID id) const
{
    return pParticleComponent_->data[GetEmitterIdx(id)];
}

inline EmitterData& ParticleSystem::GetEmitterData(const EntityID id)
{
    return pParticleComponent_->data[GetEmitterIdx(id)];
}

inline const cvector<EntityID>& ParticleSystem::GetAllEmitters() const
{
    return pParticleComponent_->ids;
}

inline Rect3d ParticleSystem::GetEmitterLocalAABB(const EntityID id) const
{
    return pBoundingSys_->GetLocalBoxRect3d(id);
}

inline Rect3d ParticleSystem::GetEmitterWorldAABB(const EntityID id) const
{
    return pBoundingSys_->GetWorldBoxRect3d(id);
}

inline const cvector<Particle>& ParticleSystem::GetParticlesOfEmitter(const EntityID id)
{
    return GetEmitterData(id).particles;
}

inline bool ParticleSystem::IsActive(const EntityID id) const
{
    return GetEmitterData(id).isActive;
}

} // namespace 
