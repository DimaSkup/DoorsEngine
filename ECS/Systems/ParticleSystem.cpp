#include "../Common/pch.h"
#include "ParticleSystem.h"
#include <math/random.h>
#include <geometry/rect3d_functions.h>
#include <math/vec_functions.h>

#pragma warning (disable : 4996)


namespace ECS
{

//---------------------------------------------------------
// Desc:  default constructor
//---------------------------------------------------------
ParticleSystem::ParticleSystem(
    ParticleEmitter* pParticleComponent,
    TransformSystem* pTransformSys,
    BoundingSystem* pBoundingSys)
    :
    pParticleComponent_(pParticleComponent),
    pTransformSys_(pTransformSys),
    pBoundingSys_(pBoundingSys)
{
    if (!pParticleComponent)
        LogFatal(LOG, "ptr to particle component == NULL");

    if (!pTransformSys)
        LogFatal(LOG, "ptr to transform system == NULL");

    if (!pBoundingSys_)
        LogFatal(LOG, "ptr to bounding system == NULL");
}

//---------------------------------------------------------
// Desc:    add a new emitter
// Args:    - id:   an identifier of the entity which owns this emitter
//---------------------------------------------------------
void ParticleSystem::AddEmitter(const EntityID id)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity id is invalid");
        return;
    }

    // push a new emitter and set related entity ID
    pParticleComponent_->ids.push_back(id);
    pParticleComponent_->data.push_back(EmitterData());
}



//---------------------------------------------------------
// Desc:  update all the particles of the input emitter
// Args:  - emitter:  particles emitter by itself
//        - dt:       delta time
//        - aabb:     emitter's axis-aligned bounding box
//---------------------------------------------------------
void UpdateParticleEmitter(EmitterData& emitter, const float dt, const Rect3d& aabb)
{
    using namespace DirectX;

    const float invLife = 1.0f / emitter.life;
    const float delta   = dt * 200;

    for (Particle& particle : emitter.particles)
    {
        // update particle's age
        particle.ageMs -= dt;

        // if this particle is already dead we remove it
        if (particle.ageMs <= 0.0f)
        {
            // swap n pop
            particle = emitter.particles.back();
            emitter.particles.pop_back();
        }
    }

    // if all dead...
    if (emitter.particles.empty())
        return;


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


    // update particles texture animation (if necessary)
    if (emitter.hasTexAnimations)
    {
        const int  numFramesX = emitter.numTexFramesByX;
        const int  numFramesY = emitter.numTexFramesByY;

        const int   numFrames = numFramesX * numFramesY;
        const float frameTimeMs = 1000.0f / (float)numFrames;

        const float dtu = 1.0f / (float)numFramesX;
        const float dtv = 1.0f / (float)numFramesY;


        for (int i = 0; Particle& particle : emitter.particles)
        {
            int currFrame = (int)floorf(frameTimeMs * particle.ageMs);
            currFrame += particle.frameRandOffset;
            currFrame %= numFrames;

            const int rowIdx = currFrame / numFramesX;
            const int colIdx = currFrame % numFramesX;

            const float tu0 = dtu * colIdx;
            const float tv0 = dtv * rowIdx;

            const float tu1 = dtu * (colIdx + 1);
            const float tv1 = dtv * (rowIdx + 1);

            particle.uv0 = { tu0, tv0 };
            particle.uv1 = { tu1, tv1 };
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
    if (pParticleComponent_->ids.empty())
        return;

    // update only visible emitters
    for (const EntityID id : visEmitters_)
    {
        UpdateParticleEmitter(GetEmitterData(id), dt, GetEmitterWorldAABB(id));
    }

    // generate particles for each active emitter
    CreateNewParticles(dt);
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
    for (const EntityID id : visEmitters_)
    {
        const EmitterData& emitter = GetEmitterData(id);

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
            renderData_.particles[i].color.x = particle.color.x;
            renderData_.particles[i].color.y = particle.color.y;
            renderData_.particles[i].color.z = particle.color.z;
            renderData_.particles[i].color.w = particle.alpha;

            DirectX::XMStoreFloat3(&renderData_.particles[i].pos, particle.pos);

            renderData_.particles[i].uv0     = particle.uv0;
            renderData_.particles[i].uv1     = particle.uv1;
            renderData_.particles[i].size    = particle.size;

            ++i;
        }
    }

    return renderData_;
}

//-----------------------------------------------------
// Desc:  get a position of emitter by input ID
//-----------------------------------------------------
const XMFLOAT3 ParticleSystem::GetEmitterPos(const EntityID id) const
{
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetEmitterData(id).position);
    return pos;
}

//-----------------------------------------------------
// Desc:  generate a new portion of particles from the emitter by id
// Args:  - numNewParticles:  how many particles we want to create
//-----------------------------------------------------
void ParticleSystem::PushNewParticles(const EntityID id, const uint numNewParticles)
{
    EmitterData& data = GetEmitterData(id);

    // force update emitter's position
    data.position = pTransformSys_->GetPositionVec(id);

    // alloc memory for new particles and generate them
    size currNumParticles = data.particles.size();
    data.particles.resize(currNumParticles + numNewParticles);

    Particle* newParticles = &data.particles[currNumParticles];

    SetupNewParticles(id, data, newParticles, numNewParticles);
}

//-----------------------------------------------------
// setup how many particles will be generated each second
//-----------------------------------------------------
void ParticleSystem::SetSpawnRate(const EntityID id, const uint spawnRate)
{
    GetEmitterData(id).spawnRate = spawnRate;
}

//-----------------------------------------------------
// setup material for particle emitter which is bound to entity by ID
//-----------------------------------------------------
void ParticleSystem::SetMaterialId(const EntityID id, const MaterialID matId)
{
    GetEmitterData(id).materialId = matId;
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

    GetEmitterData(id).life = lifeMs * 0.001f;
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

    GetEmitterData(id).mass = mass;
}

//-----------------------------------------------------
// set the size of a created particle
//-----------------------------------------------------
void ParticleSystem::SetSize(const EntityID id, const float sz)
{
    constexpr float lower = 0.001f;

    if (sz <= lower)
    {
        LogErr(LOG, "particle size can't be <= %.3f, for entt: %" PRIu32, lower, id);
        return;
    }

    GetEmitterData(id).size = sz;
}

//-----------------------------------------------------
// setup a color for particles (if there is any texture it will be blended with it)
//-----------------------------------------------------
void ParticleSystem::SetColor(const EntityID id, const float r, const float g, const float b)
{
    GetEmitterData(id).startColor = { r,g,b };
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

    GetEmitterData(id).friction = friction;
}

//-----------------------------------------------------
// set the external forces acting agains a particle (gravity, air, etc.)
//-----------------------------------------------------
void ParticleSystem::SetExternForces(const EntityID id, const float x, const float y, const float z)
{
    GetEmitterData(id).forces = { x,y,z };
}

//-----------------------------------------------------
//-----------------------------------------------------
void ParticleSystem::ResetNumSpawnedParticles(const EntityID id)
{
    GetEmitterData(id).numSpawned = 0;
}

//---------------------------------------------------------
// Desc:   generate new particles for each active emitter
// Args:   - dt: delta time
//---------------------------------------------------------
void ParticleSystem::CreateNewParticles(const float dt)
{
    // prevent generation of too big amount of particles after game pauses
    if (dt >= 0.1f)
        return;


    // update only visible particle emitters
    for (const EntityID id : visEmitters_)
    {
        EmitterData& emitter = GetEmitterData(id);

        if (!emitter.isActive)
            continue;

        if (emitter.srcType == EMITTER_SRC_TYPE_SPLASH)
            continue;

        emitter.time += dt;

        const uint numNewParticles = (uint)(emitter.time * emitter.spawnRate);

        // if too little time spent for generation of any particles
        if (numNewParticles == 0)
            continue;

        emitter.time = 0;

        // maybe TEMP: update position of emitter
        emitter.position = pTransformSys_->GetPositionVec(id);

        // alloc mem for new amount of particles and init them
        const uint newStartIdx = (uint)emitter.particles.size();
        emitter.particles.resize(newStartIdx + numNewParticles);

        Particle* newParticles = emitter.particles.data() + newStartIdx;

        SetupNewParticles(id, emitter, newParticles, numNewParticles);
    }
}

//---------------------------------------------------------
// Desc:    generate data for new particles of input emitter
// Args:    - emitter:       get from here inital params for particles
//          - particles:     init particles from this arr
//          - numParticles:  how many particles to init
//---------------------------------------------------------
void ParticleSystem::SetupNewParticles(
    const EntityID emitterId,
    const EmitterData& initData,
    Particle* particles,
    const uint numParticles)
{
    // check input args
    if (!particles)
    {
        LogErr(LOG, "input arr of particles == NULL");
        return;
    }

    // init velocity: particles move in random directions
    if (initData.velDirInitType == PARTICLE_VELOCITY_DIR_RANDOM)
    {
        for (index i = 0; i < numParticles; ++i)
        {
            // set the particle's angle
            const float yaw   = RandF(0.0f, 1.0f) * PI;
            const float pitch = DEG_TO_RAD(RandF(0.0f, 360.0f));

            // set the particle's position and velocity
            particles[i].vel = {
                cosf(pitch)             * initData.velInitMag * RandF(), // velocity X
                sinf(pitch) * cosf(yaw) * initData.velInitMag * RandF(), // velocity Y
                sinf(pitch) * sinf(yaw) * initData.velInitMag * RandF()  // velocity Z
            };
        }
    }

    // init velocity: particles move in defined direction
    else if (initData.velDirInitType == PARTICLE_VELOCITY_DIR_DEFINED)
    {
        if (initData.srcType == EMITTER_SRC_TYPE_POINT)
        {
            const XMFLOAT3 dir = initData.velInitDir;

            for (index i = 0; i < numParticles; ++i)
            {
                const float randOffsetX = RandF(0.0f, 100.0f) * PI * 0.005f - 0.5f;
                const float randOffsetZ = RandF(0.0f, 100.0f) * PI * 0.005f - 0.5f;

                particles[i].vel = {
                    (dir.x + randOffsetX) * initData.velInitMag,
                    (dir.y              ) * initData.velInitMag,
                    (dir.z + randOffsetZ) * initData.velInitMag,
                };
            }
        }
        // src type: plane/volume
        else
        {
            // calc initial velocity
            const float vx = initData.velInitDir.x * initData.velInitMag;
            const float vy = initData.velInitDir.y * initData.velInitMag;
            const float vz = initData.velInitDir.z * initData.velInitMag;

            for (index i = 0; i < numParticles; ++i)
            {
                particles[i].vel = { vx, vy, vz };
            }
        }
    }


    // init color, size, alpha, etc. for each particle
    for (uint i = 0; i < numParticles; ++i)
    {
        particles[i].color = initData.startColor;
        particles[i].size  = initData.startSize;
        particles[i].alpha = initData.startAlpha;
        particles[i].ageMs = initData.life;
    }
   

    // particles spawn from a single point
    if (initData.srcType == EMITTER_SRC_TYPE_POINT)
    {
        for (uint i = 0; i < numParticles; ++i)
            particles[i].pos = initData.position;
    }

    // particles spawn equally on plane's area
    // (for instance: rain from a plane above the player)
    else if (initData.srcType == EMITTER_SRC_TYPE_PLANE)
    {
        const Rect3d aabb = GetEmitterWorldAABB(emitterId);
        const float  posY = DirectX::XMVectorGetY(initData.position) + initData.srcPlaneHeight;

        for (uint i = 0; i < numParticles; ++i)
        {
            particles[i].pos = {
                RandF(aabb.x0, aabb.x1),
                posY,
                RandF(aabb.z0, aabb.z1),
            };
        }
    }

    // particle spawns at random point inside a volume
    else if (initData.srcType == EMITTER_SRC_TYPE_VOLUME)   
    {
        const Rect3d aabb = GetEmitterWorldAABB(emitterId);

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
    else if (initData.srcType == EMITTER_SRC_TYPE_SPLASH)
    {
        for (uint i = 0; i < numParticles; ++i)
            particles[i].pos = initData.position;
    }
    else
    {
        LogErr(LOG, "can't generate positions: unknown source type: %d", (int)initData.srcType);
    }

    //---------------------------------

    if (initData.hasTexAnimations)
    {
        // calc initial tex coords
        const float tu = 1.0f / (float)initData.numTexFramesByX;
        const float tv = 1.0f / (float)initData.numTexFramesByY;

        for (uint i = 0; i < numParticles; ++i)
        {
            particles[i].uv1             = { tu, tv };
            particles[i].frameRandOffset = (int)RandUint(0, 1000);
        }
    }
}

} // namespace
