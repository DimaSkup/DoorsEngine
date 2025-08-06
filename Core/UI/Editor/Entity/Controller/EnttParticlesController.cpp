// =================================================================================
// Filename:      EnttParticlesController.cpp
// 
// Created:       04.08.2025  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttParticlesController.h"
#include <UICommon/EditorCommands.h>
#pragma warning (disable : 4996)

namespace UI
{

//---------------------------------------------------------
// Desc:   initialize the particles controller
// Args:   - pFacade: the facade interface is used to contact with the rest of the engine
//---------------------------------------------------------
void EnttParticlesController::Initialize(IFacadeEngineToUI* pFacade)
{
    assert(pFacade != nullptr && "input ptr to the facade interface == nullptr");
    pFacade_ = pFacade;
}

//---------------------------------------------------------
// Desc:   load params of particle emitter which is bound to entt by ID
//---------------------------------------------------------
void EnttParticlesController::LoadEnttData(const EntityID id)
{
    if (pFacade_ == nullptr)
    {
        LogErr("ptr to facade interface == nullptr (you have to init it first!)");
        return;
    }

    if (!pFacade_->GetEnttParticleEmitterData(
        id,
        model_.color,
        model_.externForce,
        model_.spawnNumPerSec,
        model_.lifespanMs,
        model_.mass,
        model_.size,
        model_.friction))
    {
        LogErr(LOG, "can't load particle emitter data for entity by ID: %ld", id);
    }
}

//---------------------------------------------------------
// Desc:   execute the input command to change particles properties of entity by ID
// Args:   - pCmd:   a command to be executed
//         - id:     entity identifier
//---------------------------------------------------------
void EnttParticlesController::ExecCmd(const ICommand* pCmd, const EntityID id)
{
    switch (pCmd->type_)
    {
        case CHANGE_PARTICLES_COLOR:
        {
            if (pFacade_->SetParticlesColor(id, pCmd->GetColorRGB()))
                model_.color = pCmd->GetColorRGB();
            break;
        }
        case CHANGE_PARTICLES_EXTERNAL_FORCE:
        {
            if (pFacade_->SetParticlesExternForce(id, pCmd->GetVec3()))
                model_.externForce = pCmd->GetVec3();
            break;
        }
        case CHANGE_PARTICLES_SPAWN_NUM_PER_SECOND:
        {
            const int numParticlesPerSec = (int)pCmd->GetFloat();

            if (pFacade_->SetParticlesSpawnRate(id, numParticlesPerSec))
                model_.spawnNumPerSec = numParticlesPerSec;
            break;
        }
        case CHANGE_PARTICLES_LIFESPAN_MS:
        {
            const int lifespanMs = (int)pCmd->GetFloat();

            if (pFacade_->SetParticlesLifespanMs(id, lifespanMs))
                model_.lifespanMs = lifespanMs;
            break;
        }
        case CHANGE_PARTICLES_MASS:
        {
            if (pFacade_->SetParticlesMass(id, pCmd->GetFloat()))
                model_.mass = pCmd->GetFloat();
            break;
        }
        case CHANGE_PARTICLES_SIZE:
        {
            if (pFacade_->SetParticlesSize(id, pCmd->GetFloat()))
                model_.size = pCmd->GetFloat();
            break;
        }
        case CHANGE_PARTICLES_FRICTION:   // air resistance
        {
            if (pFacade_->SetParticlesFriction(id, pCmd->GetFloat()))
                model_.friction = pCmd->GetFloat();
        }
        {
            break;
        }
    }
}

//---------------------------------------------------------
// Desc:   undo the input command to change back particles properties of entity by ID
// Args:   - pCmd:   a command to be executed
//         - id:     entity identifier
//---------------------------------------------------------
void EnttParticlesController::UndoCmd(const ICommand* pCmd, const EntityID id)
{

}

} // namespace
