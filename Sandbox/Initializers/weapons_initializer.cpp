#include "../Common/pch.h"
#include "weapons_initializer.h"
#include "../Types/weapon_types.h"
#include <Model/animation_mgr.h>
#include <Sound/sound_mgr.h>
#include <Sound/sound.h>


namespace Game
{

//---------------------------------------------------------
// forward declaration of private helpers
//---------------------------------------------------------
void InitWeapon(FILE* pFile, const char* line, Weapon& wpn, ECS::EntityMgr* pEnttMgr);
void SetWeaponType(const char* buf, Weapon& wpn);
void SetWeaponEntity(const char* buf, Weapon& wpn, ECS::EntityMgr* pEnttMgr);
void BindWeaponSkeleton(const char* buf, Weapon& wpn);
void BindWeaponSound(const char* buf, Weapon& wpn);
void BindWeaponAnimation(const char* buf, Weapon& wpn);

//---------------------------------------------------------
//---------------------------------------------------------
void WeaponsInitializer::Init(
    const char* weaponsCfgFile,
    ECS::EntityMgr* pEnttMgr,
    cvector<Weapon>& playerWeapons)
{
    if (StrHelper::IsEmpty(weaponsCfgFile))
    {
        LogErr(LOG, "input filepath");
        return;
    }

    char buf[256];
    int weaponIdx = 0;


    FILE* pFile = fopen(weaponsCfgFile, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", weaponsCfgFile);
        exit(0);
    }

    playerWeapons.reserve(9);

    while (!feof(pFile))
    {
        // read whole line
        fgets(buf, sizeof(buf), pFile);


        if (buf[0] == ';')    // skip comment line
        {
            continue;
        }
        else if (strncmp(buf, "wpn", 3) == 0)
        {
            playerWeapons.push_back(Weapon());
            InitWeapon(pFile, buf, playerWeapons[weaponIdx], pEnttMgr);
            weaponIdx++;
        }
    }

    //weapons_.shrink_to_fit();
    fclose(pFile);


    //
    // create event handlers for some sounds so we will be able to know when
    // sound is over, or it is currently playing
    //
    Weapon& currWeapon = playerWeapons[0];
    Core::Sound* pSound = Core::g_SoundMgr.GetSound(currWeapon.soundIds[WPN_SOUND_TYPE_SHOOT]);
    
}

//---------------------------------------------------------
//---------------------------------------------------------
void Game::InitWeapon(FILE* pFile, const char* line, Weapon& wpn, ECS::EntityMgr* pEnttMgr)
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
    while (buf[0] != '}')
    {
        fgets(buf, sizeof(buf), pFile);

        // +1 to skip '\t'
        const char* buffer = buf + 1; 

        if (strncmp(buffer, "type", 4) == 0)
        {
            // define a type of the weapon
            SetWeaponType(buffer, wpn);
        }

        else if (strncmp(buffer, "entity", 6) == 0)
        {
            // define which entity will serve us as a weapon
            SetWeaponEntity(buffer, wpn, pEnttMgr);
        }

        else if (strncmp(buffer, "skeleton", 8) == 0)
        {
            // define which animation skeleton we will use
            BindWeaponSkeleton(buffer, wpn);
        }

        else if (strncmp(buffer, "sound", 5) == 0)
        {
            BindWeaponSound(buffer, wpn);
        }

        else if (strncmp(buffer, "anim", 4) == 0)
        {
            BindWeaponAnimation(buffer, wpn);
        }
    }
}

//---------------------------------------------------------

void SetWeaponType(const char* buf, Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    char typeName[64];
    int count = sscanf(buf, "type %s", typeName);

    if (count != 1)
    {
        LogErr(LOG, "can't define a type of weapon: %s", buf);
        wpn.type = WPN_TYPE_PISTOL;   // set default
        return;
    }


    if (strncmp(typeName, "pistol", 6) == 0)
        wpn.type = WPN_TYPE_PISTOL;

    else if (strncmp(typeName, "machine_gun", 11) == 0)
        wpn.type = WPN_TYPE_MACHINE_GUN;

    else if (strncmp(typeName, "shotgun", 7) == 0)
        wpn.type = WPN_TYPE_SHOTGUN;

    else
    {
        LogErr(LOG, "uknown weapon type: %s", typeName);
        wpn.type = WPN_TYPE_PISTOL;   // set default
    }
}

//---------------------------------------------------------

void SetWeaponEntity(const char* buf, Weapon& wpn, ECS::EntityMgr* pEnttMgr)
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

    wpn.enttId = pEnttMgr->nameSystem_.GetIdByName(enttName);
}


//---------------------------------------------------------

void BindWeaponSkeleton(const char* buf, Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    char skeletonName[MAX_LEN_SKELETON_NAME];

    int count = sscanf(buf, "skeleton %s", skeletonName);
    assert(count == 1);

    SkeletonID skeletonId = Core::g_AnimationMgr.GetSkeletonId(skeletonName);
    assert(skeletonId != 0);

    wpn.pSkeleton = &Core::g_AnimationMgr.GetSkeleton(skeletonId);
}


//---------------------------------------------------------

void BindWeaponSound(const char* buf, Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    eWeaponSoundType soundType = eWeaponSoundType(0);
    char soundName[MAX_LEN_SOUND_NAME];
    char soundTypename[32];
    int  count = 0;

    count = sscanf(buf, "%s %s", soundTypename, soundName);
    assert(count == 2);


    if (strcmp(soundTypename, "sound_draw") == 0)
        soundType = WPN_SOUND_TYPE_DRAW;

    else if (strcmp(soundTypename, "sound_reload") == 0)
        soundType = WPN_SOUND_TYPE_RELOAD;

    else if (strcmp(soundTypename, "sound_shoot") == 0)
        soundType = WPN_SOUND_TYPE_SHOOT;

    else
    {
        LogErr(LOG, "can't define a type of weapon's sound: %s", buf);
        return;
    }

    wpn.soundIds[soundType] = Core::g_SoundMgr.GetSoundIdByName(soundName);
}

//---------------------------------------------------------

void BindWeaponAnimation(const char* buf, Weapon& wpn)
{
    assert(buf && buf[0] != '\0');

    eWeaponAnimType animType = eWeaponAnimType(0);
    char animTypename[32]{ '\0' };
    char animName[MAX_LEN_ANIMATION_NAME]{ '\0' };
    int  count = 0;

    count = sscanf(buf, "%s %s", animTypename, animName);
    assert(count == 2);


    if (strcmp(animTypename, "anim_draw") == 0)
        animType = WPN_ANIM_TYPE_DRAW;

    else if (strcmp(animTypename, "anim_reload") == 0)
        animType = WPN_ANIM_TYPE_RELOAD;

    else if (strcmp(animTypename, "anim_shoot") == 0)
        animType = WPN_ANIM_TYPE_SHOOT;

    else if (strcmp(animTypename, "anim_run") == 0)
        animType = WPN_ANIM_TYPE_RUN;

    else if (strcmp(animTypename, "anim_idle") == 0)
        animType = WPN_ANIM_TYPE_IDLE;

    else
    {
        LogErr(LOG, "unknown type of weapon animation: %s", animTypename);
        return;
    }

    wpn.animIds[animType] = wpn.pSkeleton->GetAnimationIdx(animName);
}


} // namespace Game
