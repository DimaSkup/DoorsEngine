#pragma once

#include "../../Components/ParticleEmitter.h"
#include <Types.h>

namespace ECS
{

class ParticleSystem
{
public:
    ParticleSystem();
    ParticleSystem(const int numParticles);
    ~ParticleSystem() {}

    ParticleSystem(ParticleSystem&& rhs) noexcept
        :
        particles_       (std::move(rhs.particles_)),
        maxNumParticles_ (rhs.maxNumParticles_),
        forces_          (rhs.forces_),
        pos_             (rhs.pos_),
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

    void Update(const float deltaTime);

    void CreateParticles(const ParticleInitData* data, const int numData);
    void CreateParticle (const ParticleInitData& data);

    void GetParticlesToRender(cvector<ParticleRenderInstance>& outInstances);

    inline DirectX::XMFLOAT3 GetColor() const { return color_; }

    // set the lifespan (in milliseconds !!!) of a created particle
    inline void SetLife(const float life)
    {   life_ = life/1000;   }

    // set the particle emitter's position
    inline void SetEmitPos(const float x, const float y, const float z)
    {   pos_ = { x,y,z };   }

    // set the mass of a created particle
    inline void SetMass(const float mass)
    {   mass_ = mass;   }

    // set the size of a created particle
    inline void SetSize(const float pixelSize)
    {   size_ = pixelSize; }

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
    {   return (particles_.size() > 0);   }


private:
    cvector<Particle> particles_;
    int               maxNumParticles_ = 0;

    // gravity, air, etc.
    DirectX::XMVECTOR forces_;

    DirectX::XMVECTOR pos_ = { 0,0,0 };
    DirectX::XMFLOAT3 color_ = { 0,0,0 };

    // base particle attributes
    float             life_     = 0;
    float             mass_     = 0;
    float             size_     = 0;
    float             friction_ = 0;

    MaterialID        materialId_ = INVALID_MATERIAL_ID;
};

} // namespace 
