#include "ParticleSystem.h"
#include <log.h>
#include <DMath.h>
#include <MathHelper.h>

using namespace DirectX;

namespace ECS
{

//---------------------------------------------------------
// Desc:    constructor
//---------------------------------------------------------
ParticleSystem::ParticleSystem(TransformSystem* pTransformSys)
    : pTransformSys_(pTransformSys)
{
    if (!pTransformSys_)
    {
        LogErr(LOG, "input ptr to transform system == nullptr");
    }
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
// Desc:    generate data for new particles
// Args:    - emitter:   get from here inital params for particles
//          - outData:   output array of particles
//---------------------------------------------------------
void ParticleSystem::ParticlesInitDataGenerator(
    const ParticleEmitter& emitter,
    cvector<Particle>& outParticles)
{
    if (outParticles.empty())
    {
        LogErr(LOG, "input arr of particles is empty!");
        return;
    }

    for (Particle& particle : outParticles)
    {
        // set the particle's angle
        constexpr float magnitude = 0.15f;
        const float     yaw       = MathHelper::RandF() * 6.28318f;                    // randF * 2pi
        const float     pitch     = DEG_TO_RAD(MathHelper::RandF() * (rand() % 360));


        // set the particle's position and velocity
        particle.pos = { 0,0,0 };
        particle.vel = {
            cosf(pitch) * magnitude * MathHelper::RandF(),             // velocity X
            sinf(pitch) * cosf(yaw) * magnitude * MathHelper::RandF(), // velocity Y
            sinf(pitch) * sinf(yaw) * magnitude * MathHelper::RandF()  // velocity Z
        };

        particle.pos          = emitter.position;

        //set the particle's color and transparency
        particle.color        = emitter.color;
        particle.translucency = 1.0f;

        // set the particle's lifespan
        particle.ageMs        = emitter.life;

        // set the particle's size, mass, and air resistance
        particle.mass         = emitter.mass;
        particle.size         = emitter.size;
        particle.friction     = emitter.friction;
    }
}

//---------------------------------------------------------
// Desc:   generate particles for each active emitter
// Args:   - dt: delta time
//---------------------------------------------------------
void ParticleSystem::CreateParticles(const float dt)
{
    // maybe TEMP: update emitting position of each emitter
    for (ParticleEmitter& emitter : emitters_)
    {
        emitter.position = pTransformSys_->GetPositionVec(emitter.id);
    }


    for (ParticleEmitter& emitter : emitters_)
    {
     
        emitter.time += dt;
        const float createOneParticleEachSec = 1.0f / emitter.spawnRate;

        // if too little time spent for generation of any particles
        if (emitter.time <= createOneParticleEachSec)
            return;

        // compute how many particles we have
        const int numNewParticles = (int)(emitter.time / createOneParticleEachSec);

        emitter.time -= (numNewParticles * createOneParticleEachSec);

        // generate some init params for each new particle
        cvector<Particle> newParticles(numNewParticles);
        ParticlesInitDataGenerator(emitter, newParticles);

        // push new particles
        emitter.particles.append_vector(std::move(newParticles));
    }
}

//---------------------------------------------------------
// Desc:   update each particle emitter
// Args:   - dt:  delta time
//---------------------------------------------------------
void ParticleSystem::Update(const float dt)
{
    // if we have no emitters for this system
    if (emitters_.empty())
        return;


    const float delta = dt * 200;


    for (ParticleEmitter& emitter : emitters_)
    {
        const float invLife = 1.0f / emitter.life;

        // loop through all the particles and update them
        for (Particle& particle : emitter.particles)
        {
            // age the particle
            particle.ageMs -= dt;


            // if this particle is already dead we remove it
            if (particle.ageMs <= 0.0f)
            {
                particle = emitter.particles.back();
                emitter.particles.pop_back();
                continue;
            }

            // our particle is still alive so update its params
            else
            {
                // update the particle's position
                particle.pos += (particle.vel * particle.mass * delta);

                // now it's time for the external forces to take their toll
                particle.vel *= (1 - particle.friction * delta);
                particle.vel += emitter.forces * delta;

                // set the particle's transparency (based on its age)
                particle.translucency = particle.ageMs * invLife;
            }
        }
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
    for (ParticleEmitter& emitter : emitters_)
    {
        if (!emitter.isEmitting)
            continue;


        const vsize prevNumInstances = renderData_.particles.size();
        const vsize numParticlesInEmitter = emitter.particles.size();
        index idx = prevNumInstances;

        // for this emitter we start rendering particles from this "baseInstance" idx
        renderData_.baseInstance.push_back((UINT)renderData_.particles.size());

        // for this emitter we will render "numInstances" particles
        renderData_.numInstances.push_back((UINT)numParticlesInEmitter);

        // and use a material by this id
        renderData_.materialIds.push_back(emitter.materialId);

        // prepare enough memory
        renderData_.particles.resize(prevNumInstances + numParticlesInEmitter);

        // store data of each alive particle
        for (const Particle& particle : emitter.particles)
        {
            // copy position
            DirectX::XMStoreFloat3(&renderData_.particles[idx].pos, particle.pos);

            renderData_.particles[idx].translucency = particle.translucency;
            renderData_.particles[idx].color        = particle.color;
            renderData_.particles[idx].size         = { particle.size, particle.size };

            ++idx;
        }
    }

    return renderData_;
}

//---------------------------------------------------------
// Desc:    load data for file and create a new particle emitters using this data
// Args:    - pFile:   particles config file descriptor
//---------------------------------------------------------
void ReadAndCreateEmitter(ParticleSystem* pSys, FILE* pFile)
{
    XMFLOAT3 pos                = { 0,0,0 };
    XMFLOAT3 color              = { 0,0,0 };
    XMFLOAT3 forces             = { 0,0,0 };
    EntityID entityId           = INVALID_ENTITY_ID;
    int      genParticlesPerSec = 0;
    float    lifetimeMs         = 0;
    float    particleSize       = 0;
    float    mass               = 0;
    float    friction           = 0;          // air resistance
    int      matId              = 0;
    

    // read in data from file
    fscanf(pFile, "entity_id: %d",                  &entityId);
    fscanf(pFile, "gen_particles_per_sec: %d\n",    &genParticlesPerSec);
    fscanf(pFile, "lifetime_ms: %f\n",              &lifetimeMs);
    fscanf(pFile, "color: %f, %f, %f\n",            &color.x, &color.y, &color.z);

    fscanf(pFile, "size: %f\n",                     &particleSize);
    fscanf(pFile, "mass: %f\n",                     &mass);
    fscanf(pFile, "friction: %f\n",                 &friction);   
    fscanf(pFile, "external_forces: %f, %f, %f\n",  &forces.x, &forces.y, &forces.z);
    fscanf(pFile, "material_id: %d",                &matId);
    fscanf(pFile, "\n");


    // add and setup a particle system
    ParticleEmitter& emitter = pSys->AddEmitter(entityId);

    emitter.spawnRate  = genParticlesPerSec;
    emitter.life       = lifetimeMs / 1000;
    emitter.color      = XMFLOAT3{ color.x, color.y, color.z };
    emitter.size       = particleSize;
    emitter.mass       = mass;
    emitter.friction   = friction;
    emitter.forces     = XMVECTOR{ forces.x, forces.y, forces.z };
    emitter.materialId = matId;
}

//---------------------------------------------------------
// Desc:    load particles emitters data from config file
// Args:    - configPath:  a path to the particles config file
//                         (relatively to the working directory)
// Ret:     true if we managed to it
//---------------------------------------------------------
bool ParticleSystem::LoadFromFile(const char* configPath)
{
    // check input args
    if (!configPath || configPath[0] == '\0')
    {
        LogErr(LOG, "input path to particles config is empty");
        return false;
    }

    // open config file
    FILE* pFile = fopen(configPath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open particles config file: %s", configPath);
        return false;
    }

    int  numParticleSys  = 0;

    // read in configs
    fscanf(pFile, "num_particle_systems: %d\n", &numParticleSys);
    fscanf(pFile, "\n");

    // read in a system
    for (int i = 0; i < numParticleSys; ++i)
        ReadAndCreateEmitter(this, pFile);

    return true;
}

} // namespace
