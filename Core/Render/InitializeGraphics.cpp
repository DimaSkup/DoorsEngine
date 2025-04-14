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
#include <CoreCommon/StrHelper.h>
#include "Common/LIB_Exception.h"    // ECS exception

#include "../Texture/TextureTypes.h"
#include "../Mesh/MaterialMgr.h"
#include "../Model/ModelExporter.h"
#include "../Model/ModelImporter.h"
#include "../Model/ModelLoader.h"
#include "../Model/ModelMath.h"
#include "../Model/ModelMgr.h"

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
    LogMsg("initialize graphics stuff");
}


// =================================================================================
//                                PUBLIC FUNCTIONS
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
        d3d.SetRS({ eRenderState::CULL_BACK, eRenderState::FILL_SOLID });
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't initialize DirectX");
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

        LogMsg("is initialized");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the scene");

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
        LogErr(e, true);
        LogErr("can't initialize the cameras objects");
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
    LogDbg("create light poles entities");

    constexpr size numEntts = 10;

    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);

    XMFLOAT3        positions[numEntts];
    XMVECTOR        dirQuats[numEntts];
    float           uniformScales[numEntts];
    ECS::EntityName names[numEntts];

    // setup positions: 2 rows of lightPoles
    for (index i = 0, z = 0; i < numEntts; z += 30, i += 2)
    {
        const float x = 11;
        const float y1 = GetHeightOfGeneratedTerrainAtPoint(-x, (float)z);
        const float y2 = GetHeightOfGeneratedTerrainAtPoint(+x, (float)z);
        positions[i + 0] = { -x, y1, (float)z };
        positions[i + 1] = { +x, y2, (float)z };
    }

    // setup directions:
    for (index i = 0; i < numEntts; i += 2)
    {
        dirQuats[i + 0] = XMQuaternionRotationRollPitchYaw(0, +XM_PIDIV2, 0);
        dirQuats[i + 1] = XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, 0);
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
    LogDbg("create spheres entities");

    constexpr size numEntts = 10;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    
    // ---------------------------------------------------------
    // setup transform data for entities

    XMFLOAT3 positions[numEntts];
    XMVECTOR dirQuats[numEntts];
    float uniformScales[numEntts];

    // setup positions: make two rows of the spheres
    for (index i = 0; i < numEntts /2; i += 2)
    {
        positions[i + 0] = XMFLOAT3(-5.0f, 5.0f, 10.0f * i);
        positions[i + 1] = XMFLOAT3(+5.0f, 5.0f, 10.0f * i);
    }

    // setup directions
    for (index i = 0; i < numEntts; ++i)
        dirQuats[i] = { 0,0,0,1 };        

    // setup uniform scales
    for (index i = 0; i < numEntts; ++i)
        uniformScales[i] = 1.0f;

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);

    // ---------------------------------------------
    
    // generate a name for each sphere entity
    ECS::EntityName enttsNames[numEntts];

    for (int i = 0; const EntityID& id : enttsIDs)
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
    const MaterialID matID          = model.meshes_.subsets_[0].materialID;
    const MaterialID catMatID       = g_MaterialMgr.GetMaterialIdByName("cat");
    const MaterialID woodCrateMatID = g_MaterialMgr.GetMaterialIdByName("wood_crate_1");
    constexpr bool areMaterialsMeshBased = true;


    mgr.AddMaterialComponent(enttsIDs[0], &catMatID, numSubsets, false);
    mgr.AddMaterialComponent(enttsIDs[1], &woodCrateMatID, numSubsets, false);

    for (index i = 2; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], &matID, numSubsets, areMaterialsMeshBased);
}

///////////////////////////////////////////////////////////

void CreateSkyBox(ECS::EntityMgr& mgr, SkyModel& sky)
{
    LogDbg("create sky box entity");

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
        textureIdx = 0;

        // very cool dark gradient:
        colorCenter = { 0.5f, 0.5f, 0.5f };
        colorApex   = { 0.38f, 0.45f, 0.51f };
    }

    // load a texture for the sky
    sprintf(g_String, "%s%s", g_RelPathTexDir, skyTexPath.c_str());
    TexID skyMapID = g_TextureMgr.LoadFromFile(g_String);

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
    LogDbg("create cylinders entities");

    constexpr size numEntts = 10;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

     XMFLOAT3 positions[numEntts];
    XMVECTOR dirQuats[numEntts];
    float uniformScales[numEntts];

    // setup positions: make two rows of the spheres
    for (index i = 0; i < numEntts / 2; i += 2)
    {
        positions[i + 0] = XMFLOAT3(-5.0f, 2.0f, 10.0f * i);
        positions[i + 1] = XMFLOAT3(+5.0f, 2.0f, 10.0f * i);
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
    // set a default texture for the cylinder mesh

    sprintf(g_String, "%s%s", g_RelPathTexDir, "brick01.dds");
    const TexID brickTexID = g_TextureMgr.LoadFromFile(g_String);

    // ----------------------------------------------------
    // setup rendering params
    ECS::RenderInitParams renderParams;
    renderParams.shaderType   = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


    // setup bounding params
    const size numSubsets = 1;                    // each cylinder has only one mesh
    const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };
    const DirectX::BoundingBox* enttAABBs = model.GetSubsetsAABB();

    // setup material params
    Material cylinderMat;
    cylinderMat.SetTexture(TEX_TYPE_DIFFUSE, brickTexID);
    const MaterialID cylinderMatID = g_MaterialMgr.AddMaterial(std::move(cylinderMat));

    // ----------------------------------------------------

    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, uniformScales);

    // each cylinder has only one mesh
    constexpr int  numSubmeshes = 1;
    constexpr bool areMaterialsMeshBased = false;

    for (const EntityID id : enttsIDs)
        mgr.AddMaterialComponent(id, &cylinderMatID, numSubmeshes, areMaterialsMeshBased);

    mgr.AddNameComponent(ids, names, numEntts);
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
    LogDbg("create cubes entities");

    try
    {
        constexpr int numEntts = 6;

        const ECS::EntityName enttsNames[numEntts] =
        {
            "cat",
            "fireflame",
            "wireFence",
            "woodCrate01",
            "woodCrate02",
            "box01",
        };

        // create empty entities
        const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
        const EntityID* ids = enttsIDs.data();


        // make a map: 'entt_name' => 'entity_id'
        std::map<ECS::EntityName, EntityID> enttsNameToID;

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
        mgr.AddNameComponent(ids, enttsNames, numEntts);
        mgr.AddModelComponent(ids, model.GetID(), numEntts);

        // the cube has only one submesh
        constexpr int numSubmeshes = 1;


        // ---------------------------------------------------------
        // Prepare materials for the cubes

        // cube_0: rotated cat
        Material catMaterial;
        catMaterial.SetName("cat");
        catMaterial.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("cat"));
        const MaterialID catMatID = g_MaterialMgr.AddMaterial(std::move(catMaterial));

        // cube_1: firecamp animated
        Material firecampMaterial;
        firecampMaterial.SetName("firecamp");
        firecampMaterial.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("fireAtlas"));
        const MaterialID firecampMatID = g_MaterialMgr.AddMaterial(std::move(firecampMaterial));

        // cube_2: wirefence with alpha clipping
        Material wirefenceMaterial;
        wirefenceMaterial.SetName("wirefence");
        wirefenceMaterial.SetAlphaClip(true);
        wirefenceMaterial.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("wireFence"));
        const MaterialID wirefenceMatID = g_MaterialMgr.AddMaterial(std::move(wirefenceMaterial));

        // cube_3: wood crate 1
        Material woodCrate1Material;
        woodCrate1Material.SetName("wood_crate_1");
        woodCrate1Material.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("woodCrate01"));
        const MaterialID woodCrate1MatID = g_MaterialMgr.AddMaterial(std::move(woodCrate1Material));

        // cube_4: wood crate 2
        Material woodCrate2Material;
        woodCrate2Material.SetName("wood_crate_2");
        woodCrate2Material.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("woodCrate02"));
        const MaterialID woodCrate2MatID = g_MaterialMgr.AddMaterial(std::move(woodCrate2Material));

        // cube_5: box01
        //Material box01Material;
        //box01Material.SetName("box_01");
        //box01Material.SetTexture(TEX_TYPE_DIFFUSE, keysToTexIDs.at("box01"));
        //const MaterialID box01MatID = materialMgr.AddMaterial(std::move(box01Material));
        const MaterialID box01MatID = model.meshes_.subsets_[0].materialID;

        // add material component (materials are the same as the original model)

        constexpr bool matIsMeshBased = true;
        constexpr bool matIsNotMeshBased = false;
        mgr.AddMaterialComponent(enttsNameToID.at("cat"), &catMatID, numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("fireflame"), &firecampMatID, numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("box01"), &box01MatID, numSubmeshes, matIsMeshBased);

        // add material component (each material is unique)
        mgr.AddMaterialComponent(enttsNameToID.at("wireFence"), &wirefenceMatID, numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("woodCrate01"), &woodCrate1MatID, numSubmeshes, matIsNotMeshBased);
        mgr.AddMaterialComponent(enttsNameToID.at("woodCrate02"), &woodCrate2MatID, numSubmeshes, matIsNotMeshBased);
       
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

#if 0
        // setup blending params of the entities
        using enum ECS::eRenderState;
        mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("wireFence"), { ALPHA_CLIPPING, CULL_NONE });

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

///////////////////////////////////////////////////////////

void CreateTerrain(ECS::EntityMgr& mgr, const BasicModel& model)
{
    //
    // create and setup terrain elements
    //
    LogDbg("create terrain");

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
    constexpr size numEntts   = 1;
    constexpr size numSubsets = 1;
    ECS::BoundingType boundTypes = ECS::BoundingType::BOUND_BOX;

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        &boundTypes,
        model.GetSubsetsAABB());             // AABB data (center, extents)

    constexpr bool areMaterialsMeshBased = true;
    const MaterialID terrainMatID = model.meshes_.subsets_[0].materialID;
    mgr.AddMaterialComponent(enttID, &terrainMatID, numSubsets, areMaterialsMeshBased);

    LogDbg("Terrain is created");
}

///////////////////////////////////////////////////////////

void CreateWater(ECS::EntityMgr& mgr)
{
#if 0
    //
    // create and setup water model 
    //

    LogDbg();

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



    using enum ECS::eRenderState;

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
        LogErr(e);
        LogErr("can't create a water entity");
    }
    catch (ECS::LIB_Exception& e)
    {
        LogErr(e.GetStr());
        LogErr("can't create a water entity");
    }
#endif
}

///////////////////////////////////////////////////////////

void CreateSkull(ECS::EntityMgr& mgr, const BasicModel& model)
{
    // create and setup a skull entity

    LogDbg("create skull");

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
        LogErr(e, true);
        LogErr("can't create a skull model");
    }
}

///////////////////////////////////////////////////////////

void CreateTreesPine(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg("create tree pine entities");

    using enum ECS::eRenderState;
    constexpr size numEntts = 50;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------

    std::vector<ECS::EntityName> names(numEntts, "tree_pine_");
    std::vector<XMFLOAT3>   positions(numEntts);
    std::vector<XMVECTOR>   quats(numEntts);
    std::vector<float>      uniScales(numEntts, 1.0f);


    // generate a name for each tree
    for (int i = 0; ECS::EntityName& name : names)
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
    for (index i = 0; i < numEntts; ++i)
    {
        float angleY = MathHelper::RandF(0, 314) * 0.01f;
        quats[i] = XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, angleY, 0);
    }

    // generate a scale value for each tree
    for (index i = 0; i < numEntts; ++i)
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
    LogDbg("create tree spruce entities");

    using enum ECS::eRenderState;
    constexpr int numEntts = 50;
    const ECS::cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------

    std::vector<ECS::EntityName> names(numEntts, "tree_spruce_");
    std::vector<XMFLOAT3>   positions(numEntts);
    std::vector<XMVECTOR>   quats(numEntts);
    std::vector<float>      uniScales(numEntts, 3.5f);


    // generate a name for each tree
    for (int i = 0; ECS::EntityName & name : names)
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
    LogDbg("create power line entities");

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
    LogDbg("create a barrel entity");

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
    LogDbg("create a stalker entity");

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


    mgr.AddTransformComponent(enttID, pos, { 0,0,1,0 }, 5.0f);
    mgr.AddNameComponent(enttID, "stalker_freedom");
    mgr.AddModelComponent(enttID, model.GetID());
    mgr.AddRenderingComponent(enttID, renderParams);

    mgr.AddBoundingComponent(
        &enttID,
        numEntts,
        numSubsets,
        boundTypes.data(),
        model.GetSubsetsAABB());             // AABB data (center, extents)


    constexpr bool areMaterialsMeshBased = true;
    cvector<MaterialID> materialIDs(numSubsets);

    for (index i = 0; i < numSubsets; ++i)
        materialIDs[i] = model.meshes_.subsets_[i].materialID;

    mgr.AddMaterialComponent(enttID, materialIDs.data(), numSubsets, areMaterialsMeshBased);
}

///////////////////////////////////////////////////////////

void CreateNanoSuit(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg("create nanosuit entity");

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
    LogDbg("create a building entity");

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
    LogDbg("create a soviet statue");
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
    LogDbg("create an appartment entity");
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
    LogDbg("create ak47 entity");

    const EntityID enttID = mgr.CreateEntity();

    // setup transformation params
    const XMFLOAT3 position  = { 10, 2, 10 };
    const XMVECTOR dirQuat   = { 0, 0, 1, 1 };
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

    mgr.AddTransformComponent(enttID, position, dirQuat, uniformScale);
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

void CreateAk74(ECS::EntityMgr& mgr, const BasicModel& model)
{
    LogDbg("create ak74 entity");

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
    LogDbg("create a house entity");

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
    LogDbg("create a house entity");

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
    LogDbg("create rock entities");

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
    LogDbg("Create pillar entities");

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

 
    LogMsgf("%-------------------------------------------------", GREEN);
    LogMsgf(" ");

    LogMsgf("%sSummary about import process:", YELLOW);
    LogMsgf("time spent to import:        %.3f ms (100 %%)",  ModelImporter::s_ImportDuration_);
    LogMsgf("time spent to load scene:    %.3f ms (%.2f %%)", ModelImporter::s_SceneLoading_, ModelImporter::s_SceneLoading_ * factor);
    LogMsgf("time spent to load nodes:    %.3f ms (%.2f %%)", ModelImporter::s_NodesLoading_, ModelImporter::s_NodesLoading_ * factor);

    LogMsgf("time spent to load vertices: %.3f ms (%.2f %%)", ModelImporter::s_VerticesLoading_, ModelImporter::s_VerticesLoading_ * factor);
    LogMsgf("time spent to load textures: %.3f ms (%.2f %%)", ModelImporter::s_TexLoadingDuration_, ModelImporter::s_TexLoadingDuration_ * factor);
    LogMsgf("%s-------------------------------------------------\n", GREEN);
}

///////////////////////////////////////////////////////////

void ImportExternalModels(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    // 1. import models from different external formats (.obj, .blend, .fbx, etc.)
    // 2. create relative entities

    ModelsCreator creator;

#if 0
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
    LogDbg("Start of models importing");

    //const ModelID lightPoleID      = creator.ImportFromFile(pDevice, lightPolePath);
    //const ModelID treeSpruceID     = creator.ImportFromFile(pDevice, treeSprucePath);
    //const ModelID treePineID       = creator.ImportFromFile(pDevice, treePinePath.c_str());
    //const ModelID nanosuitID       = creator.ImportFromFile(pDevice, nanosuitPath);
    //const ModelID stalkerFreedomID = creator.ImportFromFile(pDevice, stalkerFreedom1Path.c_str());
    //const ModelID stalkerHouse1ID  = creator.ImportFromFile(pDevice, stalkerHouseSmallPath);
    //const ModelID stalkerHouse2ID  = creator.ImportFromFile(pDevice, stalkerHouseAbandonedPath);
    //const ModelID ak47ID           = creator.ImportFromFile(pDevice, ak47Path);
    //const ModelID ak74ID           = creator.ImportFromFile(pDevice, ak74uPath);
    //const ModelID barrelID         = creator.ImportFromFile(pDevice, barrelPath);
    //const ModelID powerHVTowerID   = creator.ImportFromFile(pDevice, powerHVTowerPath);
    
   
    //const ModelID buildingID       = creator.ImportFromFile(pDevice, building9Path);
    //const ModelID apartmentID      = creator.ImportFromFile(pDevice, apartmentPath);
    //const ModelID sovietStatueID   = creator.ImportFromFile(pDevice, sovientStatuePath);


    PrintImportTimingInfo();

    LogDbg("All the models are imported successfully");

    // get models by its ids
    //BasicModel& building        = g_ModelMgr.GetModelByID(buildingID);
    //BasicModel& apartment       = g_ModelMgr.GetModelByID(apartmentID);
    //BasicModel& sovietStatue    = g_ModelMgr.GetModelByID(sovietStatueID);
    //BasicModel& treeSpruce      = g_ModelMgr.GetModelByID(treeSpruceID);
    BasicModel& treePine        = g_ModelMgr.GetModelByID(treePineID);
    //BasicModel& powerHVTower    = g_ModelMgr.GetModelByID(powerHVTowerID);
    //BasicModel& nanosuit        = g_ModelMgr.GetModelByID(nanosuitID);
    BasicModel& stalkerFreedom  = g_ModelMgr.GetModelByID(stalkerFreedomID);
    //BasicModel& lightPole       = g_ModelMgr.GetModelByID(lightPoleID);
    //BasicModel& barrel          = g_ModelMgr.GetModelByID(barrelID);
    //BasicModel& stalkerHouse1   = g_ModelMgr.GetModelByID(stalkerHouse1ID);
    //BasicModel& stalkerHouse2   = g_ModelMgr.GetModelByID(stalkerHouse2ID);
    BasicModel& ak47            = g_ModelMgr.GetModelByID(ak47ID);
    //BasicModel& ak74            = g_ModelMgr.GetModelByID(ak74ID);

    // setup some models (set textures, setup materials)
    //SetupStalkerSmallHouse(stalkerHouse1);
    //SetupStalkerAbandonedHouse(stalkerHouse2);
    SetupTree(treePine);
    //SetupPowerLine(powerHVTower);
    //SetupBuilding9(building);
    SetupAk47(ak47);

    CreateTreesPine(mgr, treePine);
    //CreateTreesSpruce(mgr, treeSpruce);
    //CreatePowerLine(mgr, powerHVTower);
    //CreateLightPoles(mgr, lightPole);
    //CreateHouse(mgr, stalkerHouse1);
    //CreateHouse2(mgr, stalkerHouse2);

    CreateAk47(mgr, ak47);
    //CreateAk74(mgr, ak74);
    //CreateBarrel(mgr, barrel);
    //CreateNanoSuit(mgr, nanosuit);
    CreateStalkerFreedom(mgr, stalkerFreedom);
    //CreateBuilding(mgr, building);
    //CreateApartment(mgr, apartment);
    //CreateSovietStatue(mgr, sovietStatue);
#endif
}

///////////////////////////////////////////////////////////

void GenerateAssets(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
{
    ModelsCreator creator;

    //
    // create sky
    //
    creator.CreateSkyCube(pDevice, 2000);
    //creator.CreateSkySphere(pDevice, 2000, 30, 30);
    //MeshSphereParams skyDomeSphereParams(2000, 30, 30);
    //const ModelID skyBoxID = creator.CreateSphere(pDevice, skyDomeSphereParams);
    SkyModel& skyBox = g_ModelMgr.GetSky();

    //
    // create terrain
    //
    const ModelID terrainID = creator.CreateGeneratedTerrain(pDevice, 500, 500, 501, 501);
    BasicModel& terrain = g_ModelMgr.GetModelByID(terrainID);
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
    BasicModel& cube            = g_ModelMgr.GetModelByID(cubeID);
    BasicModel& sphere          = g_ModelMgr.GetModelByID(sphereID);
    BasicModel& boundSphere     = g_ModelMgr.GetModelByID(boundSphereID);

    boundSphere.SetName("bound_sphere");

    // manual setup of some models
    SetupCube(cube);
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

    LogMsgf(" ");
    LogMsgf("%s------------------------------------------------------------", YELLOW);
    LogMsgf("%s                 INITIALIZATION: MODELS                     ", YELLOW);
    LogMsgf("%s------------------------------------------------------------", YELLOW);
    LogMsgf(" ");

    try
    {
        ModelsCreator creator;

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
            treeTexPaths,
            numTreeTexturesPaths,
            DXGI_FORMAT_R8G8B8A8_UNORM);


        // load the "no_texture" texture from the file;
        // (this texture will serve us as "invalid")
        sprintf(g_String, "%s%s", g_RelPathTexDir, "notexture.dds");
        const TexID noTextureID = g_TextureMgr.LoadFromFile(g_String);

        // create and setup an "invalid" material
        Material invalidMaterial;
        invalidMaterial.SetTexture(TEX_TYPE_DIFFUSE, noTextureID);

        // add "invalid" material into the material manager
        MaterialID invalidMaterialID = g_MaterialMgr.AddMaterial(std::move(invalidMaterial));

        // create a cube which will serve for us as an invalid model
        const ModelID cubeID     = creator.CreateCube(pDevice);
        BasicModel& invalidModel = g_ModelMgr.GetModelByID(cubeID);

        invalidModel.SetName("invalid_model");
        invalidModel.SetMaterialForSubset(0, invalidMaterialID);

        // NOTE: the bounding line box model must be created first of all, before all the other models
        const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);

#if 1
        if (fs::exists(g_RelPathAssetsDir))
        {
            //LoadAssets(pDevice, mgr);
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
        LogErr(e.what());
        LogErr("went out of range");
        return false;
    }
    catch (const std::bad_alloc & e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for some element");
        return false;
    }
    catch (EngineException & e)
    {
        LogErr(e);
        LogErr("can't initialize models");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void InitDirectedLightEntities(ECS::EntityMgr& mgr)
{
    // setup and create directed light entities

    constexpr size numDirLights = 3;

    if constexpr (numDirLights > 0)
    {

        ECS::DirLightsInitParams dirLightsParams;
        dirLightsParams.data.resize(numDirLights);

        ECS::DirLight& dirLight0 = dirLightsParams.data[0];
        ECS::DirLight& dirLight1 = dirLightsParams.data[1];
        ECS::DirLight& dirLight2 = dirLightsParams.data[2];

        // setup main directed light source
        dirLight0.ambient = { 0.6f, 0.6f, 0.6f, 1.0f };
        dirLight0.diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
        dirLight0.specular = { 0.3f, 0.3f, 0.3f, 1.0f };

        // setup 2nd directed light source
        dirLight1.ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
        dirLight1.diffuse = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight1.specular = { 0.25f, 0.25f, 0.25f, 1.0f };

        // setup 3rd directed light source
        dirLight2.ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
        dirLight2.diffuse = { 0.2f, 0.2f, 0.2f, 1.0f };
        dirLight2.specular = { 0.0f, 0.0f, 0.0f, 1.0f };


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


        const XMFLOAT3 directions[numDirLights] =
        {
            { 0.57735f, -0.9f, 0.57735f },
            { -0.57735f, -0.57735f, 0.57735f },
            { 0.0f, -0.707f, -0.707f }
        };

        // add transform component to each directed light because we may need to manipulate directed lights icons (in editor we can change icons positions in the scene or manipulate light direction using gizmo) 
        for (index i = 0; i < numDirLights; ++i)
        {
            const XMFLOAT3 pos     = { 3, 3, (float)i };
            const XMVECTOR dirQuat = DirectX::XMLoadFloat3(&directions[i]);

            mgr.AddTransformComponent(dirLightsIds[i], pos, dirQuat, 1.0f);
        }
    }
}

///////////////////////////////////////////////////////////

void InitPointLightEntities(ECS::EntityMgr& mgr)
{
    constexpr size numPointLights = 20;

    if (numPointLights > 0)
    {
        ECS::PointLightsInitParams pointLightsParams;
        pointLightsParams.data.resize(numPointLights);


        // generate ambient/diffuse/specular color for each point light source
        for (index i = 0; i < numPointLights; ++i)
        {
            const XMFLOAT4 color = MathHelper::RandColorRGBA();
            ECS::PointLight& light = pointLightsParams.data[i];

            light.ambient = color * 0.3f;

            light.diffuse = color * 0.7f;
            light.diffuse.w = 1.0f;

            light.specular = color * 0.8f;
            light.specular.w = 1.0f;          // setup specular power
        }

        // setup attenuation params
        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].att = { 0, 0.1f, 0.005f };

        for (index i = 0; i < numPointLights; ++i)
            pointLightsParams.data[i].range = 30;

    

        // ------------------------------------------------

        // setup transformation params because we may need to manipulate point
        // lights icons (in editor we can change icons positions in the scene or
        // manipulate light position using gizmo) 
        XMFLOAT3 positions[numPointLights];
        XMVECTOR dirQuats[numPointLights];
        float uniformScales[numPointLights];

        // generate random position for each point light source
        for (index i = 0; i < numPointLights; ++i)
        {
            positions[i].x = MathHelper::RandF(-100, 100);
            positions[i].y = 4.0f;
            positions[i].z = MathHelper::RandF(-100, 100);
        }

        for (index i = 0; i < numPointLights; ++i)
            dirQuats[i] = { 0,0,0,1 };

        for (index i = 0; i < numPointLights; ++i)
            uniformScales[i] = pointLightsParams.data[i].range;


        // generate name for each point light src
        std::string names[numPointLights];

        for (index i = 0; i < numPointLights; ++i)
            names[i] = "point_light_" + std::to_string(i);

        // setup bounding params
        const size numSubsets = 1;
        const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::SPHERE);
        DirectX::BoundingSphere boundSpheres[numPointLights];

        // setup bounding sphere for each point light src
        for (index i = 0; i < numPointLights; ++i)
            boundSpheres[i].Center = positions[i];

        for (index i = 0; i < numPointLights; ++i)
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
}

///////////////////////////////////////////////////////////

void InitSpotLightEntities(ECS::EntityMgr& mgr)
{
    constexpr size numSpotLights = 10;

    if (numSpotLights > 0)
    {
        ECS::SpotLightsInitParams spotLightsParams;

        spotLightsParams.data.resize(numSpotLights);

        // setup ambient/diffuse/specular for the flashlight in a separate way
        ECS::SpotLight& flashlight = spotLightsParams.data[0];
        flashlight.ambient  = XMFLOAT4(0.01f, 0.01f, 0.01f, 1.0f);
        flashlight.diffuse  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
        flashlight.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

        // generate ambient/diffuse/specular color for each spotlight source
        for (index i = 1; i < numSpotLights; ++i)
        {
            const XMFLOAT4 color = MathHelper::RandColorRGBA();
            ECS::SpotLight& light = spotLightsParams.data[i];

            light.ambient = color * 0.1f;
            light.ambient.w = 1.0f;

            light.diffuse = color * 0.7f;
            light.diffuse.w = 1.0f;

            light.specular = color * 0.01f;
            light.specular.w = 3.0f;          // setup specular power
        }

        // setup attenuation params (const, linear, quadratic)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].att = { 1.0f, 100.0f, 100.0f };

        // setup ranges (how far the spotlight can lit)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].range = 100;

        // setup spot exponent: light intensity fallof (for control the spotlight cone)
        for (index i = 0; i < numSpotLights; ++i)
            spotLightsParams.data[i].spot = 5;


        //
        // create and setup spotlight entities
        //
        const ECS::cvector<EntityID> spotLightsIds = mgr.CreateEntities(numSpotLights);
        const EntityID* enttsIDs = spotLightsIds.data();
        const EntityID flashLightID = spotLightsIds[0];

        // setup names
        ECS::EntityName spotLightsNames[numSpotLights];
        spotLightsNames[0] = "flashlight";

        for (index i = 1; i < numSpotLights; ++i)
            spotLightsNames[i] = "spot_light_" + std::to_string(spotLightsIds[i]);


        // generate transform data for spotlight sources
        XMFLOAT3 positions[numSpotLights];
        XMVECTOR dirQuats[numSpotLights];
        float uniformScales[numSpotLights];

        // generate positions: 2 rows of spot light sources
        for (index i = 0, z = 0; i < numSpotLights / 2; z += 30, i += 2)
        {
            positions[i + 0] = { -8, 10, (float)z };
            positions[i + 1] = { +8, 10, (float)z };
        }

        // generate directions
        constexpr float PI_DIV6 = DirectX::XM_PI * 0.333f;
        constexpr float dir1 = -DirectX::XM_PIDIV2 - PI_DIV6;
        constexpr float dir2 = -DirectX::XM_PIDIV2 + PI_DIV6;

        for (index i = 0; i < numSpotLights / 2; i += 2)
        {
            dirQuats[i + 0] = { 0.0f, dir1, 0.0f, 1.0f };
            dirQuats[i + 1] = { 0.0f, dir2, 0.0f, 1.0f };
        }

        for (index i = 0; i < numSpotLights; ++i)
            uniformScales[i] = 1.0f;


        // add components to each spotlight entity
        mgr.AddTransformComponent(enttsIDs, numSpotLights, positions, dirQuats, uniformScales);
        mgr.AddLightComponent(enttsIDs, numSpotLights, spotLightsParams);
        mgr.AddNameComponent(enttsIDs, spotLightsNames, numSpotLights);

        // main flashlight is inactive by default
        mgr.lightSystem_.SetLightIsActive(flashLightID, false);
    }
}

///////////////////////////////////////////////////////////

bool InitializeGraphics::InitializeLightSources(
    ECS::EntityMgr& mgr,
    const Settings& settings)
{
    // initialize all the light sources (entities) on the scene

    InitDirectedLightEntities(mgr);
    InitPointLightEntities(mgr);
    InitSpotLightEntities(mgr);

    return true;
}

} // namespace Core
