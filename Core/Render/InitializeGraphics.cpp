// =================================================================================
// Filename:     InitializeGraphics.cpp
// Description:  there are functions for initialization of DirectX
//               and graphics parts of the engine;
//
// Created:      02.12.22
// =================================================================================
#include "InitializeGraphics.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/MathHelper.h>
#include <CoreCommon/StringHelper.h>
#include "Common/LIB_Exception.h"    // ECS exception

#include "../Model/ModelExporter.h"
#include "../Model/ModelImporter.h"
#include "../Model/ModelLoader.h"
#include "../Model/ModelMath.h"
#include "../Model/ModelStorage.h"

#include "../Engine/ProjectSaver.h"

#include "InitGraphicsHelperDataTypes.h"

//#include "../Model/SkyModel.h"
#include "SetupModels.h"
#include <filesystem>
#include <shellapi.h>

#define IMPORT_MODELS_MODE true

using namespace DirectX;
namespace fs = std::filesystem;

namespace Core
{

InitializeGraphics::InitializeGraphics()
{
    Log::Debug();
}


// =================================================================================
//
//                                PUBLIC FUNCTIONS
//
// =================================================================================

bool InitializeGraphics::InitializeDirectX(
    D3DClass& d3d,
    HWND hwnd,
    const Settings& settings)
{
    // THIS FUNC initializes the DirectX stuff 
    // (device, deviceContext, swapChain, rasterizerState, viewport, etc)

    try 
    {
        bool result = d3d.Initialize(
            hwnd,
            settings.GetBool("VSYNC_ENABLED"),
            settings.GetBool("FULL_SCREEN"),
            settings.GetBool("ENABLE_4X_MSAA"),
            settings.GetFloat("NEAR_Z"),
            settings.GetFloat("FAR_Z"));         // how far we can see

        Assert::True(result, "can't initialize the Direct3D");

        // setup the rasterizer state to default params
        using enum RenderStates::STATES;
        d3d.SetRS({ CULL_BACK, FILL_SOLID });
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        Log::Error("can't initialize DirectX");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeScene(
    const Settings& settings,
    D3DClass& d3d,
    ECS::EntityMgr& entityMgr)
    
{
    // THIS FUNC initializes some main elements of the scene:
    // models, light sources, textures

    try
    {
        bool result = false;
        
        // create and init scene elements
        result = InitializeModels(
            d3d.GetDevice(),
            d3d.GetDeviceContext(),
            entityMgr, 
            settings, 
            d3d.GetScreenDepth());
        Assert::True(result, "can't initialize models");

        // init all the light source on the scene
        result = InitializeLightSources(entityMgr, settings);
        Assert::True(result, "can't initialize light sources");

        //ShellExecuteA(NULL, "open", "C:\\", NULL, NULL, SW_SHOWDEFAULT);

        Log::Print("is initialized");
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        Log::Error("can't initialize the scene");

        return false;
    }

    return true;
} 

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeCameras(
    D3DClass& d3d,
    Camera& gameCamera,
    Camera& editorCamera,
    DirectX::XMMATRIX& baseViewMatrix,      // is used for 2D rendering
    ECS::EntityMgr& enttMgr,
    const Settings& settings)
{
    try
    {
        const SIZE windowedSize           = d3d.GetWindowedWndSize();
        const SIZE fullscreenSize         = d3d.GetFullscreenWndSize();
        const float windowedAspectRatio   = (float)windowedSize.cx   / (float)windowedSize.cy;
        const float fullScreenAspectRatio = (float)fullscreenSize.cx / (float)fullscreenSize.cy;

        const float nearZ    = settings.GetFloat("NEAR_Z");
        const float farZ     = settings.GetFloat("FAR_Z");
        const float fovInRad = settings.GetFloat("FOV_IN_RAD");         // field of view in radians

        const XMFLOAT3 editorCamPos = { -18, 1, -15 };
        const XMFLOAT3 gameCamPos = { 0, 2, -3 };

        // 1. initialize the editor camera
        editorCamera.SetProjection(fovInRad, windowedAspectRatio, nearZ, farZ);
        editorCamera.SetPosition(editorCamPos);

        // 2. initialize the game camera
        gameCamera.SetProjection(fovInRad, fullScreenAspectRatio, nearZ, farZ);
        gameCamera.SetPosition(gameCamPos);

        // initialize view matrices of the cameras
        editorCamera.UpdateViewMatrix();
        gameCamera.UpdateViewMatrix();

        // initialize a base view matrix with the camera
        // for 2D user interface rendering (in GAME mode)
        baseViewMatrix = gameCamera.View();

        editorCamera.SetFreeCamera(true);
        gameCamera.SetFreeCamera(true);

        const float sensitivity  = settings.GetFloat("CAMERA_SENSITIVITY"); // camera rotation speed
        const float camWalkSpeed = settings.GetFloat("CAMERA_WALK_SPEED");
        const float camRunSpeed  = settings.GetFloat("CAMERA_RUN_SPEED");

        editorCamera.SetWalkSpeed(camWalkSpeed);
        editorCamera.SetRunSpeed(camRunSpeed);
        editorCamera.SetSensitiviry(sensitivity);

        gameCamera.SetWalkSpeed(camWalkSpeed);
        gameCamera.SetRunSpeed(camRunSpeed);
        gameCamera.SetSensitiviry(sensitivity);

        // create and setup an editor camera entity
        EntityID editorCamID = enttMgr.CreateEntity();
        enttMgr.AddTransformComponent(editorCamID, editorCamPos);
        enttMgr.AddCameraComponent(editorCamID, editorCamera.View(), editorCamera.Proj());
        enttMgr.AddNameComponent(editorCamID, "editor_camera");

        // create and setup an editor camera entity
        EntityID gameCamID = enttMgr.CreateEntity();
        enttMgr.AddTransformComponent(gameCamID, gameCamPos);
        enttMgr.AddCameraComponent(gameCamID, gameCamera.View(), gameCamera.Proj());
        enttMgr.AddNameComponent(gameCamID, "game_camera");
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        Log::Error("can't initialize the cameras objects");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

inline float GetHeightOfGeneratedTerrainAtPoint(const float x, const float z)
{
    return 0.01f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

///////////////////////////////////////////////////////////

void CreateLightPoles(ECS::EntityMgr& mgr, const BasicModel& lightPole)
{
    // create and setup light poles entities
    Log::Debug();

    constexpr size numEntts = 10;

    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);

    std::vector<XMFLOAT3>   positions(numEntts);
    std::vector<XMVECTOR>   dirQuats(numEntts);
    std::vector<float>      uniformScales(numEntts, 1.0f);
    std::vector<EntityName> names(numEntts, "lightPole_");

    // generate 2 rows of lightPoles
    for (int i = 0, z = 0; i < numEntts; z += 30, i += 2)
    {
        const float x = 11;
        const float y1 = GetHeightOfGeneratedTerrainAtPoint(-x, (float)z);
        const float y2 = GetHeightOfGeneratedTerrainAtPoint(+x, (float)z);
        positions[i + 0] = { -x, y1, (float)z };
        positions[i + 1] = { +x, y2, (float)z };

        dirQuats[i + 0] = XMQuaternionRotationRollPitchYaw(0, +XM_PIDIV2, 0);
        dirQuats[i + 1] = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, 0);
    }

    // generate names
    for (int i = 0; i < numEntts; ++i)
        names[i] += std::to_string(enttsIDs[i]);

    // ----------------------------------------------------

    const EntityID* ids = enttsIDs.data();

    mgr.AddTransformComponent(ids, numEntts, positions.data(), dirQuats.data(), uniformScales.data());
    mgr.AddNameComponent(ids, names.data(), numEntts);
    mgr.AddModelComponent(ids, lightPole.GetID(), numEntts);

    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
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
    //
    // create and setup spheres entities
    //
    Log::Debug();

    constexpr size numEntts = 10;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    
    // ---------------------------------------------------------
    // setup transform data for entities
    TransformData transform;

    transform.positions.reserve(numEntts);
    transform.dirQuats.resize(numEntts, {0,0,0,1});   // no rotation
    transform.uniformScales.resize(numEntts, 1.0f);

    // make two rows of the spheres
    for (u32 idx = 0; idx < numEntts /2; ++idx)
    {
        transform.positions.emplace_back(-5.0f, 5.0f, 10.0f * idx);
        transform.positions.emplace_back(+5.0f, 5.0f, 10.0f * idx);
    }

    mgr.AddTransformComponent(
        ids,
        numEntts,
        transform.positions.data(),
        transform.dirQuats.data(),
        transform.uniformScales.data());


    // ---------------------------------------------------------
    // setup movement for the spheres
    MovementData movement;

    movement.translations.resize(numEntts, { 0,0,0 });
    movement.rotQuats.resize(numEntts, { 0,0,0,1 });  // XMQuaternionRotationRollPitchYaw(0, 0.001f, 0));
    movement.uniformScales.resize(numEntts, 1.0f);
    movement.uniformScales[0] = 1.0f;

    mgr.AddMoveComponent(
        ids,
        movement.translations.data(),
        movement.rotQuats.data(),
        movement.uniformScales.data(),
        numEntts);

    transform.Clear();
    movement.Clear();	

    // ---------------------------------------------
    
    // generate a name for each sphere entity
    std::vector<EntityName> enttsNames(numEntts, "sphere_");

    for (int i = 0; const EntityID& id : enttsIDs)
        enttsNames[i++] += std::to_string(id);

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // ---------------------------------------------

    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddNameComponent(ids, enttsNames.data(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    // ---------------------------------------------

    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

    // add bounding component to each entity
    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes,
        model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateSkyBox(ECS::EntityMgr& mgr, SkyModel& sky)
{
    // create and setup the sky entity
    Log::Debug();

    TexPath skyTexPath;
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
        textureIdx = 0;

        // very cool dark gradient:
        colorCenter = { 0.5f, 0.5f, 0.5f };
        colorApex   = { 0.38f, 0.45f, 0.51f };
    }

    TextureMgr& texMgr = *TextureMgr::Get();
    TexID skyMapID = texMgr.LoadFromFile(g_RelPathTexDir + skyTexPath);

    // setup sky model
    sky.SetTexture(textureIdx, skyMapID);
    sky.SetColorCenter(colorCenter);
    sky.SetColorApex(colorApex);

    // setup sky entity
    const EntityID enttID = mgr.CreateEntity();
    mgr.AddTransformComponent(enttID, { 0,990,0 });
    mgr.AddNameComponent(enttID, "sky");
}

///////////////////////////////////////////////////////////

void CreateCylinders(ECS::EntityMgr& mgr, const BasicModel& model)
{
    //
    // create and setup cylinders entities
    //
    Log::Debug();

    constexpr size numEntts = 10;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

    TransformData transform;

    transform.positions.reserve(numEntts);
    transform.dirQuats.resize(numEntts, { 0,0,0,1 });  // no rotation
    transform.uniformScales.resize(numEntts, 1.0f);

    // make two rows of the cylinders
    for (int idx = 0; idx < numEntts / 2; ++idx)
    {
        transform.positions.emplace_back(-5.0f, 2.0f, 10.0f * idx);
        transform.positions.emplace_back(+5.0f, 2.0f, 10.0f * idx);
    }

    // ----------------------------------------------------
    // generate names for the entities

    std::vector<std::string> names(numEntts, "cylinder_");

    for (int i = 0; const EntityID id : enttsIDs)
        names[i++] += std::to_string(id);

    // ----------------------------------------------------
    // set a default texture for the cylinder mesh
    TextureMgr* pTexMgr        = TextureMgr::Get();
    const TexPath brickTexPath = g_RelPathTexDir + "brick01.dds";
    const TexID brickTexID     = pTexMgr->LoadFromFile(brickTexPath);

    // ----------------------------------------------------
    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    // setup bounding params
    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };
    const DirectX::BoundingBox* enttAABBs = model.GetSubsetsAABB();

    TexID textures[22]{ 0 };
    textures[TexType::DIFFUSE] = brickTexID;

    // ----------------------------------------------------

    mgr.AddTransformComponent(
        ids,
        numEntts,
        transform.positions.data(),
        transform.dirQuats.data(),
        transform.uniformScales.data());

    for (const EntityID id : enttsIDs)
        mgr.AddTexturedComponent(id, textures, 22, 0);

    mgr.AddNameComponent(ids, names.data(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);

    mgr.AddBoundingComponent(
        ids,
        numEntts,
        numSubsets,
        boundTypes,
        enttAABBs);
}

///////////////////////////////////////////////////////////

void CreateCubes(ECS::EntityMgr& mgr, const BasicModel& model)
{
    //
    // create and setup cubes entities
    //

    try
    {
        Log::Debug();
        constexpr int numEntts = 6;

        const std::vector<EntityName> enttsNames =
        {
            "cat",
            "fireflame",
            "wireFence",
            "woodCrate01",
            "woodCrate02",
            "box01",
        };

        const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
        const EntityID* ids = enttsIDs.data();

        Assert::True(std::ssize(enttsNames) == numEntts, "num of names != num of cubes");

        // make a map: 'entt_name' => 'entity_id'
        std::map<EntityName, EntityID> enttsNameToID;

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
            "box01",
        };

        const TexPath texFilenames[numEntts] =
        {
            "cat.dds",
            "fire_atlas.dds",
            "WireFence.dds",
            "WoodCrate01.dds",
            "WoodCrate02.dds",
            "box01d.dds",
        };

        TextureMgr& texMgr = *TextureMgr::Get();
        TexID texIDs[numEntts];  // just setup only diffuse texture for each cube

        // load textures
        for (int i = 0; i < numEntts; ++i)
            texIDs[i] = texMgr.LoadFromFile(g_RelPathTexDir + texFilenames[i]);

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
            positions[i] = { -15, 1, (float)2*i };

        positions[2] = { -7, -0.3f, 0 };


        // ---------------------------------------------------------
        // prepare textures transformations

        ECS::AtlasAnimParams atlasTexAnimParams;
        ECS::RotationAroundCoordParams rotAroundCoordsParams;

        atlasTexAnimParams.Push(15, 8, 4);
        rotAroundCoordsParams.Push(0.5f, 0.5f, 0.1f);

        // ---------------------------------------------------------
        // setup the cubes entities

        mgr.AddTransformComponent(ids, numEntts, positions.data(), dirQuats.data(), uniformScales.data());
        mgr.AddNameComponent(ids, enttsNames.data(), numEntts);
        mgr.AddModelComponent(ids, model.GetID(), numEntts);

        // the cube has only one submesh
        int subsetID = 0;

        const TexID unloadedTexID = TextureMgr::TEX_ID_UNLOADED;
        const u32 texTypesCount = Texture::TEXTURE_TYPE_COUNT;

        // cube_0: rotated cat
        TexID cat1TexIDs[texTypesCount]{ unloadedTexID };
        cat1TexIDs[TexType::DIFFUSE] = keysToTexIDs.at("cat");

        mgr.AddTexturedComponent(
            enttsNameToID.at("cat"),
            cat1TexIDs,
            texTypesCount,
            subsetID);

        // cube_1: firecamp animated
        TexID fireflameTexIDs[texTypesCount]{ unloadedTexID };
        fireflameTexIDs[TexType::DIFFUSE] = keysToTexIDs.at("fireAtlas");

        mgr.AddTexturedComponent(
            enttsNameToID.at("fireflame"),
            fireflameTexIDs,
            texTypesCount,
            subsetID);

        // cube_2: wirefence with alpha clipping
        std::vector<TexID> wireFenceTexIDs(texTypesCount, unloadedTexID);
        wireFenceTexIDs[TexType::DIFFUSE] = keysToTexIDs.at("wireFence");

        mgr.AddTexturedComponent(
            enttsNameToID.at("wireFence"),
            wireFenceTexIDs.data(),
            std::ssize(wireFenceTexIDs),
            subsetID);

        // cube_3: wood crate 1
        std::vector<TexID> woodCrate01TexIDs(texTypesCount, unloadedTexID);
        woodCrate01TexIDs[TexType::DIFFUSE] = keysToTexIDs.at("woodCrate01");

        mgr.AddTexturedComponent(
            enttsNameToID.at("woodCrate01"),
            woodCrate01TexIDs.data(),
            std::ssize(woodCrate01TexIDs),
            subsetID);

        // cube_4: wood crate 2
        TexID woodCrate02TexIDs[texTypesCount]{ unloadedTexID };
        woodCrate02TexIDs[TexType::DIFFUSE] = keysToTexIDs.at("woodCrate02");

        mgr.AddTexturedComponent(
            enttsNameToID.at("woodCrate02"),
            woodCrate02TexIDs,
            texTypesCount,
            subsetID);

        // cube_5: box01
        TexID box01TexIDs[texTypesCount]{ unloadedTexID };
        box01TexIDs[TexType::DIFFUSE] = keysToTexIDs.at("box01");

        mgr.AddTexturedComponent(
            enttsNameToID.at("box01"),
            box01TexIDs,
            texTypesCount,
            subsetID);

        // ------------------------------------------

        // some texture animation for some cubes
        mgr.AddTextureTransformComponent(
            enttsNameToID.at("fireflame"),
            ECS::TexTransformType::ATLAS_ANIMATION,
            atlasTexAnimParams);

        mgr.AddTextureTransformComponent(
            enttsNameToID.at("cat"),
            ECS::TexTransformType::ROTATION_AROUND_TEX_COORD,
            rotAroundCoordsParams);


        // setup rendering params
        ECS::RenderInitParams renderParams;
        renderParams.shaderType = ECS::LIGHT_SHADER;
        renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        mgr.AddRenderingComponent(ids, numEntts, renderParams);

        // setup blending params of the entities
        using enum ECS::RSTypes;
        mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("wireFence"), { ALPHA_CLIPPING, CULL_NONE });

        mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("cat"), ADDING);
        //enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate01"), ADDING);
        //enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate02"), MULTIPLYING);


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
        Log::Error(e);
        Log::Error("can't create and setup cubes entts");
    }
}

///////////////////////////////////////////////////////////

void CreateTerrain(ECS::EntityMgr& mgr, const BasicModel& model)
{
    //
    // create and setup terrain elements
    //
    Log::Debug();

    // create and setup a terrain entity
    const EntityID enttID = mgr.CreateEntity();

    // setup a transformation for the terrain's texture (scale it)
    ECS::StaticTexTransParams terrainTexTransform;
    terrainTexTransform.Push(DirectX::XMMatrixScaling(50, 50, 0));

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    mgr.AddTransformComponent(enttID, { 0, 0, 0 });
    mgr.AddNameComponent(enttID, model.GetName());
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddTextureTransformComponent(enttID, ECS::TexTransformType::STATIC, terrainTexTransform);
    mgr.AddRenderingComponent(enttID, renderParams);

    // add bounding component to each subset of this entity
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)

    Log::Debug("Terrain is created");
}

///////////////////////////////////////////////////////////

void CreateWater(ECS::EntityMgr& mgr)
{
#if 0
    //
    // create and setup water model 
    //

    Log::Debug();

    try
    {

    // create and setup a water mesh
    ModelsCreator modelCreator;

    const float width = 500;
    const float depth = 500;

    const MeshID waterMeshID = modelCreator.CreateWater(pDevice, width, depth);


    // setup a texture transformation
    ECS::StaticTexTransParams waterTexTransform;
    waterTexTransform.Push(DirectX::XMMatrixScaling(50, 50, 0), DirectX::XMMatrixTranslation(0.1f, 0.1f, 0.0f));

    // get BOUNDING for the entt
    DirectX::BoundingBox aabb;
    MeshStorage::Get()->GetBoundingDataByID(waterMeshID, aabb);



    using enum ECS::RSTypes;

    // create and setup a water entity
    EntityID waterEnttID = mgr.CreateEntity();

    mgr.AddTransformComponent(waterEnttID, { 0, 0, 0 }, DirectX::XMQuaternionRotationRollPitchYaw(XM_PIDIV2, 0.0f, 0.0f));
    mgr.AddNameComponent(waterEnttID, "water");
    mgr.AddMeshComponent(waterEnttID, { waterMeshID });
    //mgr.AddRenderingComponent({ waterEnttID });

    mgr.renderStatesSystem_.UpdateStates( waterEnttID, { TRANSPARENCY, REFLECTION_PLANE });
    mgr.AddTextureTransformComponent(ECS::TexTransformType::STATIC, waterEnttID, waterTexTransform);
    mgr.AddBoundingComponent(waterEnttID, aabb, ECS::BoundingType::BOUND_BOX);

    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error("can't create a water entity");
    }
    catch (ECS::LIB_Exception& e)
    {
        Log::Error(e.GetStr());
        Log::Error("can't create a water entity");
    }
#endif
}

///////////////////////////////////////////////////////////

void CreateSkull(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a skull entity
    Log::Debug();

    try
    {
        const EntityID enttID = mgr.CreateEntity();

        // setup rendering params
        ECS::RenderInitParams renderParams;
        renderParams.shaderType   = ECS::LIGHT_SHADER;
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
        Log::Error(e, true);
        Log::Error("can't create a skull model");
    }
}

///////////////////////////////////////////////////////////

void CreatePlanes(ECS::EntityMgr& mgr, const BasicModel& model)
{
    Log::Debug();

    try 
    {
        constexpr int numEntts = 2;
        const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
        const EntityID* ids = enttsIDs.data();

        // ---------------------------------------------------------
        // setup transform data for entities

        TransformData transform;

        transform.positions.resize(numEntts, { 0,0,0 });
        transform.dirQuats.resize(numEntts, { 0,0,0,1 });  // no rotation
        transform.uniformScales.resize(numEntts, 1.0f);

        transform.positions[0] = { 4, 1.5f, 6 };
        transform.positions[1] = { 2, 1.5f, 6 };

        mgr.AddTransformComponent(
            ids,
            numEntts,
            transform.positions.data(),
            transform.dirQuats.data(),
            transform.uniformScales.data());

        // ---------------------------------------------------------
        // setup names for the entities

        std::vector<std::string> names(numEntts, "plane_");

        for (int i = 0; const EntityID id : enttsIDs)
            names[i++] += std::to_string(id);

        // ---------------------------------------------------------
        // setup meshes of the entities

        TextureMgr& texMgr = *TextureMgr::Get();

        constexpr int numTexPaths = 2;
        const TexPath texPaths[numTexPaths] =
        {
            "data/textures/brick01.dds",
            "data/textures/sprite01.tga",
        };

        TexID texIDs[numTexPaths];

        // create textures
        for (int i = 0; i < numTexPaths; ++i)
            texIDs[i] = texMgr.LoadFromFile(texPaths[i]);


        // setup rendering params
        ECS::RenderInitParams renderParams;
        renderParams.shaderType   = ECS::LIGHT_SHADER;
        renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        mgr.AddModelComponent(ids, model.GetID(), numEntts);
        mgr.AddNameComponent(ids, names.data(), numEntts);
        mgr.AddRenderingComponent(ids, numEntts, renderParams);

        // ---------------------------------------------------------
        // setup render states of the entities
        //using enum ECS::RSTypes;

        //mgr.renderStatesSystem_.UpdateStates(enttsIDs[0], ADDING);
        //mgr.renderStatesSystem_.UpdateStates(enttsIDs[1], SUBTRACTING);

    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error("can't create plane entities");
    }
    catch (ECS::LIB_Exception& e)
    {
        Log::Error(e.GetStr());
        Log::Error("can't create plane entities");
    }

}

///////////////////////////////////////////////////////////

void CreateTreesPine(ECS::EntityMgr& mgr, const BasicModel& model)
{
    Log::Debug();

    using enum ECS::RSTypes;
    constexpr int numEntts = 50;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------

    std::vector<EntityName> names(numEntts, "tree_pine_");
    std::vector<XMFLOAT3>   positions(numEntts);
    std::vector<XMVECTOR>   quats(numEntts);
    std::vector<float>      uniScales(numEntts, 1.0f);


    // generate a name for each tree
    for (int i = 0; EntityName & name : names)
    {
        name += std::to_string(enttsIDs[i++]);
    }

    // generate positions
    for (XMFLOAT3& pos : positions)
    {
        pos.x = MathHelper::RandF(-150, 150);
        pos.z = MathHelper::RandF(-150, 150);
        pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);
    }

    positions[0] = {0,0,0};

    // generate direction quats
    for (int i = 0; i < numEntts; ++i)
    {
        float angleY = MathHelper::RandF(0, 314) * 0.01f;
        quats[i] = XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, angleY, 0);
    }

    // generate a scale value for each tree
    for (int i = 0; i < numEntts; ++i)
    {
        //uniScales[i] += MathHelper::RandF(0.0f, 50.0f) * 0.01f;
        uniScales[i] = 0.01f;
    }

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions.data(), quats.data(), uniScales.data());
    mgr.AddNameComponent(ids, names.data(), numEntts);
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

void CreateTreesSpruce(ECS::EntityMgr& mgr, const BasicModel& model)
{
    Log::Debug();

    using enum ECS::RSTypes;
    constexpr int numEntts = 50;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------

    std::vector<EntityName> names(numEntts, "tree_spruce_");
    std::vector<XMFLOAT3>   positions(numEntts);
    std::vector<XMVECTOR>   quats(numEntts);
    std::vector<float>      uniScales(numEntts, 3.5f);


    // generate a name for each tree
    for (int i = 0; EntityName & name : names)
    {
        name += std::to_string(enttsIDs[i++]);
    }

    // generate positions
    for (XMFLOAT3& pos : positions)
    {
        pos.x = MathHelper::RandF(-150, 150);
        pos.z = MathHelper::RandF(-150, 150);
        pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);
    }

    positions[0] = { 0,0,0 };

    // generate direction quats
    for (int i = 0; i < numEntts; ++i)
    {
        float angleY = MathHelper::RandF(0, 314) * 0.01f;
        quats[i] = XMQuaternionRotationAxis({ 0,1,0 }, DirectX::XM_PIDIV2);
    }

    // generate a scale value for each tree
    for (int i = 0; i < numEntts; ++i)
    {
        uniScales[i] += MathHelper::RandF(0.0f, 50.0f) * 0.01f;
    }

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    // add components to each tree entity
    mgr.AddTransformComponent(ids, numEntts, positions.data(), quats.data(), uniScales.data());
    mgr.AddNameComponent(ids, names.data(), numEntts);
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
    // create and setup a power lines entts

    Log::Debug();

    // create and setup an entity
    constexpr int numEntts = 3;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // apply the same rotation and scale for each entt
    XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, 0, 0);
    const std::vector<XMVECTOR> quats(numEntts, rotQuat);
    const std::vector<float>    scales(numEntts, 0.5f);

    // a position for each power line
    std::vector<XMFLOAT3> positions =
    {
        { -200, 0, 8 },
        { 0,    0, 8 },
        { +200, 0, 8 },
    };

    for (XMFLOAT3& pos : positions)
        pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z) + 2.0f;


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
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(ids, numEntts, positions.data(), quats.data(), scales.data());
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddModelComponent(ids, model.GetID(), numEntts);
    mgr.AddRenderingComponent(ids, numEntts, renderParams);

    mgr.AddBoundingComponent(
        enttsIDs.data(),
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateBarrel(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a barrel entity

    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, { 0, 2, -5 });
    mgr.AddNameComponent(enttID, "barrel");
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

void CreateStalkerFreedom(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a stalker entity
    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    float posY = GetHeightOfGeneratedTerrainAtPoint(7, 3);
    DirectX::XMFLOAT3 pos = { 7, posY, 3 };
    DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(XM_PIDIV2, XM_PIDIV2, 0);

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, pos, rotQuat, 5.0f);
    mgr.AddNameComponent(enttID, "stalker_freedom");
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

void CreateNanoSuit(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a nanosuit entity
    Log::Debug();

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
    Log::Debug();

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
    Log::Debug();
    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 pos = { -50, 1.3f, 60 };
    const XMVECTOR dirQuat = { 0,0,0,1 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // add bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
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
    Log::Debug();
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
    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = { 10, 2, 10 };
    const XMVECTOR dirQuat   = { 0, 0, 0, 1 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "ak_47");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateAk74(ECS::EntityMgr& mgr, const BasicModel& model)
{
    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = { 10, 2, 6 };
    const XMVECTOR dirQuat   = { 0, 0, 0, 1 };
    const float uniformScale = 5.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
    const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);


    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
    mgr.AddNameComponent(enttID, "ak_74");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        1,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateHouse(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a house entity

    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const float posY      = GetHeightOfGeneratedTerrainAtPoint(34, 43) + 2.5f;
    const XMFLOAT3 pos    = { 34, posY, 43 };
    const XMVECTOR rotVec = { DirectX::XM_PIDIV2, 0,0 };
    const XMVECTOR quat   = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotVec);
    const float uniScale  = 1.0f;

    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // setup bounding params
    const size numEntts = 1;
    const size numSubsets = model.GetNumSubsets();
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
    // create and setup a house entity
    Log::Debug();

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const float posY      = GetHeightOfGeneratedTerrainAtPoint(-20, 107);
    const XMFLOAT3 pos    = { -20, posY, 107 };
    const XMVECTOR quat   = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, DirectX::XM_PIDIV2, 0);

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
    // create and setup rock entities

    constexpr int numEntts = 20;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
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
    for (int i = 0; std::string& name : names)
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
    const double factor = (1.0 / ModelImporter::s_ImportDuration_) * 100.0;

    Log::Print();
    Log::Print("Summary about import process:", eConsoleColor::YELLOW);
    Log::Print(std::format("time spent to import:        {}", ModelImporter::s_ImportDuration_));
    Log::Print(std::format("time spent to load scene:    {} ({:.2} percent)", ModelImporter::s_SceneLoading_, ModelImporter::s_SceneLoading_ * factor));
    Log::Print(std::format("time spent to load nodes:    {} ({:.2} percent)", ModelImporter::s_NodesLoading_, ModelImporter::s_NodesLoading_ * factor));

    Log::Print(std::format("time spent to load vertices: {} ({:.2} percent)", ModelImporter::s_VerticesLoading_, ModelImporter::s_VerticesLoading_ * factor));
    Log::Print(std::format("time spent to load textures: {} ({:.2} percent)", ModelImporter::s_TexLoadingDuration_, ModelImporter::s_TexLoadingDuration_ * factor));
    Log::Print();
}

///////////////////////////////////////////////////////////

void ImportExternalModels(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    // 1. import models from different external formats (.obj, .blend, .fbx, etc.)
    // 2. create relative entities

    ModelsCreator creator;
    ModelStorage& storage = *ModelStorage::Get();

    // paths to external models
    const std::string lightPolePath             = g_RelPathExtModelsDir + "light_pole/light_pole.obj";
    const std::string treeSprucePath            = g_RelPathExtModelsDir + "trees/tree_spruce/tree_spruce.obj";
    const std::string treePinePath              = g_RelPathExtModelsDir + "trees/FBX format/tree_pine.fbx";
    //const std::string treeDubPath             = g_RelPathExtModelsDir + "trees/dub/dub.obj";
    const std::string powerHVTowerPath          = g_RelPathExtModelsDir + "power_line/Power_HV_Tower.FBX";
    const std::string barrelPath                = g_RelPathExtModelsDir + "Barrel1/Barrel1.obj";
    const std::string nanosuitPath              = g_RelPathExtModelsDir + "nanosuit/nanosuit.obj";
    const std::string stalkerFreedom1Path       = g_RelPathExtModelsDir + "stalker_freedom_1/stalker_freedom_1.fbx";
    const std::string stalkerHouseSmallPath     = g_RelPathExtModelsDir + "stalker/stalker-house/source/SmallHouse.fbx";
    const std::string stalkerHouseAbandonedPath = g_RelPathExtModelsDir + "stalker/abandoned-house-20/source/StalkerAbandonedHouse.fbx";
    const std::string ak47Path                  = g_RelPathExtModelsDir + "aks-74_game_ready/scene.gltf";
    const std::string ak74uPath                 = g_RelPathExtModelsDir + "ak_74u/ak_74u.fbx";

    const std::string building9Path             = g_RelPathExtModelsDir + "building9/building9.obj";
    const std::string apartmentPath             = g_RelPathExtModelsDir + "Apartment/Apartment.obj";
    const std::string sovientStatuePath         = g_RelPathExtModelsDir + "sovietstatue_1/sickle&hammer.obj";
          
    // import a model from file by path
    Log::Debug("Start of models importing");

    const ModelID lightPoleID      = creator.ImportFromFile(pDevice, lightPolePath);
    const ModelID treeSpruceID     = creator.ImportFromFile(pDevice, treeSprucePath);
    const ModelID treePineID       = creator.ImportFromFile(pDevice, treePinePath);
    const ModelID nanosuitID       = creator.ImportFromFile(pDevice, nanosuitPath);
    const ModelID stalkerFreedomID = creator.ImportFromFile(pDevice, stalkerFreedom1Path);
    const ModelID stalkerHouse1ID  = creator.ImportFromFile(pDevice, stalkerHouseSmallPath);
    const ModelID stalkerHouse2ID  = creator.ImportFromFile(pDevice, stalkerHouseAbandonedPath);
    const ModelID ak47ID           = creator.ImportFromFile(pDevice, ak47Path);
    const ModelID ak74ID           = creator.ImportFromFile(pDevice, ak74uPath);
    const ModelID barrelID         = creator.ImportFromFile(pDevice, barrelPath);
    const ModelID powerHVTowerID   = creator.ImportFromFile(pDevice, powerHVTowerPath);
    
   
    const ModelID buildingID       = creator.ImportFromFile(pDevice, building9Path);
    const ModelID apartmentID      = creator.ImportFromFile(pDevice, apartmentPath);
    const ModelID sovietStatueID   = creator.ImportFromFile(pDevice, sovientStatuePath);


    PrintImportTimingInfo();

    Log::Debug("All the models are imported successfully");

    // get models by its ids
    BasicModel& building        = storage.GetModelByID(buildingID);
    BasicModel& apartment       = storage.GetModelByID(apartmentID);
    BasicModel& sovietStatue    = storage.GetModelByID(sovietStatueID);
    BasicModel& treeSpruce      = storage.GetModelByID(treeSpruceID);
    BasicModel& treePine        = storage.GetModelByID(treePineID);
    BasicModel& powerHVTower    = storage.GetModelByID(powerHVTowerID);
    BasicModel& nanosuit        = storage.GetModelByID(nanosuitID);
    BasicModel& stalkerFreedom  = storage.GetModelByID(stalkerFreedomID);
    BasicModel& lightPole       = storage.GetModelByID(lightPoleID);
    BasicModel& barrel          = storage.GetModelByID(barrelID);
    BasicModel& stalkerHouse1   = storage.GetModelByID(stalkerHouse1ID);
    BasicModel& stalkerHouse2   = storage.GetModelByID(stalkerHouse2ID);
    BasicModel& ak47            = storage.GetModelByID(ak47ID);
    BasicModel& ak74            = storage.GetModelByID(ak74ID);

    // setup some models (set textures, setup materials)
    SetupStalkerSmallHouse(stalkerHouse1);
    SetupStalkerAbandonedHouse(stalkerHouse2);
    SetupTree(treePine);
    SetupPowerLine(powerHVTower);
    SetupBuilding9(building);
    SetupAk47(ak47);

    CreateTreesPine(mgr, treePine);
    CreateTreesSpruce(mgr, treeSpruce);
    CreatePowerLine(mgr, powerHVTower);
    //CreateLightPoles(mgr, lightPole);
    //CreateHouse(mgr, stalkerHouse1);
    //CreateHouse2(mgr, stalkerHouse2);

    CreateAk47(mgr, ak47);
    CreateAk74(mgr, ak74);
    CreateBarrel(mgr, barrel);
    CreateNanoSuit(mgr, nanosuit);
    CreateStalkerFreedom(mgr, stalkerFreedom);
    CreateBuilding(mgr, building);
    CreateApartment(mgr, apartment);
    CreateSovietStatue(mgr, sovietStatue);
}

///////////////////////////////////////////////////////////

void GenerateAssets(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    ModelsCreator creator;
    ModelStorage& storage = *ModelStorage::Get();
        
    //
    // create sky
    //
    creator.CreateSkyCube(pDevice, 2000);
    //creator.CreateSkySphere(pDevice, 2000, 30, 30);
    //MeshSphereParams skyDomeSphereParams(2000, 30, 30);
    //const ModelID skyBoxID = creator.CreateSphere(pDevice, skyDomeSphereParams);
    SkyModel& skyBox = storage.GetSky();

    //
    // create terrain
    //
    const ModelID terrainID = creator.CreateGeneratedTerrain(pDevice, 500, 500, 501, 501);
    BasicModel& terrain = storage.GetModelByID(terrainID);
    SetupTerrain(terrain);
    CreateTerrain(mgr, terrain);

    // generate some models manually
    const MeshSphereParams    boundSphereParams(1, 8, 8);
    const MeshGeosphereParams boundGeoSphereParams(1, 1);
    const MeshSphereParams    sphereParams(1, 10, 10);

    const ModelID cubeID        = creator.CreateCube(pDevice);
    const ModelID boundSphereID = creator.CreateGeoSphere(pDevice, boundGeoSphereParams);
    const ModelID sphereID      = creator.CreateSphere(pDevice, sphereParams);

    // get actual model by its ID
    BasicModel& cube            = storage.GetModelByID(cubeID);
    BasicModel& sphere          = storage.GetModelByID(sphereID);
    BasicModel& boundSphere     = storage.GetModelByID(boundSphereID);

    boundSphere.SetName("bound_sphere");

    // manual setup of some models
    SetupSphere(sphere);

    // create and setup entities with models
    CreateSkyBox (mgr, skyBox);
    CreateCubes  (mgr, cube);
    CreateSpheres(mgr, sphere);
}

///////////////////////////////////////////////////////////

void LoadAssets(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    // load models from the internal .de3d format

    ModelsCreator creator;
    ModelStorage& storage = *ModelStorage::Get();

    storage.Deserialize(pDevice);

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

    CreateStalkerFreedom(mgr, storage.GetModelByName("stalker_freedom_1"));
    
#if 0
    // load models from the internal asset format and get its IDs
    ModelID treeID          = creator.CreateFromDE3D(pDevice, "tree_pine/tree_pine.de3d");
    ModelID powerHVTowerID  = creator.CreateFromDE3D(pDevice, "power_hw_tower/power_hw_tower.de3d");
    ModelID lightPoleID     = creator.CreateFromDE3D(pDevice, "light_pole/light_pole.de3d");
    ModelID stalkerHouse1ID = creator.CreateFromDE3D(pDevice, "stalker_house_1/stalker_house_1.de3d");
    ModelID stalkerHouse2ID = creator.CreateFromDE3D(pDevice, "stalker_house_2/stalker_house_2.de3d");
    ModelID ak47ID          = creator.CreateFromDE3D(pDevice, "ak_47/ak_47.de3d");

    ModelID barrelID        = creator.CreateFromDE3D(pDevice, "barrel/barrel.de3d");
    ModelID nanosuitID      = creator.CreateFromDE3D(pDevice, "nanosuit/nanosuit.de3d");
    ModelID buildingID      = creator.CreateFromDE3D(pDevice, "soviet_building/building.de3d");
    ModelID apartmentID     = creator.CreateFromDE3D(pDevice, "soviet_building/apartment.de3d");
    ModelID sovietStatueID  = creator.CreateFromDE3D(pDevice, "soviet_statue/soviet_statue.de3d");

    // get models
    BasicModel& tree            = storage.GetModelByID(treeID);
    BasicModel& powerHVTower    = storage.GetModelByID(powerHVTowerID);
    BasicModel& lightPole       = storage.GetModelByID(lightPoleID);
    BasicModel& stalkerHouse1   = storage.GetModelByID(stalkerHouse1ID);
    BasicModel& stalkerHouse2   = storage.GetModelByID(stalkerHouse2ID);
    BasicModel& ak47            = storage.GetModelByID(ak47ID);
    
    BasicModel& barrel          = storage.GetModelByID(barrelID);
    BasicModel& nanosuit        = storage.GetModelByID(nanosuitID);
    BasicModel& building        = storage.GetModelByID(buildingID);
    BasicModel& apartment       = storage.GetModelByID(apartmentID);
    BasicModel& sovietStatue    = storage.GetModelByID(sovietStatueID);


    // create and setup entities with models
    CreateTrees( mgr, tree);
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

bool InitializeGraphics::InitializeModels(
    ID3D11Device* pDevice, 
    ID3D11DeviceContext* pDeviceContext,
    ECS::EntityMgr& mgr,
    const Settings & settings,
    const float farZ)
{
    // initialize all the models on the scene

    Log::Print();
    Log::Print("------------------------------------------------------------", eConsoleColor::YELLOW);
    Log::Print("                 INITIALIZATION: MODELS                     ", eConsoleColor::YELLOW);
    Log::Print("------------------------------------------------------------", eConsoleColor::YELLOW);
    Log::Debug();

    try
    {
        ModelsCreator creator;
        ModelStorage& storage = *ModelStorage::Get();
        TextureMgr& texMgr = *TextureMgr::Get();

        // create a texture 2D array for trees billboards
        const std::vector<std::string> treeTexPaths =
        {
                g_RelPathTexDir + "tree_pine_diffuse_512.dds",
                g_RelPathTexDir + "tree1.dds",
                g_RelPathTexDir + "tree2.dds",
                g_RelPathTexDir + "tree3.dds"
        };

        texMgr.LoadFromFileTexture2DArray(treeTexPaths, DXGI_FORMAT_R8G8B8A8_UNORM);


        // create a cube which will serve for us as an invalid model
        const ModelID cubeID     = creator.CreateCube(pDevice);
        BasicModel& invalidModel = storage.GetModelByID(cubeID);
        TexID noTextureID = texMgr.LoadFromFile(g_RelPathTexDir + "notexture.dds");
        
        invalidModel.SetName("invalid_model");
        invalidModel.SetTexture(0, TexType::DIFFUSE, noTextureID);

        // NOTE: the bounding line box model must be created first of all, before all the other models
        const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);


#if 1
        if (fs::exists(g_RelPathAssetsDir))
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


        GenerateAssets(pDevice, mgr);
   
        //exit(-1);
#if 0

        // paths for loading
        const std::string powerHWTowerLoadPath = "power_line/Power_HV_Tower.de3d";
        const std::string nanosuitLoadPath = "nanosuit/nanosuit.de3d";
        const std::string terrainLoadPath = "terrain_generated_500_500/terrain_generated_500_500.de3d";
        const std::string stalkerHouse1LoadPath = "stalker_house_abandoned/stalkerHouseAbandoned.de3d";
        const std::string stalkerHouse2LoadPath = "stalker_house_small/stalkerHouseSmall.de3d";
        const std::string treeLoadPath = "tree_conifer_macedonian_pine/tree_conifer_macedonian_pine.de3d";

        // LOAD model from the internal format
        const ModelID powerHVTowerID = creator.CreateFromDE3D(pDevice, powerHWTowerLoadPath);
        const ModelID terrainID = modelCreator.CreateFromDE3D(pDevice, terrainLoadPath);

        // paths for import
        const std::string m3dDirPath = g_ModelsDirPath + "ModelsM3D/";
        const std::string pathRock = m3dDirPath + "rock.m3d";
        const std::string pathPillar1 = m3dDirPath + "pillar1.m3d";
        const std::string pathPillar2 = m3dDirPath + "pillar2.m3d";
        const std::string pathPillar5 = m3dDirPath + "pillar5.m3d";
        const std::string pathPillar6 = m3dDirPath + "pillar6.m3d";
    
        // import external models and get its ids
        const ModelID rockID = creator.ImportFromFile(pDevice, pathRock);
        const ModelID pillar1ID = creator.ImportFromFile(pDevice, pathPillar1);
        const ModelID pillar2ID = creator.ImportFromFile(pDevice, pathPillar2);
        const ModelID pillar5ID = creator.ImportFromFile(pDevice, pathPillar5);
        const ModelID pillar6ID = creator.ImportFromFile(pDevice, pathPillar6);
        

        // generated models
        const MeshCylinderParams cylParams;
        const ModelID planeID = creator.CreatePlane(pDevice);
        const ModelID skullID = creator.CreateSkull(pDevice);
        const ModelID cylinderID = creator.CreateCylinder(pDevice, cylParams);
        const ModelID terrainID = creator.CreateGeneratedTerrain(pDevice, 500, 500, 501, 501);
        
        // get model instances
        BasicModel& skull    = storage.GetModelByID(skullID);
        BasicModel& cylinder = storage.GetModelByID(cylinderID);	
        BasicModel& rock     = storage.GetModelByID(rockID);
        BasicModel& pillar1  = storage.GetModelByID(pillar1ID);
        BasicModel& pillar2  = storage.GetModelByID(pillar2ID);
        BasicModel& pillar5  = storage.GetModelByID(pillar5ID);
        BasicModel& pillar6  = storage.GetModelByID(pillar6ID);
        BasicModel& terrain  = storage.GetModelByID(terrainID);
    
        // create entts
        CreateCylinders(pDevice, mgr, cylinder);
        CreateSkull(pDevice, mgr, skull);	
        CreateRocks(pDevice, mgr, rock);
        CreatePillar(pDevice, mgr, pillar1);
        CreatePillar(pDevice, mgr, pillar2);
        CreatePillar(pDevice, mgr, pillar5);
        CreatePillar(pDevice, mgr, pillar6);
        CreateTerrain(pDevice, mgr, terrain);
        
#endif

    }
    catch (const std::out_of_range& e)
    {
        Log::Error(e.what());
        Log::Error("went out of range");
        return false;
    }
    catch (const std::bad_alloc & e)
    {
        Log::Error(e.what());
        Log::Error("can't allocate memory for some element");
        return false;
    }
    catch (EngineException & e)
    {
        Log::Error(e);
        Log::Error("can't initialize models");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeLightSources(
    ECS::EntityMgr& mgr,
    const Settings& settings)
{
    // this function initializes all the light sources on the scene

    using namespace DirectX;

    constexpr int numDirLights = 3;
    constexpr int numPointLights = 20;
    constexpr int numSpotLights = 2;

    // -----------------------------------------------------------------------------
    //                 DIRECTIONAL LIGHTS: SETUP AND CREATE
    // -----------------------------------------------------------------------------

    if constexpr (numDirLights > 0)
    {

        ECS::DirLightsInitParams dirLightsParams;
        dirLightsParams.data.resize(numDirLights);

        ECS::DirLight& dirLight0 = dirLightsParams.data[0];
        ECS::DirLight& dirLight1 = dirLightsParams.data[1];
        ECS::DirLight& dirLight2 = dirLightsParams.data[2];

        // setup main directed light source
        dirLight0.ambient   = { 0.6f, 0.6f, 0.6f, 1.0f };
        dirLight0.diffuse   = { 0.8f, 0.8f, 0.8f, 1.0f };
        dirLight0.specular  = { 0.3f, 0.3f, 0.3f, 1.0f };
        dirLight0.direction = { 0.57735f, -0.9f, 0.57735f };

        // setup 2nd directed light source
        dirLight1.ambient   = { 0.0f, 0.0f, 0.0f, 1.0f };
        dirLight1.diffuse   = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight1.specular  = { 0.25f, 0.25f, 0.25f, 1.0f };
        dirLight1.direction = { -0.57735f, -0.57735f, 0.57735f };

        // setup 3rd directed light source
        dirLight2.ambient   = { 0.0f, 0.0f, 0.0f, 1.0f };
        dirLight2.diffuse   = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight2.specular  = { 0.0f, 0.0f, 0.0f, 1.0f };
        dirLight2.direction = { 0.0f, -0.707f, -0.707f };


        // create directional light entities and add components to them
        const ECS::cvector<EntityID> dirLightsIds = mgr.CreateEntities(numDirLights);
        const EntityID* ids = dirLightsIds.data();

        const std::string names[numDirLights] =
        {
            "dir_light_1",
            "dir_light_2",
            "dir_light_3"
        };

        mgr.AddLightComponent(ids, numDirLights, dirLightsParams);
        mgr.AddNameComponent(ids, names, numDirLights);

     
        // add transform component to each directed light because we may need to manipulate directed lights icons (in editor we can change icons positions in the scene or manipulate light direction using gizmo) 
        for (index i = 0; i < (size)numDirLights; ++i)
        {
            const XMFLOAT3 pos = { 3, 3, (float)i };
            const XMFLOAT3 dir = dirLightsParams.data[i].direction;
            const XMVECTOR dirQuat = { dir.x, dir.y, dir.z };

            mgr.AddTransformComponent(dirLightsIds[i], pos, dirQuat, 1.0f);
        }
    }


    // -----------------------------------------------------------------------------
    //                    POINT LIGHTS: SETUP AND CREATE
    // -----------------------------------------------------------------------------

    if (numPointLights > 0)
    {
        ECS::PointLightsInitParams pointLightsParams;
        pointLightsParams.data.resize(numPointLights);

        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].ambient = { 0.2f, 0.2f, 0.2f, 1.0f };

        // generate diffuse and specular color for each point light source
        for (int i = 1; i < numPointLights; ++i)
        {
            const XMFLOAT4 color = MathHelper::RandColorRGBA();
            XMFLOAT4& diffColor = pointLightsParams.data[i].diffuse;
            XMFLOAT4& specColor = pointLightsParams.data[i].specular;

            diffColor = color * 0.6f;
            diffColor.w = 1.0f;

            specColor = color * 0.1f;
            specColor.w = 1.0f;
        }

        // setup attenuation params
        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].att = { 0, 0.1f, 0 };

        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].range = 30;

        // generate random position for each point light source
        for (index i = 0; i < numPointLights; ++i)
        {
            XMFLOAT3 pos;
            pos.x = MathHelper::RandF(-100, 100);
            pos.y = 4.0f;
            pos.z = MathHelper::RandF(-100, 100);

            pointLightsParams.data[i].position = pos;
        }

        // ------------------------------------------------
 
        // setup transformation params because we may need to manipulate point lights icons (in editor we can change icons positions in the scene or manipulate light position using gizmo) 
        XMFLOAT3 positions[numPointLights];
        XMVECTOR dirQuats[numPointLights];
        float uniformScales[numPointLights];

        for (index i = 0; i < numPointLights; ++i)
            positions[i] = pointLightsParams.data[i].position;

        for (index i = 0; i < numPointLights; ++i)
            dirQuats[i] = { 0,0,0,1 };

        for (index i = 0; i < numPointLights; ++i)
            uniformScales[i] = pointLightsParams.data[i].range;


        // generate name for each point light src
        std::string names[numPointLights];

        for (int i = 0; std::string& name : names)
            name = "point_light_" + std::to_string(i);

        // setup bounding params
        const size numSubsets = 1;
        const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::SPHERE);
        DirectX::BoundingSphere boundSpheres[numPointLights];

        // setup bounding sphere for each point light src
        for (int i = 0; i < numPointLights; ++i)
            boundSpheres[i].Center = pointLightsParams.data[i].position;

        for (int i = 0; i < numPointLights; ++i)
            boundSpheres[i].Radius = pointLightsParams.data[i].range;


        // ------------------------------------------------
        // create and setup point light entities

        const ECS::cvector<EntityID> pointLightsIds = mgr.CreateEntities(numPointLights);
        const EntityID* ids = pointLightsIds.data();

        mgr.AddTransformComponent(ids, numPointLights, positions, dirQuats, uniformScales);
        mgr.AddLightComponent(ids, numPointLights, pointLightsParams);
        mgr.AddNameComponent(ids, names, numPointLights);
        //mgr.AddBoundingComponent(ids, boundSpheres, numPointLights);
    }
    

    // -----------------------------------------------------------------------------
    //                   SPOT LIGHTS: SETUP AND CREATE
    // -----------------------------------------------------------------------------

#if 0
    if (numSpotLights > 0)
    {
        ECS::SpotLightsInitParams spotLightsParams;

        spotLightsParams.ambients.resize(numSpotLights, { 0.01f, 0.01f, 0.01f, 1.0f });
        spotLightsParams.diffuses.resize(numSpotLights, { 0.1f, 0.1f, 0.1f, 1.0f });
        spotLightsParams.speculars.resize(numSpotLights, { 0, 0, 0, 1 });

        spotLightsParams.positions.resize(numSpotLights, { 0, 0, 0 });
        spotLightsParams.directions.resize(numSpotLights, { 0, -DirectX::XM_PIDIV2, 0 });

        const float PI_DIV6 = DirectX::XM_PI * 0.333f;

        // generate 2 rows of spot light sources
        for (int i = 0, z = 0; i < numSpotLights / 2; z += 30, i += 2)
        {
            spotLightsParams.positions[i + 0] = { -8, 10, (float)z };
            spotLightsParams.positions[i + 1] = { +8, 10, (float)z };
        }

        for (int i = 0; i < numSpotLights / 2; i += 2)
        {
            spotLightsParams.directions[i + 0].y -= PI_DIV6;
            spotLightsParams.directions[i + 1].y += PI_DIV6;
        }

        spotLightsParams.attenuations.resize(numSpotLights, { 1.0f, 100.0f, 100.0f });
        spotLightsParams.ranges.resize(numSpotLights, 100);
        spotLightsParams.spotExponents.resize(numSpotLights, 5);


        //
        // create and setup spotlight entities
        //
        std::vector<EntityID> spotLightsIds = mgr.CreateEntities(numSpotLights);
        const EntityID flashLightID = spotLightsIds[0];

        std::vector<EntityName> spotLightsNames(numSpotLights, "spot_light_");
        spotLightsNames[0] = "flashlight";

        // generate names for each spot light (except of the flashlight: so we start from 1)
        for (int i = 1; i < numSpotLights; ++i)
            spotLightsNames[i] += std::to_string(spotLightsIds[i]);


        // generate transform data for spotlight sources
        std::vector<XMVECTOR> dirQuats(numSpotLights);
        const std::vector<float> uniformScales(numSpotLights, 1.0f);

        for (int i = 0; i < numSpotLights; ++i)
        {
            // convert light direction into quaternion and normalize it
            dirQuats[i] = XMQuaternionNormalize(XMLoadFloat3(&spotLightsParams.directions[i]));
        }

        // add components to each spotlight entity
        mgr.AddTransformComponent(spotLightsIds, spotLightsParams.positions, dirQuats, uniformScales);
        mgr.AddLightComponent(spotLightsIds, spotLightsParams);
        mgr.AddNameComponent(spotLightsIds, spotLightsNames);

        // main flashlight is inactive by default
        mgr.lightSystem_.SetLightIsActive(flashLightID, false);
    }
#endif

    return true;
}

} // namespace Core
