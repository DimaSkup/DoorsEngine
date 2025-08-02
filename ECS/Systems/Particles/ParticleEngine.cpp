#include "ParticleEngine.h"
#include <DMath.h>
#include <log.h>
#include <MathHelper.h>
#pragma warning (disable : 6031)

using namespace DirectX;


namespace ECS
{

//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
ParticleEngine::ParticleEngine()
{
    LogMsg(LOG, "creation of ParticlesEngine");

    // add a default particle system (will be bound to entity if we don't
    // specify any system when add a particle emitter component to this entity)
    ECS::ParticleSystem& sys = AddNewParticleSys();
    sys.SetName("default");
    sys.SetNumParticlesPerSec(10);
    sys.SetLife(1000);
    sys.SetColor(1, 0, 0);
    sys.SetSize(1.0f);
    sys.SetMass(1.0f);
    sys.SetFriction(0.01f);
    sys.SetExternalForces(0.0f, -0.01f, 0.0f);
    sys.SetMaterialId(INVALID_MATERIAL_ID);
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
// Desc:    load data for file and create a new particle system using this data
// Args:    - pFile:   particles config file descriptor
//---------------------------------------------------------
void ReadAndCreateSystem(ParticleEngine* pParticleEngine, FILE* pFile)
{
    char     particleSysName[32]{'\0'};
    XMFLOAT3 pos                = { 0,0,0 };
    XMFLOAT3 color              = { 0,0,0 };
    XMFLOAT3 forces             = { 0,0,0 };
    int      genParticlesPerSec = 0;
    float    lifetimeMs         = 0;
    float    particleSize       = 0;
    float    mass               = 0;
    float    friction           = 0;          // air resistance
    int      matId              = 0;
    

    // read in data from file
    fscanf(pFile, "particle_system: %s\n",          &particleSysName);
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
    ECS::ParticleSystem& sys = pParticleEngine->AddNewParticleSys();
    sys.SetName(particleSysName);
    sys.SetNumParticlesPerSec(genParticlesPerSec);
    sys.SetLife(lifetimeMs);
    sys.SetColor(color.x, color.y, color.z);
    sys.SetSize(particleSize);
    sys.SetMass(mass);
    sys.SetFriction(friction);
    sys.SetExternalForces(forces.x, forces.y, forces.z);
    sys.SetMaterialId(matId);


#if 0
    SetConsoleColor(CYAN);

    // print system data (debug)
    LogMsg("system name: %s", particleSysName);
    LogMsg("lifetime (ms): %d", lifetimeMs);
    LogMsg("color: %f %f %f", color.x, color.y, color.z);
    LogMsg("size: %f", particleSize);
    LogMsg("mass: %f", mass);
    LogMsg("friction: %f", friction);
    LogMsg("extern forces: %f %f %f", forces.x, forces.y, forces.z);
    LogMsg("material_id: %d", materialId)

    LogMsg("\n");
#endif
}

//---------------------------------------------------------
// Desc:    load particles systems data from config file
// Args:    - configPath:  a path to the particles config file
//                         (relatively to the working directory)
// Ret:     true if we managed to it
//---------------------------------------------------------
bool ParticleEngine::LoadFromFile(const char* configPath)
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
        ReadAndCreateSystem(this, pFile);

    return true;
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
    return (int)renderData_.particles.size();
}

//---------------------------------------------------------
// Desc:   prepare data for particles rendering
// Ret:    array of prepared data for rendering
//---------------------------------------------------------
const ParticlesRenderData& ParticleEngine::GetParticlesToRender()
{
    renderData_.particles.resize(0);                    // clear particles from the prev frame
    renderData_.Reserve((int)particleSystems_.size());  // reserve memory ahead
    renderData_.Reset();

    // go through each particle system and gather data needed for
    // particles rendering of this particular system
    for (int i = 0; ParticleSystem& sys : particleSystems_)
    {
        // if we have nothing to render for this system
        if (!sys.HasEmitters())
            continue;

        int numEmittersPerSys = 0;

        // go through each emitter of this system
        for (const ParticleEmitter& emitter : sys.GetEmitters())
        {
            renderData_.ids.push_back(emitter.id);
            numEmittersPerSys++;
        }

        // set how many instances (emitters) of this system we have
        renderData_.numEmittersPerSystem.push_back(numEmittersPerSys);

        // for this system we start rendering particles from this "baseInstance" idx
        renderData_.baseInstance.push_back((UINT)renderData_.particles.size());

        // for this system we will render "numInstances" particles
        renderData_.numInstances.push_back(sys.GetParticlesToRender(renderData_.particles));

        // and use a material by this id
        renderData_.materialIds[i] = sys.GetMaterialId();                                

        // increase idx if we have any particles to render for this system
        i += (bool)(renderData_.numInstances[i]);
    }

    return renderData_;
}

} // namespace 
