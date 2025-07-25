#include "ParticleSystem.h"
#include <log.h>

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
// Desc:   initialize the particle engine
// Args:   - numParticles:  number of particles in the system
// Ret:    true if we successfully initialize the particle engine
//---------------------------------------------------------
ParticleSystem::ParticleSystem(const int numParticles)
{
    if (numParticles <= 0)
    {
        LogErr(LOG, "the number of particles cannot be <= 0");
        return;
    }

    maxNumParticles_ = numParticles;
    particles_.reserve(numParticles);
}

//---------------------------------------------------------
// Desc:   create a new particle
// Args:   - velX, velY, velZ:  the new particle's velocity
//---------------------------------------------------------
void ParticleSystem::CreateParticle(const ParticleInitData& data)
{
    CreateParticles(&data, 1);
}

//---------------------------------------------------------
// Desc:    create multiple particles at a time
// Args:    - data: arr of containers for initialization of each particle
//---------------------------------------------------------
void ParticleSystem::CreateParticles(const ParticleInitData* data, const int numData)
{
    if (!data)
    {
        LogErr(LOG, "input arr of particles init data == nullptr");
        return;
    }

    if (numData <= 0)
    {
        LogErr(LOG, "input number of data elements to init particles can't be <= 0");
        return;
    }

    const int prevNumParticles = (int)particles_.size();
    const int newNumParticles  = prevNumParticles + numData;

    // check if we can add new particles
    if (maxNumParticles_ <= newNumParticles)
        return;

    // push new particles
    particles_.resize(newNumParticles);


    for (int idx = prevNumParticles, i = 0; idx < newNumParticles; ++idx, ++i)
    {
        Particle& particle = particles_[idx];

        // set the particle's lifespan
        particle.ageMs        = life_;

        // set the particle's position and velocity
        particle.pos          = pos_;
        particle.vel          = XMLoadFloat3(&data[i].vel);

        //set the particle's color and transparency
        particle.color        = data[i].color;
        particle.translucency = 1.0f;

        // set the particle's size, mass, and air resistance
        particle.size         = size_;
        particle.mass         = mass_;
        particle.friction     = friction_;
    }
}

//---------------------------------------------------------
// Desc:   update the particle system
//---------------------------------------------------------
void ParticleSystem::Update(const float deltaTime)
{
    const float invLife = 1.0f / life_;

    // loop through all the particles
    for (Particle& particle : particles_)
    {
        // age the particle
        particle.ageMs -= deltaTime;


        // if this particle is already dead
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
            particle.pos += (particle.vel * particle.mass);

            // now it's time for the external forces to take their toll
            particle.vel *= (1 - particle.friction);
            particle.vel += forces_;

            // set the particle's transparency (based on its age)
            particle.translucency = particle.ageMs * invLife;
        }
    }
}

//---------------------------------------------------------
// Desc:   push into output array data of currently alive particles to render them
// Out:    - outInstances:   array of particles rendering data
//---------------------------------------------------------
void ParticleSystem::GetParticlesToRender(cvector<ParticleRenderInstance>& outInstances)
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
}

} // namespace
