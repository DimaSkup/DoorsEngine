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
    bool LoadFromFile(const char* configPath);

    const ParticlesRenderData& GetParticlesToRender();

    // check if we have any particles to render
    bool HasParticlesToRender() const;

    // set the number of currently alive particles
    int GetNumParticlesOnScreen(void) const;

    ParticleSystem& AddNewParticleSys()
    {
        particleSystems_.push_back(ParticleSystem());
        return particleSystems_.back();
    }

    //-----------------------------------------------------
    // Desc:    search and return a particles system by input index
    //-----------------------------------------------------
    inline ParticleSystem& GetSystemByIdx(const int idx)
    {
        assert(idx < (int)particleSystems_.size() && "input idx of particles system is too big");
        return particleSystems_[idx];
    }

    //-----------------------------------------------------
    // Desc:    search and return a particles system by input name
    //-----------------------------------------------------
    inline ParticleSystem& GetSystemByName(const char* name)
    {
        assert((name != nullptr) && (name[0] != '\0') && "input name for particles system searching is empty");

        for (ParticleSystem& sys : particleSystems_)
        {
            if (strcmp(sys.GetName(), name) == 0)
                return sys;
        }

        return particleSystems_[0];
    }

public:
    ParticlesRenderData      renderData_;
    cvector<ParticleSystem>  particleSystems_;
};

} // namespace
