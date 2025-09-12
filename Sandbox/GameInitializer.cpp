#include "pch.h"
#include "GameInitializer.h"
#include "LightEnttsInitializer.h"
#include "SetupModels.h"
#include "../Core/Engine/EngineConfigs.h"
#include "../Core/Mesh/MaterialLoader.h"
#include <inttypes.h>

using namespace Core;
using namespace Render;

using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMVECTOR = DirectX::XMVECTOR;
using XMMATRIX = DirectX::XMMATRIX;


namespace Game
{


//---------------------------------------------------------
// Desc:   init multiple material so we will use them later
//         during initialization of entities
//---------------------------------------------------------
void InitMaterials()
{
    const char* path = "data/materials.json";
    Core::MaterialLoader loader;

    loader.LoadFromFile(path);
}

//---------------------------------------------------------
// Desc:   manually create and setup some particles on the scene
//---------------------------------------------------------
void GameInitializer::InitParticles(ECS::EntityMgr& enttMgr)
{
    // INIT PARTICLES SYSTEMS
    ECS::ParticleSystem& particleSys = enttMgr.particleSystem_;
    //particleSys.LoadFromFile("data/particles/particles.cfg");


    // TEMP: manually setup material for each particles system
    const MaterialID matIdFlame = g_MaterialMgr.GetMatIdByName("flameMat");
    const MaterialID matIdFlare = g_MaterialMgr.GetMatIdByName("flareMat");
    const MaterialID matIdCat   = g_MaterialMgr.GetMatIdByName("catParticleMat");
    
    //-----------------------------------------------------

    const DirectX::XMFLOAT3 aabbCenter  = { 0,0,0 };
    const DirectX::XMFLOAT3 aabbExtents = { 0.5f, 0.5f, 0.5f };
    const DirectX::BoundingBox& aabb    = { aabbCenter, aabbExtents };


    // setup point lights for sparcles
    ECS::PointLight flamePointL;
    flamePointL.diffuse     = { 0.37f, 0.25f, 0.0f, 1.0f };
    flamePointL.ambient     = flamePointL.diffuse * 0.25f;
    flamePointL.specular    = flamePointL.diffuse;
    flamePointL.att         = { 0.1f, 0.1f, 0.005f };
    flamePointL.range       = 50;

#if 0
    // create flame entity
    const EntityID flame1EnttId = enttMgr.CreateEntity("flame1");
    enttMgr.AddTransformComponent(flame1EnttId, { 257,101,235 });
    enttMgr.AddParticleEmitterComponent(flame1EnttId);
    enttMgr.AddBoundingComponent(flame1EnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(flame1EnttId, flamePointL);

    ECS::ParticleEmitter& emitterFlame1 = particleSys.GetEmitterByEnttId(flame1EnttId);
    emitterFlame1.spawnRate  = 200;
    emitterFlame1.materialId = matIdFlame;
    emitterFlame1.life       = 1;
    emitterFlame1.color      = { 0.1f, 0.1f, 0.1f };
    emitterFlame1.size       = 2.0f;
    emitterFlame1.mass       = 0.5f;
    emitterFlame1.friction   = 0.05f;
    emitterFlame1.forces     = { 0.0f, 0.003f, 0.0f };
#endif

    // create flame entity
    const EntityID flame2EnttId = enttMgr.CreateEntity("flame2");
    enttMgr.AddTransformComponent(flame2EnttId, { 245.0f, 78.1f, 247.5f });
    enttMgr.AddParticleEmitterComponent(flame2EnttId);
    enttMgr.AddBoundingComponent(flame2EnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(flame2EnttId, flamePointL);

    ECS::ParticleEmitter& emitterFlame2 = particleSys.GetEmitterByEnttId(flame2EnttId);
    emitterFlame2.spawnRate  = 50;
    emitterFlame2.materialId = matIdFlame;
    emitterFlame2.life       = 1;
    emitterFlame2.color      = { 1, 1, 1 };
    emitterFlame2.size       = 3.0f;
    emitterFlame2.mass       = 0.5f;
    emitterFlame2.friction   = 0.08f;
    emitterFlame2.forces     = { 0.0f, 0.003f, 0.0f };

    // fire sparcles
    const EntityID sparclesEnttId = enttMgr.CreateEntity("sparcles");
    enttMgr.AddTransformComponent(sparclesEnttId, { 245.0f, 78.1f, 247.5f });
    enttMgr.AddParticleEmitterComponent(sparclesEnttId);
    enttMgr.AddBoundingComponent(sparclesEnttId, ECS::BoundingType::BOUND_BOX, aabb);


    ECS::ParticleEmitter& emitterSparcle = particleSys.GetEmitterByEnttId(sparclesEnttId);
    emitterSparcle.spawnRate    = 30;
    emitterSparcle.materialId   = matIdFlare;
    emitterSparcle.life         = 1;
    emitterSparcle.color        = { 1.0f, 0.57f, 0.0f };
    emitterSparcle.size         = 0.14f;
    emitterSparcle.mass         = 1.24f;
    emitterSparcle.friction     = 0.083f;
    emitterSparcle.forces       = { 0.0f, 0.006f, 0.0f };
}

//---------------------------------------------------------
// Desc:   init all the stuff related to the player
//---------------------------------------------------------
void GameInitializer::InitPlayer(ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr)
{
    const EntityID playerId = pEnttMgr->CreateEntity("player");

    // create and set a model for the player entity
    const MeshSphereParams sphereParams(1, 20, 20);
    ModelsCreator creator;
    const ModelID sphereID = creator.CreateSphere(pDevice, sphereParams);
    BasicModel& sphere     = g_ModelMgr.GetModelById(sphereID);

    // setup material (light properties + textures) for the player entity
    MaterialID catMatID = g_MaterialMgr.GetMatIdByName("cat");

    // setup child entities of the player
    ECS::NameSystem&      nameSys      = pEnttMgr->nameSystem_;
    ECS::HierarchySystem& hierarchySys = pEnttMgr->hierarchySystem_;
    ECS::InventorySystem& inventorySys = pEnttMgr->inventorySystem_;

    const EntityID stalkerEnttId = nameSys.GetIdByName("stalker_freedom");
    const EntityID aks74uEnttId  = nameSys.GetIdByName("player_aks74u");
    const EntityID gameCameraId  = nameSys.GetIdByName("game_camera");
    const EntityID flashlightId  = nameSys.GetIdByName("flashlight");
    const EntityID swordId       = nameSys.GetIdByName("sword");
    const EntityID obrezId       = nameSys.GetIdByName("obrez_1");
    const EntityID ak74Stalker   = nameSys.GetIdByName("player_ak_74_stalker");
    const EntityID groza         = nameSys.GetIdByName("player_groza");
    const EntityID hpsa          = nameSys.GetIdByName("player_hpsa");

    // ------------------------------------------

    pEnttMgr->AddTransformComponent(playerId, { 0,0,0 }, { 0,0,1 });
    pEnttMgr->AddModelComponent(playerId, sphere.GetID());
    pEnttMgr->AddMaterialComponent(playerId, catMatID);
    pEnttMgr->AddRenderingComponent(playerId);

    // add inventory for a player and push some stuff into it
    pEnttMgr->AddInventoryComponent(playerId);

    inventorySys.AddItem(playerId, obrezId);
    inventorySys.AddItem(playerId, aks74uEnttId);
    inventorySys.AddItem(playerId, swordId);
    inventorySys.AddItem(playerId, ak74Stalker);
    inventorySys.AddItem(playerId, groza);
    inventorySys.AddItem(playerId, hpsa);

#if 0
    const EntityID item0Id = inventorySys.GetItemByIdx(playerId, 0);
    const EntityID item1Id = inventorySys.GetItemByIdx(playerId, 1);
    const EntityID item2Id = inventorySys.GetItemByIdx(playerId, 2);
    const EntityID item3Id = inventorySys.GetItemByIdx(playerId, 3);

    const char* item0name = nameSys.GetNameById(item0Id);
    const char* item1name = nameSys.GetNameById(item1Id);
    const char* item2name = nameSys.GetNameById(item2Id);
    const char* item3name = nameSys.GetNameById(item3Id);

    printf("\n\nplayers inventory:\n");
    printf("item_0: %s\n", item0name);
    printf("item_1: %s\n", item1name);
    printf("item_2: %s\n", item2name);
    printf("item_3: %s\n", item3name);
    exit(0);
#endif

    //pEnttMgr->RemoveComponent(obrezId, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(aks74uEnttId, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(swordId, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(ak74Stalker, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(groza, ECS::RenderedComponent);
    pEnttMgr->RemoveComponent(hpsa, ECS::RenderedComponent);

    // BIND some entities to the player
    hierarchySys.AddChild(playerId, obrezId);
    hierarchySys.AddChild(playerId, aks74uEnttId);
    hierarchySys.AddChild(playerId, swordId);
    hierarchySys.AddChild(playerId, ak74Stalker);
    hierarchySys.AddChild(playerId, groza);
    hierarchySys.AddChild(playerId, hpsa);

    hierarchySys.AddChild(playerId, gameCameraId);
    hierarchySys.AddChild(playerId, flashlightId);

    pEnttMgr->AddPlayerComponent(playerId);
    pEnttMgr->AddBoundingComponent(playerId, ECS::BoundingType::BOUND_BOX, *sphere.GetSubsetsAABB());

    ECS::PlayerSystem& player = pEnttMgr->playerSystem_;

    player.SetWalkSpeed(4.0f);
    player.SetRunSpeed(8.0f);
    player.SetCurrentSpeed(1.0f);

    player.SetFreeFlyMode(false);
    player.SetActiveWeapon(obrezId);

    // HACK setup (move player)
    pEnttMgr->AddEvent(ECS::EventTranslate(playerId, 245, 80, 210));
}

///////////////////////////////////////////////////////////

bool GameInitializer::InitCamera(
    ECS::EntityMgr& mgr,
    const char* cameraName,
    CameraInitParams& initParams)
{
    // check input args
    if (!cameraName || cameraName[0] == '\0')
    {
        LogErr(LOG, "input name for a camera is empty");
        return false;
    }

    if ((initParams.wndWidth <= 0) || (initParams.wndHeight <= 0))
    {
        LogErr(LOG, "input dimensions is wrong (width or height <= 0) for camera: %s", cameraName);
        return false;
    }

    if (initParams.nearZ <= 0.001f)
    {
        LogErr(LOG, "nearZ value is wrong for camera: %s", cameraName);
        return false;
    }

    if (initParams.fovInRad <= 0.001f)
    {
        LogErr(LOG, "input field of view is wrong for camera: %s", cameraName);
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
    const DirectX::XMMATRIX& editorCamView = mgr.cameraSystem_.UpdateView(camId);

    mgr.cameraSystem_.SetBaseViewMatrix(camId, editorCamView);
    mgr.cameraSystem_.SetupOrthographicMatrix(
        camId,
        initParams.wndWidth,
        initParams.wndHeight,
        initParams.nearZ,
        initParams.farZ);

    return true;
}

///////////////////////////////////////////////////////////

inline float GetHeightOfGeneratedTerrainAtPoint(const float x, const float z)
{
    return 0.1f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

///////////////////////////////////////////////////////////

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
    for (index i = 0; i < numEntts / 2; i += 2)
    {
        positions[i + 0] = XMFLOAT3(-5.0f, 5.0f, 10.0f * i);
        positions[i + 1] = XMFLOAT3(+5.0f, 5.0f, 10.0f * i);
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
    const ECS::BoundingType boundType = ECS::BoundingType::BOUND_BOX;

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        ids,
        numEntts,
        numSubsets,
        &boundType,
        model.GetSubsetsAABB());      // AABB data (center, extents)

    // add material for each sphere entity
    const MaterialID matID      = model.meshes_.subsets_[0].materialId;
    const MaterialID catMatID   = g_MaterialMgr.GetMatIdByName("cat");
    const MaterialID brickMatId = g_MaterialMgr.GetMatIdByName("brick_01");

    // specify material for some entities
    mgr.AddMaterialComponent(enttsIDs[0], catMatID);
    mgr.AddMaterialComponent(enttsIDs[1], brickMatId);

    for (index i = 2; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], matID);

    // setup a texture transformation for the sphere
    ECS::StaticTexTransInitParams sphereTexTransform;
    sphereTexTransform.Push(DirectX::XMMatrixScaling(3, 3, 0));

    mgr.AddTextureTransformComponent(enttsIDs[0], ECS::TexTransformType::STATIC, sphereTexTransform);
    mgr.AddTextureTransformComponent(enttsIDs[1], ECS::TexTransformType::STATIC, sphereTexTransform);
}

//---------------------------------------------------------
// Desc:    load a sky params from config file,
//          create sky model, and create sky entity
//---------------------------------------------------------
void CreateSkyBox(ECS::EntityMgr& mgr)
{
    LogDbg(LOG, "create sky box entity");

    // open config file
    FILE* pFile = fopen("data/sky.cfg", "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open sky config file");
        exit(0);
    }

    // read in sky params
    char     skyTexPath[64]{'\0'};
    int      skyBoxSize = 0;
    XMFLOAT3 colorCenter;
    XMFLOAT3 colorApex;
    CubeMapInitParams cubeMapParams;
    int loadCubeMapTexture = 0;
    float skyOffsetY = 0;

    fscanf(pFile, "load_cubemap_texture: %d\n", &loadCubeMapTexture);
    fscanf(pFile, "cubemap_texture: %s\n", skyTexPath);
    fscanf(pFile, "cubemap_dir %s\n", cubeMapParams.directory);
    fscanf(pFile, "sky_pos_x %s\n", cubeMapParams.texNames[0]);
    fscanf(pFile, "sky_neg_x %s\n", cubeMapParams.texNames[1]);
    fscanf(pFile, "sky_pos_y %s\n", cubeMapParams.texNames[2]);
    fscanf(pFile, "sky_neg_y %s\n", cubeMapParams.texNames[3]);
    fscanf(pFile, "sky_pos_z %s\n", cubeMapParams.texNames[4]);
    fscanf(pFile, "sky_neg_z %s\n", cubeMapParams.texNames[5]);

    fscanf(pFile, "color_center  %f %f %f\n", &colorCenter.x, &colorCenter.y, &colorCenter.z);
    fscanf(pFile, "color_apex    %f %f %f\n", &colorApex.x, &colorApex.y, &colorApex.z);
    fscanf(pFile, "sky_box_size     %d\n",    &skyBoxSize);
    fscanf(pFile, "sky_box_offset_y %f\n",    &skyOffsetY);

    // load a texture for the sky
    TexID skyMapId = INVALID_TEXTURE_ID;

    if (loadCubeMapTexture)
    {
        skyMapId = g_TextureMgr.LoadFromFile(skyTexPath);
    }
    else
    {
        char cubeMapTexturesDir[128]{ '\0' };
        strcat(cubeMapTexturesDir, g_RelPathTexDir);
        strcat(cubeMapTexturesDir, "cubemaps/cubemap0/");
        skyMapId = g_TextureMgr.CreateCubeMap("skyCube", cubeMapParams);
    }


    // setup a material
    Material& mat = g_MaterialMgr.AddMaterial("sky_mat");
    mat.SetTexture(TEX_TYPE_DIFFUSE, skyMapId);
    mat.SetAlphaClip(false);
    mat.SetCull(MAT_PROP_CULL_FRONT);
    mat.SetFrontClockwise(MAT_PROP_FRONT_COUNTER_CLOCKWISE);
    mat.SetDepthStencil(MAT_PROP_DSS_SKY_DOME);

    // create a sky model
    Core::ModelsCreator creator;
    creator.CreateSkyCube(g_pDevice, (float)skyBoxSize);
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
}

///////////////////////////////////////////////////////////

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
    for (XMFLOAT3& pos : positions)
    {
        do
        {
            pos.x = MathHelper::RandF(0, range);
            pos.z = MathHelper::RandF(0, range);
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z) + 2.5f;

        } while (maxHeight < pos.y);   // limit height for trees
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

    // setup bounding params
    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };
    const DirectX::BoundingBox* enttAABBs = model.GetSubsetsAABB();

    // ----------------------------------------------------

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);

    // each cylinder has only one mesh
    const MaterialID brickMatID = g_MaterialMgr.GetMatIdByName("brick_01");

    for (const EntityID id : enttsIDs)
        mgr.AddMaterialComponent(id, brickMatID);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddRenderingComponent(ids, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddBoundingComponent(ids, numEntts, numSubsets, boundTypes, enttAABBs);

    ECS::StaticTexTransInitParams texTransform;
    texTransform.Push(DirectX::XMMatrixScaling(3, 3, 0));
    mgr.AddTextureTransformComponent(ids[1], ECS::TexTransformType::STATIC, texTransform);
}

///////////////////////////////////////////////////////////

void CreateCubes(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create cubes entities");

    try
    {
        constexpr int numEntts = 11;

        // create empty entities
        const EntityID enttIdCat                = mgr.CreateEntity("cat");
        const EntityID enttIdFireflame          = mgr.CreateEntity("fireflame");
        const EntityID enttIdWireFence          = mgr.CreateEntity("wireFence");
        const EntityID enttIdWoodCrate01        = mgr.CreateEntity("woodCrate01");
        const EntityID enttIdWoodCrate02        = mgr.CreateEntity("woodCrate02");
        const EntityID enttIdBox01              = mgr.CreateEntity("box01");
     

        const EntityID enttIdWoodCrateBlendAdd  = mgr.CreateEntity("boxBlendAdd");
        const EntityID enttIdWoodCrateBlendSub  = mgr.CreateEntity("boxBlendSub");
        const EntityID enttIdWoodCrateBlendMul  = mgr.CreateEntity("boxBlendMul");
        const EntityID enttIdWoodBoxTransparent = mgr.CreateEntity("boxBlendTransparent");
        const EntityID enttIdBoxMirror          = mgr.CreateEntity("boxMirror");

        // NOTE: array of entities IDs must be SORTED
        const EntityID ids[numEntts] =
        {
            enttIdCat,
            enttIdFireflame,
            enttIdWireFence,
            enttIdWoodCrate01,
            enttIdWoodCrate02,
            enttIdBox01,
            enttIdWoodCrateBlendAdd,
            enttIdWoodCrateBlendSub,
            enttIdWoodCrateBlendMul,
            enttIdWoodBoxTransparent,
            enttIdBoxMirror,
        };

        // ---------------------------------------------------------
        // prepare position/rotations/scales

        XMFLOAT3 positions[numEntts];
        XMVECTOR dirQuats[numEntts];
        float    uniformScales[numEntts];

        // setup position for each box
        positions[0]  = { 230, 80, 200 };
        positions[1]  = { 230, 80, 202 };
        positions[2]  = { 230, 80, 204 };
        positions[3]  = { 230, 80, 206 };
        positions[4]  = { 230, 80, 208 };
        positions[5]  = { 230, 80, 210 };

        positions[6]  = { 230, 82, 200 };
        positions[7]  = { 230, 82, 202 };
        positions[8]  = { 230, 82, 204 };
        positions[9]  = { 230, 82, 206 };
        positions[10] = { 230, 82, 208 };
        
        // setup direction quaternion for each box
        for (XMVECTOR& q : dirQuats)
            q = { 0,0,0,1 };

        // setup uniform scale for each box
        for (float& s : uniformScales)
            s = 1.0f;

        // ---------------------------------------------------------
        // prepare textures transformations

        ECS::AtlasAnimInitParams atlasTexAnimParams;
        ECS::RotationAroundCoordInitParams rotAroundCoordsParams;

        const int   numRows         = 15;
        const int   numCols         = 8;
        const float animDuration    = 4;
        const float rotCenterX      = 0.5f;
        const float rotCenterY      = 0.5f;
        const float rotSpeed        = 0.1f;

        atlasTexAnimParams.Push(numRows, numCols, animDuration);
        rotAroundCoordsParams.Push(rotCenterX, rotCenterY, rotSpeed);

        // ---------------------------------------------------------
        // setup the cubes entities

        mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);
        mgr.AddModelComponent(ids, model.GetID(), numEntts);

        // add a material component to each box
        mgr.AddMaterialComponent(enttIdCat,                 g_MaterialMgr.GetMatIdByName("cat"));
        mgr.AddMaterialComponent(enttIdFireflame,           g_MaterialMgr.GetMatIdByName("firecamp"));
        mgr.AddMaterialComponent(enttIdBox01,               g_MaterialMgr.GetMatIdByName("box01"));

        mgr.AddMaterialComponent(enttIdWireFence,           g_MaterialMgr.GetMatIdByName("wirefence"));
        mgr.AddMaterialComponent(enttIdWoodCrate01,         g_MaterialMgr.GetMatIdByName("wood_crate_1"));
        mgr.AddMaterialComponent(enttIdWoodCrate02,         g_MaterialMgr.GetMatIdByName("wood_crate_2"));

        mgr.AddMaterialComponent(enttIdWoodCrateBlendAdd,   g_MaterialMgr.GetMatIdByName("brick_blend_add"));
        mgr.AddMaterialComponent(enttIdWoodCrateBlendSub,   g_MaterialMgr.GetMatIdByName("brick_blend_sub"));
        mgr.AddMaterialComponent(enttIdWoodCrateBlendMul,   g_MaterialMgr.GetMatIdByName("brick_blend_mul"));
        mgr.AddMaterialComponent(enttIdWoodBoxTransparent,  g_MaterialMgr.GetMatIdByName("brick_blend_transparent"));
        mgr.AddMaterialComponent(enttIdBoxMirror,           g_MaterialMgr.GetMatIdByName("mirror"));

        // ------------------------------------------

        // add some texture transformations (animations) for some cubes
        mgr.AddTextureTransformComponent(
            enttIdFireflame,
            ECS::TexTransformType::ATLAS_ANIMATION,
            atlasTexAnimParams);

        mgr.AddTextureTransformComponent(
            enttIdCat,
            ECS::TexTransformType::ROTATION_AROUND_TEX_COORD,
            rotAroundCoordsParams);

        mgr.AddRenderingComponent(ids, numEntts);


        // add bounding component to each entity
        const size numSubsets = 1;                    // each cube has only one mesh
        const ECS::BoundingType boundType = ECS::BoundingType::BOUND_BOX;

        mgr.AddBoundingComponent(
            ids,
            numEntts,
            numSubsets,
            &boundType,
            model.GetSubsetsAABB());      // AABB data (center, extents)
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't create and setup cubes entts");
    }
}

//---------------------------------------------------------
// Desc:  create and setup terrain (geomipmap) entity
// Args:  - mgr:     entity manager from ECS module
//        - terrain: our terrain obj
//---------------------------------------------------------
void CreateTerrain(ECS::EntityMgr& mgr, const Core::TerrainGeomip& terrain)
{
    LogDbg(LOG, "create terrain geomipmap");

    // create and setup a terrain entity
    const EntityID enttID = mgr.CreateEntity();

    // setup bounding params
    constexpr size             numEntts = 1;
    const ECS::BoundingType    boundType = ECS::BoundingType::BOUND_BOX;
    const DirectX::BoundingBox aabb = { terrain.center_, terrain.extents_ };

    // setup material params
    const MaterialID terrainMatID = terrain.materialID_;

    mgr.AddTransformComponent(enttID);
    mgr.AddNameComponent(enttID, "terrain_geomipmap");
    mgr.AddBoundingComponent(enttID, boundType, aabb);
    mgr.AddMaterialComponent(enttID, terrain.materialID_);

    LogDbg(LOG, "Terrain (geomipmap) is created");
}

///////////////////////////////////////////////////////////

void CreateSkull(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a skull entity

    LogDbg(LOG, "create skull");

    try
    {
        const EntityID enttID = mgr.CreateEntity();


        mgr.AddTransformComponent(enttID, { -15, 4, 10 });
        mgr.AddNameComponent(enttID, "skull");
        mgr.AddModelComponent(enttID, model.GetID());
        mgr.AddRenderingComponent(enttID);

        const size numEntts = 1;
        const size numSubsets = 1;                    // each cylinder has only one mesh
        const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

        // add bounding component to each entity
        mgr.AddBoundingComponent(
            &enttID,
            numEntts,
            numSubsets,
            boundTypes,
            model.GetSubsetsAABB());      // AABB data (center, extents)
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't create a skull model");
    }
}

///////////////////////////////////////////////////////////

void CreateTreesPine(ECS::EntityMgr& mgr, Render::CRender& render)
{
    LogDbg(LOG, "create tree pine entities");

    // load a model from file
    ModelsCreator creator;
    const char* pathModel = "data/models/ext/trees/FBX format/tree_pine.fbx";
    const ModelID modelId = creator.ImportFromFile(g_pDevice, pathModel);
    BasicModel& model = g_ModelMgr.GetModelById(modelId);
    SetupTreePine(model, render);


    constexpr size numEntts          = 100;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids              = enttsIDs.data();

    // ---------------------------------------------

    std::string names[numEntts];
    XMFLOAT3    positions[numEntts];
    XMVECTOR    directions[numEntts];
    float       uniScales[numEntts];


    // generate a name for each tree
    for (int i = 0; std::string& name : names)
    {
        char tmpName[32]{'\0'};
        sprintf(tmpName, "tree_pine_%ld", ids[i++]);
        name = tmpName;
    }

    // we will set heights according to the terrain's landscape
    TerrainGeomip& terrain  = g_ModelMgr.GetTerrainGeomip();
    const float range       = (float)terrain.heightMap_.GetWidth();
    const float maxHeight   = 150;// terrain.tiles_.regions[HIGHEST_TILE].lowHeight;

    // generate positions
    for (XMFLOAT3& pos : positions)
    {
        do
        {
            pos.x = MathHelper::RandF(0, range);
            pos.z = MathHelper::RandF(0, range);
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z) - 0.3f;

        } while (maxHeight < pos.y);   // limit height for trees
    }

    // generate directions
    for (index i = 0; i < numEntts; ++i)
    {
        float angleY  = MathHelper::RandF(0, 314) * 0.01f;
        directions[i] = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PIDIV2);
    }

    // generate a scale value for each tree
    for (index i = 0; i < numEntts; ++i)
        uniScales[i] = 0.01f;


    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniScales);
    const DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpacesByQuat(ids, numEntts, rotQuat);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts);


    // add bounding component to each entity
    constexpr size numSubsets = 3;               // tree pine model has 3 meshes
    const ECS::BoundingType boundTypes[numSubsets] =
    {
        ECS::BoundingType::BOUND_BOX,
        ECS::BoundingType::BOUND_BOX,
        ECS::BoundingType::BOUND_BOX
    };
    
    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());


    // add material component to each tree
    const MeshGeometry::Subset* subsets      = model.meshes_.subsets_;
    const MaterialID materialIds[numSubsets] =
    {
        subsets[0].materialId,
        subsets[1].materialId,
        subsets[2].materialId
    };

    for (index i = 0; i < numEntts; ++i)
        mgr.AddMaterialComponent(ids[i], materialIds, numSubsets);
}

///////////////////////////////////////////////////////////

void CreateTreesSpruce(ECS::EntityMgr& mgr, Render::CRender& render)
{
    LogDbg(LOG, "create tree spruce entities");

    // load a model from file
    ModelsCreator creator;
    const char*   pathTreeSpruce = "data/models/ext/trees/tree_spruce/tree_spruce.obj";
    const ModelID treeSpruceID   = creator.ImportFromFile(g_pDevice, pathTreeSpruce);
    BasicModel&   model          = g_ModelMgr.GetModelById(treeSpruceID);
    SetupTreeSpruce(model, render);

    constexpr int           numEntts = 100;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID*         ids      = enttsIDs.data();

    // ---------------------------------------------

    std::string names[numEntts];
    XMFLOAT3    positions[numEntts];
    XMVECTOR    directions[numEntts];
    float       uniScales[numEntts];

    // generate a name for each tree
    for (int i = 0; std::string& name : names)
    {
        name = { "tree_spruce_" + std::to_string(enttsIDs[i++]) };
    }

    // we will set heights according to the terrain's landscape
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float range      = (float)terrain.GetTerrainLength();
    const float maxHeight  = 150;

    // generate positions
    for (XMFLOAT3& pos : positions)
    {
        do
        {
            pos.x = MathHelper::RandF(0, range);
            pos.z = MathHelper::RandF(0, range);
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z) - 0.3f;

        } while (maxHeight < pos.y);   // limit height for trees
    }

    // generate direction quats
    for (int i = 0; i < numEntts; ++i)
    {
        directions[i] = { 0,0,0,1 };
       // float angleY = MathHelper::RandF(0, 314) * 0.01f;
        //directions[i] = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PIDIV2);
    }

    // generate a scale value for each tree
    for (int i = 0; i < numEntts; ++i)
    {
        uniScales[i] = 3.5f + MathHelper::RandF(0.0f, 50.0f) * 0.01f;
    }

    // setup bound data
    const size numSubsets = 3;
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniScales);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts);

    const MaterialID matId = model.meshes_.subsets_[0].materialId;

    for (int i = 0; i < numEntts; ++i)
    {
        mgr.AddMaterialComponent(ids[i], matId);
    }

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreatePowerLine(ECS::EntityMgr& mgr, BasicModel& model)
{
    LogDbg(LOG, "create power line entities");

    // setup a material for input model
    model.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("power_line"));

    // create and setup an entity
    constexpr int numEntts = 3;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();
   

    // setup transformation data for each entity
    XMFLOAT3 positions[numEntts] =
    {
        { -200, 0, 8 },
        { 0,    0, 8 },
        { +200, 0, 8 },
    };

    for (XMFLOAT3& pos : positions)
        pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z) + 2.0f;

    const XMVECTOR directions[numEntts] =
    {
        {0,0,1},
        {0,0,1},
        {0,0,1}
    };

    const float uniformScales[numEntts] = { 0.5f, 0.5f, 0.5f };

    // generate a name for each entt
    std::string names[numEntts];

    for (int i = 0; const EntityID id : enttsIDs)
        names[i++] = "power_hw_tower_" + std::to_string(id);

    // setup bounding params
    const size numSubsets = model.GetNumSubsets();
    const cvector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniformScales);

    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    for (const EntityID id : enttsIDs)
        mgr.transformSystem_.RotateLocalSpaceByQuat(id, rotQuat);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts);

    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    // ----------------------------------------------------

    const MaterialID matID = model.meshes_.subsets_[0].materialId;

    for (const EntityID enttID : enttsIDs)
        mgr.AddMaterialComponent(enttID, matID);
}

///////////////////////////////////////////////////////////

void CreateRadar(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a radar entity");

    const EntityID enttID = mgr.CreateEntity("stalker_radar");

    // setup transformation params
    XMFLOAT3 position = { 7, 20, -10 };
    XMVECTOR direction = { 0, 1, 0, 0 };
    const float uniformScale = 5.0f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialId;


    // add components
    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets);

    // rotate the stalker entity
    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);
}

///////////////////////////////////////////////////////////

void CreateStalkerFreedom(ECS::EntityMgr& mgr, Render::CRender& render)
{
    LogDbg(LOG, "create a stalker entity");

    // import stalker model from file
    ModelsCreator creator;
    const char* pathStalkerFreedom = "data/models/ext/stalker_freedom_1/stalker_freedom_1.fbx";
    const ModelID stalkerFreedomId = creator.ImportFromFile(g_pDevice, pathStalkerFreedom);
    BasicModel& model              = g_ModelMgr.GetModelById(stalkerFreedomId);
    SetupStalkerFreedom(model, render);

    // create and setup entity
    const EntityID enttId = mgr.CreateEntity("stalker_freedom");

    // setup transformation params
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float posX = 280;
    const float posZ = 230;
    const float posY = terrain.GetScaledHeightAtPoint((int)posX, (int)posZ);
    XMFLOAT3 position        = { posX, posY, posZ };
    XMVECTOR direction       = { 0, 1, 0, 0 };
    const float uniformScale = 2.0f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialId;


    // add components
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.AddModelComponent    (enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);

    mgr.AddBoundingComponent(
        &enttId,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttId, materialIDs.data(), numSubsets);

    // rotate the stalker entity
    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, rotQuat);
}

///////////////////////////////////////////////////////////

void CreateStalkerHouse(ECS::EntityMgr& mgr, BasicModel& model)
{
    // import a model from file
    ModelsCreator creator;
    
   
   

    const EntityID enttID = mgr.CreateEntity("stalker_house");

    // setup transformation params
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float posX = 261;
    const float posZ = 249;
    const float posY = terrain.GetScaledHeightAtPoint((int)posX, (int)posZ);
    XMFLOAT3 position = { posX, posY + 4.8f, posZ };
    XMVECTOR direction = { 0, 1, 0, 0 };
    const float uniformScale = 2.0f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialId;


    // add components
    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets);


    const XMVECTOR q = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, q);
}

//---------------------------------------------------------
// Desc:   create and init a new building entity based on input model
//---------------------------------------------------------
void CreateBuildingEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    const float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char name[MAX_LENGTH_ENTT_NAME]{ '\0' };

    if (!inName || inName[0] == '\0')
    {
        // model_name + entt_id
        snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, model.GetName(), enttId);
    }
    else
    {
        strncpy(name, inName, MAX_LENGTH_ENTT_NAME);

        // if input name isn't unique we append to it an ID of entity
        if (!mgr.nameSystem_.IsUnique(name))
            snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, inName, enttId);
    }

    // print debug msg
    LogDbg(LOG, "create %s entity", name);


    // check some input args
    if (uniformScale <= 0)
    {
        LogErr(LOG, "can't create a building entity: %s\n\t"
            "because input scale can't be <= 0 (input scale: %f)",
            name, uniformScale);

        //mgr.DeleteEntt(enttId);
        return;
    }


    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup materials ids
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;


    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);


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


    // add bounding component
    if (numSubsets > 1)
    {
        const DirectX::BoundingBox* subsetsAABBs = model.GetSubsetsAABB();
        cvector<ECS::BoundingType> boundTypes(numSubsets);

        for (ECS::BoundingType& type : boundTypes)
            type = ECS::BoundingType::BOUND_BOX;

        // add bounding box for each subset (mesh) of the entity
        mgr.AddBoundingComponent(&enttId, numEntts, numSubsets, boundTypes.data(), subsetsAABBs);
    }

    // this entity has single mesh so add a bounding box of this size
    else
    {
        mgr.AddBoundingComponent(enttId, ECS::BoundingType::BOUND_BOX, model.GetModelAABB());
    }


    // rotate entity around itself if necessary
    if (!(rotQuat == XMVECTOR{ 0, 0, 0, 1 }))
    {
        mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, rotQuat);
    }
}

//---------------------------------------------------------
// Desc:   create and init a new vehicle entity based on input model
//---------------------------------------------------------
void CreateVehicleEntt(
ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    const float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId   = mgr.CreateEntity();

    // setup a name
    char name[MAX_LENGTH_ENTT_NAME]{'\0'};

    if (!inName || inName[0] == '\0')
    {
        // model_name + entt_id
        snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, model.GetName(), enttId);
    }
    else
    {
        strncpy(name, inName, MAX_LENGTH_ENTT_NAME);

        // if input name isn't unique we append to it an ID of entity
        if (!mgr.nameSystem_.IsUnique(name))
            snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, inName, enttId);
    }

    // print debug msg
    LogDbg(LOG, "create %s entity", name);


    // check some input args
    if (uniformScale <= 0)
    {
        LogErr(LOG, "can't create a vehicle entity: %s\n\t"
                    "because input scale can't be <= 0 (input scale: %f)",
                    name, uniformScale);

        //mgr.DeleteEntt(enttId);
        return;
    }


    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup materials ids
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);


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


    // add bounding component
    if (numSubsets > 1)
    {
        const DirectX::BoundingBox* subsetsAABBs = model.GetSubsetsAABB();
        cvector<ECS::BoundingType> boundTypes(numSubsets);

        for (ECS::BoundingType& type : boundTypes)
            type = ECS::BoundingType::BOUND_BOX;

        // add bounding box for each subset (mesh) of the entity
        mgr.AddBoundingComponent(&enttId, numEntts, numSubsets, boundTypes.data(), subsetsAABBs);
    }

    // this entity has single mesh so add a bounding box of this size
    else
    {
        mgr.AddBoundingComponent(enttId, ECS::BoundingType::BOUND_BOX, model.GetModelAABB());
    }


    // rotate entity around itself if necessary
    if (!(rotQuat == XMVECTOR{ 0, 0, 0, 1 }))
    {
        mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, rotQuat);
    }
}

//---------------------------------------------------------
// Desc:   create and init a new weapon entity based on input model
//---------------------------------------------------------
void CreateWeaponEntt(
    ECS::EntityMgr& mgr,
    const BasicModel& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    const float uniformScale,
    const char* inName = nullptr)
{
    using namespace DirectX;

    const EntityID enttId   = mgr.CreateEntity();

    // setup a name
    char name[MAX_LENGTH_ENTT_NAME]{'\0'};

    if (!inName || inName[0] == '\0')
    {
        // model_name + entt_id
        snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, model.GetName(), enttId);
    }
    else
    {
        strncpy(name, inName, MAX_LENGTH_ENTT_NAME);

        // if input name isn't unique we append to it an ID of entity
        if (!mgr.nameSystem_.IsUnique(name))
            snprintf(name, MAX_LENGTH_ENTT_NAME, "%s_%" PRIu32, inName, enttId);
    }

    // print debug msg
    LogDbg(LOG, "create %s entity", name);


    // check some input args
    if (uniformScale <= 0)
    {
        LogErr(LOG, "can't create a weapon entity: %s\n\t"
                    "because input scale can't be <= 0 (input scale: %f)",
                    name, uniformScale);

        //mgr.DeleteEntt(enttId);
        return;
    }


    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup materials ids
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    


    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, uniformScale);
    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);


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


    // add bounding component
    if (numSubsets > 1)
    {
        const DirectX::BoundingBox* subsetsAABBs = model.GetSubsetsAABB();
        cvector<ECS::BoundingType> boundTypes(numSubsets);

        for (ECS::BoundingType& type : boundTypes)
            type = ECS::BoundingType::BOUND_BOX;

        // add bounding box for each subset (mesh) of the entity
        mgr.AddBoundingComponent(&enttId, numEntts, numSubsets, boundTypes.data(), subsetsAABBs);
    }

    // this entity has single mesh so add a bounding box of this size
    else
    {
        mgr.AddBoundingComponent(enttId, ECS::BoundingType::BOUND_BOX, model.GetModelAABB());
    }


    // rotate entity around itself if necessary
    if (!(rotQuat == XMVECTOR{ 0, 0, 0, 1 }))
    {
        mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, rotQuat);
    }
}

///////////////////////////////////////////////////////////

void CreateCastleTower(ECS::EntityMgr& mgr)
{
    // load a castle model from file
    ModelsCreator creator;
    const char* pathCastleTower = "data/models/ext/castle-tower/fougeres gate.obj";
    const ModelID castleTowerID = creator.ImportFromFile(g_pDevice, pathCastleTower);
    BasicModel& model           = g_ModelMgr.GetModelById(castleTowerID);

    // setup a material of the castle tower
    const MaterialID matId = model.meshes_.subsets_[0].materialId;
    Material& mat = g_MaterialMgr.GetMatById(matId);
    mat.SetSpecular(0.2f, 0.2f, 0.2f);
    mat.SetSpecularPower(15.0f);
    mat.SetReflection(0,0,0,0);
    

    // compute the terrain's middle point
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float terrainSize = (float)terrain.heightMap_.GetWidth();
    const float posX = terrainSize / 2;
    const float posZ = terrainSize / 2;
    const float posY = terrain.GetScaledHeightAtPoint((int)posX, (int)posZ) - 7;

    // setup transformation params
    const XMFLOAT3 position = { posX, posY, posZ };
    const XMVECTOR dirQuat = { 0, 0, 1, 0 };
    const float uniformScale = 5.0f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    // create and setup a castle tower entity
    const EntityID enttID = mgr.CreateEntity();

    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2 - 0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, q1);

    mgr.AddNameComponent(enttID, "castle_tower");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, &matId, numSubsets);

    // TEMP: fix rotation
    const DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, -DirectX::XM_PIDIV2+0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);
}

///////////////////////////////////////////////////////////

void CreateHouse(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a house entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const float posY        = GetHeightOfGeneratedTerrainAtPoint(34, 43) + 2.5f;
    const XMFLOAT3 pos      = { 34, posY, 43 };
    const XMVECTOR rotVec   = { DirectX::XM_PIDIV2, 0,0 };
    const XMVECTOR quat     = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotVec);
    const float uniScale    = 1.0f;

    // setup bounding params
    const size numEntts     = 1;
    const size numSubsets   = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    mgr.AddTransformComponent(enttID, pos, quat, uniScale);
    mgr.AddNameComponent(enttID, "kordon_house");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateHouse2(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a house entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const float posY = GetHeightOfGeneratedTerrainAtPoint(-20, 107);
    const XMFLOAT3 pos = { -20, posY, 107 };
    const XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, DirectX::XM_PIDIV2, 0);

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    mgr.AddTransformComponent(enttID, pos, quat, 0.01f);
    mgr.AddNameComponent(enttID, "blockpost");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

//---------------------------------------------------------
// print into the console information about the duration of the whole
// process of importing models from the external formats
//---------------------------------------------------------
void PrintImportTimingInfo()
{
    const double factor = (1.0 / ModelImporter::s_ImportDuration_) * 100.0;

    SetConsoleColor(GREEN);

    LogMsg("-------------------------------------------------\n");
    LogMsg("Summary about import process:");
    LogMsg("time spent to import:        %.3f ms (100 %%)",  ModelImporter::s_ImportDuration_);
    LogMsg("time spent to load scene:    %.3f ms (%.2f %%)", ModelImporter::s_SceneLoading_, ModelImporter::s_SceneLoading_ * factor);
    LogMsg("time spent to load nodes:    %.3f ms (%.2f %%)", ModelImporter::s_NodesLoading_, ModelImporter::s_NodesLoading_ * factor);

    LogMsg("time spent to load vertices: %.3f ms (%.2f %%)", ModelImporter::s_VerticesLoading_, ModelImporter::s_VerticesLoading_ * factor);
    LogMsg("time spent to load textures: %.3f ms (%.2f %%)", ModelImporter::s_TexLoadingDuration_, ModelImporter::s_TexLoadingDuration_ * factor);
    LogMsg("-------------------------------------------------\n");

    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:   generate random params for grass and fill in a vertex buffer with this data
//---------------------------------------------------------
void CreateGrass()
{
    constexpr int numGrass       = 1000000;
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float terrainSize      = (float)terrain.GetTerrainLength()-5;

    cvector<VertexGrass>& grassVertices = g_ModelMgr.GetGrassVertices();
    grassVertices.resize(numGrass);

    for (VertexGrass& grass : grassVertices)
    {
        grass.pos.x  = MathHelper::RandF(0, terrainSize);
        grass.pos.z  = MathHelper::RandF(0, terrainSize);
        grass.pos.y  = terrain.GetScaledInterpolatedHeightAtPoint(grass.pos.x, grass.pos.z);

        grass.size.x = MathHelper::RandF(0.5f, 2.5f);
        grass.size.y = grass.size.x;
    }

    VertexBuffer<VertexGrass>& vb = g_ModelMgr.GetGrassVB();
    vb.UpdateDynamic(g_pContext, grassVertices.data(), grassVertices.size());
}

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
// Desc:  1. import models from different external formats (.obj, .blend, .fbx, etc.)
//        2. create relative entities
//---------------------------------------------------------
void ImportExternalModels(
    ID3D11Device* pDevice,
    ECS::EntityMgr& mgr,
    Render::CRender& render)
{
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    ModelsCreator creator;

#if 1
    // paths to weapon models
    const char* pathAks74u              = "data/models/ext/weapon/ak_74u/ak_74u.fbx";
    const char* pathSword               = "data/models/ext/weapon/sword/sword.obj";
    const char* pathObrez               = "data/models/ext/weapon/obrez_bm16/bm16.fbx";
    const char* pathAk74Stalker         = "data/models/ext/weapon/ak_74_stalker/ak_74.fbx";
    const char* pathGroza               = "data/models/ext/weapon/groza/groza.fbx";
    const char* pathHpsa                = "data/models/ext/weapon/hpsa/hpsa.fbx";

    // paths to vehicle models
    const char* pathBtr80               = "data/models/ext/vehicle/btr_80/btr.fbx";
    const char* pathGaz2101             = "data/models/ext/vehicle/gaz_2101/2101.fbx";
    const char* pathGazM24              = "data/models/ext/vehicle/gaz_m24/gaz.fbx";
    const char* pathPaz3201             = "data/models/ext/vehicle/paz_3201/paz3201.fbx";
    const char* pathTraktorMTZ          = "data/models/ext/vehicle/traktor_mtz/tractor_mtz_01.fbx";
    const char* pathZaz968              = "data/models/ext/vehicle/zaz_968m/zaz968m.fbx";
    const char* pathZil131              = "data/models/ext/vehicle/zil_131/1.fbx";
    const char* pathBtr                 = "data/models/ext/vehicle/veh_btr_br/veh_btr_br.fbx";
    const char* pathTraktor             = "data/models/ext/vehicle/tr13/tr13.fbx";

    // paths to building models
    const char* pathMilitaryBlockpost   = "data/models/ext/building/military-blockpost-stalker/source/Millitary_blockpost.fbx";
    const char* pathStalkerHouse        = "data/models/ext/building/stalker-house/source/SmallHouse.fbx";
    const char* pathMilitaryHouse       = "data/models/ext/building/abandoned-military-house/source/BigBuilding.fbx";


    // import weapon models
    const ModelID modelIdAks74u         = creator.ImportFromFile(g_pDevice, pathAks74u);
    const ModelID swordId               = creator.ImportFromFile(g_pDevice, pathSword);
    const ModelID obrezId               = creator.ImportFromFile(g_pDevice, pathObrez);
    const ModelID ak74StalkerId         = creator.ImportFromFile(g_pDevice, pathAk74Stalker);
    const ModelID grozaId               = creator.ImportFromFile(g_pDevice, pathGroza);
    const ModelID hpsaId                = creator.ImportFromFile(g_pDevice, pathHpsa);

    // import vehicle models
    const ModelID btr80Id               = creator.ImportFromFile(g_pDevice, pathBtr80);
    const ModelID gaz2101Id             = creator.ImportFromFile(g_pDevice, pathGaz2101);
    const ModelID gazM24Id              = creator.ImportFromFile(g_pDevice, pathGazM24);
    const ModelID paz3201Id             = creator.ImportFromFile(g_pDevice, pathPaz3201);
    const ModelID traktorMtzId          = creator.ImportFromFile(g_pDevice, pathTraktorMTZ);
    const ModelID zaz968Id              = creator.ImportFromFile(g_pDevice, pathZaz968);
    const ModelID zil131Id              = creator.ImportFromFile(g_pDevice, pathZil131);
    const ModelID btrId                 = creator.ImportFromFile(g_pDevice, pathBtr);
    const ModelID traktorId             = creator.ImportFromFile(g_pDevice, pathTraktor);

    // import building models
    //const ModelID militaryBlockpostId   = creator.ImportFromFile(g_pDevice, pathMilitaryBlockpost);
    const ModelID stalkerHouseId        = creator.ImportFromFile(g_pDevice, pathStalkerHouse);
    const ModelID militaryHouseId       = creator.ImportFromFile(g_pDevice, pathMilitaryHouse);


    BasicModel& aks74u                  = g_ModelMgr.GetModelById(modelIdAks74u);
    BasicModel& cube                    = g_ModelMgr.GetModelByName("basic_cube");
    BasicModel& sword                   = g_ModelMgr.GetModelById(swordId);
    BasicModel& obrez                   = g_ModelMgr.GetModelById(obrezId);
    BasicModel& ak74Stalker             = g_ModelMgr.GetModelById(ak74StalkerId);
    BasicModel& groza                   = g_ModelMgr.GetModelById(grozaId);
    BasicModel& hpsa                    = g_ModelMgr.GetModelById(hpsaId);

    BasicModel& btr80                   = g_ModelMgr.GetModelById(btr80Id);
    BasicModel& gaz2101                 = g_ModelMgr.GetModelById(gaz2101Id);
    BasicModel& gazM24                  = g_ModelMgr.GetModelById(gazM24Id);
    BasicModel& paz3201                 = g_ModelMgr.GetModelById(paz3201Id);
    BasicModel& traktorMtz              = g_ModelMgr.GetModelById(traktorMtzId);
    BasicModel& zaz968                  = g_ModelMgr.GetModelById(zaz968Id);
    BasicModel& zil131                  = g_ModelMgr.GetModelById(zil131Id);
    BasicModel& btr                     = g_ModelMgr.GetModelById(btrId);
    BasicModel& traktorTr13             = g_ModelMgr.GetModelById(traktorId);

    //BasicModel& militaryBlockpost       = g_ModelMgr.GetModelById(militaryBlockpostId);
    BasicModel& stalkerHouse            = g_ModelMgr.GetModelById(stalkerHouseId);
    BasicModel& militaryHouse           = g_ModelMgr.GetModelById(militaryHouseId);


    // manually setup some models (mostly its materials)
    SetupAks74u(aks74u, render);
    SetupSword(sword, render);
    SetupObrez(obrez, render);
    SetupAk74Stalker(ak74Stalker, render);
    SetupGroza(groza, render);
    SetupHpsa(hpsa, render);

    //SetupDefault(btr80);
    //SetupDefault(gaz2101);
    //SetupDefault(gazM24);
    //SetupDefault(paz3201);
    //SetupDefault(traktorMtz);
    SetupPaz3201(paz3201, render);
    SetupZaz968(zaz968, render);
    SetupZil131(zil131, render);
    SetupBtr(btr, render);
    SetupTraktor(traktorTr13, render);

    //SetupBuildingMilitaryBlockpost(militaryBlockpost);
    SetupStalkerSmallHouse(stalkerHouse, render);
    SetupMilitaryHouse(militaryHouse, render);

    //-------------------------------------------

    constexpr float pi      = DirectX::XM_PI;
    constexpr float piDiv2  = DirectX::XM_PIDIV2;
    constexpr float piDiv4  = DirectX::XM_PIDIV4;
    constexpr float piDiv16 = DirectX::XM_PIDIV4 * 0.25f;

    const XMVECTOR xAxis = { 1,0,0 };
    const XMVECTOR yAxis = { 0,1,0 };
    const XMVECTOR zAxis = { 0,0,1 };

    const XMVECTOR dirDefault    = XMVECTOR{ 0,0,1,0 };
    const XMVECTOR rotQuatZero   = XMVECTOR{ 0,0,0,1 };

    const XMVECTOR rotQuatAks74u        = QuatRotAxis(xAxis, piDiv2-0.1f);
    const XMVECTOR rotQuatAk74          = QuatRotAxis(xAxis, piDiv2-0.1f);
    const XMVECTOR rotQuatSword         = QuatRotAxis(yAxis, piDiv2-0.1f);
    const XMVECTOR rotQuatObrez         = QuatRotAxis(xAxis, piDiv2);
    const XMVECTOR rotQuatZaz968        = QuatMul(QuatRotAxis(xAxis, -0.07f), QuatRotAxis(yAxis, piDiv4), QuatRotAxis(zAxis, 0.07f));
    const XMVECTOR rotQuatZil131        = QuatMul(QuatRotAxis(zAxis, -0.25f), QuatRotAxis(xAxis, -0.08f));
    const XMVECTOR rotQuatPaz3201       = QuatMul(QuatRotAxis(xAxis, -0.1f), QuatRotAxis(yAxis, -0.6f), QuatRotAxis(zAxis, -0.1f));
    const XMVECTOR rotQuatBlockPost     = QuatRotAxis(xAxis, piDiv2);
    const XMVECTOR rotQuatStalkerHouse  = QuatRotAxis(xAxis, piDiv2);
    const XMVECTOR rotQuatStalkerBtr    = QuatMul(QuatRotAxis(xAxis, piDiv2), QuatRotAxis(zAxis, piDiv16-0.05f));
    const XMVECTOR rotQuatTraktorTr13   = QuatMul(QuatRotAxis(xAxis, piDiv2), QuatRotAxis(yAxis, pi+piDiv4));


    // create player's weapons
    CreateWeaponEntt(mgr, aks74u, { 0.7f, -1.35f, 0.96f }, dirDefault, rotQuatAks74u, 3.5f, "player_aks74u");

    // create aks-74u entities
    CreateWeaponEntt(mgr, aks74u, { 250, 80, 205 }, dirDefault, rotQuatAks74u, 3.5f);
    CreateWeaponEntt(mgr, aks74u, { 245, 80, 205 }, dirDefault, rotQuatAks74u, 3.5f);
    CreateWeaponEntt(mgr, aks74u, { 240, 80, 205 }, dirDefault, rotQuatAks74u, 3.5f);

    // create a sword entity
    CreateWeaponEntt(mgr, sword, { 0.7f,-0.4f,1.1f }, dirDefault, rotQuatSword, 0.015f, "sword");

    // create some stalker weapon entities
    CreateWeaponEntt(mgr, obrez, { 0.3f,-0.4f,0.5f }, dirDefault, rotQuatObrez, 2.0f, "obrez_1");
    CreateWeaponEntt(mgr, obrez, { 240, 83, 210 },    dirDefault, rotQuatObrez, 2.0f, "obrez_2");

    CreateWeaponEntt(mgr, ak74Stalker, { 0.50f, -0.35f, 0.51f }, dirDefault, rotQuatObrez, 2.0f, "player_ak_74_stalker");
    CreateWeaponEntt(mgr, groza,       { 0.50f, -0.39f, 0.63f }, dirDefault, rotQuatObrez, 2.0f, "player_groza");
    CreateWeaponEntt(mgr, hpsa,        { 0.63f, -0.35f, 0.77f }, dirDefault, rotQuatObrez, 2.0f, "player_hpsa");

    CreateWeaponEntt(mgr, ak74Stalker, { 240, 83, 215 }, dirDefault, rotQuatObrez, 2.0f, "ak_74_stalker");
    CreateWeaponEntt(mgr, groza,       { 240, 83, 220 }, dirDefault, rotQuatObrez, 2.0f, "groza");
    CreateWeaponEntt(mgr, hpsa,        { 245, 83, 215 }, dirDefault, rotQuatObrez, 2.0f, "hpsa");



    //CreateVehicleEntt(mgr, btr80, { 240, 80, 200 }, dirDefault, rotQuatZero, 1.0f, "btr_80");
    //CreateVehicleEntt(mgr, gaz2101, { 240, 80, 200 }, dirDefault, rotQuatZero, 1.0f, "btr_80");
    //CreateVehicleEntt(mgr, gazM24, { 240, 80, 200 }, dirDefault, rotQuatZero, 1.0f, "btr_80");
    CreateVehicleEntt(mgr, paz3201, { 236.7f, 73.1f, 186 }, dirDefault, rotQuatPaz3201, 2.0f, "paz_3201");
    //CreateVehicleEntt(mgr, traktorMtz, { 240, 80, 200 }, dirDefault, rotQuatZero, 1.0f, "btr_80");
    CreateVehicleEntt(mgr, zaz968, { 228.9f, 75.2f, 201.0f }, dirDefault, rotQuatZaz968, 2.0f, "zaz_968");
    CreateVehicleEntt(mgr, zil131, { 289.2f, 76.8f, 222.9f }, dirDefault, rotQuatZero, 2.0f, "zil_131");
    CreateVehicleEntt(mgr, btr,    { 234.5f, 76.1f, 241.0f }, dirDefault, rotQuatStalkerBtr, 3.0f, "stalker_btr");
    CreateVehicleEntt(mgr, traktorTr13, { 261, 76.8f, 220 }, dirDefault, rotQuatTraktorTr13, 3.0f, "traktor_tr13");

    //CreateBuildingEntt(mgr, militaryBlockpost, { 280, 80, 220 },  dirDefault, rotQuatBlockPost, 0.01f, "military_blockpost");
    CreateBuildingEntt(mgr, stalkerHouse,  { 284, 81.5f, 244 },     dirDefault, rotQuatStalkerHouse, 2.0f, "stalker_house");
    CreateBuildingEntt(mgr, militaryHouse, { 261, 76.81f, 275.1f }, dirDefault, rotQuatStalkerHouse, 0.027f, "military_house");
#endif
    CreateCubes(mgr, g_ModelMgr.GetModelByName("basic_cube"));
    CreateTreesSpruce(mgr, render);
    //CreateTreesPine(mgr);
    CreateGrass();

    //CreateStalkerFreedom(mgr);

#if 0
    // BIND some entities to the traktor
    const EntityID enttIdTraktorTr13       = mgr.nameSystem_.GetIdByName("traktor_tr13");
    const EntityID enttIdTraktorSpotlightL = mgr.nameSystem_.GetIdByName("traktor_spotlight_L");
    const EntityID enttIdTraktorSpotlightR = mgr.nameSystem_.GetIdByName("traktor_spotlight_R");
    mgr.hierarchySystem_.AddChild(enttIdTraktorTr13, enttIdTraktorSpotlightL);
    mgr.hierarchySystem_.AddChild(enttIdTraktorTr13, enttIdTraktorSpotlightR);
#endif
}

//---------------------------------------------------------
// Desc:   create and setup a terrain (type: geomipmap)
// Args:   - mgr:         ECS entities manager
//         - configPath:  a path to terrain config file
//---------------------------------------------------------
void CreateTerrainGeomip(ECS::EntityMgr& mgr, const char* configPath)
{
    if (!configPath || configPath[0] == '\0')
    {
        LogErr(LOG, "LOL, your config path is wrong");
        exit(0);
    }

    ModelsCreator creator;
    const char* terrainConfigPath      = "data/terrain/terrain.cfg";
    creator.CreateTerrain(terrainConfigPath);

    Core::TerrainGeomip& terrainGeomip = g_ModelMgr.GetTerrainGeomip();

    // create and setup material for terrain (geomipmap)
    Material& mat   = g_MaterialMgr.AddMaterial("terrain_mat_geomip");
    TexID texIdNorm = g_TextureMgr.LoadFromFile("data/terrain/dirt01n.dds");

    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    mat.SetTexture(TEX_TYPE_DIFFUSE,           terrainGeomip.texture_.GetID());
    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrainGeomip.detailMap_.GetID());
    mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrainGeomip.lightmap_.id);
    mat.SetTexture(TEX_TYPE_NORMALS,           texIdNorm);

    terrainGeomip.materialID_ = mat.id;

    // create a terrain entity
    CreateTerrain(mgr, terrainGeomip);
}

//---------------------------------------------------------
// Desc:   generate and setup different assets/entities
// Args:   - pDevice:  a ptr to DirectX11 device
//         - mgr:      ECS entity manager
//---------------------------------------------------------
void GenerateEntities(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    ModelsCreator creator;

    // create terrain
    const char* terrainConfigPath = "data/terrain/terrain.cfg";
    CreateTerrainGeomip(mgr, terrainConfigPath);
    //CreateTerrainQuadtree(mgr, terrainConfigPath);


    // generate some models manually
    const MeshSphereParams    boundSphereParams(1, 8, 8);
    const MeshGeosphereParams boundGeoSphereParams(1, 1);
    const MeshSphereParams    sphereParams(0.5f, 20, 20);
    const MeshCylinderParams  cylParams(1, 0.8f, 5, 15, 1);

    const ModelID cubeID        = creator.CreateCube(pDevice);
    const ModelID boundSphereID = creator.CreateGeoSphere(pDevice, boundGeoSphereParams);
    const ModelID sphereID      = creator.CreateSphere(pDevice, sphereParams);
    const ModelID cylinderID    = creator.CreateCylinder(pDevice, cylParams);

    // get actual model by its ID
    BasicModel& cube        = g_ModelMgr.GetModelById(cubeID);
    BasicModel& sphere      = g_ModelMgr.GetModelById(sphereID);
    BasicModel& boundSphere = g_ModelMgr.GetModelById(boundSphereID);
    BasicModel& cylinder    = g_ModelMgr.GetModelById(cylinderID);

    // set names for some models
    cube.SetName("basic_cube");
    sphere.SetName("basic_sphere");
    boundSphere.SetName("bound_sphere");
    cylinder.SetName("basic_cylinder");

    // manual setup of some models
    cube.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("box01"));
    sphere.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("gigachad"));

    // create and setup entities with models
    CreateSkyBox(mgr);
    
    //CreateSpheres(mgr, sphere);
    //CreateCylinders(mgr, cylinder);
}

//---------------------------------------------------------
// Desc:   initialize all the entities on the scene
//---------------------------------------------------------
bool GameInitializer::InitModelEntities(
    ID3D11Device* pDevice,
    ECS::EntityMgr& mgr,
    Render::CRender& render)
{
    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("------------------------------------------------------------");
    LogMsg("                  INITIALIZATION: MODELS                    ");
    LogMsg("------------------------------------------------------------\n");
    SetConsoleColor(RESET);

    try
    {
        Core::ModelsCreator creator;

        // load the "no_texture" texture from the file;
        // (this texture will serve us as "invalid")
        const TexID noTexID = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "notexture.dds");

        const Material& invalidMat = g_MaterialMgr.GetMatById(INVALID_MATERIAL_ID);

        // create a cube which will serve for us as an invalid model
        const ModelID cubeID = creator.CreateCube(pDevice);
        BasicModel& invalidModel = g_ModelMgr.GetModelById(cubeID);

        invalidModel.SetName("invalid_model");
        invalidModel.SetMaterialForSubset(0, invalidMat.id);

        // NOTE: the bounding line box model must be created first of all, before all the other models
        const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);

        InitMaterials();
        GenerateEntities(pDevice, mgr);
        ImportExternalModels(pDevice, mgr, render);

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

//---------------------------------------------------------
// Desc:   initialize all the light sources (entities) on the scene
// Args:   - mgr:   ECS entities manger
//---------------------------------------------------------
bool GameInitializer::InitLightSources(ECS::EntityMgr& mgr)
{
    InitDirectedLightEntities(mgr);
    InitPointLightEntities(mgr);
    InitSpotLightEntities(mgr);

    return true;
}

} // namespace Game
