#include "ParticleEngine.h"
#include <DMath.h>
#include <log.h>
#include <MathHelper.h>

using namespace DirectX;

namespace ECS
{


//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
ParticleEngine::ParticleEngine()
{
    LogMsg(LOG, "creation of ParticlesEngine");
}

ParticleEngine::~ParticleEngine()
{
}

//---------------------------------------------------------
// Desc:   update the particle engine
//---------------------------------------------------------
void ParticleEngine::Update(const float deltaTime)
{
    for (ParticleSystem& sys : particleSystems_)
        sys.Update(deltaTime);
}

//---------------------------------------------------------
// Desc:   check if we have any particles to render
//---------------------------------------------------------
bool ParticleEngine::HasParticlesToRender() const
{
    for (const ParticleSystem& sys : particleSystems_)
    {
        if (sys.HasParticlesToRender())
            return true;
    }

    return false;
}

//---------------------------------------------------------
// Desc:   get the number of currently alive particles
//---------------------------------------------------------
int ParticleEngine::GetNumParticlesOnScreen(void) const
{
    return (int)particlesToRender_.size();
}

//---------------------------------------------------------
// Desc:   prepare data for particles rendering
// Ret:    array of prepared data for rendering
//---------------------------------------------------------
const cvector<ParticleRenderInstance>& ParticleEngine::GetParticlesToRender()
{
    particlesToRender_.resize(0);

    // go through each particle system and get alive particles to render them
    for (ParticleSystem& sys : particleSystems_)
    {
        sys.GetParticlesToRender(particlesToRender_);
    }

    return particlesToRender_;
}

//---------------------------------------------------------
// Desc:   make an explosion of particles
// Args:   - magnitude:     power of explosion
//         - numParticles:  number of particles involved
//---------------------------------------------------------
void ParticleEngine::Explode(const float magnitude, int numParticles)
{
    float yaw   = 0;
    float pitch = 0;

    cvector<ParticleInitData> initData(numParticles);

    // prepare initial data for particles
    for (int i = 0; i < numParticles; ++i)
    {
        // set the particle's angle
        yaw   = MathHelper::RandF() * 6.28318f;                    // randF * 2pi
        pitch = DEG_TO_RAD(MathHelper::RandF()*(rand()%360));

        initData[i].vel.x = cosf(pitch) * magnitude * MathHelper::RandF();
        initData[i].vel.y = sinf(pitch) * cosf(yaw) * magnitude * MathHelper::RandF();
        initData[i].vel.z = sinf(pitch) * sinf(yaw) * magnitude * MathHelper::RandF();
    }


    // create particles for system 0
    const XMFLOAT3 colorSys0 = particleSystems_[0].GetColor();

    for (int i = 0; i < numParticles; ++i)
        initData[i].color = colorSys0;

    particleSystems_[0].CreateParticles(initData.data(), numParticles);


    // create particles for system 1 (fire)
    const XMFLOAT3 colorSys1 = particleSystems_[1].GetColor();

    for (int i = 0; i < numParticles; ++i)
        initData[i].color = colorSys1;

    particleSystems_[1].CreateParticles(initData.data(), 1);


    // generate random RGB color
    for (int i = 0; i < numParticles; ++i)
        initData[i].color = MathHelper::RandColorRGB();

    particleSystems_[2].CreateParticles(initData.data(), numParticles);
}

} // namespace 
