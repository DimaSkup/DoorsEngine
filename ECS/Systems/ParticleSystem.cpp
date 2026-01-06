#include "../Common/pch.h"
#include "ParticleSystem.h"
#include <math/random.h>
#include <geometry/rect_3d_functions.h>
#include <math/vec_functions.h>

#pragma warning (disable : 4996)

using namespace DirectX;


namespace ECS
{

//---------------------------------------------------------
// Desc:    constructor
//---------------------------------------------------------
ParticleSystem::ParticleSystem(
    TransformSystem* pTransformSys,
    BoundingSystem* pBoundingSys)
    :
    pTransformSys_(pTransformSys),
    pBoundingSys_(pBoundingSys)
{
    if (!pTransformSys_)
        LogErr(LOG, "input ptr to transform system == nullptr");

    if (!pBoundingSys_)
        LogErr(LOG, "input ptr to bounding system == nullptr");
}

//---------------------------------------------------------
// Desc:    add a new emitter (instance of this system)
// Args:    - id:   an identifier of the entity which owns this emitter
// 
// Ret:     a ref to the added emitter
//---------------------------------------------------------
ParticleEmitter& ParticleSystem::AddEmitter(const EntityID id)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity id is invalid");
    }

    // push a new emitter and set related entity ID
    emitters_.push_back(ParticleEmitter(id));

    return emitters_.back();
}

//---------------------------------------------------------
// Desc:    generate data for new particles of input emitter
// Args:    - emitter:       get from here inital params for particles
//          - particles:     init particles from this arr
//          - numParticles:  how many particles to init
//---------------------------------------------------------
void ParticleSystem::InitParticles(
    const ParticleEmitter& emitter,
    Particle* particles,
    const uint numParticles)
{
    // check input args
    if (!particles)
    {
        LogErr(LOG, "input arr of particles == nullptr");
        return;
    }


    // init velocity: particles move in random directions
    if (emitter.velDirInitType == PARTICLE_VELOCITY_DIR_RANDOM)
    {
        for (index i = 0; i < numParticles; ++i)
        {
            // set the particle's angle
            const float     yaw = RandF(0.0f, 1.0f) * PI;
            const float     pitch = DEG_TO_RAD(RandF(0.0f, 360.0f));

            Particle& particle = particles[i];

            // set the particle's position and velocity
            particle.vel = {
                cosf(pitch)             * emitter.velInitMag * RandF(), // velocity X
                sinf(pitch) * cosf(yaw) * emitter.velInitMag * RandF(), // velocity Y
                sinf(pitch) * sinf(yaw) * emitter.velInitMag * RandF()  // velocity Z
            };
        }
    }

    // init velocity: particles move in defined direction
    else if (emitter.velDirInitType == PARTICLE_VELOCITY_DIR_DEFINED)
    {

        if (emitter.srcType == EMITTER_SRC_TYPE_POINT)
        {
            for (index i = 0; i < numParticles; ++i)
            {
                const float randOffsetX = RandF(0.0f, 100.0f) * PI * 0.005f - 0.5f;
                const float randOffsetZ = RandF(0.0f, 100.0f) * PI * 0.005f - 0.5f;

                particles[i].vel = {
                    (emitter.velInitDir.x + randOffsetX) * emitter.velInitMag,
                    emitter.velInitDir.y * emitter.velInitMag,
                    (emitter.velInitDir.z + randOffsetZ) * emitter.velInitMag,
                };
            }
        }
        // src type: plane/volume
        else
        {
            for (index i = 0; i < numParticles; ++i)
            {
                particles[i].vel = {
                    emitter.velInitDir.x * emitter.velInitMag,
                    emitter.velInitDir.y * emitter.velInitMag,
                    emitter.velInitDir.z * emitter.velInitMag,
                };
            }
        }
    }


    // init color, size, alpha, etc. for each particle
    for (uint i = 0; i < numParticles; ++i)
    {
        particles[i].color = emitter.startColor;
        particles[i].size  = emitter.startSize;
        particles[i].alpha = emitter.startAlpha;
        particles[i].ageMs = emitter.life;
    }
   

    // particles spawn from a single point
    if (emitter.srcType == EMITTER_SRC_TYPE_POINT)        
    {
        for (uint i = 0; i < numParticles; ++i)
            particles[i].pos = emitter.position;
    }

    // particles spawn equally on plane's area
    else if (emitter.srcType == EMITTER_SRC_TYPE_PLANE)   
    {
        const Rect3d aabb = GetEmitterAABB(emitter.id);
        const float  posY = XMVectorGetY(emitter.position) + emitter.srcPlaneHeight;

        for (uint i = 0; i < numParticles; ++i)
        {
            particles[i].pos = {
                RandF(aabb.x0, aabb.x1),
                posY,
                RandF(aabb.z0, aabb.z1),
            };
        }
    }

    // particles spawn equally at random point inside a volume
    else if (emitter.srcType == EMITTER_SRC_TYPE_VOLUME)   
    {
        const Rect3d aabb = GetEmitterAABB(emitter.id);

        for (uint i = 0; i < numParticles; ++i)
        {
            particles[i].pos = {
                RandF(aabb.x0, aabb.x1),
                RandF(aabb.y0, aabb.y1),
                RandF(aabb.z0, aabb.z1),
            };
        }
    }

    // generate multiple particles at once and then stop generation
    else if (emitter.srcType == EMITTER_SRC_TYPE_SPLASH)
    {
        for (uint i = 0; i < numParticles; ++i)
            particles[i].pos = emitter.position;
    }

    else
    {
        LogErr(LOG, "can't generate positions: unknown source type: %d", (int)emitter.srcType);
    }
}

//---------------------------------------------------------
// Desc:   generate new particles for each active emitter
// Args:   - dt: delta time
//---------------------------------------------------------
void ParticleSystem::CreateParticles(const float dt)
{
    // prevent generation of too big amount of particles after game pauses
    if (dt >= 0.1f)
        return;

    // maybe TEMP: update emitting position of each emitter
    for (ParticleEmitter& emitter : emitters_)
    {
        emitter.position = pTransformSys_->GetPositionVec(emitter.id);
    }


    // update only visible particle emitters
    for (const index i : visibleEmittersIdxs_)
    {
        ParticleEmitter& emitter = emitters_[i];

        if (!emitter.isActive)
            continue;

        if (emitter.srcType == EMITTER_SRC_TYPE_SPLASH)
        {
            bool ableEmit = (emitter.numSpawned < emitter.spawnRate);

            if (!ableEmit)
                continue;

            // emit particles for the splash
            emitter.particles.resize(emitter.spawnRate);

            InitParticles(
                emitter,
                emitter.particles.data(),
                emitter.spawnRate);

            emitter.numSpawned = emitter.spawnRate;

            //emitter.isActive = false;
            continue;
        }


        emitter.time += dt;
        const uint numNewParticles = (uint)(emitter.time * emitter.spawnRate);

        // if too little time spent for generation of any particles
        if (numNewParticles == 0)
            continue;

        emitter.time = 0;

        // alloc memory for new amount of particles and init them
        const uint newStartIdx = (uint)emitter.particles.size();
        emitter.particles.resize(newStartIdx + numNewParticles);

        InitParticles(
            emitter,
            emitter.particles.data() + newStartIdx,
            numNewParticles);
    }
}

//---------------------------------------------------------
// Desc:  update all the particles of the input emitter
// Args:  - emitter:  particles emitter by itself
//        - dt:       delta time
//        - aabb:     emitter's axis-aligned bounding box
//---------------------------------------------------------
void UpdateParticleEmitter(
    ParticleEmitter& emitter,
    const float dt,
    const Rect3d& aabb)
{
    const float invLife = 1.0f / emitter.life;
    const float delta   = dt * 200;

    // update particles age
    for (Particle& particle : emitter.particles)
        particle.ageMs -= dt;

    // if this particle is already dead we remove it
    for (Particle& particle : emitter.particles)
    {
        if (particle.ageMs <= 0.0f)
        {
            // swap with the last and pop_back (remove particle)
            particle = emitter.particles.back();
            emitter.particles.pop_back();
        }
    }

    // update particles positions
    const float velChangeFactor = emitter.mass * delta;

    for (Particle& particle : emitter.particles)
        particle.pos += (particle.vel * velChangeFactor);


    // 1. if particle still in AABB we do nothing
    // 2. according to emitter's hit event:
    //    a) kill particle if it is out of AABB
    //    b) reflect particle from AABB (so it remains withing AABB)
    for (Particle& particle : emitter.particles)
    {
        const float posX = XMVectorGetX(particle.pos);
        const float posY = XMVectorGetY(particle.pos);
        const float posZ = XMVectorGetZ(particle.pos);

        const bool outX = (posX < aabb.x0) || (posX > aabb.x1);
        const bool outY = (posY < aabb.y0) || (posY > aabb.y1);
        const bool outZ = (posZ < aabb.z0) || (posZ > aabb.z1);

        if (outX || outY || outZ)
        {
            // kill particle
            if (emitter.hitEvent == EVENT_PARTICLE_HIT_BOX_DIE)
            {
                particle = emitter.particles.back();
                emitter.particles.pop_back();
            }

            // reflect particle
            else if (emitter.hitEvent == EVENT_PARTICLE_HIT_BOX_REFLECT)
            {
                const float reflectX = (outX) ? -1.0f : 1.0f;
                const float reflectY = (outY) ? -1.0f : 1.0f;
                const float reflectZ = (outZ) ? -1.0f : 1.0f;

                particle.vel *= XMVECTOR{ reflectX, reflectY, reflectZ };
                particle.color = emitter.colorAfterReflect;
            }
        }
    }

    // now it's time for the external forces to take their toll
    const float    friction = (1 - emitter.friction * delta);
    const XMVECTOR forces   = emitter.forces * delta;

    for (Particle& particle : emitter.particles)
    {
        particle.vel *= friction;
        particle.vel += forces;
    }


    // change transparency during life
    if (emitter.startAlpha != emitter.endAlpha)
    {
        const float startAlpha = emitter.startAlpha;
        const float endAlpha   = emitter.endAlpha;

        for (Particle& particle : emitter.particles)
        {
            const float lerpFactor = particle.ageMs * invLife;
            particle.alpha         = lerp(startAlpha, endAlpha, lerpFactor);
        }
    }


    // change particle color during life
    // (if hit box reflect then particle can have only 2 colors: before and after hit, so we don't update them here)
    if (emitter.hitEvent != EVENT_PARTICLE_HIT_BOX_REFLECT)
    {
        const Vec3 colorStart(&emitter.startColor.x);
        const Vec3 colorEnd(&emitter.endColor.x);

        for (Particle& particle : emitter.particles)
        {
            const float lerpFactor = particle.ageMs * invLife;
            Vec3 color;

            Vec3Lerp(colorStart, colorEnd, lerpFactor, color);
            particle.color = XMFLOAT3(color.x, color.y, color.z);
        }
    }


    // change particle size during life
    const Vec2 sizeStart(&emitter.startSize.x);
    const Vec2 sizeEnd(&emitter.endSize.x);

    if (sizeStart != sizeEnd)
    {
        for (Particle& particle : emitter.particles)
        {
            const float lerpFactor = particle.ageMs * invLife;
            Vec2 size;

            Vec2Lerp(sizeStart, sizeEnd, lerpFactor, size);
            particle.size = XMFLOAT2(size.x, size.y);
        }
    }
}

//---------------------------------------------------------
// Desc:   update each particle emitter
// Args:   - dt:  delta time
//---------------------------------------------------------
void ParticleSystem::Update(const float dt)
{
    // prevent updating after game pauses
    if (dt >= 0.1f)
        return;

    // if we have no emitters for this system
    if (emitters_.empty())
        return;

    // update only visible emitters
    for (const index i : visibleEmittersIdxs_)
    {
        ParticleEmitter& emitter = emitters_[i];
        const Rect3d aabb = GetEmitterAABB(emitter.id);
        UpdateParticleEmitter(emitter, dt, aabb);
    }
}

//---------------------------------------------------------
// Desc:   push into output array rendering data of currently alive particles
// Out:    - outInstances:   array of particles rendering data
// Ret:    the number of added particles for rendering
//---------------------------------------------------------
ParticlesRenderData& ParticleSystem::GetParticlesToRender()
{
    renderData_.particles.resize(0);                    // clear particles from the prev frame
    renderData_.Reset();


    // go through each active particle emitter and gather alive particles
    for (const index idx : visibleEmittersIdxs_)
    {
        const ParticleEmitter& emitter = emitters_[idx];

        if (!emitter.isActive)
            continue;


        const vsize prevNumInstances      = renderData_.particles.size();
        const vsize numParticlesInEmitter = emitter.particles.size();
        index i                           = prevNumInstances;

        //
        // for this emitter...
        // 

        // ... we start rendering particles from this "baseInstance" idx
        renderData_.baseInstance.push_back((UINT)renderData_.particles.size());

        // ... we will render "numInstances" particles
        renderData_.numInstances.push_back((UINT)numParticlesInEmitter);

        // ... and use a material by this id
        renderData_.materialIds.push_back(emitter.materialId);


        // store data of each alive particle
        renderData_.particles.resize(prevNumInstances + numParticlesInEmitter);

        for (const Particle& particle : emitter.particles)
        {
            DirectX::XMStoreFloat3(&renderData_.particles[i].pos, particle.pos);

            renderData_.particles[i].alpha  = particle.alpha;
            renderData_.particles[i].color  = particle.color;
            renderData_.particles[i].size   = particle.size;

            ++i;
        }
    }

    return renderData_;
}

//-----------------------------------------------------
// find a particle emitter which is bound to entity by ID
//-----------------------------------------------------
ParticleEmitter& ParticleSystem::GetEmitterByEnttId(const EntityID id)
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
// Desc:  return an AABB of emitter by input ID (the AABB will be in WORLD space)
//-----------------------------------------------------
Rect3d ParticleSystem::GetEmitterAABB(const EntityID id)
{
    const XMFLOAT3 pos = GetEmitterPos(id);
    const DirectX::BoundingBox& aabb = pBoundingSys_->GetAABB(id);

    const XMFLOAT3& c = aabb.Center;
    const XMFLOAT3& e = aabb.Extents;

    return Rect3d(
        c.x - e.x + pos.x,   // minX
        c.x + e.x + pos.x,   // maxX
        c.y - e.y + pos.y,   // minY
        c.y + e.y + pos.y,   // maxY
        c.z - e.z + pos.z,   // minY
        c.z + e.z + pos.z);  // maxZ
}

//-----------------------------------------------------
// Desc:  return an AABB of emitter by input ID (the AABB will be in LOCAL space)
//-----------------------------------------------------
Rect3d ParticleSystem::GetEmitterLocalAABB(const EntityID id)
{
    const DirectX::BoundingBox& aabb = pBoundingSys_->GetAABB(id);

    const XMFLOAT3& c = aabb.Center;
    const XMFLOAT3& e = aabb.Extents;

    return Rect3d(
        c.x - e.x,   // minX
        c.x + e.x,   // maxX
        c.y - e.y,   // minY
        c.y + e.y,   // maxY
        c.z - e.z,   // minY
        c.z + e.z);  // maxZ
}

//-----------------------------------------------------
// Desc:  get a position of emitter by input ID
//-----------------------------------------------------
const XMFLOAT3 ParticleSystem::GetEmitterPos(const EntityID id)
{
    const ParticleEmitter& emitter = GetEmitterByEnttId(id);
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, emitter.position);
    return pos;
}

//-----------------------------------------------------
// setup how many particles will be generated each second
//-----------------------------------------------------
void ParticleSystem::SetSpawnRate(const EntityID id, const uint spawnRate)
{
    GetEmitterByEnttId(id).spawnRate = spawnRate;
}

//-----------------------------------------------------
// setup material for particle emitter which is bound to entity by ID
//-----------------------------------------------------
void ParticleSystem::SetMaterialId(const EntityID id, const MaterialID matId)
{
    GetEmitterByEnttId(id).materialId = matId;
}

//-----------------------------------------------------
// set the lifespan of emitter's particles
//-----------------------------------------------------
void ParticleSystem::SetLife(const EntityID id, const float lifeMs)
{
    if (lifeMs < 1.0f)
    {
        LogErr(LOG, "you can't set particles lifetime to be below 1 ms for entt: %" PRIu32, id);
        return;
    }

    GetEmitterByEnttId(id).life = lifeMs * 0.001f;
}

//-----------------------------------------------------
// set the mass of a created particle
//-----------------------------------------------------
void ParticleSystem::SetMass(const EntityID id, const float mass)
{
    if (mass < 0)
    {
        LogErr(LOG, "you can't set particles mass to be below 0 for entt: %" PRIu32, id);
        return;
    }

    GetEmitterByEnttId(id).mass = mass;
}

//-----------------------------------------------------
// set the size of a created particle
//-----------------------------------------------------
void ParticleSystem::SetSize(const EntityID id, const float sz)
{
    constexpr float lower = 0.001f;

    if (sz <= lower)
    {
        LogErr(LOG, "particle size can't be <= %f, for entt: %" PRIu32, lower, id);
        return;
    }

    GetEmitterByEnttId(id).size = sz;
}

//-----------------------------------------------------
// setup a color for particles (if there is any texture it will be blended with it)
//-----------------------------------------------------
void ParticleSystem::SetColor(const EntityID id, const float r, const float g, const float b)
{
    GetEmitterByEnttId(id).startColor = DirectX::XMFLOAT3{ r,g,b };
}

//-----------------------------------------------------
// set the friction (air resistance) of a created particle
//-----------------------------------------------------
void ParticleSystem::SetFriction(const EntityID id, const float friction)
{
    if (friction <= 0)
    {
        LogErr(LOG, "input friction (air resistance) can't be <= 0 for entt: %" PRIu32, id);
        return;
    }

    GetEmitterByEnttId(id).friction = friction;
}

//-----------------------------------------------------
// set the external forces acting agains a particle (gravity, air, etc.)
//-----------------------------------------------------
void ParticleSystem::SetExternForces(const EntityID id, const float x, const float y, const float z)
{
    GetEmitterByEnttId(id).forces = DirectX::XMVECTOR{ x,y,z };
}

//-----------------------------------------------------
//-----------------------------------------------------
void ParticleSystem::ResetNumSpawnedParticles(const EntityID id)
{
    GetEmitterByEnttId(id).numSpawned = 0;
}

} // namespace
