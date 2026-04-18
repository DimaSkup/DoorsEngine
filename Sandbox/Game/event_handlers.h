// 12.04.2026 by DimaSkup
#pragma once
#include <Engine/engine.h>
#include <Render/debug_draw_manager.h>
#include "event_mgr.h"

namespace Game
{

// helpers
void PlayerPlayFootstepSound    (ECS::PlayerData& player, const float dt);

// common
void HandleRadiationZone        (Core::Engine* pEngine, const EventData* pData);

// player actions
void PlayerMove                 (Core::Engine* pEngine, const EventData* pData);

void PlayerSwitchWeapon         (Core::Engine* pEngine, const EventData* pData);
void PlayerReloadWeapon         (Core::Engine* pEngine, const EventData* pData);

void PlayerPlayAnimWeaponDraw   (Core::Engine* pEngine, const EventData* pData);
void PlayerPlayAnimWeaponReload (Core::Engine* pEngine, const EventData* pData);
void PlayerPlayAnimWeaponShot   (Core::Engine* pEngine, const EventData* pData);
void PlayerPlayAnimWeaponRun    (Core::Engine* pEngine, const EventData* pData);
void PlayerPlayAnimWeaponIdle   (Core::Engine* pEngine, const EventData* pData);
void PlayerToggleFlashLight     (Core::Engine* pEngine, const EventData* pData);

void PlayerPlayShotSound        (Core::Engine* pEngine, const EventData* pData);


// collision stuff
void PlayerShot                 (Core::Engine* pEngine, const EventData* pData);
void PlayerMultipleShots        (Core::Engine* pEngine, const EventData* pData);

// scene events
void HandleFireAnomaly          (Core::Engine* pEngine, const EventData* pData);
void HandleHouseRadiation       (Core::Engine* pEngine, const EventData* pData);

} // namespace
