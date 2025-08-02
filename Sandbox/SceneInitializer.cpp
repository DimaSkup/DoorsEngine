#include "pch.h"
#include "SceneInitializer.h"
#include "LightEnttsInitializer.h"
#include "SetupModels.h"
#include "../Core/Engine/EngineConfigs.h"

using namespace Core;

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMVECTOR = DirectX::XMVECTOR;
using XMMATRIX = DirectX::XMMATRIX;


namespace Game
{

bool SceneInitializer::Initialize(
    ID3D11Device* pDevice,
    ECS::EntityMgr& enttMgr,
    const CameraInitParams& editorCamParams,
    const CameraInitParams& gameCamParams)
{
    LogMsg("scene initialization (start)");

    bool result = false;

    // create and init scene elements
    if (!InitModelEntities(pDevice, enttMgr))
    {
        LogErr("can't initialize models");
    }

    // init all the light source on the scene
    if (!InitLightSources(enttMgr))
    {
        LogErr("can't initialize light sources");
    }

    // init all the cameras
    if (!InitCameras(enttMgr, editorCamParams, gameCamParams))
    {
        LogErr("can't initialize cameras");
    }

    // create and setup a player entity
    InitPlayer(pDevice, &enttMgr);

    LogMsg("is initialized");

    return true;
}

///////////////////////////////////////////////////////////

void SceneInitializer::InitPlayer(ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr)
{
    // create and setup the player's entity

    const EntityID playerID = pEnttMgr->CreateEntity("player");
    pEnttMgr->AddTransformComponent(playerID, { 0,0,0 }, { 0,0,1 });

    // ------------------------------------------

    // create and set a model for the player entity
    const MeshSphereParams sphereParams(1, 20, 20);
    ModelsCreator creator;
    const ModelID sphereID = creator.CreateSphere(pDevice, sphereParams);
    BasicModel& sphere = g_ModelMgr.GetModelByID(sphereID);

    pEnttMgr->AddModelComponent(playerID, sphere.GetID());

    // setup material (light properties + textures) for the player entity
    MaterialID catMatID = g_MaterialMgr.GetMaterialIdByName("cat");
    pEnttMgr->AddMaterialComponent(playerID, &catMatID, 1, false);

    // ------------------------------------------

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    pEnttMgr->AddRenderingComponent(playerID, renderParams);

    // ----------------------------------------------------

    ECS::NameSystem&      nameSys      = pEnttMgr->nameSystem_;
    ECS::HierarchySystem& hierarchySys = pEnttMgr->hierarchySystem_;

    const EntityID stalkerEnttID = nameSys.GetIdByName("stalker_freedom");
    const EntityID ak47EnttID    = nameSys.GetIdByName("ak_47");
    const EntityID gameCameraID  = nameSys.GetIdByName("game_camera");
    const EntityID ak74ID        = nameSys.GetIdByName("ak_74");
    const EntityID flashlightID  = nameSys.GetIdByName("flashlight");
    const EntityID swordID       = nameSys.GetIdByName("sword");

    // BIND some entities to the player
    hierarchySys.AddChild(playerID, swordID);
    hierarchySys.AddChild(playerID, gameCameraID);
    hierarchySys.AddChild(playerID, flashlightID);


    // ------------------------------------------

    pEnttMgr->AddPlayerComponent(playerID);
    pEnttMgr->AddBoundingComponent(playerID, ECS::BoundingType::BOUND_BOX, *sphere.GetSubsetsAABB());

    ECS::PlayerSystem& player = pEnttMgr->playerSystem_;
    player.SetWalkSpeed(5.0f);
    player.SetRunSpeed(20.0f);
    player.SetCurrentSpeed(5.0f);

    // HACK setup
    pEnttMgr->AddEvent(ECS::EventTranslate(playerID, 270, 90, 190));
    player.SetFreeFlyMode(true);
}

///////////////////////////////////////////////////////////

bool SceneInitializer::InitCameras(
    ECS::EntityMgr& enttMgr,
    const CameraInitParams& editorCamParams,
    const CameraInitParams& gameCamParams)
{
    try
    {
        EntityID editorCamID = enttMgr.CreateEntity("editor_camera");
        EntityID gameCamID   = enttMgr.CreateEntity("game_camera");
       // EntityID matBrowserCamID = enttMgr.CreateEntity("material_browser_camera");

        // add transform component: positions and directions
        const XMFLOAT3 editorCamPos = { 260, 80, 190 };
        const XMFLOAT3 gameCamPos   = { 0, 0, 0 };
        const XMFLOAT3 matBrowserCamPos = { 0, 0, -2.0f };

        enttMgr.AddTransformComponent(editorCamID, editorCamPos, { 0,0,1,0 });
        enttMgr.AddTransformComponent(gameCamID, gameCamPos, { 0,0,1,0 });

        // add camera component
        ECS::CameraData editorCamData;
        editorCamData.fovY        = editorCamParams.fovInRad;
        editorCamData.aspectRatio = editorCamParams.aspectRatio;
        editorCamData.nearZ       = editorCamParams.nearZ;
        editorCamData.farZ        = editorCamParams.farZ;

        ECS::CameraData gameCamData;
        gameCamData.fovY          = gameCamParams.fovInRad;
        gameCamData.aspectRatio   = gameCamParams.aspectRatio;
        gameCamData.nearZ         = gameCamParams.nearZ;
        gameCamData.farZ          = gameCamParams.farZ;

        enttMgr.AddCameraComponent(editorCamID, editorCamData);
        enttMgr.AddCameraComponent(gameCamID, gameCamData);

        // initialize view/projection matrices of the editor/game camera
        ECS::CameraSystem& camSys = enttMgr.cameraSystem_;

        // TODO: move initial UpdateView and SetBaseViewMatrix into the CameraSystem::AddRecord()
        const DirectX::XMMATRIX& editorCamView = camSys.UpdateView(editorCamID);
        camSys.SetBaseViewMatrix(editorCamID, editorCamView);
        camSys.SetupOrthographicMatrix(
            editorCamID,
            editorCamParams.wndWidth,
            editorCamParams.wndHeight,
            editorCamParams.nearZ,
            editorCamParams.farZ);

        const DirectX::XMMATRIX& gameCamView = camSys.UpdateView(gameCamID);
        camSys.SetBaseViewMatrix(gameCamID, gameCamView);
        camSys.SetupOrthographicMatrix(
            gameCamID,
            gameCamParams.wndWidth,
            gameCamParams.wndHeight,
            gameCamParams.nearZ,
            gameCamParams.farZ);
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize the cameras entities");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

inline float GetHeightOfGeneratedTerrainAtPoint(const float x, const float z)
{
    return 0.1f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

///////////////////////////////////////////////////////////

void CreateLightPoles(ECS::EntityMgr& mgr, const BasicModel& lightPole)
{
    LogDbg(LOG, "create light poles entities");

    constexpr size numEntts = 10;

    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);

    XMFLOAT3    positions[numEntts];
    XMVECTOR    dirQuats[numEntts];
    float       uniformScales[numEntts];
    std::string names[numEntts];

    // setup positions: 2 rows of lightPoles
    for (index i = 0, z = 0; i < numEntts; z += 30, i += 2)
    {
        const float x = 11;
        const float y1 = GetHeightOfGeneratedTerrainAtPoint(-x, (float)z);
        const float y2 = GetHeightOfGeneratedTerrainAtPoint(+x, (float)z);
        positions[i + 0] = { -x, y1, (float)z };
        positions[i + 1] = { +x, y2, (float)z };
    }

    constexpr float pidiv2 = DirectX::XM_PIDIV2;

    // setup directions:
    for (index i = 0; i < numEntts; i += 2)
    {
        dirQuats[i + 0] = DirectX::XMQuaternionRotationRollPitchYaw(0, +pidiv2, 0);
        dirQuats[i + 1] = DirectX::XMQuaternionRotationRollPitchYaw(0, -pidiv2, 0);
    }

    // setup scales
    for (index i = 0; i < numEntts; ++i)
        uniformScales[i] = 1.0f;

    // generate names
    for (index i = 0; i < numEntts; ++i)
        names[i] += "lightPole_" + std::to_string(enttsIDs[i]);

    // ----------------------------------------------------

    const EntityID* ids = enttsIDs.data();

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, lightPole.GetID(), numEntts);

    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    const size numSubsets = 2;                       // number of submeshes
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        ids,
        numEntts,
        numSubsets,
        boundTypes,
        lightPole.GetSubsetsAABB());      // AABB data (center, extents)
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

    for (int i = 0; const EntityID & id : enttsIDs)
        enttsNames[i++] = "sphere_" + std::to_string(id);

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // ---------------------------------------------

    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddNameComponent(ids, enttsNames, numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    // ---------------------------------------------

    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        ids,
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());      // AABB data (center, extents)


    // add material for each sphere entity
    const MaterialID matID = model.meshes_.subsets_[0].materialID;
    const MaterialID catMatID = g_MaterialMgr.GetMaterialIdByName("cat");
    const MaterialID woodCrateMatID = g_MaterialMgr.GetMaterialIdByName("brick_01");
    constexpr bool areMaterialsMeshBased = true;


    mgr.AddMaterialComponent(enttsIDs[0], &catMatID, numSubsets, false);
    mgr.AddMaterialComponent(enttsIDs[1], &woodCrateMatID, numSubsets, false);

    for (index i = 2; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], &matID, numSubsets, areMaterialsMeshBased);

    // setup a texture transformation for the sphere
    ECS::StaticTexTransInitParams sphereTexTransform;
    sphereTexTransform.Push(DirectX::XMMatrixScaling(3, 3, 0));

    mgr.AddTextureTransformComponent(enttsIDs[0], ECS::TexTransformType::STATIC, sphereTexTransform);
    mgr.AddTextureTransformComponent(enttsIDs[1], ECS::TexTransformType::STATIC, sphereTexTransform);
}

///////////////////////////////////////////////////////////

void CreateSkyBox(ECS::EntityMgr& mgr, SkyModel& sky)
{
    LogDbg(LOG, "create sky box entity");

    std::string skyTexPath;
    int textureIdx = 0;
    XMFLOAT3 colorCenter;
    XMFLOAT3 colorApex;

    std::ifstream fin("data/sky_config.txt");

    // read in params for the sky from the config file
    if (fin.is_open())
    {
        std::string ignore;

        // read in a path to the sky texture
        fin >> ignore;
        fin >> textureIdx;
        fin >> skyTexPath;

        // read in color of the center (horizon)
        fin >> ignore;
        fin >> colorCenter.x >> colorCenter.y >> colorCenter.z;

        // read in color of the apex (top)
        fin >> ignore;
        fin >> colorApex.x >> colorApex.y >> colorApex.z;
    }
    // or apply default params
    else
    {
        // load only one texture by idx 0 for the sky
        skyTexPath = "cubemaps/cubemap_1.dds";
        skyTexPath = "cubemaps/deep_space_hdr.jpg";
        
        textureIdx = 0;

        // very cool dark gradient:
        colorCenter = { 0.5f, 0.5f, 0.5f };
        colorApex = { 0.38f, 0.45f, 0.51f };
    }

    // load a texture for the sky
    TexID skyMapID = g_TextureMgr.LoadFromFile(g_RelPathTexDir, skyTexPath.c_str());

    // make and setup a material for the sky
    Material& mat = g_MaterialMgr.AddMaterial("sky_mat");
    mat.SetTexture(TEX_TYPE_DIFFUSE, skyMapID);
    mat.SetCull(MAT_PROP_CULL_NONE);
    mat.SetFrontClockwise(MAT_PROP_FRONT_COUNTER_CLOCKWISE);
    mat.SetDepthStencil(MAT_PROP_SKY_DOME);

    sky.SetMaterialId(mat.id);

    // setup sky colors
    sky.SetColorCenter(colorCenter);
    sky.SetColorApex(colorApex);

    // create and setup a sky entity
    const EntityID enttID = mgr.CreateEntity("sky");
    mgr.AddTransformComponent(enttID, { 0,990,0 });
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
    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    // setup bounding params
    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };
    const DirectX::BoundingBox* enttAABBs = model.GetSubsetsAABB();

    // ----------------------------------------------------

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);

    // each cylinder has only one mesh
    constexpr int  numSubmeshes = 1;
    constexpr bool areMaterialsMeshBased = false;

    const MaterialID brickMatID = g_MaterialMgr.GetMaterialIdByName("brick_01");

    for (const EntityID id : enttsIDs)
        mgr.AddMaterialComponent(id, &brickMatID, numSubmeshes, areMaterialsMeshBased);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);
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
        constexpr int numEntts = 6;

        const std::string enttsNames[numEntts] =
        {
            "cat",
            "fireflame",
            "wireFence",
            "woodCrate01",
            "woodCrate02",
            "box01",
        };

        // create empty entities
        const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
        const EntityID* ids = enttsIDs.data();


        // make a map: 'entt_name' => 'entity_id'
        std::map<std::string, EntityID> enttsNameToID;

        for (int i = 0; i < numEntts; ++i)
            enttsNameToID.insert({ enttsNames[i], enttsIDs[i] });


        // ---------------------------------------------------------
        // setup textures for the cubes

        const std::string texKeys[numEntts] =
        {
            "cat",
            "fireAtlas",
            "wireFence",
            "woodCrate01",
            "woodCrate02",
            "box01d",
        };

        const std::string texFilenames[numEntts] =
        {
            "cat.dds",
            "fire_atlas.dds",
            "WireFence.dds",
            "WoodCrate01.dds",
            "WoodCrate02.dds",
            "box01d.dds",
        };

        // load and setup only diffuse texture for each cube
        TexID texIDs[numEntts];

        for (int i = 0; i < numEntts; ++i)
        {
            sprintf(g_String, "%s%s", g_RelPathTexDir, texFilenames[i].c_str());
            texIDs[i] = g_TextureMgr.LoadFromFile(g_String);
        }

        // ---------------------------------------------

        std::map<std::string, TexID> keysToTexIDs;

        for (int i = 0; i < numEntts; ++i)
            keysToTexIDs.insert({ texKeys[i], texIDs[i] });

        // ---------------------------------------------------------
        // prepare position/rotations/scales

        const std::vector<XMVECTOR> dirQuats(numEntts, { 0,0,0,1 }); // no rotation
        const std::vector<float> uniformScales(numEntts, 1.0f);
        std::vector<XMFLOAT3> positions(numEntts);

        for (index i = 0; i < numEntts; ++i)
            positions[i] = { -15, 1, (float)2 * i };

        positions[2] = { -7, -0.3f, 0 };


        // ---------------------------------------------------------
        // prepare textures transformations

        ECS::AtlasAnimInitParams atlasTexAnimParams;
        ECS::RotationAroundCoordInitParams rotAroundCoordsParams;

        atlasTexAnimParams.Push(15, 8, 4);
        rotAroundCoordsParams.Push(0.5f, 0.5f, 0.1f);

        // ---------------------------------------------------------
        // setup the cubes entities

        mgr.AddTransformComponent(ids, numEntts, positions.data(), dirQuats.data(), uniformScales.data());
        mgr.AddNameComponent(ids, enttsNames, numEntts);
        mgr.AddModelComponent(ids, model.GetID(), numEntts);

        // the cube has only one submesh
        constexpr int numSubmeshes = 1;


        // ---------------------------------------------------------
        // Prepare materials for the cubes

        // cube_0: rotated cat
        Material& catMat= g_MaterialMgr.AddMaterial("catMat");
        catMat.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("cat"));

        // cube_1: firecamp animated
        Material& firecampMat = g_MaterialMgr.AddMaterial("firecamp");
        firecampMat.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("fireAtlas"));

        // cube_2: wirefence with alpha clipping
        Material& wirefenceMat = g_MaterialMgr.AddMaterial("wirefence");
        wirefenceMat.SetAlphaClip(true);
        wirefenceMat.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("wireFence"));

        // cube_3: wood crate 1
        Material& woodCrate1Mat = g_MaterialMgr.AddMaterial("wood_crate_1");
        woodCrate1Mat.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("woodCrate01"));

        // cube_4: wood crate 2
        Material& woodCrate2Mat = g_MaterialMgr.AddMaterial("wood_crate_2");
        woodCrate2Mat.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("woodCrate02"));


        // add material component to each box
        const MaterialID matIdDefaultBox = model.meshes_.subsets_[0].materialID;
        constexpr bool matIsMeshBased    = true;
        constexpr bool matIsNotMeshBased = false;

        mgr.AddMaterialComponent(enttsNameToID.at("cat"),         &catMat.id,        numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("fireflame"),   &firecampMat.id,   numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("box01"),       &matIdDefaultBox, numSubmeshes, matIsMeshBased);

        // add material component (each material is unique)
        mgr.AddMaterialComponent(enttsNameToID.at("wireFence"),   &wirefenceMat.id,  numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("woodCrate01"), &woodCrate1Mat.id, numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("woodCrate02"), &woodCrate2Mat.id, numSubmeshes, matIsNotMeshBased);

        // ------------------------------------------

        // add some texture transformations (animations) for some cubes
#if 1
        mgr.AddTextureTransformComponent(
            enttsNameToID.at("fireflame"),
            ECS::TexTransformType::ATLAS_ANIMATION,
            atlasTexAnimParams);

        mgr.AddTextureTransformComponent(
            enttsNameToID.at("cat"),
            ECS::TexTransformType::ROTATION_AROUND_TEX_COORD,
            rotAroundCoordsParams);
#endif

        // setup rendering params
        ECS::RenderInitParams renderParams;
        renderParams.shaderType = ECS::LIGHT_SHADER;
        renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        mgr.AddRenderingComponent(ids, numEntts, renderParams);

        // setup blending params of the entities
        using enum ECS::eRenderState;
        mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("wireFence"), { ALPHA_CLIPPING, CULL_NONE });

#if 0
        
        mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("cat"), ADDING);
        //enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate01"), ADDING);
        //enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate02"), MULTIPLYING);
#endif

        const size numSubsets = 1;                    // each cube has only one mesh
        const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

        // add bounding component to each entity
        mgr.AddBoundingComponent(
            ids,
            numEntts,
            numSubsets,
            boundTypes,
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    constexpr size             numEntts = 1;
    constexpr size             numSubsets = 1;
    const ECS::BoundingType    boundType = ECS::BoundingType::BOUND_BOX;
    const DirectX::BoundingBox aabb = { terrain.center_, terrain.extents_ };

    // setup material params
    constexpr bool areMaterialsMeshBased = true;
    const MaterialID terrainMatID = terrain.materialID_;


    mgr.AddTransformComponent(enttID);
    mgr.AddNameComponent(enttID, "terrain_geomipmap");
    mgr.AddBoundingComponent(enttID, boundType, aabb);
    mgr.AddMaterialComponent(enttID, &terrainMatID, numSubsets, areMaterialsMeshBased);

    LogDbg(LOG, "Terrain (geomipmap) is created");
}

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

///////////////////////////////////////////////////////////

void CreateSkull(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a skull entity

    LogDbg(LOG, "create skull");

    try
    {
        const EntityID enttID = mgr.CreateEntity();

        // setup rendering params
        ECS::RenderInitParams renderParams;
        renderParams.shaderType = ECS::LIGHT_SHADER;
        renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        mgr.AddTransformComponent(enttID, { -15, 4, 10 });
        mgr.AddNameComponent(enttID, "skull");
        mgr.AddModelComponent(enttID, model.GetID());
        mgr.AddRenderingComponent(enttID, renderParams);

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

    using enum ECS::eRenderState;
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniScales);
    const DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpacesByQuat(ids, numEntts, rotQuat);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    // apply alpha_clipping and cull_none to each tree entt
    mgr.renderStatesSystem_.UpdateStates(enttsIDs, { ALPHA_CLIPPING, CULL_NONE });

    //const DirectX::XMMATRIX world = mgr.transformSystem_.GetWorldMatrixOfEntt(enttI)
    constexpr size numSubsets = 3;    // tree pine model has 3 meshes
    const ECS::BoundingType boundTypes[numSubsets] = { ECS::BoundingType::BOUND_BOX, ECS::BoundingType::BOUND_BOX, ECS::BoundingType::BOUND_BOX };

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());      // AABB data (center, extents)


    // get material ID of each tree subset (mesh)
    MaterialID materialIDs[numSubsets];

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialID;

    // add material component to each tree pine
    constexpr bool matIsMeshBased = true;

    for (index i = 0; i < numEntts; ++i)
        mgr.AddMaterialComponent(ids[i], materialIDs, numSubsets, matIsMeshBased);
}

///////////////////////////////////////////////////////////

void CreateTreesSpruce(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create tree spruce entities");

    using enum ECS::eRenderState;
    constexpr int numEntts = 100;
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniScales);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    // apply alpha_clipping and cull_none to each tree entt
    mgr.renderStatesSystem_.UpdateStates(enttsIDs, { ALPHA_CLIPPING, CULL_NONE });

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

void CreatePowerLine(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create power line entities");

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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numSubsets = model.GetNumSubsets();
    const cvector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(ids, numEntts, positions, directions, uniformScales);

    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    for (const EntityID id : enttsIDs)
        mgr.transformSystem_.RotateLocalSpaceByQuat(id, rotQuat);

    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    // ----------------------------------------------------

    const MaterialID matID = model.meshes_.subsets_[0].materialID;
    constexpr size numSubmeshes = 1;
    constexpr bool isMatMeshBased = true;

    for (const EntityID enttID : enttsIDs)
        mgr.AddMaterialComponent(enttID, &matID, numSubmeshes, isMatMeshBased);
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    constexpr bool areMaterialsMeshBased = true;
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialID;


    // add components
    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets, areMaterialsMeshBased);

    // rotate the stalker entity
    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);

    // apply alpha_clipping and cull_none to each tree entt
    using enum ECS::eRenderState;
    mgr.renderStatesSystem_.UpdateStates(enttID, { ALPHA_CLIPPING, CULL_NONE });

}

///////////////////////////////////////////////////////////

void CreateStalkerFreedom(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a stalker entity");
    
    const EntityID enttID = mgr.CreateEntity("stalker_freedom");

    // setup transformation params
    XMFLOAT3 position        = { 7, GetHeightOfGeneratedTerrainAtPoint(7, -10), -10};
    XMVECTOR direction       = { 0, 1, 0, 0 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    constexpr bool areMaterialsMeshBased = true;
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialID;


    // add components
    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddModelComponent    (enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets, areMaterialsMeshBased);

    // rotate the stalker entity
    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);
}

///////////////////////////////////////////////////////////

void CreateTraktor13(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create a traktor (tr-13) entity");
    
    const EntityID enttID = mgr.CreateEntity("traktor_13");

    // setup transformation params
    XMFLOAT3 position        = { 40, GetHeightOfGeneratedTerrainAtPoint(40, 3), 3};
    XMVECTOR direction       = { 0, 1, 0, 0 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup materials params
    constexpr bool areMaterialsMeshBased = true;
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialID;


    // add components
    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddModelComponent    (enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets, areMaterialsMeshBased);

    // setup blending params of the entities
    using enum ECS::eRenderState;
    mgr.renderStatesSystem_.UpdateStates(enttID, ALPHA_CLIPPING);

    // rotate the stalker entity
    const XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, rotQuat, 0.5f);
    mgr.AddNameComponent(enttID, model.GetName());
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "building");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType     = ECS::LIGHT_SHADER;
    renderParams.topologyType   = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // add bounding params
    const size numEntts         = 1;
    const size numSubsets       = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "soviet_statue");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

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
    const XMFLOAT3 pos = { -50, 0, 0 };
    const XMVECTOR dirQuat = { 0,0,0,1 };
    const float uniformScale = 0.1f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "apartment");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateAk47(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create ak47 entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position = { 10, 2, 10 };
    const XMVECTOR direction = { 0, 0, 1, 0 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    constexpr size numEntts = 1;
    constexpr size numSubsets = 4;                // model of ak-47 has 4 meshes
    ECS::BoundingType boundTypes[numSubsets];

    for (index i = 0; i < numSubsets; ++i)
        boundTypes[i] = ECS::BoundingType::BOUND_BOX;

    // prepare materials IDs for the material component
    MaterialID materialsIDs[numSubsets]{ 0 };

    for (index i = 0; i < numSubsets; ++i)
        materialsIDs[i] = model.meshes_.subsets_[i].materialID;

    // ----------------------------------------------------

    mgr.AddTransformComponent(enttID, position, direction, uniformScale);
    mgr.AddNameComponent(enttID, "ak_47");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());      // AABB data (center, extents)

    constexpr bool areMaterialsMeshBased = true;
    mgr.AddMaterialComponent(enttID, materialsIDs, numSubsets, areMaterialsMeshBased);
}

///////////////////////////////////////////////////////////

void CreateSword(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create sword entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position = { 0.7f,-0.4f,1.1f };
    const XMVECTOR dirQuat = { 0, 0, 1, 0 };
    const float uniformScale = 0.015f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)

    constexpr bool areMaterialsMeshBased = true;
    const MaterialID matID = model.meshes_.subsets_[0].materialID;
    mgr.AddMaterialComponent(enttID, &matID, numSubsets, areMaterialsMeshBased);
}


///////////////////////////////////////////////////////////

void CreateCastleTower(ECS::EntityMgr& mgr, const BasicModel& model)
{
    const EntityID enttID = mgr.CreateEntity();

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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup transformation
    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2 - 0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, q1);

    mgr.AddNameComponent(enttID, "castle_tower");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)

    constexpr bool areMaterialsMeshBased = true;
    const MaterialID matID = model.meshes_.subsets_[0].materialID;
    Material& mat = g_MaterialMgr.GetMaterialById(matID);
    mat.SetSpecular(0.2f, 0.2f, 0.2f);
    mat.SetSpecularPower(15.0f);
    mgr.AddMaterialComponent(enttID, &matID, numSubsets, areMaterialsMeshBased);

    // TEMP: fix rotation
    const DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, -DirectX::XM_PIDIV2+0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, rotQuat);
}

////////////////////////////////////////////////////////////

void CreateAk74(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create ak74 entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position = { 0.5f,-1.3f,0.7f };
    const XMVECTOR dirQuat = { 0, 0, 1, 0 };
    const float uniformScale = 4.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    // setup transformation
    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    const XMVECTOR q1 = DirectX::XMQuaternionRotationAxis({ 1,0,0 }, DirectX::XM_PIDIV2 - 0.1f);
    const XMVECTOR q2 = DirectX::XMQuaternionRotationAxis({ 0,1,0 }, 0.1f);
    mgr.transformSystem_.RotateLocalSpaceByQuat(enttID, DirectX::XMQuaternionMultiply(q1, q2));

    mgr.AddNameComponent(enttID, "ak_74");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)

    constexpr bool areMaterialsMeshBased = true;
    const MaterialID matID = model.meshes_.subsets_[0].materialID;
    mgr.AddMaterialComponent(enttID, &matID, numSubsets, areMaterialsMeshBased);
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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts     = 1;
    const size numSubsets   = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    mgr.AddTransformComponent(enttID, pos, quat, uniScale);
    mgr.AddNameComponent(enttID, "kordon_house");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

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

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    mgr.AddTransformComponent(enttID, pos, quat, 0.01f);
    mgr.AddNameComponent(enttID, "blockpost");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateRocks(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "create rock entities");

    constexpr int numEntts = 20;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    XMFLOAT3    positions[numEntts];
    XMVECTOR    quats[numEntts];
    float       uniformScales[numEntts];
    std::string names[numEntts];


    // setup transformation params
    for (XMFLOAT3& pos : positions)
    {
        pos.x = MathHelper::RandF(-250.0f, 250.0f);
        pos.z = MathHelper::RandF(-250.0f, 250.0f);
        pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);
    }

    // apply the same rotation to each entt
    for (XMVECTOR& quat : quats)
        quat = { 0,0,0,1 };

    // apply the same scale to each entt
    for (float& scale : uniformScales)
        scale = 1.0f;

    // generate names for entts
    for (int i = 0; std::string & name : names)
        name = "rock_" + std::to_string(enttsIDs[i++]);

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(ids, numEntts, positions, quats, uniformScales);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    mgr.AddBoundingComponent(
        ids,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreatePillar(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg(LOG, "Create pillar entities");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMVECTOR quat = { 0,0,0,1 };
    const float uniformScale = 1.0f;
    XMFLOAT3 pos;
    pos.x = MathHelper::RandF(-250, 250);
    pos.z = MathHelper::RandF(-250, 250);
    pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, { 0,0,0,1 }, 1.0f);
    mgr.AddNameComponent(enttID, "pillar_" + std::to_string(enttID));
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void PrintImportTimingInfo()
{
    // print into the console information about the duration of the whole
    // process of importing models from the external formats

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


    // paths to external models
    const char* pathNanosuit                = "data/models/ext/nanosuit/nanosuit.obj";
    const char* pathSovietBuilding          = "data/models/ext/building9/building9.obj";
    const char* pathSovietStatue            = "data/models/ext/sovietstatue_1/sickle&hammer.obj";
    const char* pathSovietApartment         = "data/models/ext/Apartment/Apartment.obj";
    const char* pathPwerHWTower             = "data/models/ext/power_line/Power_HV_Tower.FBX";

    const char* pathStalkerHouseSmall       = "data/models/ext/stalker/stalker-house/source/SmallHouse.fbx";
    const char* pathStalkerHouseAbandoned   = "data/models/ext/stalker/abandoned-house-20/source/StalkerAbandonedHouse.fbx";
    const char* pathStalkerTraktor          = "data/models/ext/tr13/tr13.fbx";
    const char* pathStalkerFreedom          = "data/models/ext/stalker_freedom_1/stalker_freedom_1.fbx";
    const char* pathAk47                    = "data/models/ext/aks-74_game_ready/scene.gltf";
    const char* pathAk74u                   = "data/models/ext/ak_74u/ak_74u.fbx";

    const char* pathTreeSpruce              = "data/models/ext/trees/tree_spruce/tree_spruce.obj";
    const char* pathTreePine                = "data/models/ext/trees/FBX format/tree_pine.fbx";
    const char* pathRadar                   = "data/models/ext/radar/radar.fbx";
    const char* pathCastleTower             = "data/models/ext/castle-tower/fougeres gate.obj";
    const char* pathSword                   = "data/models/ext/sword/sword.obj";
#if 1
    // import a model from file by path
    LogDbg(LOG, "Start of models importing");

    ModelsCreator creator;

    //const ModelID lightPoleID      = creator.ImportFromFile(pDevice, lightPolePath);
    
    //const ModelID nanosuitID       = creator.ImportFromFile(pDevice, nanosuitPath);
    //const ModelID stalkerFreedomID = creator.ImportFromFile(pDevice, stalkerFreedom1Path.c_str());
    //const ModelID traktorID        = creator.ImportFromFile(pDevice, stalkerTraktor13Path.c_str());
    //const ModelID stalkerHouse1ID  = creator.ImportFromFile(pDevice, stalkerHouseSmallPath.c_str());
    //const ModelID stalkerHouse2ID  = creator.ImportFromFile(pDevice, stalkerHouseAbandonedPath.c_str());
    //const ModelID ak47ID = creator.ImportFromFile(pDevice, ak47Path.c_str());
#if CREATE_SWORD
    const ModelID swordID          = creator.ImportFromFile(pDevice, pathSword);
#endif
    //const ModelID ak74ID           = creator.ImportFromFile(pDevice, pathAk74u);

#if CREATE_CASTLE
    const ModelID castleTowerID    = creator.ImportFromFile(pDevice, pathCastleTower);
#endif

#if CREATE_TREES
    const ModelID treeSpruceID     = creator.ImportFromFile(pDevice, pathTreeSpruce);
    const ModelID treePineID       = creator.ImportFromFile(pDevice, pathTreePine);
#endif
    //const ModelID barrelID         = creator.ImportFromFile(pDevice, barrelPath);
    //const ModelID powerHVTowerID   = creator.ImportFromFile(pDevice, powerHVTowerPath.c_str());
    //const ModelID radarID = creator.ImportFromFile(pDevice, radarPath.c_str());

    //const ModelID buildingID       = creator.ImportFromFile(pDevice, building9Path);
    //const ModelID apartmentID      = creator.ImportFromFile(pDevice, apartmentPath);
    //const ModelID sovietStatueID   = creator.ImportFromFile(pDevice, sovientStatuePath);


    PrintImportTimingInfo();

    LogDbg(LOG, "All the models are imported successfully");

    // get models by its ids
    //BasicModel& building        = g_ModelMgr.GetModelByID(buildingID);
    //BasicModel& apartment       = g_ModelMgr.GetModelByID(apartmentID);
    //BasicModel& sovietStatue    = g_ModelMgr.GetModelByID(sovietStatueID);
#if CREATE_TREES
    BasicModel& treeSpruce      = g_ModelMgr.GetModelByID(treeSpruceID);
    BasicModel& treePine        = g_ModelMgr.GetModelByID(treePineID);
#endif
    //BasicModel& powerHVTower    = g_ModelMgr.GetModelByID(powerHVTowerID);
    //BasicModel& stalkerFreedom  = g_ModelMgr.GetModelByID(stalkerFreedomID);
    //BasicModel& traktor13       = g_ModelMgr.GetModelByID(traktorID);
    //BasicModel& stalkerHouse1   = g_ModelMgr.GetModelByID(stalkerHouse1ID);
    //BasicModel& stalkerHouse2   = g_ModelMgr.GetModelByID(stalkerHouse2ID);
    //BasicModel& ak47            = g_ModelMgr.GetModelByID(ak47ID);
#if CREATE_SWORD
    BasicModel& sword           = g_ModelMgr.GetModelByID(swordID);
#endif
    //BasicModel& ak74            = g_ModelMgr.GetModelByID(ak74ID);
#if CREATE_CASTLE
    BasicModel& castleTower     = g_ModelMgr.GetModelByID(castleTowerID);
#endif
    //BasicModel& radar           = g_ModelMgr.GetModelByID(radarID);

    // setup some models (set textures, setup materials)
    //SetupStalkerSmallHouse(stalkerHouse1);
    //SetupStalkerAbandonedHouse(stalkerHouse2);
#if CREATE_TREES
    SetupTree(treePine);
    SetupTreeSpruce(treeSpruce);
#endif
    //SetupPowerLine(powerHVTower);
    //SetupBuilding9(building);
    //SetupStalkerFreedom(stalkerFreedom);
    //SetupAk47(ak47);
    //SetupAk74(ak74);
    //SetupTraktor(traktor13);

#if CREATE_TREES
    //CreateTreesPine(mgr, treePine);
    CreateTreesSpruce(mgr, treeSpruce);
#endif
    //CreatePowerLine(mgr, powerHVTower);
    //CreateLightPoles(mgr, lightPole);
    //CreateHouse(mgr, stalkerHouse1);
    //CreateHouse2(mgr, stalkerHouse2);

    //CreateAk47(mgr, ak47);

#if CREATE_SWORD
    CreateSword(mgr, sword);
#endif
    //CreateAk74(mgr, ak74);
#if CREATE_CASTLE
    CreateCastleTower(mgr, castleTower);
#endif
    //CreateBarrel(mgr, barrel);
    //CreateNanoSuit(mgr, nanosuit);
    //CreateRadar(mgr, radar);
    //CreateStalkerFreedom(mgr, stalkerFreedom);
    //CreateTraktor13(mgr, traktor13);
    //CreateBuilding(mgr, building);
    //CreateApartment(mgr, apartment);
    //CreateSovietStatue(mgr, sovietStatue);
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
    Material& mat = g_MaterialMgr.AddMaterial("terrain_mat_geomip");
    mat.SetAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    mat.SetDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    mat.SetTexture(TEX_TYPE_DIFFUSE,           terrainGeomip.texture_.GetID());
    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrainGeomip.detailMap_.GetID());
    mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrainGeomip.lightmap_.id);
    //terrainMat.SetTexture(TEX_TYPE_NORMALS, terrain.normalMap_.GetID());

    terrainGeomip.materialID_ = mat.id;

    // create a terrain entity
    CreateTerrain(mgr, terrainGeomip);
}

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

//---------------------------------------------------------
// Desc:   generate and setup different assets/entities
// Args:   - pDevice:  a ptr to DirectX11 device
//         - mgr:      ECS entity manager
//---------------------------------------------------------
void GenerateEntities(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    ModelsCreator creator;

    // create sky model
    creator.CreateSkyCube(pDevice, 2000);
    //creator.CreateSkySphere(pDevice, 2000, 30, 30);
    //MeshSphereParams skyDomeSphereParams(2000, 30, 30);
    //const ModelID skyBoxID = creator.CreateSphere(pDevice, skyDomeSphereParams);
    SkyModel& skyBox = g_ModelMgr.GetSky();


    // create terrain
    const char* terrainConfigPath = "data/terrain/terrain.cfg";
    CreateTerrainGeomip(mgr, terrainConfigPath);
    //CreateTerrainQuadtree(mgr, terrainConfigPath);


    // generate some models manually
    const MeshSphereParams    boundSphereParams(1, 8, 8);
    const MeshGeosphereParams boundGeoSphereParams(1, 1);
    const MeshSphereParams    sphereParams(0.5f, 20, 20);
    const MeshCylinderParams  cylParams(1, 1, 5, 15, 1);

    const ModelID cubeID        = creator.CreateCube(pDevice);
    const ModelID boundSphereID = creator.CreateGeoSphere(pDevice, boundGeoSphereParams);
    const ModelID sphereID      = creator.CreateSphere(pDevice, sphereParams);
    const ModelID cylinderID    = creator.CreateCylinder(pDevice, cylParams);

    // get actual model by its ID
    BasicModel& cube        = g_ModelMgr.GetModelByID(cubeID);
    BasicModel& sphere      = g_ModelMgr.GetModelByID(sphereID);
    BasicModel& boundSphere = g_ModelMgr.GetModelByID(boundSphereID);
    BasicModel& cylinder    = g_ModelMgr.GetModelByID(cylinderID);

    sphere.SetName("basic_sphere");
    boundSphere.SetName("bound_sphere");
    cylinder.SetName("basic_cylinder");

    // ----------------------------------------------------
    // create some materials:

    // 1. create brick_01 material
    const TexID texIdStoneDiff = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "brick01d.dds");
    const TexID texIdStoneNorm = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "brick01n.dds");

    Material& cylinderMat = g_MaterialMgr.AddMaterial("brick_01");
    cylinderMat.SetTexture(TEX_TYPE_DIFFUSE, texIdStoneDiff);
    cylinderMat.SetTexture(TEX_TYPE_NORMALS, texIdStoneNorm);
    cylinderMat.SetAmbient(0.4f, 0.4f, 0.4f, 1.0f);

    // ----------------------------------------------------

    // manual setup of some models
    SetupCube(cube);
    SetupSphere(sphere);

    // create and setup entities with models
    CreateSkyBox(mgr, skyBox);
    CreateCubes(mgr, cube);
    CreateSpheres(mgr, sphere);
    CreateCylinders(mgr, cylinder);
}

//---------------------------------------------------------
// Desc:   manually create some materials
//---------------------------------------------------------
void GenerateMaterials()
{
    // load some textures for billboards and particles
    const TexID texIdFlare  = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "flare.png");
    const TexID texIdFlame0 = g_TextureMgr.LoadFromFile(g_RelPathTexDir, "flame0.dds");
    const TexID texIdCat    = g_TextureMgr.GetTexIdByName("cat");

    // create material for flame (fire) particles
    Material& flameParticleMat = g_MaterialMgr.AddMaterial("flameMat");
    flameParticleMat.SetTexture(TEX_TYPE_DIFFUSE, texIdFlame0);
    flameParticleMat.SetAlphaClip(true);
    flameParticleMat.SetFill(MAT_PROP_FILL_SOLID);
    flameParticleMat.SetCull(MAT_PROP_CULL_NONE);
    flameParticleMat.SetBlending(MAT_PROP_ALPHA_ENABLE);
    flameParticleMat.SetDepthStencil(MAT_PROP_DEPTH_DISABLED);

    // create material for flare particles
    Material& flareParticleMat = g_MaterialMgr.AddMaterial("flareMat");
    flareParticleMat.SetTexture(TEX_TYPE_DIFFUSE, texIdFlare);
    flareParticleMat.SetAlphaClip(true);
    flareParticleMat.SetFill(MAT_PROP_FILL_SOLID);
    flareParticleMat.SetCull(MAT_PROP_CULL_NONE);
    flareParticleMat.SetBlending(MAT_PROP_ALPHA_ENABLE);
    flareParticleMat.SetDepthStencil(MAT_PROP_DEPTH_DISABLED);

    // create material for flare particles
    Material& catParticleMat = g_MaterialMgr.AddMaterial("catParticleMat");
    catParticleMat.SetTexture(TEX_TYPE_DIFFUSE, texIdCat);
    catParticleMat.SetAlphaClip(false);
    catParticleMat.SetFill(MAT_PROP_FILL_SOLID);
    catParticleMat.SetCull(MAT_PROP_CULL_NONE);
    catParticleMat.SetBlending(MAT_PROP_ALPHA_ENABLE);
    catParticleMat.SetDepthStencil(MAT_PROP_DEPTH_DISABLED);
}

//---------------------------------------------------------
// 
//---------------------------------------------------------
void LoadAssets(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    // load models from the internal .de3d format

    ModelsCreator creator;

    g_ModelMgr.Deserialize(pDevice);

    // create and setup entities with models
    //CreateTreesPine   (mgr, storage.GetModelByName("tree_pine"));
    //CreateTreesSpruce (mgr, storage.GetModelByName("tree_spruce"));
    //CreatePowerLine   (mgr, storage.GetModelByName("Power_HV_Tower"));
    //CreateLightPoles  (mgr, storage.GetModelByName("light_pole"));
    //CreateHouse       (mgr, storage.GetModelByName("SmallHouse"));
    //CreateHouse2      (mgr, storage.GetModelByName("StalkerAbandonedHouse"));
    //CreateAk47        (mgr, storage.GetModelByName("ak_47"));
    //CreateAk74        (mgr, storage.GetModelByName("ak_74u"));

    //CreateBarrel      (mgr, storage.GetModelByName("Barrel1"));
    //CreateNanoSuit    (mgr, storage.GetModelByName("nanosuit"));
    //CreateBuilding    (mgr, storage.GetModelByName("building9"));
    //CreateApartment   (mgr, storage.GetModelByName("Apartment"));
    //CreateSovietStatue(mgr, storage.GetModelByName("sickle&hammer"));

    CreateStalkerFreedom(mgr, g_ModelMgr.GetModelByName("stalker_freedom_1"));

#if 0
    // load models from the internal asset format and get its IDs
    ModelID treeID = creator.CreateFromDE3D(pDevice, "tree_pine/tree_pine.de3d");
    ModelID powerHVTowerID = creator.CreateFromDE3D(pDevice, "power_hw_tower/power_hw_tower.de3d");
    ModelID lightPoleID = creator.CreateFromDE3D(pDevice, "light_pole/light_pole.de3d");
    ModelID stalkerHouse1ID = creator.CreateFromDE3D(pDevice, "stalker_house_1/stalker_house_1.de3d");
    ModelID stalkerHouse2ID = creator.CreateFromDE3D(pDevice, "stalker_house_2/stalker_house_2.de3d");
    ModelID ak47ID = creator.CreateFromDE3D(pDevice, "ak_47/ak_47.de3d");

    ModelID barrelID = creator.CreateFromDE3D(pDevice, "barrel/barrel.de3d");
    ModelID nanosuitID = creator.CreateFromDE3D(pDevice, "nanosuit/nanosuit.de3d");
    ModelID buildingID = creator.CreateFromDE3D(pDevice, "soviet_building/building.de3d");
    ModelID apartmentID = creator.CreateFromDE3D(pDevice, "soviet_building/apartment.de3d");
    ModelID sovietStatueID = creator.CreateFromDE3D(pDevice, "soviet_statue/soviet_statue.de3d");

    // get models
    BasicModel& tree = storage.GetModelByID(treeID);
    BasicModel& powerHVTower = storage.GetModelByID(powerHVTowerID);
    BasicModel& lightPole = storage.GetModelByID(lightPoleID);
    BasicModel& stalkerHouse1 = storage.GetModelByID(stalkerHouse1ID);
    BasicModel& stalkerHouse2 = storage.GetModelByID(stalkerHouse2ID);
    BasicModel& ak47 = storage.GetModelByID(ak47ID);

    BasicModel& barrel = storage.GetModelByID(barrelID);
    BasicModel& nanosuit = storage.GetModelByID(nanosuitID);
    BasicModel& building = storage.GetModelByID(buildingID);
    BasicModel& apartment = storage.GetModelByID(apartmentID);
    BasicModel& sovietStatue = storage.GetModelByID(sovietStatueID);


    // create and setup entities with models
    CreateTrees(mgr, tree);
    CreatePowerLine(mgr, powerHVTower);
    CreateLightPoles(mgr, lightPole);
    CreateHouse(mgr, stalkerHouse1);
    CreateHouse2(mgr, stalkerHouse2);
    CreateAK(mgr, ak47);

    CreateBarrel(mgr, barrel);
    CreateNanoSuit(mgr, nanosuit);
    CreateBuilding(mgr, building);
    CreateApartment(mgr, apartment);
    CreateSovietStatue(mgr, sovietStatue);
#endif
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
bool SceneInitializer::InitModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
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
        BasicModel& invalidModel = g_ModelMgr.GetModelByID(cubeID);

        invalidModel.SetName("invalid_model");
        invalidModel.SetMaterialForSubset(0, invalidMat.id);

        // NOTE: the bounding line box model must be created first of all, before all the other models
        const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);

        LoadTreesBillboardsTextures();
        GenerateEntities(pDevice, mgr);
        GenerateMaterials();

       

#if 1
        if (FileSys::Exists(g_RelPathAssetsDir))
        {
            LoadAssets(pDevice, mgr);
        }
        else
        {
            ImportExternalModels(pDevice, mgr);
        }
#else
        ImportExternalModels(pDevice, mgr);
        ProjectSaver saver;
        saver.StoreModels(pDevice);
#endif

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
bool SceneInitializer::InitLightSources(ECS::EntityMgr& mgr)
{
    InitDirectedLightEntities(mgr);
    InitPointLightEntities(mgr);
    InitSpotLightEntities(mgr);

    return true;
}

} // namespace Game
