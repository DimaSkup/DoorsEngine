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
    char              particleSysName[32]{'\0'};
    DirectX::XMFLOAT3 pos             = { 0,0,0 };
    DirectX::XMFLOAT3 color           = { 0,0,0 };
    DirectX::XMFLOAT3 forces          = { 0,0,0 };
    int               maxNumParticles = 0;
    float             lifetimeMs      = 0;
    float             particleSize    = 0;
    float             mass            = 0;
    float             friction        = 0;          // air resistance
    

    // read in data from file
    fscanf(pFile, "particle_system: %s\n",          &particleSysName);
    fscanf(pFile, "max_num_particles: %d\n",        &maxNumParticles);
    fscanf(pFile, "emitter_pos: %f, %f, %f\n",      &pos.x, &pos.y, &pos.z);
    fscanf(pFile, "lifetime_ms: %f\n",              &lifetimeMs);
    fscanf(pFile, "color: %f, %f, %f\n",            &color.x, &color.y, &color.z);

    fscanf(pFile, "size: %f\n",                     &particleSize);
    fscanf(pFile, "mass: %f\n",                     &mass);
    fscanf(pFile, "friction: %f\n",                 &friction);   
    fscanf(pFile, "external_forces: %f, %f, %f\n",  &forces.x, &forces.y, &forces.z);
    fscanf(pFile, "\n");


    // add and setup particle system
    ECS::ParticleSystem& sys = pParticleEngine->AddNewParticleSys(maxNumParticles);
    sys.SetEmitPos(pos.x, pos.y, pos.z);
    sys.SetLife(lifetimeMs);
    sys.SetColor(color.x, color.y, color.z);
    sys.SetSize(particleSize);
    sys.SetMass(mass);
    sys.SetFriction(friction);
    sys.SetExternalForces(forces.x, forces.y, forces.z);


#if 0
    SetConsoleColor(CYAN);

    // print system data (debug)
    LogMsg("system name: %s", particleSysName);
    LogMsg("emitter pos: %f %f %f", pos.x, pos.y, pos.z);
    LogMsg("lifetime (ms): %d", lifetimeMs);
    LogMsg("color: %f %f %f", color.x, color.y, color.z);
    LogMsg("size: %f", particleSize);
    LogMsg("mass: %f", mass);
    LogMsg("friction: %f", friction);
    LogMsg("extern forces: %f %f %f", forces.x, forces.y, forces.z);

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
