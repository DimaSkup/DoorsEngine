// =================================================================================
// Filename:   ParticleEmitter.h
// Desc:       data structures and ECS compute for particle emission
//
// Created:    23.07.2025  by DimaSkup
// =================================================================================
#pragma once

#include <Types.h>
#include <cvector.h>
#include <DirectXMath.h>


namespace ECS
{

struct Particle
{
    DirectX::XMVECTOR pos   = { 0,0,0 };  // current position: this is the particle's current position in 2D/3D space
    DirectX::XMVECTOR vel   = { 1,1,1 };  // velocity: this is the particle's direction and speed
    DirectX::XMFLOAT3 color = { 1,1,1 };  // the current color of the particle (RGB triplet)

    float translucency = 0;               // current alpha value (transparency) of the particle
    float ageMs        = 0;               // Life span in milliseconds(!!!). This is how long the particle will live
    float mass         = 0;               // is used to accurately model particle motion
    float size         = 0;               // particle's visual size
    float friction     = 0;               // air resistance: this is the particle's susceptibility to friction in the air
};

//-----------------------------------------------

struct ParticleRenderInstance
{
    DirectX::XMFLOAT3 pos   = {0,0,0};
    float translucency      = 0.0f;
    DirectX::XMFLOAT3 color = {1,1,1};
    DirectX::XMFLOAT2 size  = {1,1};
};

//-----------------------------------------------

struct ParticlesRenderData
{
    void Reset()
    {
        materialIds.resize(0);
        baseInstance.resize(0);
        numInstances.resize(0);
    }

    // particles data
    cvector<ParticleRenderInstance> particles;      // bunch of the all particles to render (from different systems)
    cvector<MaterialID>             materialIds;    // material identifier per each particles system
    cvector<UINT>                   baseInstance;   // start idx of instances for particular particles system
    cvector<UINT>                   numInstances;   // how many particles we have per each particles system
};

//-----------------------------------------------

struct ParticleEmitter
{
    ParticleEmitter() {}
    ParticleEmitter(EntityID enttId) : id(enttId) {}

    EntityID          id         = INVALID_ENTITY_ID;       // identifier of entity to which this emitter is bound
    MaterialID        materialId = INVALID_MATERIAL_ID;

    cvector<Particle> particles;                            // alive particles

    DirectX::XMVECTOR position   = { 0,0,0 };
    DirectX::XMVECTOR forces     = { 0,0,0 };               // gravity, air, etc.
    DirectX::XMFLOAT3 color      = { 1,0,0 };

    float             life       = 1000;                    // lifespan of particle
    float             friction   = 0.1f;                    // air resistance
    float             mass       = 1.0f;
    float             size       = 0.1f;
    float             time       = 0.0f;                    // need for particles generation (to be independent from fps)
  
    int               spawnRate  = 0;                       // number of particles generated per 1 second
    bool              isEmitting = true;
};

} // namespace
