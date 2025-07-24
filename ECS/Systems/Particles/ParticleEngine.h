#pragma once

#include "ParticleSystem.h"


namespace ECS
{
    
class ParticleEngine
{
public:
    ParticleEngine();
    ~ParticleEngine();

    void Update(const float deltaTime);

    void Explode(const float magnitude, int numParticles);
    const cvector<ParticleRenderInstance>& GetParticlesToRender();

    // check if we have any particles to render
    bool HasParticlesToRender() const;

    // set the number of currently alive particles
    int GetNumParticlesOnScreen(void) const;

    ParticleSystem& AddNewParticleSys(const int maxNumParticles)
    {
        particleSystems_.push_back(ParticleSystem(maxNumParticles));
        return particleSystems_.back();
    }

    ParticleSystem& GetParticleSys(const int idx) { return particleSystems_[idx]; }

private:
    cvector<ParticleRenderInstance> particlesToRender_;
    cvector<ParticleSystem>         particleSystems_;


};

} // namespace
