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

#include "TransformSystem.h"
#include "BoundingSystem.h"
#include "../Components/ParticleEmitter.h"
#include <geometry/rect_3d.h>

namespace ECS
{

class ParticleSystem
{
public:
    ParticleSystem(TransformSystem* pTransformSys, BoundingSystem* pBoundSys);
    ~ParticleSystem() {}

    //-----------------------------------------------------

    ParticleEmitter& AddEmitter     (const EntityID id);
    void             Update         (const float deltaTime);
    void             CreateParticles(const float gameTime);

    ParticlesRenderData& GetParticlesToRender();

    inline cvector<ParticleEmitter>& GetEmitters()
    {
        return emitters_;
    }

    inline const cvector<Particle>& GetParticlesOfEmitter(const EntityID id)
    {
        const ParticleEmitter& emitter = GetEmitterByEnttId(id);
        return emitter.particles;
    }

    ParticleEmitter& GetEmitterByEnttId(const EntityID id);
    Rect3d GetEmitterAABB(const EntityID id);
    Rect3d GetEmitterLocalAABB(const EntityID id);

    const XMFLOAT3 GetEmitterPos(const EntityID id);

    void SetSpawnRate   (const EntityID id, const uint spawnRate);
    void SetMaterialId  (const EntityID id, const MaterialID matId);
    void SetLife        (const EntityID id, const float lifeMs);
    void SetMass        (const EntityID id, const float mass);
    void SetSize        (const EntityID id, const float sz);
    void SetColor       (const EntityID id, const float r, const float g, const float b);
    void SetFriction    (const EntityID id, const float friction);
    void SetExternForces(const EntityID id, const float x, const float y, const float z);

    void ResetNumSpawnedParticles(const EntityID id);


    //-----------------------------------------------------
    // check if we have any particle emitters
    //-----------------------------------------------------
    inline bool HasEmitters() const { return emitters_.size() > 0; }

private:
    void InitParticles(
        const ParticleEmitter& emitter,
        Particle* particles,
        const uint numParticles);

public:
    // update and render only particles of visible emitters
    cvector<index>   visibleEmittersIdxs_;

private:
    TransformSystem* pTransformSys_ = nullptr;
    BoundingSystem*  pBoundingSys_  = nullptr;

    // each system can have multiple emitters
    // (for instance: fire system can have multiple flames at different positions)
    cvector<ParticleEmitter> emitters_;

    ParticlesRenderData      renderData_;
};

} // namespace 
