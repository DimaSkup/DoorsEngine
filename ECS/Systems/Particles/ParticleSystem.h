#pragma once

#include "../../Systems/TransformSystem.h"
#include "../../Components/ParticleEmitter.h"
#include <Types.h>
#include <log.h>
#include <inttypes.h>

namespace ECS
{

class ParticleSystem
{
public:
    ParticleSystem(TransformSystem* pTransformSys);
    ~ParticleSystem() {}

    //-----------------------------------------------------

    ParticleEmitter& AddEmitter     (const EntityID id);
    void             Update         (const float deltaTime);
    void             CreateParticles(const float deltaTime);
    bool             LoadFromFile   (const char* configPath);

    ParticlesRenderData& GetParticlesToRender();

    inline cvector<ParticleEmitter>& GetEmitters() { return emitters_; }

    
    //-----------------------------------------------------
    // find a particle emitter which is bound to entity by ID
    //-----------------------------------------------------
    inline ParticleEmitter& GetEmitterByEnttId(const EntityID id)
    {
        for (ParticleEmitter& emitter : emitters_)
        {
            if (emitter.id == id)
                return emitter;
        }

        LogErr(LOG, "can't find a particles emitter by entity ID: %" PRIu32, id);
        return emitters_[0];
    }

    //-----------------------------------------------------
    // setup how many particles will be generated each second
    //-----------------------------------------------------
    inline void SetSpawnRate(const EntityID id, const int numParticles)
    {
        if (numParticles <= 0)
            return;

        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.spawnRate = numParticles;
    }

    //-----------------------------------------------------
    // setup material for particle emitter which is bound to entity by ID
    //-----------------------------------------------------
    inline void SetMaterialId(const EntityID id, const MaterialID matId)
    {
        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.materialId = matId;
    }

    //-----------------------------------------------------
    // set the lifespan of emitter's particles
    //-----------------------------------------------------
    inline void SetLife(const EntityID id, const float lifeMs)
    {
        if (lifeMs < 1.0f)
            return;

        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.life = lifeMs/1000;
    }

    //-----------------------------------------------------
    // set the mass of a created particle
    //-----------------------------------------------------
    inline void SetMass(const EntityID id, const float mass)
    {
        if (mass < 0)
            return;

        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.mass = mass;
    }

    //-----------------------------------------------------
    // set the size of a created particle
    //-----------------------------------------------------
    inline void SetSize(const EntityID id, const float sz)
    {
        if (sz <= 0.001f)
            return;

        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.size = sz;
    }

    //-----------------------------------------------------
    // set the color of a created particle
    //-----------------------------------------------------
    inline void SetColor(const EntityID id, const float r, const float g, const float b)
    {
        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.color = DirectX::XMFLOAT3{ r,g,b };
    }

    //-----------------------------------------------------
    // set the friction (air resistance) of a created particle
    //-----------------------------------------------------
    inline void SetFriction(const EntityID id, const float friction)
    {
        if (friction <= 0)
            return;

        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.friction = friction;
    }

    //-----------------------------------------------------
    // set the external forces acting agains a particle (gravity, air, etc.)
    //-----------------------------------------------------
    inline void SetExternForces(const EntityID id, const float x, const float y, const float z)
    {
        ParticleEmitter& emitter = GetEmitterByEnttId(id);
        emitter.forces = DirectX::XMVECTOR{ x,y,z };
    }

    //-----------------------------------------------------
    // check if we have any particle emitters
    //-----------------------------------------------------
    inline bool HasEmitters() const
    {   return emitters_.size() > 0; }


private:
    void ParticlesInitDataGenerator(
        const ParticleEmitter& emitter,
        cvector<Particle>& outParticles);

private:
    TransformSystem* pTransformSys_ = nullptr;

    // each system can have multiple emitters
    // (for instance: fire system can have multiple flames at different positions)
    cvector<ParticleEmitter> emitters_;

    ParticlesRenderData      renderData_;
};

} // namespace 
