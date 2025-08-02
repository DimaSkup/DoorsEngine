#include "ParticleSystem.h"
#include <log.h>
#include <DMath.h>
#include <MathHelper.h>

using namespace DirectX;

namespace ECS
{

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
ParticleSystem::ParticleSystem()
{
}

//---------------------------------------------------------
// Desc:    add a new emitter (instance of this system)
// Args:    - id:   an identifier of the entity which owns this emitter
// 
// Ret:     true if everything is ok
//---------------------------------------------------------
bool ParticleSystem::AddEmitter(const EntityID id)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity id is invalid");
        return false;
    }

    // push a new emitter and set related entity ID
    emitters_.push_back(ParticleEmitter(id));

    return true;
}

//---------------------------------------------------------
// Desc:    setup a name for this particles system
// Args:    - name:  a new name for the system
//---------------------------------------------------------
void ParticleSystem::SetName(const char* name)
{
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, "input name for particles system is empty");
        return;
    }

    size_t len = strlen(name);
    len = (len <= 32) ? len : 32;

    strncpy(sysName_, name, len);
    sysName_[len] = '\0';
}

//---------------------------------------------------------
// Desc:    generate data for new particles
// Args:    - outData:       output array of particles
//          - numParticles:  how many particles we have
//---------------------------------------------------------
void ParticleSystem::ParticlesInitDataGenerator(cvector<Particle>& outParticles)
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

        //set the particle's color and transparency
        particle.color          = color_;
        particle.translucency   = 1.0f;

        // set the particle's lifespan
        particle.ageMs          = life_;

        // set the particle's size, mass, and air resistance
        particle.mass           = mass_;
        particle.size           = size_;
        particle.friction       = friction_;
    }
}

//---------------------------------------------------------
// Desc:   generate amount of new particles according to genParticlesPerSec parameter
// Args:   - dt: delta time in seconds
//---------------------------------------------------------
void ParticleSystem::CreateParticles(const float dt)
{
    time_ += dt;
    const float createOneParticlePerSec = 1.0f / genNumParticlesPerSec_;

    // if too little time spent for generation of any particles
    if (time_ <= createOneParticlePerSec)
        return;

    // compute how many particles we have
    const int numNewParticles  = (int)(time_ / createOneParticlePerSec);

    time_ -= (numNewParticles * createOneParticlePerSec);

    // generate some init params for each new particle
    cvector<Particle> newParticles(numNewParticles);
    ParticlesInitDataGenerator(newParticles);

    // push new particles
    particles_.append_vector(std::move(newParticles));
}

//---------------------------------------------------------
// Desc:   update the particle system
// Args:   - dt:  delta time in seconds
//---------------------------------------------------------
void ParticleSystem::Update(const float dt)
{
    // if we have no emitters for this system
    if (emitters_.empty())
        return;


    const float invLife = 1.0f / life_;
    const float delta   = dt * 200;

    // loop through all the particles and update them
    for (Particle& particle : particles_)
    {
        // age the particle
        particle.ageMs -= dt;


        // if this particle is already dead we remove it
        if (particle.ageMs <= 0.0f)
        {
            particle = particles_.back();
            particles_.pop_back();
            continue;
        }

        // our particle is still alive so update its params
        else
        {
            // update the particle's position
            particle.pos += (particle.vel * particle.mass * delta);

            // now it's time for the external forces to take their toll
            particle.vel *= (1 - particle.friction * delta);
            particle.vel += forces_ * delta;

            // set the particle's transparency (based on its age)
            particle.translucency = particle.ageMs * invLife;
        }
    }
}

//---------------------------------------------------------
// Desc:   push into output array rendering data of currently alive particles
// Out:    - outInstances:   array of particles rendering data
// Ret:    the number of added particles for rendering
//---------------------------------------------------------
UINT ParticleSystem::GetParticlesToRender(cvector<ParticleRenderInstance>& outInstances)
{
    const vsize curNumAliveParticles = particles_.size();

    // prepare enough memory
    const vsize prevNumInstances = outInstances.size();
    outInstances.resize(prevNumInstances + curNumAliveParticles);

    index idx = prevNumInstances;

    // store data of each alive particle
    for (int i = 0; i < curNumAliveParticles; ++i)
    {
        // copy position
        DirectX::XMStoreFloat3(&outInstances[idx].pos, particles_[i].pos);

        outInstances[idx].translucency = particles_[i].translucency;
        outInstances[idx].color = particles_[i].color;
        outInstances[idx].size  = { particles_[i].size, particles_[i].size };

        ++idx;
    }

    return (UINT)curNumAliveParticles;
}

} // namespace
