/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: weapons_initializer.cpp
    Desc:     implementation functional for weapons initialization
              and binding some to the player

    Created:  05.01.2025  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "weapons_initializer.h"
#include <Model/animation_mgr.h>
#include <Sound/sound_mgr.h>
#include <Sound/sound.h>
#include <Timers/game_timer.h>


namespace Game
{

//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void InitWeapon         (FILE* pFile, const char* line, ECS::Weapon& wpn, ECS::EntityMgr* pEnttMgr);
void SetWeaponType      (const char* buf, ECS::Weapon& wpn);
void SetWeaponEntity    (const char* buf, ECS::Weapon& wpn, ECS::EntityMgr* pEnttMgr);
void BindWeaponSkeleton (const char* buf, ECS::Weapon& wpn);
void BindWeaponSound    (const char* buf, ECS::Weapon& wpn);
void BindWeaponAnimation(const char* buf, ECS::Weapon& wpn);


//---------------------------------------------------------
// read in declarations of weapons from the file and create these weapons
//---------------------------------------------------------
void WeaponsInitializer::Init(const char* cfgFilepath, ECS::EntityMgr* pEnttMgr)
{
    assert(pEnttMgr);

    LogDbg(LOG, "init weapons");

    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogErr(LOG, "empty filepath");
        return;
    }

    const TimePoint start = GetTimePoint();
    char buf[256];
    int weaponIdx = 0;


    FILE* pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogFatal(LOG, "can't open file: %s", cfgFilepath);
    }

    while (!feof(pFile))
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);

        // skip comments and empty lines
        if (buf[0] == ';' || buf[0] == '\n')
            continue;

        if (strncmp(buf, "wpn", 3) == 0)
        {
            // load weapon's data from file
            ECS::Weapon wpn;
            InitWeapon(pFile, buf, wpn, pEnttMgr);

            if (wpn.enttId == INVALID_ENTT_ID)
                continue;

            // bind a Weapon component to entity
            pEnttMgr->AddWeaponComponent(wpn.enttId, wpn);
        }
    }

    fclose(pFile);

    const TimeDurationMs dur = GetTimePoint() - start;
    LogDbg(LOG, "weapons are initialized (took time: %.2 sec)", dur.count() / 1000.0f);
}

//---------------------------------------------------------
//---------------------------------------------------------
void Game::InitWeapon(
    FILE* pFile,
    const char* line,
    ECS::Weapon& wpn,
    ECS::EntityMgr* pEnttMgr)
{
    assert(pFile);
    assert(line && line[0] != '\0');
    assert(pEnttMgr);

    int count = 0;
    char buf[256]{'\0'};
    char weaponName[64];

    count = sscanf(line, "wpn \"%s", weaponName);
    assert(count == 1);

    // skip the last quote (") symbol in the weapon name
    weaponName[strlen(weaponName) - 1] = '\0';


    // while we read params for the current weapon
    while (fgets(buf, sizeof(buf), pFile))
    {
        // if we finished reading params
        if (buf[0] == '}')
            return;

        // +1 to skip '\t'
        const char* buffer = buf + 1;

        // read in a field key
        char key[32];
        count = sscanf(buf, "%s", key);
        assert(count == 1);


        if (strcmp(key, "type") == 0)
        {
            // define a type of the weapon
            SetWeaponType(buffer, wpn);
        }

        else if (strcmp(key, "entity") == 0)
        {
            // define which entity will serve us as a weapon
            SetWeaponEntity(buffer, wpn, pEnttMgr);
        }

        else if (strcmp(key, "skeleton") == 0)
        {
            // define which animation skeleton we will use
            BindWeaponSkeleton(buffer, wpn);
        }

        else if (strncmp(key, "sound", 5) == 0)
        {
            BindWeaponSound(buffer, wpn);
        }

        else if (strncmp(key, "anim", 4) == 0)
        {
            BindWeaponAnimation(buffer, wpn);
        }
    }
}

//---------------------------------------------------------
// define a kind of this weapon
//---------------------------------------------------------
void SetWeaponType(const char* buf, ECS::Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    char typeName[64];
    int count = sscanf(buf, "type %s", typeName);

    if (count != 1)
    {
        LogErr(LOG, "can't define a type of weapon: %s", buf);
        wpn.type = ECS::WPN_TYPE_PISTOL;   // set default
        return;
    }

    if (strncmp(typeName, "pistol", 6) == 0)
        wpn.type = ECS::WPN_TYPE_PISTOL;

    else if (strncmp(typeName, "machine_gun", 11) == 0)
        wpn.type = ECS::WPN_TYPE_MACHINE_GUN;

    else if (strncmp(typeName, "shotgun", 7) == 0)
        wpn.type = ECS::WPN_TYPE_SHOTGUN;

    else
    {
        LogErr(LOG, "uknown weapon type: %s", typeName);
        wpn.type = ECS::WPN_TYPE_PISTOL;   // set default
    }
}

//---------------------------------------------------------
// define which entity (and what geometry) we will use for this weapon 
//---------------------------------------------------------
void SetWeaponEntity(const char* buf, ECS::Weapon& wpn, ECS::EntityMgr* pEnttMgr)
{
    assert(buf && buf[0] != '\0');
    assert(pEnttMgr);

    char enttName[MAX_LEN_ENTT_NAME];
    int count = sscanf(buf, "entity %s", enttName);

    if (count != 1)
    {
        LogErr(LOG, "can't define an entity for weapon: %s", buf);
        wpn.enttId = 0;
        return;
    }

    wpn.enttId = pEnttMgr->nameSys_.GetIdByName(enttName);

    if (wpn.enttId == INVALID_ENTT_ID)
        LogErr(LOG, "no entity to set as a weapon: %s", enttName);
}

//---------------------------------------------------------
// define which skeleton to use to animate this weapon
//---------------------------------------------------------
void BindWeaponSkeleton(const char* buf, ECS::Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    char skeletonName[MAX_LEN_SKELETON_NAME];

    // read skeleton's name
    int count = sscanf(buf, "skeleton %s", skeletonName);
    assert(count == 1);

    // get skeleton's id
    wpn.skeletonId = Core::g_AnimationMgr.GetSkeletonId(skeletonName);
    assert(wpn.skeletonId != 0);
}

//---------------------------------------------------------
// bind a sound (its ID) to particular action with this weapon
//---------------------------------------------------------
void BindWeaponSound(const char* buf, ECS::Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    ECS::eWeaponSoundType soundType = ECS::eWeaponSoundType(0);
    char soundName[MAX_LEN_SOUND_NAME];
    char soundTypename[32];
    int  count = 0;

    // read sound type and sound name
    count = sscanf(buf, "%s %s", soundTypename, soundName);
    assert(count == 2);


    // define sound type
    if (strcmp(soundTypename, "sound_draw") == 0)
        soundType = ECS::WPN_SOUND_TYPE_DRAW;

    else if (strcmp(soundTypename, "sound_reload") == 0)
        soundType = ECS::WPN_SOUND_TYPE_RELOAD;

    else if (strcmp(soundTypename, "sound_shoot") == 0)
        soundType = ECS::WPN_SOUND_TYPE_SHOT;

    else
    {
        LogErr(LOG, "can't define a type of weapon's sound: %s", buf);
        return;
    }

    // bind id of the sound to our weapon
    wpn.soundIds[soundType] = Core::g_SoundMgr.GetSoundIdByName(soundName);
}

//---------------------------------------------------------
// bind an animation to particular action with the weapon
//---------------------------------------------------------
void BindWeaponAnimation(const char* buf, ECS::Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    ECS::eWeaponAnimType animType = ECS::eWeaponAnimType(0);
    char animTypename[32]{ '\0' };
    char animName[MAX_LEN_ANIMATION_NAME]{ '\0' };
    int  count = 0;

    // read animation type and name
    count = sscanf(buf, "%s %s", animTypename, animName);
    assert(count == 2);

    // define animation type
    if (strcmp(animTypename, "anim_draw") == 0)
        animType = ECS::WPN_ANIM_TYPE_DRAW;

    else if (strcmp(animTypename, "anim_reload") == 0)
        animType = ECS::WPN_ANIM_TYPE_RELOAD;

    else if (strcmp(animTypename, "anim_shoot") == 0)
        animType = ECS::WPN_ANIM_TYPE_SHOT;

    else if (strcmp(animTypename, "anim_run") == 0)
        animType = ECS::WPN_ANIM_TYPE_RUN;

    else if (strcmp(animTypename, "anim_idle") == 0)
        animType = ECS::WPN_ANIM_TYPE_IDLE;

    else
    {
        LogErr(LOG, "unknown type of weapon animation: %s", animTypename);
        return;
    }

    assert(wpn.skeletonId != 0);
    const Core::AnimSkeleton& skeleton = Core::g_AnimationMgr.GetSkeleton(wpn.skeletonId);
    const AnimationID           animId = skeleton.GetAnimationIdx(animName);

    // bind animation id to the weapon action
    wpn.animIds[animType] = animId;

    // setup a shot interval for this weapon
    if (animType == ECS::WPN_ANIM_TYPE_SHOT)
    {
        const Core::AnimationClip& anim = skeleton.GetAnimation(animId);
        constexpr float intervalScale = 1.0f / 2.5f;

        wpn.shotInterval = anim.GetEndTime() * intervalScale;
    }
}

} // namespace Game
