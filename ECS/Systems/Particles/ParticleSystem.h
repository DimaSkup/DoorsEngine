#pragma once

#include "../../Components/ParticleEmitter.h"
#include <Types.h>

namespace ECS
{

class ParticleSystem
{
public:
    ParticleSystem();
    ~ParticleSystem() {}

    //-----------------------------------------------------

    ParticleSystem(ParticleSystem&& rhs) noexcept
        :
        emitters_        (std::move(rhs.emitters_)),
        forces_          (rhs.forces_),
        color_           (rhs.color_),
        life_            (rhs.life_),
        mass_            (rhs.mass_),
        size_            (rhs.size_),
        friction_        (rhs.friction_)
    {
    }

    ParticleSystem& operator=(ParticleSystem&& rhs) noexcept
    {
        if (this != &rhs)
        {
            std::construct_at(this, std::move(rhs));
        }

        return *this;
    }

    //-----------------------------------------------------

    bool AddEmitter     (const EntityID id);
    void Update         (const float deltaTime);
    void CreateParticles(const float deltaTime);

    //-----------------------------------------------------

    
    UINT GetParticlesToRender(cvector<ParticleRenderInstance>& outInstances);

    inline const cvector<ParticleEmitter>& GetEmitters()   const { return emitters_; }
    inline const char*                     GetName()       const { return sysName_;}
    inline DirectX::XMFLOAT3               GetColor()      const { return color_; }
    inline MaterialID                      GetMaterialId() const { return materialId_; }

    //-----------------------------------------------------

    // set name for this particle system
    void SetName(const char* name);

    // setup how many particles will be generated each second
    inline void SetNumParticlesPerSec(const int numParticles)
    {
        if (numParticles > 0)
            genNumParticlesPerSec_ = numParticles;
    }

    // setup material for this particles system
    inline void SetMaterialId(const MaterialID matId)
    {   materialId_ = matId;    }

    // set the lifespan (in milliseconds !!!) of a created particle
    inline void SetLife(const float lifeMs)
    {   life_ = lifeMs/1000;   }

    // set the mass of a created particle
    inline void SetMass(const float mass)
    {   mass_ = mass;   }

    // set the size of a created particle
    inline void SetSize(const float sz)
    {   size_ = sz; }

    // set the color of a created particle
    inline void SetColor(const float r, const float g, const float b)
    {   color_ = DirectX::XMFLOAT3(r,g,b);   }

    // set the friction (air resistance) of a created particle
    inline void SetFriction(const float friction)
    {   friction_ = friction;   }

    // set the external forces acting agains a particle (gravity, air, etc.)
    inline void SetExternalForces(const float x, const float y, const float z)
    {   forces_ = DirectX::XMVECTOR{x,y,z};  }

    // check if we have any alive particles for rendering
    inline bool HasParticlesToRender() const
    {   return particles_.size() > 0;   }

    // check if we have any instances of this system
    inline bool HasEmitters() const
    {   return emitters_.size() > 0; }

private:
    void ParticlesInitDataGenerator(cvector<Particle>& outParticles);

private:
    // each system can have multiple emitters
    // (for instance: fire system can have multiple flames at different positions)
    cvector<ParticleEmitter> emitters_;

    // actual particles (the same for each emitter)
    cvector<Particle> particles_;

    // number of particles generated per 1 second
    int               genNumParticlesPerSec_ = 0;

    // gravity, air, etc.
    DirectX::XMVECTOR forces_;

    DirectX::XMFLOAT3 color_       = { 0,0,0 };

    float             life_        = 0;
    float             mass_        = 0;
    float             size_        = 0;
    float             friction_    = 0;

    MaterialID        materialId_  = INVALID_MATERIAL_ID;

    // need for particles generation (to be independent from fps)
    float             time_        = 0;

    char              sysName_[32] = { "particle_system_name" };
};

} // namespace 
