#include "pch.h"
#include "GameInitializer.h"
#include "LightEnttsInitializer.h"
#include "SetupModels.h"
#include "../Core/Engine/EngineConfigs.h"

using namespace Core;
using namespace Render;

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
    const TexID texIdGray           = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "gray.png");
    const TexID texIdBlankNorm      = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "blank_NRM.dds");
    const TexID texIdCat            = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "cat.dds");
    const TexID texIdFireAtlas      = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "fire_atlas.dds");
    const TexID texIdWireFence      = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "wireFence.dds");
    const TexID texIdWoodCrate1     = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "woodCrate01.dds");
    const TexID texIdWoodCrate2     = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "woodCrate02.dds");
    const TexID texIdBox01d         = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "box01d.dds");
    const TexID texIdBox01n         = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "box01n.dds");
    const TexID texIdBrickDiff      = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "brick01d.dds");
    const TexID texIdBrickNorm      = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "brick01n.dds");
    const TexID texIdPerlinNoise    = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "perlin_noise.png");
    //const TexID texIdBigPoleDiff    = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "power_line/bigpoleiron_co.png");
    //const TexID texIdBigPoleNorm    = g_TextureMgr.LoadFromFile(g_RelPathExtModelsDir, "power_line/bigpoleiron_nohq.png");
    //const TexID texIdGigachad       = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "gigachad.dds");

    // load some textures for particles
    
    const TexID texIdFlare          = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "flare.png");
    const TexID texIdFlame0         = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "flame0.dds");


    // create material for flame (fire) particles
    Material& flameParticleMat = g_MaterialMgr.AddMaterial("flameMat");
    flameParticleMat.SetTexture(TEX_TYPE_DIFFUSE, texIdFlame0);
    flameParticleMat.SetAlphaClip(true);
    flameParticleMat.SetFill(MAT_PROP_FILL_SOLID);
    flameParticleMat.SetCull(MAT_PROP_CULL_NONE);
    flameParticleMat.SetBlending(MAT_PROP_BLEND_ENABLE);
    flameParticleMat.SetDepthStencil(MAT_PROP_DEPTH_DISABLED);

    // create a material for flare particles
    Material& flareParticleMat = g_MaterialMgr.AddMaterial("flareMat");
    flareParticleMat.SetTexture(TEX_TYPE_DIFFUSE, texIdFlare);
    flareParticleMat.SetAlphaClip(true);
    flareParticleMat.SetFill(MAT_PROP_FILL_SOLID);
    flareParticleMat.SetCull(MAT_PROP_CULL_NONE);
    flareParticleMat.SetBlending(MAT_PROP_BLEND_ENABLE);
    flareParticleMat.SetDepthStencil(MAT_PROP_DEPTH_DISABLED);

    // cat
    Material& catMat = g_MaterialMgr.AddMaterial("cat");
    catMat.SetTexture(TEX_TYPE_DIFFUSE, texIdCat);
    catMat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);

    // firecamp animated
    Material& firecampMat = g_MaterialMgr.AddMaterial("firecamp");
    firecampMat.SetTexture(TEX_TYPE_DIFFUSE, texIdFireAtlas);
    firecampMat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);

    // wirefence with alpha clipping
    Material& wirefenceMat = g_MaterialMgr.AddMaterial("wirefence");
    wirefenceMat.SetAlphaClip(true);
    wirefenceMat.SetTexture(TEX_TYPE_DIFFUSE, texIdWireFence);
    wirefenceMat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    wirefenceMat.SetCull(MAT_PROP_CULL_NONE);

    // wood crate 1
    Material& woodCrate1Mat = g_MaterialMgr.AddMaterial("wood_crate_1");
    woodCrate1Mat.SetTexture(TEX_TYPE_DIFFUSE, texIdWoodCrate1);
    woodCrate1Mat.SetTexture(TEX_TYPE_NORMALS, texIdBox01n);

    // wood crate 2
    Material& woodCrate2Mat = g_MaterialMgr.AddMaterial("wood_crate_2");
    woodCrate2Mat.SetTexture(TEX_TYPE_DIFFUSE, texIdWoodCrate2);
    woodCrate2Mat.SetTexture(TEX_TYPE_NORMALS, texIdBox01n);

    // box01
    Material& box01Mat = g_MaterialMgr.AddMaterial("box01");
    box01Mat.SetTexture(TEX_TYPE_DIFFUSE, texIdBox01d);
    box01Mat.SetTexture(TEX_TYPE_NORMALS, texIdBox01n);
    box01Mat.SetSpecular(0.140f, 0.140f, 0.140f);
    box01Mat.SetSpecularPower(0.4f);

    // brick
    Material& brickMat = g_MaterialMgr.AddMaterial("brick_01");
    brickMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBrickDiff);
    brickMat.SetTexture(TEX_TYPE_NORMALS, texIdBrickNorm);
    brickMat.SetAmbient(0.4f, 0.4f, 0.4f, 1.0f);
    brickMat.SetReflection(0, 0, 0, 0);

    // box blending add
    Material& brickBlendAddMat = g_MaterialMgr.AddMaterial("brick_blend_add");
    brickBlendAddMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBrickDiff);
    brickBlendAddMat.SetTexture(TEX_TYPE_NORMALS, texIdBrickNorm);
    brickBlendAddMat.SetBlending(MAT_PROP_ADDING);

    // box brick blending subtract
    Material& brickBlendSubMat = g_MaterialMgr.AddMaterial("brick_blend_sub");
    brickBlendSubMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBrickDiff);
    brickBlendSubMat.SetTexture(TEX_TYPE_NORMALS, texIdBrickNorm);
    brickBlendSubMat.SetBlending(MAT_PROP_SUBTRACTING);

    // box brick blending multiply
    Material& brickBlendMulMat = g_MaterialMgr.AddMaterial("brick_blend_mul");
    brickBlendMulMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBrickDiff);
    brickBlendMulMat.SetTexture(TEX_TYPE_NORMALS, texIdBrickNorm);
    brickBlendMulMat.SetBlending(MAT_PROP_MULTIPLYING);

    // box brick blending transparent
    Material& boxBlendTransparentMat = g_MaterialMgr.AddMaterial("brick_blend_transparent");
    boxBlendTransparentMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBrickDiff);
    boxBlendTransparentMat.SetTexture(TEX_TYPE_NORMALS, texIdBrickNorm);
    boxBlendTransparentMat.SetBlending(MAT_PROP_TRANSPARENCY);

    // mirror box
    Material& boxMirrorMat = g_MaterialMgr.AddMaterial("mirror");
    boxMirrorMat.SetTexture(TEX_TYPE_DIFFUSE, texIdGray);
    boxMirrorMat.SetTexture(TEX_TYPE_NORMALS, texIdBlankNorm);
    boxMirrorMat.reflect = { 1,1,1,1 };

#if 0
    // power line
    Material powerLineMat = g_MaterialMgr.AddMaterial("power_line");
    powerLineMat.SetTexture(TEX_TYPE_DIFFUSE, texIdBigPoleDiff);
    powerLineMat.SetTexture(TEX_TYPE_NORMALS, texIdBigPoleNorm);
    powerLineMat.ambient = { 0.4f, 0.4f, 0.4f, 1.0f };
    powerLineMat.specular = { 0.3f, 0.3f, 0.3f, 1.0f };

    // gigachad
    Material gigachadMat = g_MaterialMgr.AddMaterial("gigachad");
    gigachadMat.SetTexture(TEX_TYPE_DIFFUSE, texIdGigachad);
#endif
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
    flamePointL.diffuse     = { 0.8f, 0.6f, 0.05f, 1.0f };
    flamePointL.ambient     = flamePointL.diffuse * 0.25f;
    flamePointL.specular    = flamePointL.diffuse;
    flamePointL.att         = { 0.1f, 0.1f, 0.005f };
    flamePointL.range       = 50;

    ECS::PointLight sparclesPointL;
    sparclesPointL.diffuse  = { 0.0f, 0.8f, 0.05f, 1.0f };
    sparclesPointL.ambient  = sparclesPointL.diffuse * 0.25f;
    sparclesPointL.specular = sparclesPointL.diffuse;
    sparclesPointL.att      = { 0, 0.1f, 0.005f };
    sparclesPointL.range    = 50;


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


    // create flame entity
    const EntityID flame2EnttId = enttMgr.CreateEntity("flame2");
    enttMgr.AddTransformComponent(flame2EnttId, { 240,77,215 });
    enttMgr.AddParticleEmitterComponent(flame2EnttId);
    enttMgr.AddBoundingComponent(flame2EnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(flame2EnttId, flamePointL);

    ECS::ParticleEmitter& emitterFlame2 = particleSys.GetEmitterByEnttId(flame2EnttId);
    emitterFlame2.spawnRate  = 50;
    emitterFlame2.materialId = matIdFlame;
    emitterFlame2.life       = 1;
    emitterFlame2.color      = { 1, 1, 1 };
    emitterFlame2.size       = 2.0f;
    emitterFlame2.mass       = 0.5f;
    emitterFlame2.friction   = 0.05f;
    emitterFlame2.forces     = { 0.0f, 0.003f, 0.0f };


    // create green sparcles entity
    const EntityID sparclesEnttId = enttMgr.CreateEntity("sparcles");
    enttMgr.AddTransformComponent(sparclesEnttId, { 250,77,215 });
    enttMgr.AddParticleEmitterComponent(sparclesEnttId);
    enttMgr.AddBoundingComponent(sparclesEnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(sparclesEnttId, sparclesPointL);

    ECS::ParticleEmitter& emitterSparcle = particleSys.GetEmitterByEnttId(sparclesEnttId);
    emitterSparcle.spawnRate    = 3000;
    emitterSparcle.materialId   = matIdFlare;
    emitterSparcle.life         = 1;
    emitterSparcle.color        = { 0.1f, 1.0f, 0.25f };
    emitterSparcle.size         = 0.2f;
    emitterSparcle.mass         = 1.24f;
    emitterSparcle.friction     = 0.01f;
    emitterSparcle.forces       = { 0.0f, -0.001f, 0.0f };
}

//---------------------------------------------------------
// Desc:   init all the stuff related to the player
//---------------------------------------------------------
void GameInitializer::InitPlayer(ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr)
{
    const EntityID playerID = pEnttMgr->CreateEntity("player");

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

    const EntityID stalkerEnttID = nameSys.GetIdByName("stalker_freedom");
    const EntityID aks74EnttID   = nameSys.GetIdByName("aks_74");
    const EntityID gameCameraID  = nameSys.GetIdByName("game_camera");
    const EntityID flashlightID  = nameSys.GetIdByName("flashlight");
    const EntityID swordID       = nameSys.GetIdByName("sword");

    // ------------------------------------------

    pEnttMgr->AddTransformComponent(playerID, { 0,0,0 }, { 0,0,1 });
    pEnttMgr->AddModelComponent(playerID, sphere.GetID());
    pEnttMgr->AddMaterialComponent(playerID, catMatID);
    pEnttMgr->AddRenderingComponent(playerID);

    // BIND some entities to the player
    //hierarchySys.AddChild(playerID, aks74EnttID);
    hierarchySys.AddChild(playerID, gameCameraID);
    hierarchySys.AddChild(playerID, flashlightID);

    pEnttMgr->AddPlayerComponent(playerID);
    pEnttMgr->AddBoundingComponent(playerID, ECS::BoundingType::BOUND_BOX, *sphere.GetSubsetsAABB());

    ECS::PlayerSystem& player = pEnttMgr->playerSystem_;
    player.SetWalkSpeed(1.0f);
    player.SetRunSpeed(10.0f);
    player.SetCurrentSpeed(1.0f);

    // HACK setup (move player)
    pEnttMgr->AddEvent(ECS::EventTranslate(playerID, 230, 80, 190));
    //pEnttMgr->AddEvent(ECS::EventTranslate(playerID, 0, 100, 0));
    player.SetFreeFlyMode(true);
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
    mat.SetCull(MAT_PROP_CULL_NONE);
    mat.SetFrontClockwise(MAT_PROP_FRONT_COUNTER_CLOCKWISE);
    mat.SetDepthStencil(MAT_PROP_SKY_DOME);

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

#if 0
    // setup positions: make two rows of the spheres
    for (index i = 0; i < numEntts / 2; i += 2)
    {
        float posY = 0.0f;
        XMFLOAT3& pos0 = positions[i + 0];
        XMFLOAT3& pos1 = positions[i + 1];

        pos0 = XMFLOAT3(-5.0f, 0.0f, 10.0f * i);
        pos0.y = GetHeightOfGeneratedTerrainAtPoint(pos0.x, pos0.z) + halfHeight;

        pos1 = XMFLOAT3(+5.0f, 2.0f, 10.0f * i);
        pos1.y = GetHeightOfGeneratedTerrainAtPoint(pos1.x, pos1.z) + halfHeight;
    }
#endif

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

#if 0
//---------------------------------------------------------
// Desc:  create and setup terrain (quadtree) entity
// Args:  - mgr:     entity manager from ECS module
//        - terrain: our terrain obj
//---------------------------------------------------------
void CreateTerrain(ECS::EntityMgr& mgr, const Core::TerrainQuadtree& terrain)
{
    //
    // create and setup terrain elements
    //
    LogDbg(LOG, "create terrain quadtree");

    // create and setup a terrain entity
    const EntityID enttID = mgr.CreateEntity();

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    constexpr size             numEntts = 1;
    constexpr size             numSubsets = 1;
    const ECS::BoundingType    boundType = ECS::BoundingType::BOUND_BOX;
    const DirectX::BoundingBox aabb = { terrain.GetAABBCenter(), terrain.GetAABBExtents() };

    // setup material params
    constexpr bool areMaterialsMeshBased = true;
    const MaterialID terrainMatID = terrain.GetMaterialId();


    mgr.AddTransformComponent(enttID);
    mgr.AddNameComponent(enttID, "terrain_quadtree");
    mgr.AddBoundingComponent(enttID, boundType, aabb);
    mgr.AddMaterialComponent(enttID, &terrainMatID, numSubsets, areMaterialsMeshBased);

    LogDbg(LOG, "Terrain (quadtree) is created");
}
#endif

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

void CreateTreesPine(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create tree pine entities");

    constexpr size numEntts = 100;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------

    std::string names[numEntts];
    XMFLOAT3    positions[numEntts];
    XMVECTOR    directions[numEntts];
    float       uniScales[numEntts];


    // generate a name for each tree
    for (int i = 0; std::string& name : names)
    {
        sprintf(g_String, "tree_pine_%ld", ids[i++]);
        name = g_String;
    }

    // we will set heights according to the terrain's landscape
    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float range = (float)terrain.heightMap_.GetWidth();
    const float maxHeight = 60;// terrain.tiles_.regions[HIGHEST_TILE].lowHeight;

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
        directions[i] = { 0,0,1 };

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
    constexpr bool              matIsMeshBased = true;
    const MeshGeometry::Subset* subsets        = model.meshes_.subsets_;
    const MaterialID materialIDs[numSubsets] =
    {
        subsets[0].materialId,
        subsets[1].materialId,
        subsets[2].materialId
    };

    for (index i = 0; i < numEntts; ++i)
        mgr.AddMaterialComponent(ids[i], materialIDs, numSubsets);
}

///////////////////////////////////////////////////////////

void CreateTreesSpruce(ECS::EntityMgr& mgr)
{
    LogDbg(LOG, "create tree spruce entities");

    // load a model from file
    ModelsCreator creator;
    const char*   pathTreeSpruce = "data/models/ext/trees/tree_spruce/tree_spruce.obj";
    const ModelID treeSpruceID   = creator.ImportFromFile(g_pDevice, pathTreeSpruce);
    BasicModel&   model          = g_ModelMgr.GetModelById(treeSpruceID);
    SetupTreeSpruce(model);

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
        float angleY = MathHelper::RandF(0, 314) * 0.01f;
        directions[i] = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PIDIV2);
    }

    // generate a scale value for each tree
    for (int i = 0; i < numEntts; ++i)
    {
        uniScales[i] = 3.5f + MathHelper::RandF(0.0f, 50.0f) * 0.01f;
    }

    const MaterialID matId = model.meshes_.subsets_[0].materialId;
    const int numSubmeshes = 1;

    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniScales);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts);

    for (int i = 0; i < numEntts; ++i)
    {
        mgr.AddMaterialComponent(ids[i], matId);
    }
  

    //const DirectX::XMMATRIX world = mgr.transformSystem_.GetWorldMatrixOfEntt(enttI)
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

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

void CreateStalkerFreedom(ECS::EntityMgr& mgr)
{
    LogDbg(LOG, "create a stalker entity");

    // import stalker model from file
    ModelsCreator creator;
    const char* pathStalkerFreedom = "data/models/ext/stalker_freedom_1/stalker_freedom_1.fbx";
    const ModelID stalkerFreedomId = creator.ImportFromFile(g_pDevice, pathStalkerFreedom);
    BasicModel& model              = g_ModelMgr.GetModelById(stalkerFreedomId);
    SetupStalkerFreedom(model);

    // create and setup entity
    const EntityID enttId = mgr.CreateEntity("stalker_freedom");

    // setup transformation params
    XMFLOAT3 position        = { 270, 80, 200 };
    XMVECTOR direction       = { 0, 1, 0, 0 };
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

void CreateTraktor13(ECS::EntityMgr& mgr)
{
    LogDbg(LOG, "create a traktor (tr-13) entity");

    // import stalker model from file
    ModelsCreator creator;
    const char* pathTraktor = "data/models/ext/tr13/tr13.fbx";
    const ModelID traktorId = creator.ImportFromFile(g_pDevice, pathTraktor);
    BasicModel& model       = g_ModelMgr.GetModelById(traktorId);
    SetupTraktor(model);
    
    const EntityID enttID = mgr.CreateEntity("traktor_13");

    // setup transformation params
    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    const float posX         = 261;
    const float posZ         = 249;
    const float posY         = terrain.GetScaledHeightAtPoint((int)posX, (int)posZ);
    XMFLOAT3 position        = { posX, posY, posZ };
    XMVECTOR direction       = { 0, 1, 0, 0 };
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
    mgr.AddModelComponent    (enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets);

    // rotate the stalker entity
    const XMVECTOR rotQuat1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    const XMVECTOR rotQuat2 = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PI + DirectX::XM_PIDIV4);
    const XMVECTOR q = DirectX::XMQuaternionMultiply(rotQuat1, rotQuat2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, q);
}

///////////////////////////////////////////////////////////

void CreateNanoSuit(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create nanosuit entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    float posY = GetHeightOfGeneratedTerrainAtPoint(20, 8);
    DirectX::XMFLOAT3 pos = { 20, posY, 8 };
    DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(0, DirectX::XM_PIDIV2, 0);

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, rotQuat, 0.5f);
    mgr.AddNameComponent(enttID, model.GetName());
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

void CreateBuilding(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a building entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    float posY = GetHeightOfGeneratedTerrainAtPoint(-130, 70);
    const DirectX::XMFLOAT3 pos = { -130, posY, 70 };
    const DirectX::XMVECTOR dirQuat = { 0,0,0,1 };
    const float uniformScale = 2.5f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "building");
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

void CreateSovietStatue(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a soviet statue");
    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 pos          = { -50, 1.3f, 60 };
    const XMVECTOR dirQuat      = { 0,0,0,1 };
    const float uniformScale    = 5.0f;

    // add bounding params
    const size numEntts         = 1;
    const size numSubsets       = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "soviet_statue");
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

void CreateApartment(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create an appartment entity");
    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 pos       = { -50, 0, 0 };
    const XMVECTOR dirQuat   = { 0,0,0,1 };
    const float uniformScale = 0.1f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "apartment");
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

void CreateAks(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create ak47 entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = { 10, 2, 10 };
    const XMVECTOR direction = { 0, 0, 1, 0 };
    const float uniformScale = 5.0f;

    // setup bounding params
    constexpr size numEntts = 1;
    constexpr size numSubsets = 4;                // model of ak-47 has 4 meshes
    ECS::BoundingType boundTypes[numSubsets];

    for (index i = 0; i < numSubsets; ++i)
        boundTypes[i] = ECS::BoundingType::BOUND_BOX;

    // prepare materials IDs for the material component
    MaterialID materialsIDs[numSubsets]{ 0 };

    for (index i = 0; i < numSubsets; ++i)
        materialsIDs[i] = model.meshes_.subsets_[i].materialId;

    // ----------------------------------------------------

    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddNameComponent(enttID, "ak_47");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());      // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialsIDs, numSubsets);
}

///////////////////////////////////////////////////////////

void CreateSword(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create sword entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = { 0.7f,-0.4f,1.1f };
    const XMVECTOR dirQuat   = { 0, 0, 1, 0 };
    const float uniformScale = 0.015f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup transformation
    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PIDIV2 - 0.1f);
    const XMVECTOR q2 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, 0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, DirectX::XMQuaternionMultiply(q1, q2));

    mgr.AddNameComponent(enttID, "sword");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)

    // setup a sword material and add a material component to entity
    const MaterialID matId = model.meshes_.subsets_[0].materialId;
    Material& mat          = g_MaterialMgr.GetMatById(matId);
    mat.SetName("sword_mat");
    mat.SetSpecular(0.5f, 0.5f, 0.5f);
    mat.SetReflection(0, 0, 0, 0);
    mgr.AddMaterialComponent(enttID, &matId, numSubsets);
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

//---------------------------------------------------------
// Desc:   create an entity with aks-74u model
//---------------------------------------------------------
void CreateAks74u(ECS::EntityMgr& mgr, const BasicModel& model, const XMFLOAT3& pos)
{
    LogDbg(LOG, "create aks-74u entity");

    const EntityID enttId = mgr.CreateEntity();

    if (enttId == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "can't create an aks74u entity");
        return;
    }

    // setup transformation params
    const XMFLOAT3 position  = pos;
    const XMVECTOR dirQuat   = { 0, 0, 1, 0 };
    const float uniformScale = 3.5f;
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2 - 0.1f);

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup name
    char name[32]{'\0'};
    snprintf(name, 32, "aks_74u_%u", enttId);

    // setup materials Ids
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    const MaterialID matId = subsets[0].materialId;


    mgr.AddTransformComponent(enttId, position, dirQuat, uniformScale);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, q1);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddBoundingComponent(enttId, ECS::BoundingType::BOUND_BOX, model.GetModelAABB());
    mgr.AddMaterialComponent(enttId, matId);
}

//---------------------------------------------------------
// Desc:   create an entity with ak-74 model
//---------------------------------------------------------
void CreateAk74(ECS::EntityMgr& mgr, const BasicModel& model, const XMFLOAT3& pos)
{
    LogDbg(LOG, "create ak-74 entity");

    const EntityID enttId = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = pos;
    const XMVECTOR dirQuat   = { 0, 0, 1, 0 };
    const float uniformScale = 4.0f;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();

    // setup name
    char name[32]{ '\0' };
    snprintf(name, 32, "aks_74_%u", enttId);

    // setup materials Ids
    const MeshGeometry::Subset* subsets = model.meshes_.subsets_;
    const MaterialID matsIds[4] =
    {
        subsets[0].materialId,
        subsets[1].materialId,
        subsets[2].materialId,
        subsets[3].materialId,
    };

    mgr.AddTransformComponent(enttId, position, dirQuat, uniformScale);

    mgr.AddModelComponent(enttId, model.GetID());
    mgr.AddRenderingComponent(enttId);
    mgr.AddBoundingComponent(enttId, ECS::BoundingType::BOUND_BOX, model.GetModelAABB());
    mgr.AddMaterialComponent(enttId, matsIds, numSubsets);

    // rotate entity
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2 - 0.1f);
    const XMVECTOR q2 = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, 0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttId, DirectX::XMQuaternionMultiply(q1, q2));

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

///////////////////////////////////////////////////////////

#define CREATE_SWORD 1
#define CREATE_CASTLE 1
#define CREATE_TREES 1

void ImportExternalModels(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    // 1. import models from different external formats (.obj, .blend, .fbx, etc.)
    // 2. create relative entities

    ModelsCreator creator;
#if 0
    // import a model (AKS-74u) from file
    const char* pathAks74u      = "data/models/ext/ak_74u/ak_74u.fbx";
    const char* pathAk74        = "data/models/ext/ak_74/scene.gltf";

    const ModelID modelIdAks74u = creator.ImportFromFile(g_pDevice, pathAks74u);
    const ModelID modelIdAk74   = creator.ImportFromFile(g_pDevice, pathAk74);

    BasicModel& aks74u          = g_ModelMgr.GetModelById(modelIdAks74u);
    BasicModel& ak74            = g_ModelMgr.GetModelById(modelIdAk74);
    BasicModel& cube            = g_ModelMgr.GetModelByName("basic_cube");

    SetupAks74u(aks74u);
    SetupAk74(ak74);

    //-------------------------------------------

    CreateAks74u(mgr, aks74u, { 255, 80, 205 });
    CreateAks74u(mgr, aks74u, { 250, 80, 205 });
    CreateAks74u(mgr, aks74u, { 245, 80, 205 });
    CreateAks74u(mgr, aks74u, { 240, 80, 205 });

    CreateAk74(mgr, ak74, { 260, 80, 205 });
    CreateAk74(mgr, ak74, { 260, 82, 205 });

    CreateStalkerFreedom(mgr);
    
    //CreateCubes(mgr, cube);
    CreateTreesSpruce(mgr);
#endif
    CreateTraktor13(mgr);
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
    Material& mat = g_MaterialMgr.AddMaterial("terrain_mat_geomip");
    TexID texIdNorm = g_TextureMgr.LoadFromFile("data/terrain/dirt01n.dds");

    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    mat.SetTexture(TEX_TYPE_DIFFUSE,           terrainGeomip.texture_.GetID());
    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrainGeomip.detailMap_.GetID());
    mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrainGeomip.lightmap_.id);
    mat.SetTexture(TEX_TYPE_NORMALS,           texIdNorm);
    //terrainMat.SetTexture(TEX_TYPE_NORMALS, terrain.normalMap_.GetID());

    terrainGeomip.materialID_ = mat.id;

    // create a terrain entity
    CreateTerrain(mgr, terrainGeomip);
}

#if 0
//---------------------------------------------------------
// Desc:   create and setup a terrain (type: quadtreen)
// Args:   - mgr:         ECS entities manager
//         - configPath:  a path to terrain config file
//---------------------------------------------------------
void CreateTerrainQuadtree(ECS::EntityMgr& mgr, const char* configPath)
{
    if (!configPath || configPath[0] == '\0')
    {
        LogErr(LOG, "LOL, your config path is wrong");
        exit(0);
    }

    ModelsCreator creator;
    
    creator.CreateTerrain(configPath);
    Core::TerrainQuadtree& terrainQuadtree = g_ModelMgr.GetTerrainQuadtree();

    // create and setup material for terrain (quadtree)
    Material& mat = g_MaterialMgr.AddMaterial("terrain_mat_quadtree");
    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    mat.SetTexture(TEX_TYPE_DIFFUSE,           terrainQuadtree.texture_.GetID());
    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrainQuadtree.detailMap_.GetID());
    mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrainQuadtree.lightmap_.id);

    // create a terrain entity
    CreateTerrain(mgr, terrainQuadtree);
}
#endif

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

///////////////////////////////////////////////////////////

void LoadTreesBillboardsTextures()
{
    constexpr int numTreeTexturesPaths = 4;

    // create a texture 2D array for trees billboards
    std::string treeTexPaths[numTreeTexturesPaths] =
    {
            std::string(g_RelPathTexDir) + "tree_pine_diffuse_512.dds",
            std::string(g_RelPathTexDir) + "tree1.dds",
            std::string(g_RelPathTexDir) + "tree2.dds",
            std::string(g_RelPathTexDir) + "tree3.dds"
    };

    g_TextureMgr.LoadTextureArray(
        "tree_billboard",
        treeTexPaths,
        numTreeTexturesPaths,
        DXGI_FORMAT_R8G8B8A8_UNORM);
}

//---------------------------------------------------------
// Desc:   initialize all the entities on the scene
//---------------------------------------------------------
bool GameInitializer::InitModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
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

        // create and setup an "invalid" material
        Material& invalidMat = g_MaterialMgr.AddMaterial("invalid");
        invalidMat.SetTexture(TEX_TYPE_DIFFUSE, noTexID);

        // create a cube which will serve for us as an invalid model
        const ModelID cubeID = creator.CreateCube(pDevice);
        BasicModel& invalidModel = g_ModelMgr.GetModelById(cubeID);

        invalidModel.SetName("invalid_model");
        invalidModel.SetMaterialForSubset(0, invalidMat.id);

        // NOTE: the bounding line box model must be created first of all, before all the other models
        const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);

        //LoadTreesBillboardsTextures();
        InitMaterials();
        GenerateEntities(pDevice, mgr);
        ImportExternalModels(pDevice, mgr);

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
