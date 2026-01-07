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

enum eEmitterSrcType : uint8
{
    EMITTER_SRC_TYPE_POINT,    // particles generates at a single point
    EMITTER_SRC_TYPE_PLANE,    // generate in random point on plane (plane is defined by upper side of the emitter's bounding box)
    EMITTER_SRC_TYPE_VOLUME,   // generate in random point within emitter's bounding box
    EMITTER_SRC_TYPE_SPLASH,   // generate multiple particles at once and then stop generation
};

//---------------------------------------------------------

// what to do with particle when it hit its emitter's bounding box
enum eEventParticleHitBox : uint8
{
    EVENT_PARTICLE_HIT_BOX_DIE,
    EVENT_PARTICLE_HIT_BOX_REFLECT,
};

//---------------------------------------------------------

enum eVelocityDirInitType : uint8
{
    PARTICLE_VELOCITY_DIR_RANDOM,
    PARTICLE_VELOCITY_DIR_DEFINED,
};

//---------------------------------------------------------

struct Particle
{
    DirectX::XMVECTOR pos   = { 0,0,0 };  // current position: this is the particle's current position in 2D/3D space
    DirectX::XMVECTOR vel   = { 1,1,1 };  // velocity: this is the particle's direction and speed
    DirectX::XMFLOAT3 color = { 1,1,1 };  // the current color of the particle (RGB triplet)
    DirectX::XMFLOAT2 size  = { 1,1 };    // particle's visual size by X and Y in world

    float alpha = 0;               // current alpha value (transparency) of the particle
    float ageMs = 0;               // Life span in milliseconds(!!!). This is how long the particle will live
};

//-----------------------------------------------

struct ParticleRenderInstance
{
    DirectX::XMFLOAT3 pos   = {0,0,0};
    float alpha             = 0.0f;
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

    // alive particles
    cvector<Particle> particles;                            

    // initial values for each particles of this emitter
    DirectX::XMVECTOR position          = { 0,0,0 };
    DirectX::XMVECTOR forces            = { 0,0,0 };        // gravity, air, etc.
    DirectX::XMFLOAT3 startColor        = { 1,1,1 };
    DirectX::XMFLOAT3 endColor          = { 1,1,1 };
    DirectX::XMFLOAT3 colorAfterReflect = { 1,1,1 };        // is used when we set hitEvent to REFLECT
    DirectX::XMFLOAT2 startSize         = { 1,1 };
    DirectX::XMFLOAT2 endSize           = { 1,1 };
    DirectX::XMFLOAT3 velInitDir        = { 0,0,0 };        // velocity initial direction

    float             velInitMag        = 0.15f;            // velocity initial magnitude
    float             life              = 1;                // lifespan of particle in seconds
    float             friction          = 0.1f;             // air resistance
    float             mass              = 1.0f;             // mass of particle
    float             size              = 0.1f;             // size of particle in world
    float             time              = 0.0f;             // need for particles generation (to be independent from fps)
    float             startAlpha        = 1.0f;
    float             endAlpha          = 0.0f;
    float             srcPlaneHeight    = 0.0f;             // local space height of generation plane (if srcType is PLANE)

    int               spawnRate  = 0;                       // number of particles generated per 1 second
    int               numSpawned = 0;
    bool              isActive = true;



    eEmitterSrcType      srcType        = EMITTER_SRC_TYPE_POINT;
    eEventParticleHitBox hitEvent       = EVENT_PARTICLE_HIT_BOX_DIE;  // what to do with particle when it hit its emitter's bounding box
    eVelocityDirInitType velDirInitType = PARTICLE_VELOCITY_DIR_RANDOM;
};

} // namespace
