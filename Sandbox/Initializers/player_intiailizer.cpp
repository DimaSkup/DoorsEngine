/*********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: player_initializer.cpp
    Desc:     implementation of functional for the PlayerInitializer class

    Created:  03.04.2026  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "player_initializer.h"
#include <Model/model_creator.h>
#include <Model/model_mgr.h>
#include <Mesh/material_mgr.h>
#include <DirectXMath.h>
#pragma warning (disable : 4996)

namespace Game
{

using Core::ModelsCreator;
using Core::Model;
using Core::g_ModelMgr;
using Core::g_MaterialMgr;

using DirectX::XMFLOAT3;
using DirectX::XMVECTOR;


//---------------------------------------------------------
// private helpers
//---------------------------------------------------------
EntityID AddWeapon(const char* buf, const EntityID playerId, ECS::EntityMgr& enttMgr)
{
    assert(buf && buf[0] != '\0');

    char wpnEnttName[MAX_LEN_ENTT_NAME];
    char key[32];

    // get weapon name
    int count = sscanf(buf, "%s %s", key, wpnEnttName);
    assert(count == 2);

    // push weapon into inventory and bind to the player
    const EntityID wpnId = enttMgr.nameSys_.GetIdByName(wpnEnttName);
    if (wpnId == INVALID_ENTT_ID)
    {
        LogErr(LOG, "invalid weapon (no entt by name: %s)", wpnEnttName);
        return INVALID_ENTT_ID;
    }

    enttMgr.inventorySys_.AddItem(playerId, wpnId);
    enttMgr.hierarchySys_.AddChild(playerId, wpnId);

    return wpnId;
}

//---------------------------------------------------------
// Desc:  read in player's params from config file,
//        create and setup the player
//---------------------------------------------------------
bool PlayerInitializer::Init(const char* cfgFilepath, ECS::EntityMgr& enttMgr)
{
    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogErr(LOG, "empty config filepath");
        return false;
    }

    int count = 0;
    char buf[128];
    char key[32];
    DirectX::BoundingSphere playerBoundSphere;

    ECS::NameSystem&           nameSys = enttMgr.nameSys_;
    ECS::HierarchySystem& hierarchySys = enttMgr.hierarchySys_;
    ECS::InventorySystem& inventorySys = enttMgr.inventorySys_;

    FILE* pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return false;
    }

    //
    // create the player and do some setup
    //
    const EntityID playerId = enttMgr.CreateEntity("player");
    enttMgr.AddInventoryComponent(playerId);
    enttMgr.AddPlayerComponent(playerId);
    ECS::PlayerSystem& player = enttMgr.playerSys_;

    const XMFLOAT3 pos = { 0,0,0 };
    const XMVECTOR dir = { 0,0,1,0 };
    enttMgr.AddTransformComponent(playerId, pos, dir);

#if 0
    const MaterialID catMatID = g_MaterialMgr.GetMatIdByName("cat");
    enttMgr.AddMaterialComponent(playerId, catMatID);

    // setup player's model
    const MeshSphereParams sphereParams(1, 20, 20);
    ModelsCreator creator;
    const ModelID playerModelId = creator.CreateSphere(sphereParams);
    const Model&    playerModel = g_ModelMgr.GetModelById(playerModelId);
    enttMgr.AddModelComponent(playerId, playerModel.GetId());
#endif
   

    //
    // read player's params
    //
    while (fgets(buf, sizeof(buf), pFile))
    {
        // skip comment
        if (buf[0] == ';')
            continue;

        if (!isalpha(buf[0]))
            continue;


        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // define a type of player's property
        if (strcmp(key, "player_bound_sphere") == 0)
        {
            float params[4];   // pos (vec3), radius (float)
            ReadFloat4(buf, "player_bound_sphere %f %f %f %f", params);

            playerBoundSphere.Center = { params[0], params[1], params[2] };
            playerBoundSphere.Radius = params[3];
        }

        else if (strcmp(key, "weapon_0") == 0)
        {
            const EntityID wpnId = AddWeapon(buf, playerId, enttMgr);
            player.BindWeapon(0, wpnId);
        }

        else if (strcmp(key, "weapon_1") == 0)
        {
            const EntityID wpnId = AddWeapon(buf, playerId, enttMgr);
            player.BindWeapon(1, wpnId);
        }

        else if (strcmp(key, "weapon_2") == 0)
        {
            const EntityID wpnId = AddWeapon(buf, playerId, enttMgr);
            player.BindWeapon(2, wpnId);
        }

        else if (strcmp(key, "active_weapon") == 0)
        {
            char activeWpnName[MAX_LEN_ENTT_NAME];
            count = sscanf(buf, "%s %s", key, activeWpnName);
            assert(count == 2);

            player.SetActiveWeaponId(nameSys.GetIdByName(activeWpnName));
        }

        else if (strcmp(key, "flashlight") == 0)
        {
            // get flashlight entt and bind it to the player
            char flashlightName[MAX_LEN_ENTT_NAME];
            count = sscanf(buf, "%s %s", key, flashlightName);
            assert(count == 2);

            hierarchySys.AddChild(playerId, nameSys.GetIdByName(flashlightName));
        }

        else if (strcmp(key, "camera") == 0)
        {
            // get camera entt and bind it to the player
            char cameraName[MAX_LEN_ENTT_NAME];
            count = sscanf(buf, "%s %s", key, cameraName);
            assert(count == 2);

            hierarchySys.AddChild(playerId, nameSys.GetIdByName(cameraName));
        }

        //
        // setup speed params
        //
        else if (strcmp(key, "speed_walk") == 0)
        {
            float speed = -1;
            ReadFloat(buf, "speed_walk %f", &speed);
            player.SetSpeedWalk(speed);
            player.SetCurrentSpeed(speed);
        }

        else if (strcmp(key, "speed_run") == 0)
        {
            float speed = -1;
            ReadFloat(buf, "speed_run %f", &speed);
            player.SetSpeedRun(speed);
        }

        else if (strcmp(key, "speed_crawl") == 0)
        {
            float speed = -1;
            ReadFloat(buf, "speed_crawl %f", &speed);
            player.SetSpeedCrawl(speed);
        }

        else if (strcmp(key, "speed_noclip") == 0)
        {
            float speed = -1;
            ReadFloat(buf, "speed_noclip %f", &speed);
            player.SetSpeedFreeFly(speed);
        }

        else if (strcmp(key, "offset_y") == 0)
        {
            float offsetY = 0;
            ReadFloat(buf, "offset_y %f", &offsetY);
            player.SetOffsetOverTerrain(offsetY);
        }

        else if (strcmp(key, "jump_height") == 0)
        {
            float h = 0;
            ReadFloat(buf, "jump_height %f", &h);
            player.SetJumpMaxHeight(h);
        }

        else if (strcmp(key, "start_in_noclip") == 0)
        {
            int bStartInNoClip = 0;
            ReadInt(buf, "start_in_noclip %d", &bStartInNoClip);
            player.SetFreeFlyMode((bool)bStartInNoClip);
        }

        else if (strcmp(key, "player_init_pos") == 0)
        {
            float px, py, pz;
            //ReadFloat3(buf, "player_init_pos %f %f %f", &px);
            count = sscanf(buf, "player_init_pos %f %f %f", &px, &py, &pz);
            assert(count == 3);

            enttMgr.PushEvent(ECS::EventTranslate(playerId, px, py, pz));
        }
    };

    //
    // setup boundings for the player
    //
    const XMFLOAT3            playerPos = enttMgr.transformSys_.GetPosition(playerId);

    DirectX::BoundingSphere localSphere = playerBoundSphere;
    DirectX::BoundingSphere worldSphere = localSphere;

    worldSphere.Center.x += playerPos.x;
    worldSphere.Center.y += playerPos.y;
    worldSphere.Center.z += playerPos.z;

    enttMgr.AddBoundingComponent(playerId, localSphere, worldSphere);

    fclose(pFile);
    return true;
}

} // namespace

