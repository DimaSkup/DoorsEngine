/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: GameInitializer.cpp
    Desc:     initialize scene elements
\**********************************************************************************/
#include "../Common/pch.h"
#include "GameInitializer.h"

#include <parse_helpers.h>              // helpers for parsing string buffers, or reading data from file
#include <Render/d3dclass.h>

#include <Render/debug_draw_manager.h>
#include <Model/model_exporter.h>
#include <Model/grass_mgr.h>
#include <Model/model_loader.h>
#include <Model/animation_mgr.h>
#include <Model/animation_saver.h>
#include <Model/animation_loader.h>
#include <Mesh/material_reader.h>

#include "particles_initializer.h"

#include <inttypes.h>                   // for using PRIu32, SCNu32, etc.

using namespace Core;
using namespace Render;
using namespace DirectX;


namespace Game
{

//---------------------------------------------------------
// some typedefs and helpers to get timings
//---------------------------------------------------------
using TimeDurationMs = std::chrono::duration<float, std::milli>;
using TimePoint      = std::chrono::steady_clock::time_point;

inline TimePoint GetTimePoint()
{
    return std::chrono::steady_clock::now();
}


//---------------------------------------------------------
// declare some helper structures
//---------------------------------------------------------
struct NatureParams
{
    char modelName[MAX_LEN_MODEL_NAME]{ "" };

    // tree params
    int numEntts = 0;
    float treeScale = 1.0f;
    Vec3 rotAxis = { 0,0,0 };
    float angleAroundAxis = 1.0f;

    // grass params
    int grassDistFullSize = 0;
    int grassDistVisible = 40;
    int grassCount = 0;
};

//---------------------------------------------------------
// Desc:   load and create materials from file
//---------------------------------------------------------
void InitMaterials(const char* materialsFilepath)
{
    assert(!StrHelper::IsEmpty(materialsFilepath));

    const auto start = GetTimePoint();

    MaterialReader matReader;
    matReader.Read(materialsFilepath);

    // calc the duration of the whole process of materials loading
    const TimeDurationMs elapsed = GetTimePoint() - start;

    SetConsoleColor(MAGENTA);
    LogMsg("Material loading duration: %f ms", elapsed.count());
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:   manually create and setup some particles on the scene
// Args:   - filepath:  a file with definitions of particle emitters 
//---------------------------------------------------------
void GameInitializer::InitParticles(const char* filepath, ECS::EntityMgr& enttMgr)
{
    ParticlesInitializer initializer;

    bool result = initializer.Init(filepath, enttMgr);
    assert(result == true);
}

//---------------------------------------------------------
// Helper structure for light sources initialization
//---------------------------------------------------------
struct LightSrcInitParams
{
    char enttName[MAX_LEN_ENTT_NAME]{'\0'};
    char parentEnttName[MAX_LEN_ENTT_NAME]{'\0'};
    char archetype[32]{'\0'};

    ECS::LightType  lightType = ECS::NUM_LIGHT_TYPES;
    XMFLOAT3        pos       = { 0,0,0 };            // position
    XMFLOAT3        dir       = { 0,0,0 };            // direction
    XMFLOAT4        dirQuat   = { 0,0,0,1 };          // direction quaternion
    int             isActive  = true;                 // 0 - inactive, 1 - active


    void Reset()
    {
        memset(enttName, '\0', MAX_LEN_ENTT_NAME);
        memset(parentEnttName, '\0', MAX_LEN_ENTT_NAME);
        memset(archetype, '\0', 32);

        pos = { 0,0,0 };
        dir = { 0,0,0 };
        dirQuat = { 0,0,0,1 };
        lightType = ECS::NUM_LIGHT_TYPES;
        isActive = true;
    }
};

//---------------------------------------------------------
// Desc:  read params from file, create and setup a new directed light entity
//---------------------------------------------------------
void InitDirectedLightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile);
    printf("\tinit directed light: %s\n", params.enttName);

    ECS::DirLight light;

    // read in params from file
    ReadFileFloat4(pFile, "ambient",   &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",   &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",  &light.specular.x);
    ReadFileFloat3(pFile, "direction", &params.dir.x);
    ReadFileInt   (pFile, "active",    &params.isActive);
    ReadFileStr   (pFile, "archetype",  params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId  = mgr.CreateEntity(params.enttName);
    const XMFLOAT3 pos     = { 0,0,0 };
    const XMVECTOR dirQuat = { params.dir.x, params.dir.y, params.dir.z };

    mgr.AddTransformComponent(enttId, params.pos, dirQuat);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);
}

//---------------------------------------------------------
// Desc:  read params from file, create and setup a new point light entity
//---------------------------------------------------------
void InitPointLightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile != nullptr);
    printf("\tinit point light: %s\n", params.enttName);

    ECS::PointLight light;

    // read in params from file
    ReadFileFloat3(pFile, "pos",       &params.pos.x);
    ReadFileFloat4(pFile, "ambient",   &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",   &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",  &light.specular.x);
    ReadFileFloat3(pFile, "att",       &light.att.x);      // attenuation
    ReadFileFloat (pFile, "range",     &light.range);
    ReadFileInt   (pFile, "active",    &params.isActive);
    ReadFileStr   (pFile, "parent",    params.parentEnttName);
    ReadFileStr   (pFile, "archetype", params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);
    const DirectX::BoundingSphere boundSphereLocal({ 0,0,0 }, light.range);

    mgr.AddTransformComponent(enttId, params.pos);
    mgr.AddBoundingComponent(enttId, boundSphereLocal);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);

    // set parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }
}

//---------------------------------------------------------
// Desc:  read params from file, create and setup a new spotlight entity
//---------------------------------------------------------
void InitSpotlightEntt(ECS::EntityMgr& mgr, FILE* pFile, LightSrcInitParams& params)
{
    assert(pFile);
    printf("\tinit spotlight: %s\n", params.enttName);

    ECS::SpotLight light;

    // read in params from file
    ReadFileFloat3(pFile, "pos",         &params.pos.x);
    ReadFileFloat4(pFile, "dir_quat",    &params.dirQuat.x);
    ReadFileFloat4(pFile, "ambient",     &light.ambient.x);
    ReadFileFloat4(pFile, "diffuse",     &light.diffuse.x);
    ReadFileFloat4(pFile, "specular",    &light.specular.x);
    ReadFileFloat3(pFile, "att",         &light.att.x);       // attenuation
    ReadFileFloat (pFile, "range",       &light.range);       // distance
    ReadFileFloat (pFile, "spot_fallof", &light.spot);        // light intensity fallof (for control the spotlight cone)
    ReadFileInt   (pFile, "active",      &params.isActive);
    ReadFileStr   (pFile, "parent",       params.parentEnttName);
    ReadFileStr   (pFile, "archetype",    params.archetype);

    if (strcmp(params.archetype, "light") != 0)
    {
        LogErr(LOG, "entt archetype isn't \"light\": (name: %s, archetype: %s)", params.enttName, params.archetype);
        return;
    }

    // create entity and add components
    const EntityID enttId = mgr.CreateEntity(params.enttName);

    mgr.AddTransformComponent(enttId, params.pos, XMLoadFloat4(&params.dirQuat), 1.0f);
    mgr.AddLightComponent(enttId, light);
    mgr.lightSys_.SetLightIsActive(enttId, params.isActive);

    // set parent for this point light if we have any
    // (so it will move together with its parent)
    if (params.parentEnttName[0] != '0')
    {
        const EntityID parentId = mgr.nameSys_.GetIdByName(params.parentEnttName);
        mgr.hierarchySys_.AddChild(parentId, enttId);
    }
}

//---------------------------------------------------------
// Desc:  load light entities params from file and create these entities
// Args:  - filepath:  a path to file with definitions of light sources (entities)
//---------------------------------------------------------
void GameInitializer::InitLightEntities(const char* filepath, ECS::EntityMgr& mgr)
{
    LogMsg(LOG, "Initialize light entities");

    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty filename");
        return;
    }

    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file: %s", filepath);
        return;
    }

    constexpr int bufsize = 128;
    char buf[bufsize]{'\0'};
    LightSrcInitParams initParams;

    while (fgets(buf, bufsize, pFile))
    {
        // we always expect declaration of new entity after reading a line
        if (buf[0] != 'n')
        {
            LogErr(LOG, "buffer doesn't contain declaration of newentt: %s", buf);
            LogErr(LOG, "interrupt creation of light sources");
            return;
        }

        // read in a name and light type for a new entity
        char typeName[32]{ '\0' };
        ReadStr    (buf,   "newentt %s", initParams.enttName);
        ReadFileStr(pFile, "type", typeName);


        if (typeName[0] == 'd')
            InitDirectedLightEntt(mgr, pFile, initParams);

        else if (typeName[0] == 'p')
            InitPointLightEntt(mgr, pFile, initParams);

        else if (typeName[0] == 's')
            InitSpotlightEntt(mgr, pFile, initParams);

        else
        {
            LogErr(LOG, "uknown type of light: %s", typeName);
            LogErr(LOG, "interrupt creation of light sources");
            return;
        }

        // after all reset initial params for the following entity
        initParams.Reset();

    } // while fgets

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:   init all the stuff related to the main player
//---------------------------------------------------------
void GameInitializer::InitPlayer(
    ID3D11Device* pDevice,
    ECS::EntityMgr* pEnttMgr,
    const Core::EngineConfigs* pConfigs)
{
    assert(pDevice != nullptr);
    assert(pEnttMgr != nullptr);
    assert(pConfigs != nullptr);

    const EntityID playerId = pEnttMgr->CreateEntity("player");

    // create and set a model for the player entity
    const MeshSphereParams sphereParams(1, 20, 20);
    ModelsCreator creator;
    const ModelID sphereId = creator.CreateSphere(sphereParams);
    BasicModel&   sphere   = g_ModelMgr.GetModelById(sphereId);

   
    // setup material (light properties + textures) for the player entity
    MaterialID catMatID = g_MaterialMgr.GetMatIdByName("cat");

    // setup child entities of the player
    ECS::NameSystem&      nameSys      = pEnttMgr->nameSys_;
    ECS::HierarchySystem& hierarchySys = pEnttMgr->hierarchySys_;
    ECS::InventorySystem& inventorySys = pEnttMgr->inventorySys_;

    const EntityID gameCameraId  = nameSys.GetIdByName("game_camera");
    const EntityID flashlightId  = nameSys.GetIdByName("player_flashlight");
    const EntityID pmHudEnttId   = nameSys.GetIdByName("wpn_pm_hud");
    const EntityID ak74hudEnttId = nameSys.GetIdByName("wpn_ak74_hud");
    const EntityID bm16hudEnttId = nameSys.GetIdByName("wpn_bm16_hud");

    // ------------------------------------------

    pEnttMgr->AddTransformComponent (playerId, { 0,0,0 }, { 0,0,1 });
    pEnttMgr->AddModelComponent     (playerId, sphere.GetID());
    pEnttMgr->AddMaterialComponent  (playerId, catMatID);

    // add inventory for a player and push some stuff into it
    pEnttMgr->AddInventoryComponent(playerId);

    inventorySys.AddItem(playerId, pmHudEnttId);
    inventorySys.AddItem(playerId, ak74hudEnttId);
    inventorySys.AddItem(playerId, bm16hudEnttId);

    // we will render only selected weapon in a separate way
    pEnttMgr->RemoveComponent(pmHudEnttId,   ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(ak74hudEnttId, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(bm16hudEnttId, ECS::RenderedComponent);

    // BIND some entities to the player
    hierarchySys.AddChild(playerId, gameCameraId);
    hierarchySys.AddChild(playerId, flashlightId);
    hierarchySys.AddChild(playerId, pmHudEnttId);
    hierarchySys.AddChild(playerId, ak74hudEnttId);
    hierarchySys.AddChild(playerId, bm16hudEnttId);


    pEnttMgr->AddPlayerComponent(playerId);
    pEnttMgr->AddBoundingComponent(playerId, sphere.GetModelAABB());


    // setup player's params
    ECS::PlayerSystem& player = pEnttMgr->playerSys_;
    const Core::EngineConfigs& cfgs = *pConfigs;

    player.SetSpeedWalk         (cfgs.GetFloat("PLAYER_SPEED_WALK"));
    player.SetSpeedRun          (cfgs.GetFloat("PLAYER_SPEED_RUN"));
    player.SetSpeedCrawl        (cfgs.GetFloat("PLAYER_SPEED_CRAWL"));
    player.SetSpeedFreeFly      (cfgs.GetFloat("PLAYER_SPEED_FREE_FLY"));
    player.SetCurrentSpeed      (cfgs.GetFloat("PLAYER_SPEED_WALK"));

    player.SetOffsetOverTerrain (cfgs.GetFloat("PLAYER_OFFSET_Y"));
    player.SetJumpMaxHeight     (cfgs.GetFloat("PLAYER_JUMP_MAX_HEIGHT"));

    player.SetFreeFlyMode(cfgs.GetBool("PLAYER_START_IN_FREE_FLY"));
    player.SetActiveWeapon(pmHudEnttId);

    // HACK setup (move the player at its inital position)
    const float posX = cfgs.GetFloat("PLAYER_POS_X_AFTER_INIT");
    const float posY = cfgs.GetFloat("PLAYER_POS_Y_AFTER_INIT");
    const float posZ = cfgs.GetFloat("PLAYER_POS_Z_AFTER_INIT");

    pEnttMgr->PushEvent(ECS::EventTranslate(playerId, posX, posY, posZ));
}

//---------------------------------------------------------
// Desc:  create and setup a new camera entity
//---------------------------------------------------------
bool GameInitializer::InitCamera(
    ECS::EntityMgr& mgr,
    const char* cameraName,
    CameraInitParams& initParams)
{
    // check input args
    if (!cameraName || cameraName[0] == '\0')
    {
        LogErr(LOG, "camera name is empty");
        return false;
    }

    if ((initParams.wndWidth <= 0) || (initParams.wndHeight <= 0))
    {
        LogErr(LOG, "wrong dimensions (width or height <= 0) for camera: %s", cameraName);
        return false;
    }

    if (initParams.nearZ <= 0.001f)
    {
        LogErr(LOG, "invalid nearZ for camera: %s", cameraName);
        return false;
    }

    if (initParams.fovInRad <= 0.001f)
    {
        LogErr(LOG, "input FOV for camera: %s", cameraName);
        return false;
    }

    //-------------------------------------------

    // setup params for components
    const float aspectRatio = initParams.wndWidth / initParams.wndHeight;
    const XMFLOAT3 camPos   = { initParams.posX, initParams.posY, initParams.posZ };

    ECS::CameraData camData;
    camData.fovY            = initParams.fovInRad;
    camData.aspectRatio     = aspectRatio;
    camData.nearZ           = initParams.nearZ;
    camData.farZ            = initParams.farZ;


    // create an entity and add components
    const EntityID camId = mgr.CreateEntity(cameraName);
    mgr.AddTransformComponent(camId, camPos, { 0,0,1,1 });
    mgr.AddCameraComponent(camId, camData);

    // initialize view/projection matrices of the editor/game camera
    // 
    // TODO?: move initial UpdateView and SetBaseViewMatrix into the CameraSystem::AddRecord()
    const DirectX::XMMATRIX& editorCamView = mgr.cameraSys_.UpdateView(camId);

    mgr.cameraSys_.SetBaseViewMatrix(camId, editorCamView);
    mgr.cameraSys_.SetupOrthographicMatrix(
        camId,
        initParams.wndWidth,
        initParams.wndHeight,
        initParams.nearZ,
        initParams.farZ);

    return true;
}

//---------------------------------------------------------
// Desc:  create and setup spheres entities
//---------------------------------------------------------
void CreateSpheres(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create spheres entities");

    constexpr size numEntts = 10;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

    XMFLOAT3 positions[numEntts];
    XMVECTOR directions[numEntts];
    float uniformScales[numEntts];

    // setup positions: make two rows of the spheres
    for (index i = 0; i < numEntts; i += 2)
    {
        positions[i + 0] = XMFLOAT3(240.0f, 80.0f, 210.0f + i);
        positions[i + 1] = XMFLOAT3(242.0f, 80.0f, 210.0f + i);
    }

    // setup directions
    for (index i = 0; i < numEntts; ++i)
        directions[i] = { 0,0,0,1 };

    // setup uniform scales
    for (index i = 0; i < numEntts; ++i)
        uniformScales[i] = 1.0f;

    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniformScales);

    // ---------------------------------------------

    // generate a name for each sphere entity
    std::string enttsNames[numEntts];

    for (int i = 0; const EntityID id : enttsIDs)
        enttsNames[i++] = "sphere_" + std::to_string(id);

    // ---------------------------------------------

    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddNameComponent(ids, enttsNames, numEntts);
    mgr.AddRenderingComponent(ids, numEntts);

    // ---------------------------------------------

    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundType = ECS::BOUND_BOX;

    // add bounding component to each entity
    mgr.AddBoundingComponent(ids, numEntts, model.GetModelAABB());


     // add bounding sphere (sphere of lines) to the debug shapes render list
    const DirectX::BoundingBox aabb = model.GetModelAABB();
    const Vec3 orange(1.0f, 0.65f, 0.0f);

    for (int i = 0; i < numEntts; ++i)
    {
        const Vec3 pos(positions[i].x, positions[i].y, positions[i].z);
        const float scale = uniformScales[i];

        // transform bounding box
        Vec3 boxCenter(aabb.Center.x, aabb.Center.y, aabb.Center.z);
        Vec3 boxExtents(aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);

        boxCenter += pos;
        boxExtents *= scale;

        Vec3 radiusVec(boxCenter);
        radiusVec.y += boxExtents.y;
        radiusVec -= boxCenter;

        const float radius = Vec3Length(radiusVec);

        //Core::g_DebugDrawMgr.AddAABB(boxCenter, boxExtents, orange);
        Core::g_DebugDrawMgr.AddSphere(boxCenter, radius, orange);
    }

    

    // add material for each sphere entity
    const MaterialID matID         = model.meshes_.subsets_[0].materialId;
    const MaterialID ground04MatId = g_MaterialMgr.GetMatIdByName("ground04");
    const MaterialID catMatID      = g_MaterialMgr.GetMatIdByName("cat");
    const MaterialID brickMatId    = g_MaterialMgr.GetMatIdByName("brick_01");

    // specify material for some entities
    mgr.AddMaterialComponent(enttsIDs[0], catMatID);
    mgr.AddMaterialComponent(enttsIDs[1], brickMatId);

    for (index i = 2; i < numEntts-3; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], matID);

    for (index i = numEntts-3; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], ground04MatId);
        

    // setup a texture transformation for the sphere
    ECS::StaticTexTransInitParams sphereTexTransform;
    sphereTexTransform.Push(DirectX::XMMatrixScaling(3, 3, 0));

    mgr.AddTextureTransformComponent(enttsIDs[0], ECS::TexTransformType::STATIC, sphereTexTransform);
    mgr.AddTextureTransformComponent(enttsIDs[1], ECS::TexTransformType::STATIC, sphereTexTransform);
}

//---------------------------------------------------------
// Desc:    load a sky params from config file,
//          create sky model, and create sky entity
// Args:    - filepath:  a path to file with sky configuration
//---------------------------------------------------------
void CreateSkyBox(const char* filepath, ECS::EntityMgr& mgr)
{
    LogDbg(LOG, "create a sky box entity");

    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty filepath");
        return;
    }

    // open config file
    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open sky config file");
        exit(0);
    }

    // read in sky params
    char     skyTexName[MAX_LEN_TEX_NAME]{'\0'};
    char     skyMaterialName[MAX_LEN_MAT_NAME]{'\0'};
    int      skyBoxSize = 0;
    int      loadCubeMapTexture = 0;
    int      count = 0;
    float    skyOffsetY = 0;
    XMFLOAT3 colorCenter;
    XMFLOAT3 colorApex;
    CubeMapInitParams cubeMapParams;
    

    // do we want to load cubemap or we will create it manually?
    ReadFileInt   (pFile, "load_cubemap_texture:", &loadCubeMapTexture);
    ReadFileStr   (pFile, "material_name:",        skyMaterialName);
    ReadFileStr   (pFile, "cubemap_texture:",      skyTexName);
    ReadFileStr   (pFile, "cubemap_dir",           cubeMapParams.directory);

    ReadFileStr   (pFile, "sky_pos_x",             cubeMapParams.texNames[0]);
    ReadFileStr   (pFile, "sky_neg_x",             cubeMapParams.texNames[1]);
    ReadFileStr   (pFile, "sky_pos_y",             cubeMapParams.texNames[2]);
    ReadFileStr   (pFile, "sky_neg_y",             cubeMapParams.texNames[3]);
    ReadFileStr   (pFile, "sky_pos_z",             cubeMapParams.texNames[4]);
    ReadFileStr   (pFile, "sky_neg_z",             cubeMapParams.texNames[5]);

    ReadFileFloat3(pFile, "color_center",          &colorCenter.x);
    ReadFileFloat3(pFile, "color_apex",            &colorApex.x);
    ReadFileInt   (pFile, "sky_box_size",          &skyBoxSize);
    ReadFileFloat (pFile, "sky_box_offset_y",      &skyOffsetY);


    // load a texture for the sky
    TexID skyMapId = INVALID_TEX_ID;

    if (loadCubeMapTexture)
        skyMapId = g_TextureMgr.GetTexIdByName(skyTexName);

    else
        skyMapId = g_TextureMgr.CreateCubeMap("sky_cube_map_0", cubeMapParams);


    // setup a material
    Material& mat = g_MaterialMgr.GetMatByName(skyMaterialName);
    mat.SetTexture(TEX_TYPE_DIFFUSE, skyMapId);
    //mat.SetCull(MAT_PROP_CULL_FRONT);
    //mat.SetFrontClockwise(MAT_PROP_FRONT_COUNTER_CLOCKWISE);
    //mat.SetDepthStencil(MAT_PROP_DSS_SKY_DOME);

    // create a sky model
    Core::ModelsCreator creator;
    creator.CreateSkyCube((float)skyBoxSize);
    //creator.CreateSkySphere(pDevice, 2000, 30, 30);
    //MeshSphereParams skyDomeSphereParams(2000, 30, 30);
    //const ModelID skyBoxID = creator.CreateSphere(pDevice, skyDomeSphereParams);
    SkyModel& skyModel = g_ModelMgr.GetSky();

    // setup sky model
    skyModel.SetMaterialId(mat.id);
    skyModel.SetColorCenter(colorCenter);
    skyModel.SetColorApex(colorApex);

    // create and setup a sky entity
    const EntityID enttId = mgr.CreateEntity("sky");
    mgr.AddTransformComponent(enttId, { 0, skyOffsetY,0 });
    mgr.AddMaterialComponent(enttId, mat.id);

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:  add a new axis-aligned bounding box for visualization
//---------------------------------------------------------
void AddAabbToRender(const DirectX::BoundingBox& aabb)
{
    const Vec3 color(0, 1, 1);
    const Vec3 center(aabb.Center.x, aabb.Center.y, aabb.Center.z);
    const Vec3 extent(aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);
    const Vec3 minPoint(center - extent);
    const Vec3 maxPoint(center + extent);

    g_DebugDrawMgr.AddAABB(minPoint, maxPoint, color);
}

//---------------------------------------------------------
// Desc:  create and setup cylinder entities
//---------------------------------------------------------
void CreateCylinders(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create cylinders entities");

    constexpr size numEntts = 10;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

    XMFLOAT3 positions[numEntts];
    XMVECTOR dirQuats[numEntts];
    float uniformScales[numEntts];
    constexpr float halfHeight = 2.5f;

    // we will set heights according to the terrain's landscape
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float range = (float)terrain.GetTerrainLength();
    const float maxHeight = 150;

    // generate positions
    for (index i = 0; i < numEntts / 2; i += 2)
    {
        positions[i + 0] = XMFLOAT3(240.0f, 77.0f, 210.0f + i);
        positions[i + 1] = XMFLOAT3(242.0f, 77.0f, 210.0f + i);
    }

    // setup directions
    for (index i = 0; i < numEntts; ++i)
        dirQuats[i] = { 0,0,0,1 };

    // setup uniform scales
    for (index i = 0; i < numEntts; ++i)
        uniformScales[i] = 1.0f;

    // ----------------------------------------------------
    // generate names for the entities

    std::string names[numEntts];

    for (int i = 0; const EntityID id : enttsIDs)
        names[i++] = "cylinder_" + std::to_string(id);

    // ----------------------------------------------------

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);

    // each cylinder has only one mesh
    const MaterialID brickMatID = g_MaterialMgr.GetMatIdByName("brick_01");

    for (const EntityID id : enttsIDs)
        mgr.AddMaterialComponent(id, brickMatID);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddRenderingComponent(ids, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddBoundingComponent(ids, numEntts, model.GetModelAABB());

    ECS::StaticTexTransInitParams texTransform;
    texTransform.Push(DirectX::XMMatrixScaling(3, 3, 0));
    mgr.AddTextureTransformComponent(ids[1], ECS::TexTransformType::STATIC, texTransform);
}

//---------------------------------------------------------
// Desc:  generate trees entities at random places and based on input model
//---------------------------------------------------------
void CreateTreesEntities(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const char* modelName,
    const uint numEntts,
    const float uniformScale,
    const XMVECTOR quatRotAxis)
{
    using namespace DirectX;

    if (StrHelper::IsEmpty(modelName))
    {
        LogErr(LOG, "input model name is empty!");
        return;
    }
    if (numEntts == 0)
    {
        LogErr(LOG, "your input number of entities == 0, why do you call me?");
        return;
    }


    LogDbg(LOG, "create trees: %s", modelName);

    // find a model which will be used as trees
    const BasicModel& tree = g_ModelMgr.GetModelByName(modelName);

    if (tree.id_ == INVALID_MODEL_ID)
    {
        LogErr(LOG, "can't create trees: there is no model by name: %s", modelName);
        return;
    }


    const cvector<EntityID> entities = mgr.CreateEntities(numEntts);
    const EntityID*         enttsIds = entities.data();

    cvector<std::string> names(numEntts);
    cvector<XMFLOAT3>    positions(numEntts);
    cvector<float>       scales(numEntts);
    cvector<XMVECTOR>    yRotationQuats(numEntts);
    cvector<XMVECTOR>    directions(numEntts);

    // generate names
    for (uint i = 0; i < numEntts; ++i)
    {
        char tmpName[MAX_LEN_ENTT_NAME]{ '\0' };
        snprintf(tmpName, MAX_LEN_ENTT_NAME, "%s_%" PRIu32, modelName, enttsIds[i]);
        names[i] = tmpName;
    }

    // set heights according to the terrain's landscape
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float maxHeight  = 150;                                   // max height where tree can appear
    const float range      = (float)terrain.heightMap_.GetWidth();

    // generate RANDOM position for each tree
    for (uint i = 0; i < numEntts; ++i)
    {
        XMFLOAT3& pos = positions[i];
        bool noNatureArea = false;

        do
        {
            pos.x = RandF(0, range-5);
            pos.z = RandF(0, range-5);
            pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z);

            // regenerate position/height if we got at area with no/low nature density
            while (terrain.GetNatureDensityAtPoint(pos.x, pos.z) < 1)
            {
                pos.x = RandF(0, range - 5);
                pos.z = RandF(0, range - 5);
                pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z);
            }

        } while (maxHeight < pos.y);   // limit height for trees
    }

    // calc quaternions for rotation around Y-axis
    for (uint i = 0; i < numEntts; ++i)
    {
        const float angle = DEG_TO_RAD(RandF(0, 3600) * 0.1f);
        yRotationQuats[i] = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, angle);
    }
        
    // generate directions (rotate vector <0,0,1> by quaternion)
    for (uint i = 0; i < numEntts; ++i)
    {
        const XMVECTOR quat     = yRotationQuats[i];
        const XMVECTOR invQuat  = XMQuaternionInverse(quat);
        const XMVECTOR tmpVec   = XMQuaternionMultiply(invQuat, { 0,0,1 });

        directions[i]           = XMQuaternionMultiply(tmpVec, quat);
    }

    // generate uniform scales
    for (uint i = 0; i < numEntts; ++i)
        scales[i] = uniformScale;


    // prepare data for material component
    const Core::Subset* subsets = tree.meshes_.subsets_;
    const size       numSubsets = tree.GetNumSubsets();
    cvector<MaterialID> matsIds(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        matsIds[i] = subsets[i].materialId;


    // add component to each tree entity
    mgr.AddTransformComponent(
        enttsIds,
        numEntts,
        positions.data(),
        directions.data(),
        scales.data());

    mgr.AddNameComponent(enttsIds, names.data(), numEntts);
    mgr.AddModelComponent(enttsIds, tree.GetID(), numEntts);
    mgr.AddRenderingComponent(enttsIds, numEntts);
    mgr.AddBoundingComponent(enttsIds, numEntts, tree.GetModelAABB());

    for (index i = 0; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIds[i], matsIds.data(), numSubsets);


    // rotate each tree using input quaternion
    const XMVECTOR noRotQuat{ 0,0,0,1 };

    if (quatRotAxis != noRotQuat)
        mgr.transformSys_.RotateLocalSpacesByQuat(enttsIds, numEntts, quatRotAxis);

    // rotate each tree around Y-axis by random angle
    for (uint i = 0; i < numEntts; ++i)
    {
        mgr.transformSys_.RotateLocalSpaceByQuat(enttsIds[i], yRotationQuats[i]);
    }

    // add AABB of each tree to the debug shapes render list
    BoundingBox aabb = tree.GetModelAABB();

    for (uint i = 0; i < numEntts; ++i)
    {
        BoundingBox tmpAABB = aabb;
        const XMMATRIX& M = mgr.transformSys_.GetWorld(enttsIds[i]);
        aabb.Transform(tmpAABB, M);
        AddAabbToRender(tmpAABB);
    }
}

//---------------------------------------------------------
// Desc:  handler for entity name to make it proper and unique
//---------------------------------------------------------
void HandleName(
    const char* inName,
    char* outName,
    const EntityID id,
    const BasicModel& model,
    ECS::EntityMgr& mgr)
{
    if (StrHelper::IsEmpty(inName))
    {
        // model_name + entt_id
        LogErr(LOG, "input entt's name is empty, so we will generate a new one");
        snprintf(outName, MAX_LEN_ENTT_NAME, "%s_%" PRIu32, model.GetName(), id);
        return;
    }

    if (!mgr.nameSys_.IsUnique(inName))
    {
        // in_name + entt_id
        LogErr(LOG, "input entt's name (%s) isn't unique, we will generate a new one", inName);
        snprintf(outName, MAX_LEN_ENTT_NAME, "%s_%" PRIu32, inName, id);
        return;
    }

    strncpy(outName, inName, MAX_LEN_ENTT_NAME);
}

//---------------------------------------------------------
// Desc:  create a new NPC entity
//---------------------------------------------------------
void CreateNPC(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float uniformScale,
    const char* inName)
{
    const EntityID enttId = mgr.CreateEntity();

 
    char enttName[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, enttName, enttId, model, mgr);

    LogDbg(LOG, "create entity: %s", enttName);


    if (uniformScale <= 0)
    {
        LogErr(LOG, "input uniform scale is <= 0, so reset it to 1");
        uniformScale = 1;
    }

    // if for any reason we got "invalid" model (cube) prevent it to be too big or small
    if (model.id_ == INVALID_MODEL_ID)
        uniformScale = 3.0f;
}

//---------------------------------------------------------
// Desc:   create and init a new building entity based on input model
//---------------------------------------------------------
EntityID CreateBuildingEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float uniformScale,
    const char* inName)
{
    using namespace DirectX;

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char enttName[MAX_LEN_ENTT_NAME]{ '\0' };
    HandleName(inName, enttName, enttId, model, mgr);

    LogDbg(LOG, "create entity: %s", enttName);

    // check some input args
    if (uniformScale <= 0)
    {
        LogErr(LOG, "input uniform scale is <= 0, so reset it to 1");
        uniformScale = 1;
    }

    // if for any reason we got "invalid" model (cube) prevent it to be too big or small
    if (model.id_ == INVALID_MODEL_ID)
        uniformScale = 3.0f;


    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup materials ids
    const Core::Subset* subsets = model.meshes_.subsets_;

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, enttName);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddBoundingComponent(enttId, model.GetModelAABB());

    // add material component
    if (numSubsets > 1)
    {
        cvector<MaterialID> materialsIds(numSubsets, INVALID_MATERIAL_ID);

        for (index i = 0; i < numSubsets; ++i)
            materialsIds[i] = subsets[i].materialId;

        // bound material to each mesh of this entity
        mgr.AddMaterialComponent(enttId, materialsIds.data(), numSubsets);
    }
    else
    {
        mgr.AddMaterialComponent(enttId, subsets[0].materialId);
    }


    // add this AABB to the debug shapes render list
    BoundingBox aabb = model.GetModelAABB();
    XMMATRIX M = mgr.transformSys_.GetWorld(enttId);
    aabb.Transform(aabb, M);

    const Vec3 color(0, 1, 1);
    const Vec3 center(aabb.Center.x, aabb.Center.y, aabb.Center.z);
    const Vec3 extent(aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);
    const Vec3 minPoint(center - extent);
    const Vec3 maxPoint(center + extent);

    g_DebugDrawMgr.AddAABB(minPoint, maxPoint, color);

    return enttId;
}

//---------------------------------------------------------
// Desc:   create and init a new vehicle entity based on input model
//---------------------------------------------------------
EntityID CreateVehicleEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    // print debug msg
    LogDbg(LOG, "create %s entity", name);


    if (uniformScale <= 0)
    {
        LogErr(LOG, "scale is invalid (%f) reset it to 1.0f (for entt: %s)", uniformScale, name);
        uniformScale = 1.0f;
    }

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddBoundingComponent(enttId, model.GetModelAABB());


    // add material component
    const Core::Subset* subsets = model.meshes_.subsets_;
    const size numSubsets = model.GetNumSubsets();

    if (numSubsets > 1)
    {
        MaterialID matsIds[32]{ INVALID_MATERIAL_ID };
        assert(numSubsets <= 32 && "can't create a vehicle entt");

        for (index i = 0; i < numSubsets; ++i)
            matsIds[i] = subsets[i].materialId;

        // bound material to each mesh of this entity
        mgr.AddMaterialComponent(enttId, matsIds, numSubsets);
    }
    else
    {
        mgr.AddMaterialComponent(enttId, subsets[0].materialId);
    }

   
    // add this AABB to the debug shapes render list
    BoundingBox aabb = model.GetModelAABB();
    const XMMATRIX M = mgr.transformSys_.GetWorld(enttId);
    aabb.Transform(aabb, M);

    const Vec3 color(0, 1, 1);
    const Vec3 center(aabb.Center.x, aabb.Center.y, aabb.Center.z);
    const Vec3 extent(aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);
    const Vec3 minPoint(center - extent);
    const Vec3 maxPoint(center + extent);

    g_DebugDrawMgr.AddAABB(minPoint, maxPoint, color);

    return enttId;
}

//---------------------------------------------------------
// Desc:   create and init a new weapon entity based on input model
//---------------------------------------------------------
EntityID CreateWeaponEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId   = mgr.CreateEntity();

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    // print debug msg
    LogDbg(LOG, "create %s entity", name);

    if (uniformScale <= 0)
    {
        LogErr(LOG, "scale is invalid (%f) reset it to 1.0f (for entt: %s)", uniformScale, name);
        uniformScale = 1.0f;
    }


    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);

    // rotate entity around itself
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddBoundingComponent(enttId, model.GetModelAABB());


    // add material to multiple subsets of the entt
    const Core::Subset* subsets = model.meshes_.subsets_;
    const size numSubsets       = model.GetNumSubsets();

    if (numSubsets > 1)
    {
        MaterialID matsIds[32]{ INVALID_MATERIAL_ID };
        assert(numSubsets < 32 && "can't create a weapon entt");

        for (index i = 0; i < numSubsets; ++i)
            matsIds[i] = subsets[i].materialId;

        // bound material to each mesh of this entity
        mgr.AddMaterialComponent(enttId, matsIds, numSubsets);
    }
    // add material to a single subset of the entt
    else
    {
        mgr.AddMaterialComponent(enttId, subsets[0].materialId);
    }

 
    // add this AABB to the debug shapes render list
    BoundingBox aabb = model.GetModelAABB();
    XMMATRIX M = mgr.transformSys_.GetWorld(enttId);
    aabb.Transform(aabb, M);

    const Vec3 color(0, 1, 1);
    const Vec3 center(aabb.Center.x, aabb.Center.y, aabb.Center.z);
    const Vec3 extent(aabb.Extents.x, aabb.Extents.y, aabb.Extents.z);
    const Vec3 minPoint(center - extent);
    const Vec3 maxPoint(center + extent);

    g_DebugDrawMgr.AddAABB(minPoint, maxPoint, color);

    return enttId;
}


//---------------------------------------------------------
// Desc:   create and init a cube entity
//---------------------------------------------------------
EntityID CreateCubeEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    // print debug msg
    LogDbg(LOG, "create %s entity", name);

    if (uniformScale <= 0)
    {
        LogErr(LOG, "scale is invalid (%f) reset it to 1.0f (for entt: %s)", uniformScale, name);
        uniformScale = 1.0f;
    }

 
    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);

    // rotate entity around itself
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddMaterialComponent(enttId, model.meshes_.subsets_[0].materialId);
    mgr.AddBoundingComponent(enttId, model.GetModelAABB());


    // add this AABB to the debug shapes render list
    BoundingBox aabb = model.GetModelAABB();
    const XMMATRIX& M = mgr.transformSys_.GetWorld(enttId);
    aabb.Transform(aabb, M);
    AddAabbToRender(aabb);

    return enttId;
}

//---------------------------------------------------------
// Desc:  generate random params for grass and fill in a vertex buffer with this data
//---------------------------------------------------------
void CreateGrass(const float grassDistVisible, const uint numGrassInstances)
{
    assert(grassDistVisible >= 0);

    if (numGrassInstances == 0)
        return;

    LogDbg(LOG, "generage grass instances");


    // cover full terrain
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float terrainSize      = (float)terrain.GetTerrainLength()-5;
    cvector<VertexGrass> grassVertices(numGrassInstances);

    // generate random positions
    for (VertexGrass& grass : grassVertices)
    {
        grass.pos.x  = RandF(0, terrainSize);
        grass.pos.z  = RandF(0, terrainSize);
        grass.pos.y  = terrain.GetScaledInterpolatedHeightAtPoint(grass.pos.x, grass.pos.z);

        grass.size.x = RandF(0.75f, 1.0f);
        grass.size.y = grass.size.x;

        // regenerate position/height if we got at area with no/low nature density
        while (terrain.GetNatureDensityAtPoint(grass.pos.x, grass.pos.z) < 1)
        {
            grass.pos.x = RandF(0, terrainSize);
            grass.pos.z = RandF(0, terrainSize);
            grass.pos.y = terrain.GetScaledInterpolatedHeightAtPoint(grass.pos.x, grass.pos.z);
        }
    }

    g_GrassMgr.Init(terrain.GetPatchSize(), terrain.GetNumPatchesPerSide());
    g_GrassMgr.AddGrassVertices(grassVertices);
    g_GrassMgr.SetGrassRange(grassDistVisible);
}

//---------------------------------------------------------
// Desc:  helpers for work with DX quaternions
//---------------------------------------------------------
inline XMVECTOR QuatRotAxis(const XMVECTOR& axis, const float angle)
{
    return DirectX::XMQuaternionRotationAxis(axis, angle);
}

inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2)
{
    return DirectX::XMQuaternionMultiply(q1, q2);
}

inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2, const XMVECTOR& q3)
{
    return DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q1, q2), q3);
}

//---------------------------------------------------------
// Desc:   read in a list of models from file and create them
//---------------------------------------------------------
void LoadModelAssets(const char* filepath, Render::CRender& render)
{
    assert(filepath && filepath[0] != '\0');

    // calc the duration of the whole process of loading
    const TimePoint start = GetTimePoint();

    char buf[256]{ '\0' };
    char modelPath[256]{ '\0' };
    char modelName[MAX_LEN_MODEL_NAME]{ '\0' };
    ModelID modelId = 0;
    ModelLoader loader;
    ID3D11Device* pDevice = render.GetDevice();


    // open a file for models reading
    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file for models creation: %s", filepath);
        exit(0);
    }

    // read and init each model
    while (fgets(buf, sizeof(buf), pFile))
    {
        // skip a comment line
        if (buf[0] == '/')
            continue;

        int count = sscanf(buf, "%s", modelPath);
        if (count != 1)
        {
            LogErr(LOG, "can't properly path to model from a buffer: %s", buf);
            continue;
        }

        // add empty model into the manager and setup its name
        BasicModel& model = g_ModelMgr.AddEmptyModel();
        FileSys::GetFileStem(modelPath, modelName);
        g_ModelMgr.SetModelName(model.GetID(), modelName);

        const bool isLoaded = loader.Load(modelPath, &model);

        // if failed to load model...
        if (!isLoaded)
        {
            LogErr(LOG, "can't load model from file: %s", modelPath);
            LogMsg("%s Set model (%s) to default (cube)%s", YELLOW, modelName, RESET);

            const BasicModel& invalidModel = g_ModelMgr.GetModelById(0);
            model.Copy(pDevice, invalidModel);

            g_ModelMgr.SetModelName(model.GetID(), modelName);
        }

        // init vb/ib buffers
        model.InitializeBuffers(pDevice);
    }


    const TimeDurationMs elapsed = GetTimePoint() - start;

    SetConsoleColor(MAGENTA);
    LogMsg("-------------------------------------");
    LogMsg("Models loading duration: %f sec", elapsed.count() * 0.001f);
    LogMsg("-------------------------------------\n");
    SetConsoleColor(RESET);

    fclose(pFile);
}


//---------------------------------------------------------
// Desc:   load and create entities from file
//---------------------------------------------------------
void LoadEntities(const char* filePath, ECS::EntityMgr& enttMgr)
{
    assert(filePath && filePath[0] != '\0');

    FILE* pFile = fopen(filePath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file for entities creation: %s", filePath);
        exit(0);
    }

    EntityID enttId = INVALID_ENTITY_ID;
    constexpr int bufSize = 128;
    char buf[bufSize]{'\0'};

    char enttName[MAX_LEN_ENTT_NAME]{'\0'};
    char archetype[32]{'\0'};
    XMFLOAT3 pos       = { 0,0,0 };
    XMFLOAT3 dir       = { 0,0,0 };
    
    XMVECTOR direction = { 0,0,1,0 };
    XMVECTOR rotQuat   = { 0,0,0,1 };
    float uniScale     = 1.0;
    BasicModel* pModel = nullptr;

    cvector<MaterialID> materialIds(8, INVALID_MATERIAL_ID);
    cvector<SubsetID>   subsetIds(8, 0);


    //printf("\n\n\n");

    while (fgets(buf, bufSize, pFile))
    {
        // create a new entity
        if (strncmp(buf, "newentt", 7) == 0)
        {
            ReadStr(buf, "newentt %s", enttName);

            pModel     = nullptr;
            materialIds.clear();
            subsetIds.clear();
            rotQuat = { 0,0,0,1 };
        }

        // read in a model name
        else if (strncmp(buf, "model", 5) == 0)
        {
            char modelName[MAX_LEN_MODEL_NAME]{'\0'};
            ReadStr(buf, "model %s\n", modelName);

            pModel = &g_ModelMgr.GetModelByName(modelName);
            assert(pModel != nullptr);
        }

        // read in material (subsetId => materialId)
        else if (strncmp(buf, "material", 8) == 0)
        {
            int subsetId = -1;
            char matName[MAX_LEN_MAT_NAME]{ '\0' };
            int count = sscanf(buf, "material %d %s\n", &subsetId, matName);
            assert(count == 2);

            MaterialID matId = g_MaterialMgr.GetMatIdByName(matName);

            if (matId == INVALID_MODEL_ID)
            {
                LogErr(LOG, "there is no material by name: %s", matName);
                continue;
            }

            materialIds.push_back(matId);
            subsetIds.push_back((SubsetID)subsetId);
        }

        // read in position in world
        else if (strncmp(buf, "pos", 3) == 0)
        {
            ReadFloat3(buf, "pos %f %f %f\n", &pos.x);
        }

        else if (strncmp(buf, "dir", 3) == 0)
        {
            ReadFloat3(buf, "dir %f %f %f\n", &dir.x);
            direction = { dir.x, dir.y, dir.z };
        }

        else if (strncmp(buf, "rot_axis_x", 10) == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_x %f\n", &angle);
            XMVECTOR q = QuatRotAxis({ 1,0,0 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strncmp(buf, "rot_axis_y", 10) == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_y %f\n", &angle);
            XMVECTOR q = QuatRotAxis({ 0,1,0 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strncmp(buf, "rot_axis_z", 10) == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_z %f\n", &angle);
            XMVECTOR q = QuatRotAxis({ 0,0,1 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strncmp(buf, "rot_axis", 8) == 0)
        {
            Vec4 rotAxis = { 0,0,0,0 };
            ReadFloat4(buf, "rot_axis %f %f %f %f\n", &rotAxis.x);
            rotQuat = QuatRotAxis({ rotAxis.x, rotAxis.y, rotAxis.z }, rotAxis.w);
        }

        else if (strncmp(buf, "rot_quat", 8) == 0)
        {
            Vec4 q = { 0,0,0,1 };
            ReadFloat4(buf, "rot_quat %f %f %f %f\n", &q.x);
            rotQuat = { q.x, q.y, q.z, q.w };
        }

        else if (strncmp(buf, "uni_scale", 9) == 0)
        {
            ReadFloat(buf, "uni_scale %f\n", &uniScale);
        }

        else if (strncmp(buf, "archetype", 9) == 0)
        {
            ReadStr(buf, "archetype %s", archetype);

            // make sure that we have any model for this entity
            assert(pModel != nullptr);

            EntityID enttId = 0;

            // create entity: building / vehicle / weapon / etc.
            if (strcmp(archetype, "building") == 0)
            {
                enttId = CreateBuildingEntt(enttMgr, *pModel, pos, direction, rotQuat, uniScale, enttName);
            }

            else if (strcmp(archetype, "vehicle") == 0)
            {
                enttId = CreateVehicleEntt(enttMgr, *pModel, pos, direction, rotQuat, uniScale, enttName);
            }

            else if (strcmp(archetype, "weapon") == 0)
            {
                enttId = CreateWeaponEntt(enttMgr, *pModel, pos, direction, rotQuat, uniScale, enttName);
            }

            else if (strcmp(archetype, "cube") == 0)
            {
                enttId = CreateCubeEntt(enttMgr, *pModel, pos, direction, rotQuat, uniScale, enttName);
            }

            else
            {
                LogErr(LOG, "can't create entity (%s): unknown archetype (%s)", enttName, archetype);
            }

            // setup material for subset 0
            for (index i = 0; i < materialIds.size(); ++i)
            {
                enttMgr.materialSys_.SetMaterial(enttId, subsetIds[i], materialIds[i]);
            }
        }
    }

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:  read in some grass parameter from input buffer
//---------------------------------------------------------
void ReadGrassParams(const char* buf, NatureParams& params)
{
    if (strncmp(buf, "grass_dist_full_size", 20) == 0)
    {
        ReadInt(buf, "grass_dist_full_size %d", &params.grassDistFullSize);
    }
    else if (strncmp(buf, "grass_dist_visible", 18) == 0)
    {
        ReadInt(buf, "grass_dist_visible %d", &params.grassDistVisible);
    }
    else if (strncmp(buf, "grass_count", 11) == 0)
    {
        ReadInt(buf, "grass_count %d", &params.grassCount);
    }
}

//---------------------------------------------------------
// Desc:  read in some trees parameter from input buffer
//---------------------------------------------------------
void ReadTreesParams(const char* buf, NatureParams& params)
{
    int count = 0;

    if (strncmp(buf, "newtree", 7) == 0)
        ReadStr(buf, "newtree %s", params.modelName);

    else if (strncmp(buf, "count", 5) == 0)
        ReadInt(buf, "count %d", &params.numEntts);

    else if (strncmp(buf, "scale", 5) == 0)
        ReadFloat(buf, "scale %f", &params.treeScale);

    else if (strncmp(buf, "quat_rot_axis", 13) == 0)
    {
        int count = sscanf(buf, "quat_rot_axis %f %f %f %f",
            &params.rotAxis.x,
            &params.rotAxis.y,
            &params.rotAxis.z,
            &params.angleAroundAxis);
        assert(count == 4);
    }
}

//---------------------------------------------------------
// Desc:  generate/initialize nature elements
//---------------------------------------------------------
void CreateNature(const char* filename, ECS::EntityMgr& enttMgr, Render::CRender& render)
{
    assert(filename && filename[0] != '\0');
    LogDbg(LOG, "initialize nature");


    constexpr int bufSize = 128;
    char buf[bufSize]{ '\0' };
    NatureParams params;

    // open config file
    FILE* pFile = fopen(filename, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open config file for nature initialization: %s", filename);
        exit(0);
    }

    while (fgets(buf, bufSize, pFile))
    {
        // if "new line" we create trees
        if (strcmp(buf, "\n") == 0)
        {
            // if we didn't read any data for tree
            if (StrHelper::IsEmpty(params.modelName))
                continue;


            XMVECTOR rotQuat = { 0,0,0,1 };

            // calc rotation quat around axis
            if (params.rotAxis != Vec3{ 0,0,0 })
            {
                const XMVECTOR axisVec = { params.rotAxis.x, params.rotAxis.y, params.rotAxis.z };
                rotQuat = QuatRotAxis(axisVec, params.angleAroundAxis);
            }

            CreateTreesEntities(
                enttMgr,
                render,
                params.modelName,
                params.numEntts,
                params.treeScale,
                rotQuat);

            // reset some params
            params.numEntts = 0;
            params.treeScale = 1.0f;
            params.rotAxis = { 0,0,0 };
            params.angleAroundAxis = 1.0f;
            memset(params.modelName, 0, MAX_LEN_MODEL_NAME);
        }

        else if (strncmp(buf, "grass_", 6) == 0)
            ReadGrassParams(buf, params);

        else
            ReadTreesParams(buf, params);
       
    }

    // generate positions of glass instances 
    CreateGrass((float)params.grassDistVisible, params.grassCount);


    if (pFile)
        fclose(pFile);

    LogDbg(LOG, "nature initialization is finished");
}

//---------------------------------------------------------

void SaveImportedModel(const char* modelName)
{
    assert(modelName && modelName[0] != '\0');

    BasicModel& model = g_ModelMgr.GetModelByName(modelName);
    assert(model.id_ != INVALID_MODEL_ID);

    char dirPath[256];
    memset(dirPath, 0, 256);
    snprintf(dirPath, 256, "%s/", model.GetName());

    ModelExporter exporter;
    exporter.ExportIntoDE3D(&model, dirPath, model.GetName());
}

//---------------------------------------------------------

void ImportModels()
{
    TimePoint start = GetTimePoint();

    ModelsCreator creator;

    creator.ImportFromFile("data/models/ext/boblampclean/boblampclean.md5mesh");
    //creator.ImportFromFile("data/models/stalker_freedom_1/stalker_freedom_1.fbx");
    creator.ImportFromFile("data/models/ext/bm_hud/wpn_bm-16_hud.fbx");
    creator.ImportFromFile("data/models/ext/pm/wpn_pm_hud.fbx");
    creator.ImportFromFile("data/models/ext/ak_74_hud/ak_74_hud.fbx");

    const TimeDurationMs dur = GetTimePoint() - start;

    SetConsoleColor(MAGENTA);
    LogMsg("------------------------------------------------");
    LogMsg("import of models with animations took:  %f ms", dur.count());
    LogMsg("------------------------------------------------\n");
}

//---------------------------------------------------------

void SaveSkeletonAnimations(const char* skeletonName, const char* filename)
{
    assert(skeletonName && skeletonName[0] != '\0');
    assert(filename     && filename[0] != '\0');

    const AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(skeletonName);
    assert(skeleton.id_ != 0);

    AnimationSaver animSaver;
    animSaver.Save(&skeleton, filename);
}

//---------------------------------------------------------
// Desc:  1. read in path to file with skeleton and animations
//        2. load its data
//        3. store it in the animations manager
// 
// Args:  - filepath:  a path to file where we register such skeletons/animations files
//---------------------------------------------------------
void LoadAnimations(const char* filepath)
{
    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "empty filepath");
        return;
    }


    const TimePoint start = GetTimePoint();
    AnimationLoader animLoader;
    char buf[512];
    int count = 0;

    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file: %s", filepath);
        exit(0);
    }

    // check if we opened a valid file
    fgets(buf, sizeof(buf), pFile);

    if (strncmp(buf, "animations", 10) != 0)
    {
        LogErr(LOG, "invalid file for animations: %s", filepath);
        exit(0);
    }

    // read in each path to animation and load this animation
    while (!feof(pFile))
    {
        count = fscanf(pFile, "%s", buf);
        assert(count == 1);

        animLoader.Load(buf);
    }

    const TimePoint      end = GetTimePoint();
    const TimeDurationMs dur = end - start;

    LogMsg("------------------------------------------------");
    LogMsg("loading of animations took:  %f sec", dur.count() * 0.001f);
    LogMsg("------------------------------------------------\n");
}

//---------------------------------------------------------
// Desc:  manually bind skeletons and animations to some entities
//---------------------------------------------------------
void BindAnimationsToEntts(ECS::EntityMgr& enttMgr)
{
    const EntityID boblampId             = enttMgr.nameSys_.GetIdByName("boblampclean");
    const EntityID ak74hudId             = enttMgr.nameSys_.GetIdByName("wpn_ak74_hud");
    const EntityID bm16hudId             = enttMgr.nameSys_.GetIdByName("wpn_bm16_hud");
    const EntityID pmHudId               = enttMgr.nameSys_.GetIdByName("wpn_pm_hud");
    const EntityID bm16hudTestId         = enttMgr.nameSys_.GetIdByName("bm_16_hud_test");
    const EntityID ak74hudTestId         = enttMgr.nameSys_.GetIdByName("ak_74_hud_test");
    const EntityID pmHudTestId           = enttMgr.nameSys_.GetIdByName("pm_hud_test");

    // add animation component to ak74
    AnimSkeleton&        ak74hudSkeleton = g_AnimationMgr.GetSkeleton("wpn_ak74_hud");
    const AnimationID    ak74AnimId      = ak74hudSkeleton.GetAnimationIdx("wpn_ak74_hud_ogf_idle");
    const AnimationClip& ak74Anim        = ak74hudSkeleton.GetAnimation(ak74AnimId);

    enttMgr.AddAnimationComponent(ak74hudId,     ak74hudSkeleton.id_, ak74AnimId, ak74Anim.GetEndTime());
    ak74hudSkeleton.DumpAnimations();


    // add animation component to boblampclean
    AnimSkeleton&        bobSkeleton     = g_AnimationMgr.GetSkeleton("boblampclean");
    const AnimationID    bobAnimId       = bobSkeleton.GetAnimationIdx("boblampclean_0");
    const AnimationClip& bobAnim         = bobSkeleton.GetAnimation(bobAnimId);

    enttMgr.AddAnimationComponent(boblampId, bobSkeleton.id_, bobAnimId, bobAnim.GetEndTime());


    // add animation component to bm16
    AnimSkeleton& bm16hudSkeleton        = g_AnimationMgr.GetSkeleton("wpn_bm-16_hud");
    const AnimationID bm16AnimId         = bm16hudSkeleton.GetAnimationIdx("wpn_bm_16_hud_ogf_idle");
    const AnimationClip& bm16Anim        = bm16hudSkeleton.GetAnimation(bm16AnimId);

    enttMgr.AddAnimationComponent(bm16hudId,     bm16hudSkeleton.id_, bm16AnimId, bm16Anim.GetEndTime());
    enttMgr.AddAnimationComponent(bm16hudTestId, bm16hudSkeleton.id_, bm16AnimId, bm16Anim.GetEndTime());


    // add animation component to pm
    AnimSkeleton& pmSkeleton    = g_AnimationMgr.GetSkeleton("wpn_pm_hud");
    const AnimationID pmAnimId  = pmSkeleton.GetAnimationIdx("wpn_pm_hud_ogf_idle");
    const AnimationClip& pmAnim = pmSkeleton.GetAnimation(pmAnimId);

    enttMgr.AddAnimationComponent(pmHudId,     pmSkeleton.id_, pmAnimId, pmAnim.GetEndTime());
    enttMgr.AddAnimationComponent(pmHudTestId, pmSkeleton.id_, pmAnimId, pmAnim.GetEndTime());
}

//---------------------------------------------------------
// Desc:  1. load/import models
//        2. create relative entities
//---------------------------------------------------------
void LoadModelsAndEntities(
    const char* modelsFilepath,
    const char* animationsFilepath,
    const char* entitiesFilepath,
    const char* natureFilepath,
    ECS::EntityMgr& enttMgr,
    Render::CRender& render,
    const EngineConfigs& cfgs)
{
    LoadModelAssets(modelsFilepath, render);

#if 0
    TimePoint       start;
    TimePoint       end;
    TimeDurationMs  dur;

    ImportModels();

    //SaveImportedModel("ak_74_hud");
    SaveImportedModel("wpn_bm-16_hud");
    SaveImportedModel("wpn_pm_hud");
    SaveImportedModel("boblampclean");

    //--------------------------------

    start = GetTimePoint();

    SaveSkeletonAnimations("ak_74_hud",     "data/animations/wpn_ak74_hud.anim");
    SaveSkeletonAnimations("wpn_bm-16_hud", "data/animations/wpn_bm-16_hud.anim");
    SaveSkeletonAnimations("wpn_pm_hud",    "data/animations/wpn_pm_hud.anim");
    SaveSkeletonAnimations("boblampclean",  "data/animations/boblampclean.anim");

    end = GetTimePoint();
    dur = end - start;

    SetConsoleColor(MAGENTA);
    LogMsg("------------------------------------------------");
    LogMsg("saving of animations took:  %f sec", dur.count() * 0.001f);
    LogMsg("------------------------------------------------\n");

    exit(0);
#endif

    //--------------------------------
    
    LoadAnimations(animationsFilepath);

    LoadEntities(entitiesFilepath, enttMgr);
    BindAnimationsToEntts(enttMgr);


    //---------------------------------

    BasicModel& cube         = g_ModelMgr.GetModelByName("basic_cube");
    BasicModel& sphere       = g_ModelMgr.GetModelByName("basic_sphere");
    BasicModel& cylinder     = g_ModelMgr.GetModelByName("basic_cylinder");
    BasicModel& treeSpruce   = g_ModelMgr.GetModelByName("tree_spruce");
    BasicModel& treePine     = g_ModelMgr.GetModelByName("tree_pine");


    const DirectX::BoundingBox& treePineAABB   = treePine.GetModelAABB();
    const DirectX::BoundingBox& treeSpruceAABB = treeSpruce.GetModelAABB();

    //---------------------------------

    const float treePineWidth       = treePineAABB.Extents.x * 3;
    const float treePineHeight      = treePineAABB.Extents.y * 5.5f;

    const float treeSpruceWidth     = treeSpruceAABB.Extents.x * 2;
    const float treeSpruceHeight    = treeSpruceAABB.Extents.y * 2;

    const bool originAtBottom       = true;
    const float rotatePineAroundX   = -90.0f;
    const float rotateSpruceAroundX = 0.0f;

    //---------------------------------

    ModelsCreator creator;

    const ModelID    idTreePineLod1         = creator.CreateTreeLod1(treePineWidth, treePineHeight, originAtBottom, rotatePineAroundX);
    const ModelID    idTreeSpruceLod1       = creator.CreateTreeLod1(treeSpruceWidth, treeSpruceHeight, originAtBottom, rotateSpruceAroundX);

    const ModelID    idTreePineLod2         = creator.CreateTreeLod1(treePineWidth, treePineHeight, originAtBottom, rotatePineAroundX);
    const ModelID    idTreeSpruceLod2       = creator.CreateTreeLod1(treeSpruceWidth, treeSpruceHeight, originAtBottom, rotateSpruceAroundX);

    BasicModel&      treePineLod1           = g_ModelMgr.GetModelById(idTreePineLod1);
    BasicModel&      treeSpruceLod1         = g_ModelMgr.GetModelById(idTreeSpruceLod1);

    BasicModel&      treePineLod2           = g_ModelMgr.GetModelById(idTreePineLod2);
    BasicModel&      treeSpruceLod2         = g_ModelMgr.GetModelById(idTreeSpruceLod2);

    const MaterialID treePineLod1MatId      = g_MaterialMgr.GetMatIdByName("tree_pine_lod_1");
    const MaterialID treeSpruceLod1MatId    = g_MaterialMgr.GetMatIdByName("tree_spruce_lod_1");

    const MaterialID treePineLod2MatId      = g_MaterialMgr.GetMatIdByName("tree_pine_lod_2");
    const MaterialID treeSpruceLod2MatId    = g_MaterialMgr.GetMatIdByName("tree_spruce_lod_2");


    treePineLod1.SetMaterialForSubset(0, treePineLod1MatId);
    treeSpruceLod1.SetMaterialForSubset(0, treeSpruceLod1MatId);

    treePineLod2.SetMaterialForSubset(0, treePineLod2MatId);
    treeSpruceLod2.SetMaterialForSubset(0, treeSpruceLod2MatId);

    
    treeSpruce.SetLod(LOD_1, idTreeSpruceLod1);
    treeSpruce.SetLodDistance(LOD_1, 100);

    treeSpruce.SetLod(LOD_2, idTreeSpruceLod2);
    treeSpruce.SetLodDistance(LOD_2, 150);

    treePine.SetLod(LOD_1, idTreePineLod1);
    treePine.SetLodDistance(LOD_1, 50);

    treePine.SetLod(LOD_2, idTreePineLod2);
    treePine.SetLodDistance(LOD_2, 150);



    CreateNature(natureFilepath, enttMgr, render);
}

//---------------------------------------------------------
// Desc:   create and setup a terrain (type: geomipmap)
// Args:   - mgr:         ECS entities manager
//         - configPath:  a path to terrain config file
//---------------------------------------------------------
void CreateTerrainGeomip(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const char* configFilepath)
{
    if (StrHelper::IsEmpty(configFilepath))
    {
        LogErr(LOG, "LOL, your terrain config's path is empty");
        exit(0);
    }

    ModelsCreator creator;
    creator.CreateTerrain(configFilepath);

    Core::TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    Core::TerrainConfig  terrainCfg;


    // load terrain configs (we do it again since we need some params here)
    if (!terrain.LoadSetupFile(configFilepath, terrainCfg))
    {
        LogErr(LOG, "can't load terrain's configs");
        exit(-1);
    }

    // create and setup material for terrain (geomipmap)
    Material& mat   = g_MaterialMgr.AddMaterial("terrain_mat");

    const TexID texIdMap1     = g_TextureMgr.GetTexIdByName(terrainCfg.texDiffName1);
    const TexID texIdMap2     = g_TextureMgr.GetTexIdByName(terrainCfg.texDiffName2);
    const TexID texIdMap3     = g_TextureMgr.GetTexIdByName(terrainCfg.texDiffName3);
    const TexID texIdAlphaMap = g_TextureMgr.GetTexIdByName(terrainCfg.texAlphaName);

    const TexID texIdNorm0    = g_TextureMgr.GetTexIdByName(terrainCfg.texNormName0);
    const TexID texIdNorm1    = g_TextureMgr.GetTexIdByName(terrainCfg.texNormName1);
    const TexID texIdNorm2    = g_TextureMgr.GetTexIdByName(terrainCfg.texNormName2);
    const TexID texIdNorm3    = g_TextureMgr.GetTexIdByName(terrainCfg.texNormName3);

    const TexID texIdSpec0    = g_TextureMgr.GetTexIdByName(terrainCfg.texSpecName0);
    const TexID texIdSpec1    = g_TextureMgr.GetTexIdByName(terrainCfg.texSpecName1);
    const TexID texIdSpec2    = g_TextureMgr.GetTexIdByName(terrainCfg.texSpecName2);
    const TexID texIdSpec3    = g_TextureMgr.GetTexIdByName(terrainCfg.texSpecName3);

    const Vec4 amb(terrainCfg.ambient);
    const Vec4 diff(terrainCfg.diffuse);
    const Vec3 spec(terrainCfg.specular);
    const Vec4 refl(terrainCfg.reflect);

    mat.SetAmbient   (amb.r, amb.g, amb.b, amb.a);
    mat.SetDiffuse   (diff.r, diff.g, diff.b, diff.a);
    mat.SetSpecular  (spec.r, spec.g, spec.b);
    mat.SetGlossiness(terrainCfg.glossiness);
    mat.SetReflection(refl.r, refl.g, refl.b, refl.a);


    // bind maps to texture types
    // (so later we bind them to texture slots in shader)
    mat.SetTexture(eTexType(1), terrain.texture_.GetID());
    mat.SetTexture(eTexType(2), texIdMap1);
    mat.SetTexture(eTexType(3), texIdMap2);
    mat.SetTexture(eTexType(4), texIdMap3);

    mat.SetTexture(eTexType(5), texIdNorm0);
    mat.SetTexture(eTexType(6), texIdNorm1);
    mat.SetTexture(eTexType(7), texIdNorm2);
    mat.SetTexture(eTexType(8), texIdNorm3);

    mat.SetTexture(eTexType(9),  texIdSpec0);
    mat.SetTexture(eTexType(10), texIdSpec1);
    mat.SetTexture(eTexType(11), texIdSpec2);
    mat.SetTexture(eTexType(12), texIdSpec3);

    mat.SetTexture(eTexType(13), texIdAlphaMap);

    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrain.detailMap_.GetID());
    //mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrain.lightmap_.id);

    terrain.materialID_ = mat.id;

    // update constant buffer to hold terrain's material colors
    render.UpdateCbTerrainMaterial(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

    // create and setup a terrain entity
    const EntityID enttId = mgr.CreateEntity();

    // setup bounding params
    const DirectX::BoundingBox aabb = { terrain.center_, terrain.extents_ };

    mgr.AddTransformComponent(enttId);
    mgr.AddNameComponent     (enttId, "terrain_geomipmap");
    mgr.AddBoundingComponent (enttId, aabb);
    mgr.AddMaterialComponent (enttId, terrain.materialID_);
}

//---------------------------------------------------------
// Desc:   generate and setup different assets/entities
// Args:   - pDevice:  a ptr to DirectX11 device
//         - mgr:      ECS entity manager
//---------------------------------------------------------
void GenerateEntities(
    ECS::EntityMgr& mgr,
    Render::CRender& render)
{
    ID3D11Device* pDevice = render.GetDevice();
    ModelsCreator creator;

    // generate some models manually
    const MeshSphereParams    boundSphereParams(1, 8, 8);
    const MeshGeosphereParams boundGeoSphereParams(1, 1);
    const MeshSphereParams    sphereParams(0.5f, 20, 20);
    const MeshCylinderParams  cylParams(1, 0.8f, 5, 15, 1);

    const ModelID cubeID        = creator.CreateCube();
    //const ModelID boundSphereID = creator.CreateGeoSphere(pDevice, boundGeoSphereParams);
    const ModelID sphereID      = creator.CreateSphere(sphereParams);
    const ModelID cylinderID    = creator.CreateCylinder(cylParams);

    // get actual model by its ID
    BasicModel& cube        = g_ModelMgr.GetModelById(cubeID);
    BasicModel& sphere      = g_ModelMgr.GetModelById(sphereID);
    BasicModel& cylinder    = g_ModelMgr.GetModelById(cylinderID);

    // set names for some models
    g_ModelMgr.SetModelName(cube.id_, "basic_cube");
    g_ModelMgr.SetModelName(sphere.id_, "basic_sphere");
    g_ModelMgr.SetModelName(cylinder.id_, "basic_cylinder");

    // setup LODs for some models
    sphere.SetLod(LOD_1, cube.id_);
    sphere.SetLod(LOD_2, cylinder.id_);

    sphere.SetLodDistance(LOD_1, 10);
    sphere.SetLodDistance(LOD_2, 20);

    // manual setup of some models
    cube.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("box01"));
    sphere.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("gigachad"));
    cylinder.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("cat"));


    // create and setup entities with models  
    CreateSpheres(mgr, sphere);
    //CreateCylinders(mgr, cylinder);


    // create 2D sprites
    const TexID    crossTexId       = g_TextureMgr.GetTexIdByName("wpn/crosshair");
    const TexID    radiationTexId   = g_TextureMgr.GetTexIdByName("ui/mn_radiations_hard");
    const TexID    starvTexId       = g_TextureMgr.GetTexIdByName("ui/mn_starvation_hard");
    const TexID    woundTexId       = g_TextureMgr.GetTexIdByName("ui/mn_wounds_hard");

    const EntityID crossId          = mgr.CreateEntity();
    const EntityID radiationIconId  = mgr.CreateEntity();
    const EntityID starvationIconId = mgr.CreateEntity();
    const EntityID woundsIconId     = mgr.CreateEntity();


    const uint16 screenW = (uint16)render.GetD3D().GetWindowWidth();
    const uint16 screenH = (uint16)render.GetD3D().GetWindowHeight();

    const uint16 screenCX = screenW / 2;
    const uint16 screenCY = screenH / 2;

    const uint16 crossW = 48;
    const uint16 crossH = 48;

    const uint16 iconW = 64;
    const uint16 iconH = 64;

    const uint16 paddingW = 0;
    const uint16 paddingH = 10;

    uint16 left = 0; 
    uint16 top  = 0; 


    // init a "cross" sprite
    left = screenCX - crossW / 2;
    top  = screenCY - crossH / 2;

    mgr.AddNameComponent(crossId, "cross_sprite");
    mgr.AddSpriteComponent(crossId, crossTexId, left, top, crossW, crossH);


    // init a "radiation icon" sprite
    left = screenW - paddingW - iconW;
    top  = screenH - paddingH - iconH - 30;

    mgr.AddNameComponent(radiationIconId, "radiation_sprite");
    mgr.AddSpriteComponent(radiationIconId, radiationTexId, left, top, iconW, iconH);

    // init a "starvation icon" sprite
    top -= (paddingH + iconH);
    mgr.AddNameComponent(starvationIconId, "starvation_sprite");
    mgr.AddSpriteComponent(starvationIconId, starvTexId, left, top, iconW, iconH);

    // init a "wounds icon" sprite
    top -= (paddingH + iconH);
    mgr.AddNameComponent(woundsIconId, "wounds_sprite");
    mgr.AddSpriteComponent(woundsIconId, woundTexId, left + 7, top, iconW, iconH);
}

//---------------------------------------------------------
// Desc:   initialize all the entities on the scene
//---------------------------------------------------------
bool GameInitializer::InitModelEntities(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const Core::EngineConfigs* pConfigs)
{
    assert(pConfigs != nullptr);

    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("------------------------------------------------------------");
    LogMsg("                  INITIALIZATION: MODELS                    ");
    LogMsg("------------------------------------------------------------\n");
    SetConsoleColor(RESET);

    try
    {
        const char* materialsFilepath   = "data/materials.demat";
        const char* terrainCfgFilepath  = "data/terrain/terrain.cfg";
        const char* skyCfgFilepath      = "data/sky_plane.cfg";
        const char* modelsFilepath      = "data/models.demdl";
        const char* animationsFilepath  = "data/animations.cfg";
        const char* entitiesFilepath    = "data/entities.dentt";
        const char* natureFilepath      = "data/nature.cfg";
        const char* skyFilepath         = "data/sky.cfg";


        InitMaterials(materialsFilepath);
        GenerateEntities(mgr, render);
        CreateSkyBox(skyFilepath, mgr);

        // create terrain
        CreateTerrainGeomip(mgr, render, terrainCfgFilepath);

        LoadModelsAndEntities(
            modelsFilepath,
            animationsFilepath,
            entitiesFilepath,
            natureFilepath,
            mgr,
            render,
            *pConfigs);

        g_ModelMgr.GetSkyPlane().Init(skyCfgFilepath);
    }
    catch (const std::out_of_range& e)
    {
        LogErr(e.what());
        LogErr("went out of range");
        return false;
    }
    catch (const std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for some element");
        return false;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't initialize models");
        return false;
    }

    return true;
}

} // namespace Game
