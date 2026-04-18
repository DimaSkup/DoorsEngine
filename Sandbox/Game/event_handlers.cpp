// 12.04.2026 by DimaSkup
#include "../Common/pch.h"
#include "event_handlers.h"
#include <Model/animation_mgr.h>
#include <Sound/sound_mgr.h>
#include <Sound/sound.h>

namespace Game
{

//---------------------------------------------------------
// little converters
//---------------------------------------------------------
inline Vec3 ToVec3(const DirectX::XMVECTOR& vec)
{
    DirectX::XMFLOAT3 v;
    DirectX::XMStoreFloat3(&v, vec);
    return Vec3(v.x, v.y, v.z);
}

//---------------------------------------------------------
// Desc:  start playing footsteps sound
//        (call it when player's position is changed)
//---------------------------------------------------------
void PlayerPlayFootstepSound(ECS::PlayerData& player, const float dt)
{
    if (!player.soundStepL_Playing)
    {
        player.stepTimer += dt;

        // if we can't change footstep sound (from left to right and vise versa)
        if (player.stepTimer <= player.currStepInterval)
            return;

        Core::g_SoundMgr.GetSound(player.soundStepL)->PlayTrack();

        player.soundStepL_Playing = true;
        player.soundStepR_Played = false;
        player.stepTimer = 0;
    }
}

//---------------------------------------------------------
// handle player's movement
//---------------------------------------------------------
void PlayerMove(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData);
    assert(pData->deltaTime > 0.0f);

    ECS::EntityMgr*    pEnttMgr         = pEngine->GetECS();
    ECS::PlayerSystem& player           = pEnttMgr->playerSys_;
    ECS::eEventType    playerEventType  = (ECS::eEventType)pData->ix;

    switch (playerEventType)
    {
        case ECS::EVENT_PLAYER_JUMP:
        {
            player.Move(ECS::ePlayerState::JUMP);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_FORWARD:
        {
            player.Move(ECS::ePlayerState::MOVE_FORWARD);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_BACKWARD:
        {
            player.Move(ECS::ePlayerState::MOVE_BACK);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_RIGHT:
        {
            player.Move(ECS::ePlayerState::MOVE_RIGHT);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_LEFT:
        {
            player.Move(ECS::ePlayerState::MOVE_LEFT);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_UP:
        {
            player.Move(ECS::ePlayerState::MOVE_UP);
            break;
        }
        case ECS::EVENT_PLAYER_MOVE_DOWN:
        {
            player.Move(ECS::ePlayerState::MOVE_DOWN);
            break;
        }
        default:
        {
            LogErr(LOG, "invalid direction for the player movement: %d", (int)playerEventType);
            return;
        }
    }

    PlayerPlayFootstepSound(pEnttMgr->playerSys_.GetData(), pData->deltaTime);
}


//**********************************************************************************
//                        PLAYER ANIMATIONS SWITCHERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  play weapon "appearing" animation
//---------------------------------------------------------
void PlayerPlayAnimWeaponDraw(Core::Engine* pEngine)
{
    assert(pEngine);
    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    ECS::PlayerSystem&  player      = pEnttMgr->playerSys_;
    const ECS::Weapon&  wpn         = player.GetActiveWeapon();
    const AnimationID   animId      = wpn.animIds[ECS::WPN_ANIM_TYPE_DRAW];

    Core::AnimSkeleton& skeleton    = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const float         animEndTime = skeleton.GetAnimation(animId).GetEndTime();

    player.SetCurrActTime(0);
    player.SetEndActTime(animEndTime);
    player.SetCurrAnimId(animId);

    pEnttMgr->animationSys_.SetAnimation(
        wpn.enttId,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);

    pEnttMgr->animationSys_.RestartAnimation(wpn.enttId);
}

//---------------------------------------------------------
// switch to "reloading" animation
//---------------------------------------------------------
void PlayerPlayAnimWeaponReload(Core::Engine* pEngine)
{
    assert(pEngine);
    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    ECS::PlayerSystem&  player      = pEnttMgr->playerSys_;
    const ECS::Weapon&  wpn         = player.GetActiveWeapon();
    const AnimationID   animId      = wpn.animIds[ECS::WPN_ANIM_TYPE_RELOAD];

    Core::AnimSkeleton& skeleton    = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const float         animEndTime = skeleton.GetAnimation(animId).GetEndTime();

    player.SetCurrActTime(0);
    player.SetEndActTime(animEndTime);
    player.SetCurrAnimId(animId);

    pEnttMgr->animationSys_.SetAnimation(
        wpn.enttId,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);
}

//---------------------------------------------------------
// switch to "shooting" animation
//---------------------------------------------------------
void PlayerPlayAnimWeaponShot(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData == nullptr);

    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    ECS::PlayerSystem&  player      = pEnttMgr->playerSys_;
    const ECS::Weapon&  wpn         = player.GetActiveWeapon();
    const AnimationID   animId      = wpn.animIds[ECS::WPN_ANIM_TYPE_SHOT];

    Core::AnimSkeleton& skeleton    = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const float         animEndTime = skeleton.GetAnimation(animId).GetEndTime();

    player.SetCurrActTime(0);
    player.SetEndActTime(animEndTime);
    player.SetCurrAnimId(animId);

    pEnttMgr->animationSys_.SetAnimation(
        wpn.enttId,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);

    pEnttMgr->animationSys_.RestartAnimation(wpn.enttId);
}

//---------------------------------------------------------
// switch to "run/sprint" animation
//---------------------------------------------------------
void PlayerPlayAnimWeaponRun(Core::Engine* pEngine)
{
    assert(pEngine);
    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    ECS::PlayerSystem&  player      = pEnttMgr->playerSys_;
    const ECS::Weapon&  wpn         = player.GetActiveWeapon();
    const AnimationID   animId      = wpn.animIds[ECS::WPN_ANIM_TYPE_RUN];

    Core::AnimSkeleton& skeleton    = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const float         animEndTime = skeleton.GetAnimation(animId).GetEndTime();

    player.SetCurrActTime(0);
    player.SetEndActTime(animEndTime);
    player.SetCurrAnimId(animId);

    pEnttMgr->animationSys_.SetAnimation(
        wpn.enttId,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);
}

//---------------------------------------------------------
// switch to "idle" animation
//---------------------------------------------------------
void PlayerPlayAnimWeaponIdle(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData == nullptr);

    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    ECS::PlayerSystem&  player      = pEnttMgr->playerSys_;
    const ECS::Weapon&  wpn         = player.GetActiveWeapon();
    const AnimationID   animId      = wpn.animIds[ECS::WPN_ANIM_TYPE_IDLE];

    Core::AnimSkeleton& skeleton    = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const float         animEndTime = skeleton.GetAnimation(animId).GetEndTime();

    player.SetCurrActTime(0);
    player.SetEndActTime(animEndTime);
    player.SetCurrAnimId(animId);

    pEnttMgr->animationSys_.SetAnimation(
        wpn.enttId,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_LOOP);
}

//---------------------------------------------------------
// Desc:   switch on/off the player's flashlight
//---------------------------------------------------------
void PlayerToggleFlashLight(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData == nullptr);

    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();
    ECS::PlayerSystem& player = pEnttMgr->playerSys_;
    const bool isActive = !player.IsFlashLightActive();;
    player.SwitchFlashLight(isActive);

    // update the state of the flashlight entity
    const EntityID flashlightId = pEnttMgr->nameSys_.GetIdByName("player_flashlight");

    pEnttMgr->lightSys_.SetLightIsActive(flashlightId, isActive);
    pEngine->GetRender()->SwitchFlashLight(isActive);

    // if we just turned on the flashlight we update its position and direction
    if (isActive)
    {
        pEnttMgr->transformSys_.SetPosition(flashlightId, player.GetPosition());
        pEnttMgr->transformSys_.SetDirection(flashlightId, player.GetDirVec());
    }
}

//---------------------------------------------------------
// Desc:  start playing sound of shot by the player
//---------------------------------------------------------
void PlayerPlayShotSound(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData == nullptr);

    ECS::PlayerSystem&   player = pEngine->GetECS()->playerSys_;
    const ECS::Weapon&      wpn = player.GetActiveWeapon();
    //const bool bCanRestartSound = (wpn.type == ECS::WPN_TYPE_PISTOL);

    if (!player.IsSoundShotPlaying())
   // if (!player.IsSoundShotPlaying() || bCanRestartSound)
    {
        const SoundID shotSoundId = wpn.soundIds[ECS::WPN_SOUND_TYPE_SHOT];
        Core::g_SoundMgr.GetSound(shotSoundId)->PlayTrack();
        player.SetSoundShotPlaying(true);
    }
}

//---------------------------------------------------------
// Desc:  player decided to switch its weapon so handle this event:
//        1. bind another weapon as a current
//        2. play weapon's "switching/drawing" sound
//        3. play weapon's "switching/drawing" animation
//---------------------------------------------------------
void PlayerSwitchWeapon(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData);

    ECS::EntityMgr*   pEnttMgr = pEngine->GetECS();
    ECS::PlayerSystem&  player = pEnttMgr->playerSys_;
    ECS::WeaponSystem& weapons = pEnttMgr->weaponSys_;

    // stop all the sounds for the previous weapon
    const ECS::Weapon& prevWpn = player.GetActiveWeapon();

    for (int i = 0; i < (int)ECS::NUM_WPN_SOUND_TYPES; ++i)
    {
        const SoundID id = prevWpn.soundIds[i];
        Core::g_SoundMgr.GetSound(id)->StopTrack();
    }
    
    // set another weapon
    const int   weaponSlot = pData->ix;
    const EntityID   wpnId = player.GetWeapon(weaponSlot);
    const ECS::Weapon& wpn = weapons.GetWeaponById(wpnId);

    player.SetActiveWeaponId(wpn.enttId);
    player.SetIsDrawWeapon(true);

    // play sound and switch animation
    const SoundID drawSoundId = wpn.soundIds[ECS::WPN_SOUND_TYPE_DRAW];
    Core::g_SoundMgr.GetSound(drawSoundId)->PlayTrack();
    PlayerPlayAnimWeaponDraw(pEngine);
}

//---------------------------------------------------------
// Desc:  reload our current weapon
//---------------------------------------------------------
void PlayerReloadWeapon(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData == nullptr);

    ECS::EntityMgr*       pEnttMgr = pEngine->GetECS();
    ECS::PlayerSystem&      player = pEnttMgr->playerSys_;
    const ECS::Weapon&         wpn = player.GetActiveWeapon();

    const AnimationID animIdReload = wpn.animIds[ECS::WPN_ANIM_TYPE_RELOAD];
    const AnimationID animIdDraw   = wpn.animIds[ECS::WPN_ANIM_TYPE_DRAW];

    // we can't reload if already reloading or weapon is drawing (appearing)
    if (player.GetCurrAnimId() == animIdReload ||
        player.GetCurrAnimId() == animIdDraw)
        return;

    player.SetIsReloading(true);

    const SoundID reloadSoundId = wpn.soundIds[ECS::WPN_SOUND_TYPE_RELOAD];
    Core::g_SoundMgr.GetSound(reloadSoundId)->PlayTrack();
    PlayerPlayAnimWeaponReload(pEngine);
}


//**********************************************************************************
//                             COLLISION STUFF
//**********************************************************************************

// forward declaration of collision helpers 
void HandleBulletHit(ECS::EntityMgr* pEnttMgr, const IntersectionData& data);

DirectX::XMVECTOR PixelCoordToRayDir(
    const int sx,
    const int sy,
    const int wndWidth,
    const int wndHeight,
    const EntityID currCameraId,
    ECS::EntityMgr& enttMgr);

void EmitBulletHitParticles(ECS::EntityMgr* pEnttMgr, const IntersectionData& data);
void CreateBulletHitDecal(const IntersectionData& data);

//---------------------------------------------------------
// Desc:  execute a single shot by the player and handle collisions (if we have any)
//---------------------------------------------------------
void PlayerShot(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);

    const Core::CGraphics& graphics = pEngine->GetGraphics();
    ECS::EntityMgr*        pEnttMgr = pEngine->GetECS();
    Render::CRender*       pRender  = pEngine->GetRender();

    const EntityID    currCameraId = graphics.GetActiveCamera();
    const DirectX::XMFLOAT3 camPos = pEnttMgr->cameraSys_.GetPos(currCameraId);

    const ECS::PlayerSystem& player = pEnttMgr->playerSys_;

    const ECS::Weapon& wpn              = player.GetActiveWeapon();
    const AnimationID  animIdReload     = wpn.animIds[ECS::WPN_ANIM_TYPE_RELOAD];
    const AnimationID  animIdDraw       = wpn.animIds[ECS::WPN_ANIM_TYPE_DRAW];

    // aren't able to shoot while reloading or drawing (appearing) weapon
    if (player.GetCurrAnimId() == animIdReload ||
        player.GetCurrAnimId() == animIdDraw)
        return;

    // restart shooting sound, animation, etc.
    PlayerPlayAnimWeaponShot(pEngine, pData);
    PlayerPlayShotSound(pEngine, pData);

    DirectX::XMVECTOR rayOrigW = { camPos.x, camPos.y, camPos.z, 1 };
    DirectX::XMVECTOR rayDirW;
    Vec3              rayOrig = ToVec3(rayOrigW);

    const Core::Terrain& terrain = Core::g_ModelMgr.GetTerrain();
    const int mouseX = pEngine->GetMouse().GetPosX();
    const int mouseY = pEngine->GetMouse().GetPosY();
    int numBullets = 1;
    bool bShotgun = false;
    bool bIntersectTrn = false;
    bool bIntersectEntt = false;

    // for shotgun we have multiple bullets, so execute multiple tests
    if (wpn.type == ECS::WPN_TYPE_SHOTGUN)
    {
        numBullets = 10;
        bShotgun = true;
    }

    for (int i = 0; i < numBullets; ++i)
    {
        IntersectionData intersectDataEntt;
        IntersectionData intersectDataTrn;

        memset(&intersectDataEntt, 0, sizeof(intersectDataEntt));
        memset(&intersectDataTrn,  0, sizeof(intersectDataTrn));

        int offsetX = 0;
        int offsetY = 0;

        // if shotgun: generate random angle for each bullet
        if (bShotgun)
        {
            const int rnd1 = rand() & 63;
            const int rnd2 = rand() & 63;
            offsetX = rnd1 - 32;
            offsetY = rnd2 - 32;
        }

        // calc a direction vector by pixel coords
        rayDirW = PixelCoordToRayDir(
            mouseX + offsetX,
            mouseY + offsetY,
            pRender->GetD3D().GetWindowWidth(),
            pRender->GetD3D().GetWindowHeight(),
            currCameraId,
            *pEnttMgr);

        // test intersection with terrain and entities
        bIntersectTrn  = terrain.TestRayIntersection(rayOrig, ToVec3(rayDirW), intersectDataTrn);
        bIntersectEntt = graphics.TestRayIntersectEntts(rayOrigW, rayDirW, intersectDataEntt);
        
        if (bIntersectEntt && bIntersectTrn)
        {
            // find closest intersection point
            if (intersectDataTrn.distToIntersect < intersectDataEntt.distToIntersect)
                HandleBulletHit(pEnttMgr, intersectDataTrn);
            else
                HandleBulletHit(pEnttMgr, intersectDataEntt);

            continue;
        }

        if (bIntersectEntt)
            HandleBulletHit(pEnttMgr, intersectDataEntt);

        if (bIntersectTrn)
            HandleBulletHit(pEnttMgr, intersectDataTrn);
    }
}

//---------------------------------------------------------
// Desc:  handle the case when LMB is down for several frames
//        so we do multiple shots (if our weapon is able to do so)
//---------------------------------------------------------
void PlayerMultipleShots(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    using namespace DirectX;

    const Core::CGraphics& graphics = pEngine->GetGraphics();
    ECS::EntityMgr*        pEnttMgr = pEngine->GetECS();
    Render::CRender*        pRender = pEngine->GetRender();

    const ECS::PlayerSystem& player = pEnttMgr->playerSys_;
    const ECS::Weapon&          wpn = player.GetActiveWeapon();

    // if current weapon can't do multiple shots...
    if (wpn.type == ECS::WPN_TYPE_PISTOL || wpn.type == ECS::WPN_TYPE_SHOTGUN)
        return;

    // we aren't able to shoot while reloading or drawing (appearing) weapon
    const AnimationID animIdReload = wpn.animIds[ECS::WPN_ANIM_TYPE_RELOAD];
    const AnimationID animIdDraw = wpn.animIds[ECS::WPN_ANIM_TYPE_DRAW];

    if (player.GetCurrAnimId() == animIdReload ||
        player.GetCurrAnimId() == animIdDraw)
        return;

    // if not each time spent since the previous shot...
    if (player.GetCurrActTime() < wpn.shotInterval)
        return;

    // restart shooting sound, animation, etc.
    PlayerPlayAnimWeaponShot(pEngine, pData);
    PlayerPlayShotSound(pEngine, pData);


    const EntityID currCamId = graphics.GetActiveCamera();
    const XMFLOAT3 camPos = pEnttMgr->cameraSys_.GetPos(currCamId);

    // ray origin + ray direction
    XMVECTOR rayOrigW = { camPos.x, camPos.y, camPos.z, 1 };
    XMVECTOR rayDirW;
    Vec3     rayOrig = ToVec3(rayOrigW);

    // calc a direction vector by pixel coords
    rayDirW = PixelCoordToRayDir(
        pEngine->GetMouse().GetPosX(),
        pEngine->GetMouse().GetPosY(),
        pRender->GetD3D().GetWindowWidth(),
        pRender->GetD3D().GetWindowHeight(),
        currCamId,
        *pEnttMgr);

    IntersectionData data;
    memset(&data, 0, sizeof(data));

    const Core::Terrain& terrain = Core::g_ModelMgr.GetTerrain();

    IntersectionData intersectDataEntt;
    IntersectionData intersectDataTrn;

    bool bIntersectTrn = false;
    bool bIntersectEntt = false;

    memset(&intersectDataEntt, 0, sizeof(intersectDataEntt));
    memset(&intersectDataTrn, 0, sizeof(intersectDataTrn));


    // test intersection with terrain and entities
    bIntersectTrn = terrain.TestRayIntersection(rayOrig, ToVec3(rayDirW), intersectDataTrn);
    bIntersectEntt = graphics.TestRayIntersectEntts(rayOrigW, rayDirW, intersectDataEntt);

    if (bIntersectEntt && bIntersectTrn)
    {
        // find closest intersection point
        if (intersectDataTrn.distToIntersect < intersectDataEntt.distToIntersect)
            HandleBulletHit(pEnttMgr, intersectDataTrn);
        else
            HandleBulletHit(pEnttMgr, intersectDataEntt);

        return;
    }

    if (bIntersectEntt)
        HandleBulletHit(pEnttMgr, intersectDataEntt);

    if (bIntersectTrn)
        HandleBulletHit(pEnttMgr, intersectDataTrn);
}

//---------------------------------------------------------
// handle intersection of bullet with some stuff on the scene
//---------------------------------------------------------
void HandleBulletHit(ECS::EntityMgr* pEnttMgr, const IntersectionData& data)
{
    assert(pEnttMgr);

    // add a line (bullet trace) for debug rendering
    if (Core::g_DebugDrawMgr.IsRenderable())
    {
        const EntityID           wpnId = pEnttMgr->playerSys_.GetActiveWeaponId();
        const DirectX::XMFLOAT3 relPos = pEnttMgr->hierarchySys_.GetRelativePos(wpnId);

        const Vec3 rayOrig   = { data.rayOrigX, data.rayOrigY, data.rayOrigZ };
        const Vec3 fromPos   = { rayOrig.x + relPos.x, rayOrig.y + relPos.y, rayOrig.z + relPos.z };
        const Vec3 intersect = { data.px, data.py, data.pz };
        const Vec3 magenta   = { 1, 0, 1 };

        Core::g_DebugDrawMgr.AddLine(fromPos, intersect, magenta);
    }

    // generate some particles as a response of bullet hit
    EmitBulletHitParticles(pEnttMgr, data);

    // create decal from bullet hit
    CreateBulletHitDecal(data);
}

//---------------------------------------------------------
// 
//---------------------------------------------------------
DirectX::XMVECTOR PixelCoordToRayDir(
    const int sx,
    const int sy,
    const int wndWidth,
    const int wndHeight,
    const EntityID currCameraId,
    ECS::EntityMgr& enttMgr)
{
    using namespace DirectX;

    const XMMATRIX& proj    = enttMgr.cameraSys_.GetProj(currCameraId);
    const XMMATRIX& invView = enttMgr.cameraSys_.GetInverseView(currCameraId);

    const float xndc = (+2.0f * sx / wndWidth  - 1.0f);
    const float yndc = (-2.0f * sy / wndHeight + 1.0f);

    // compute picking ray in view space
    const float vx = xndc / proj.r[0].m128_f32[0];
    const float vy = yndc / proj.r[1].m128_f32[1];
    const XMVECTOR rayDirV = { vx, vy, 1, 0 };

    // normalized ray direction in world space
    return XMVector3Normalize(XMVector3TransformNormal(rayDirV, invView));     // supposed to take a vec (w == 0)
}

//---------------------------------------------------------
//---------------------------------------------------------
void EmitBulletHitParticles(ECS::EntityMgr* pEnttMgr, const IntersectionData& data)
{
    assert(pEnttMgr);

    ECS::NameSystem&      nameSys     = pEnttMgr->nameSys_;
    ECS::TransformSystem& transSys    = pEnttMgr->transformSys_;
    ECS::ParticleSystem&  particleSys = pEnttMgr->particleSys_;
    ECS::BoundingSystem&  boundSys    = pEnttMgr->boundingSys_;

    const EntityID emitShotSplash0Id = nameSys.GetIdByName("shot_splash_0");
    const EntityID emitShotSplash1Id = nameSys.GetIdByName("shot_splash_1");
    const EntityID emitShotSplash2Id = nameSys.GetIdByName("shot_splash_2");
    const EntityID emitShotSplash3Id = nameSys.GetIdByName("shot_splash_3");

    const EntityID emitSplashSmoke0Id = nameSys.GetIdByName("shot_splash_smoke_0");
    const EntityID emitSplashSmoke1Id = nameSys.GetIdByName("shot_splash_smoke_1");
    const EntityID emitSplashSmoke2Id = nameSys.GetIdByName("shot_splash_smoke_2");

    // relocate particle emitters
    const Vec3 intersectPoint = { data.px, data.py, data.pz };

    // each new shot translates emitters to its new position
    transSys.SetPosition(emitShotSplash0Id, intersectPoint);
    transSys.SetPosition(emitShotSplash1Id, intersectPoint);
    transSys.SetPosition(emitShotSplash2Id, intersectPoint);
    transSys.SetPosition(emitShotSplash3Id, intersectPoint);

    transSys.SetPosition(emitSplashSmoke0Id, intersectPoint);
    transSys.SetPosition(emitSplashSmoke1Id, intersectPoint);
    transSys.SetPosition(emitSplashSmoke2Id, intersectPoint);

    // translate AABB of each emitter to its new position
    boundSys.TranslateWorldBoundings(emitShotSplash0Id);
    boundSys.TranslateWorldBoundings(emitShotSplash1Id);
    boundSys.TranslateWorldBoundings(emitShotSplash2Id);
    boundSys.TranslateWorldBoundings(emitShotSplash3Id);

    boundSys.TranslateWorldBoundings(emitSplashSmoke0Id);
    boundSys.TranslateWorldBoundings(emitSplashSmoke1Id);
    boundSys.TranslateWorldBoundings(emitSplashSmoke2Id);

    // update emitters location within the quadtree
    pEnttMgr->UpdateQuadTreeMembership(emitShotSplash0Id);
    pEnttMgr->UpdateQuadTreeMembership(emitShotSplash1Id);
    pEnttMgr->UpdateQuadTreeMembership(emitShotSplash2Id);
    pEnttMgr->UpdateQuadTreeMembership(emitShotSplash3Id);

    pEnttMgr->UpdateQuadTreeMembership(emitSplashSmoke0Id);
    pEnttMgr->UpdateQuadTreeMembership(emitSplashSmoke1Id);
    pEnttMgr->UpdateQuadTreeMembership(emitSplashSmoke2Id);

    const ECS::EmitterData& emitter0 = particleSys.GetEmitterData(emitShotSplash0Id);
    const ECS::EmitterData& emitter1 = particleSys.GetEmitterData(emitShotSplash1Id);
    const ECS::EmitterData& emitter2 = particleSys.GetEmitterData(emitShotSplash2Id);
    const ECS::EmitterData& emitter3 = particleSys.GetEmitterData(emitShotSplash3Id);

    const ECS::EmitterData& emitterSmoke0 = particleSys.GetEmitterData(emitSplashSmoke0Id);
    const ECS::EmitterData& emitterSmoke1 = particleSys.GetEmitterData(emitSplashSmoke1Id);
    const ECS::EmitterData& emitterSmoke2 = particleSys.GetEmitterData(emitSplashSmoke2Id);

    particleSys.PushNewParticles(emitShotSplash0Id, emitter0.spawnRate);
    particleSys.PushNewParticles(emitShotSplash1Id, emitter1.spawnRate);
    particleSys.PushNewParticles(emitShotSplash2Id, emitter2.spawnRate);
    particleSys.PushNewParticles(emitShotSplash3Id, emitter3.spawnRate);

    particleSys.PushNewParticles(emitSplashSmoke0Id, emitterSmoke0.spawnRate);
    particleSys.PushNewParticles(emitSplashSmoke1Id, emitterSmoke1.spawnRate);
    particleSys.PushNewParticles(emitSplashSmoke2Id, emitterSmoke2.spawnRate);

    // particles will go along the normal vector of the surface (where bullet hit)
    const Vec3 forceDir = { data.nx, data.ny, data.nz };
    const Vec3 extForce = (forceDir * 0.00005f);
    const Vec3 force0 = { extForce.x * 1.1f, extForce.y,        extForce.z };
    const Vec3 force1 = { extForce.x,        extForce.y * 0.7f, extForce.z };
    const Vec3 force2 = { extForce.x,        extForce.y * 0.9f, extForce.z * 1.1f };

    particleSys.SetExternForces(emitSplashSmoke0Id, force0.x, force0.y, force0.z);
    particleSys.SetExternForces(emitSplashSmoke1Id, force1.x, force1.y, force1.z);
    particleSys.SetExternForces(emitSplashSmoke2Id, force2.x, force2.y, force2.z);
}

//---------------------------------------------------------
//---------------------------------------------------------
void CreateBulletHitDecal(const IntersectionData& data)
{
    const Vec3 intersectPoint = { data.px, data.py, data.pz };
    const Vec3 v0             = { data.vx1, data.vy1, data.vz1 };
    const Vec3 decalTangent   = v0 - intersectPoint;
    const Vec3 decalNormal    = { data.nx, data.ny, data.nz };

    const float decalWidth       = 0.1f;
    const float decalHeight      = 0.1f;
    const float decalLifeTimeSec = 30.0f;

    Core::g_ModelMgr.AddDecal3D(
        intersectPoint,          // center of decal
        decalTangent,
        decalNormal,
        decalWidth,
        decalHeight,
        decalLifeTimeSec);
}


//---------------------------------------------------------
// if player near fire anomaly
//---------------------------------------------------------
void HandleFireAnomaly(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData);

    ECS::EntityMgr*  pEnttMgr = pEngine->GetECS();
    Core::CGraphics& graphics = pEngine->GetGraphics();

    const EntityID           anomalyId = pEnttMgr->nameSys_.GetIdByName("anomaly_flame_0");
    const DirectX::XMFLOAT3 anomalyPos = pEnttMgr->transformSys_.GetPosition(anomalyId);
    const DirectX::XMFLOAT3  playerPos = { pData->fx, pData->fy, pData->fz };

    const float anomDistSqr = SQR(anomalyPos.x-playerPos.x) + SQR(anomalyPos.y-playerPos.y) + SQR(anomalyPos.z-playerPos.z);
    const float anomRange = 10.0f;

    if (anomDistSqr < SQR(anomRange))
    {
        const float anomDist = sqrtf(anomDistSqr);
        const float colorVal = anomDist / anomRange;

        // closer to anomaly - more red the screen becomes
        graphics.EnablePostFxs(true);

        if (!graphics.IsPostFxEnabled(POST_FX_COLOR_TINT))
            graphics.PushPostFx(POST_FX_COLOR_TINT);

        pEngine->GetRender()->SetPostFxParam(POST_FX_PARAM_COLOR_TINT_G, colorVal);
        pEngine->GetRender()->SetPostFxParam(POST_FX_PARAM_COLOR_TINT_B, colorVal);
    }
    else
    {
        graphics.RemovePostFx(POST_FX_COLOR_TINT);
        if (graphics.GetNumUsedPostFxs() == 0)
            graphics.EnablePostFxs(false);
    }
}

//---------------------------------------------------------
// if player inside radioactive house
//---------------------------------------------------------
void HandleHouseRadiation(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData);

    Core::CGraphics& graphics = pEngine->GetGraphics();
    ECS::EntityMgr* pEnttMgr = pEngine->GetECS();

    const EntityID               houseId = pEnttMgr->nameSys_.GetIdByName("stalker_house");
    const DirectX::BoundingBox& houseBox = pEnttMgr->boundingSys_.GetWorldBoundBox(houseId);

    const Vec3 playerPos = { pData->fx, pData->fy, pData->fz };
    const bool bInHouse  = houseBox.Contains({ playerPos.x, playerPos.y, playerPos.z });
    static bool bWasInHouse = false;

    // if player came into house
    if (!bWasInHouse && bInHouse)
    {
        graphics.EnablePostFxs(true);
        graphics.PushPostFx(POST_FX_FILM_GRAIN);
        graphics.PushPostFx(POST_FX_GRAYSCALE);
        bWasInHouse = true;
    }

    // if player left house
    else if (bWasInHouse && !bInHouse)
    {
        graphics.RemovePostFx(POST_FX_FILM_GRAIN);
        graphics.RemovePostFx(POST_FX_GRAYSCALE);

        if (graphics.GetNumUsedPostFxs() == 0)
            graphics.EnablePostFxs(false);

        bWasInHouse = false;
    }
}


//---------------------------------------------------------
//---------------------------------------------------------
void HandleRadiationZone(Core::Engine* pEngine, const EventData* pData)
{
    assert(pEngine);
    assert(pData);

    Core::CGraphics& graphics = pEngine->GetGraphics();

    graphics.EnablePostFxs(true);
    graphics.PushPostFx(POST_FX_FILM_GRAIN);
    graphics.PushPostFx(POST_FX_GRAYSCALE);
}

} // namespace
