/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: game_initializer.cpp
    Desc:     initialize scene elements
\**********************************************************************************/
#include "../Common/pch.h"
#include "game_initializer.h"

#include <Render/d3dclass.h>
#include <Timers/game_timer.h>

#include <Model/model_exporter.h>
#include <Model/model_loader.h>
#include <Model/animation_mgr.h>
#include <Model/animation_saver.h>
#include <Model/animation_loader.h>
#include <Mesh/material_reader.h>
#include <Terrain/terrain_initializer.h>

// initializers of specific entities
#include "particles_initializer.h"
#include "light_initializer.h"
#include "grass_initializer.h"
#include "sky_initializer.h"
#include "sprite2d_initializer.h"
#include "player_initializer.h"

#include <geometry/rect3d_functions.h>

#include <inttypes.h>                   // for using PRIu32, SCNu32, etc.
#include <math/dx_math_helpers.h>

#include "quad_tree_attach_control.h"

using namespace Core;
using namespace DirectX;

namespace Game
{

//---------------------------------------------------------
// Desc:  read in a game level declaration
//---------------------------------------------------------
void GameInitializer::ReadGameInitPaths(const char* level, GameInitPaths& initPaths)
{
    if (!level || level[0] == '\0')
    {
        LogFatal(LOG, "empty name of level");
    }

    char buf[128];
    const char* path = "data/levels.cfg";
    bool bFoundLevelDecl = false;

    FILE* pFile = fopen(path, "r");
    if (!pFile)
    {
        LogFatal(LOG, "can't open file: %s", path);
    }

    // reset all the output paths
    memset(&initPaths, 0, sizeof(initPaths));

    // read in line by line a file and find a declaration of the searched level
    while (fgets(buf, sizeof(buf), pFile))
    {
        // search for beginning of level declaration
        if (strncmp(buf, "level", 5) != 0)
            continue;


        char levelName[64];
        memset(levelName, 0, sizeof(levelName));

        // get name of level
        int count = sscanf(buf, "level \"%s\"", levelName);
        assert(count == 1);

        // remove the last quote (") symbol from the name
        levelName[strlen(levelName) - 1] = '\0';

        // check if it is a searched level
        if (strcmp(levelName, level) == 0)
        {
            strcpy(initPaths.levelName, levelName);
            ReadLevelInitPaths(pFile, initPaths);
            bFoundLevelDecl = true;
        }
    }

    if (!bFoundLevelDecl)
        LogFatal(LOG, "can't find a declaration of level: %s", level);

    fclose(pFile);
}

//---------------------------------------------------------
// Desc:  read in all the main paths to config files which
//        are used for the game initialization
//---------------------------------------------------------
void GameInitializer::ReadLevelInitPaths(FILE* pFile, GameInitPaths& initPaths)
{
    assert(pFile);

    char buf[128];
    char key[64];
    char path[64];
    int  count = 0;

    memset(buf,  0, sizeof(buf));
    memset(key,  0, sizeof(key));
    memset(path, 0, sizeof(path));


    while (fgets(buf, sizeof(buf), pFile))
    {
        // if we finished reading params 
        if (buf[0] == '}')
            break;

        count = sscanf(buf, "%s %s", key, path);
        if (count != 2)
        {
            LogFatal(LOG, "can't read params for level (%s):  (key: %s, path: %s)", initPaths.levelName, key, path);
        }


        if (strcmp(key, "materials") == 0)
            strcpy(initPaths.materialsFilepath, path);

        else if (strcmp(key, "terrain") == 0)
            strcpy(initPaths.terrainFilepath, path);

        else if (strcmp(key, "sky") == 0)
            strcpy(initPaths.skyFilepath, path);

        else if (strcmp(key, "sky_plane") == 0)
            strcpy(initPaths.skyPlaneFilepath, path);

        else if (strcmp(key, "models") == 0)
            strcpy(initPaths.modelsFilepath, path);

        else if (strcmp(key, "animations") == 0)
            strcpy(initPaths.animationsFilepath, path);

        else if (strcmp(key, "entities") == 0)
            strcpy(initPaths.entitiesFilepath, path);

        else if (strcmp(key, "nature_gen") == 0)
            strcpy(initPaths.natureGenFilepath, path);

        else if (strcmp(key, "particles") == 0)
            strcpy(initPaths.particlesFilepath, path);

        else if (strcmp(key, "grass") == 0)
            strcpy(initPaths.grassFilepath, path);

        else if (strcmp(key, "lights") == 0)
            strcpy(initPaths.lightsFilepath, path);

        else if (strcmp(key, "weapons") == 0)
            strcpy(initPaths.weaponsFilepath, path);

        else if (strcmp(key, "sprites2d") == 0)
            strcpy(initPaths.sprites2dFilepath, path);

        else if (strcmp(key, "player") == 0)
            strcpy(initPaths.playerFilepath, path);

        else if (strcmp(key, "textures") == 0)
            strcpy(initPaths.texturesFilepath, path);

        else if (strcmp(key, "sounds") == 0)
            strcpy(initPaths.soundsFilepath, path);

        else
            LogFatal(LOG, "unknown key (%s) for level initialization (%s)", key, initPaths.levelName);

    }
}


//---------------------------------------------------------
// some convertation helpers
//---------------------------------------------------------
inline Vec3 ToVec3(const XMFLOAT3& v)
{
    return Vec3(v.x, v.y, v.z);
}

inline XMFLOAT3 ToFloat3(const Vec3& v)
{
    return XMFLOAT3(v.x, v.y, v.z);
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
// Desc:   create and setup particle emitters on the scene
// Args:   - filepath:  a path to file with definitions of particle emitters 
//---------------------------------------------------------
void GameInitializer::InitParticles(const char* filepath, ECS::EntityMgr& mgr)
{
    const TimePoint start = GetTimePoint();

    SetConsoleColor(YELLOW);
    LogMsg("---------------------------------------------------------");
    LogMsg("            INITIALIZATION: PARTICLE EMITTERS            ");
    LogMsg("---------------------------------------------------------");
    

    ParticlesInitializer initializer;
    if (!initializer.Init(filepath, mgr))
    {
        LogFatal(LOG, "can't init particle emitters");
    }

    const TimeDurationMs dur = GetTimePoint() - start;

    LogMsg(LOG, "all the PARTICLE emitters are initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("--------------------------------------");
    LogMsg("Init of particles took: %.3f ms", dur.count());
    LogMsg("--------------------------------------\n");
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:  create and setup all the grass on the scene
// Args:  - filepath:  a path to file with definitions
//                     of grass fields and other common params
//---------------------------------------------------------
void GameInitializer::InitGrass(const char* filepath, ECS::EntityMgr& mgr)
{
    SetConsoleColor(YELLOW);
    LogMsg("---------------------------------------------------------");
    LogMsg("            INITIALIZATION: GRASS                        ");
    LogMsg("---------------------------------------------------------");
    const TimePoint start = GetTimePoint();


    GrassInitializer initializer;
    if (!initializer.Init(filepath, mgr))
    {
        LogFatal(LOG, "can't init grass");
    }

    const TimeDurationMs dur = GetTimePoint() - start;
    LogMsg(LOG, "all the GRASS instances are initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("--------------------------------------");
    LogMsg("Init of grass took: %.3f ms", dur.count());
    LogMsg("--------------------------------------\n");
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:  create and setup light sources on the scene
// Args:  - filepath:  a path to file with definitions of the light sources
//---------------------------------------------------------
void GameInitializer::InitLights(const char* filepath, ECS::EntityMgr& mgr)
{
    LightInitializer initializer;

    if (!initializer.Init(filepath, mgr))
    {
        LogFatal(LOG, "can't init light sources");
    }
}

//---------------------------------------------------------
// Desc:   init all the stuff related to the main player
//---------------------------------------------------------
void GameInitializer::InitPlayer(
    const char* cfgFilepath,
    ECS::EntityMgr& enttMgr)
{
    if (!PlayerInitializer::Init(cfgFilepath, enttMgr))
    {
        LogFatal(LOG, "can't initializer the player");
    }
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
void CreateSpheres(ECS::EntityMgr& mgr, const Model& model)
{
    LogDbg(LOG, "create spheres entities");

    constexpr size numEntts = 10;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

    XMFLOAT3 positions[numEntts];
    XMVECTOR rotQuats[numEntts];
    float scales[numEntts];

    // setup positions: make two rows of the spheres
    for (index i = 0; i < numEntts; i += 2)
    {
        positions[i + 0] = XMFLOAT3(240.0f, 80.0f, 210.0f + i);
        positions[i + 1] = XMFLOAT3(242.0f, 80.0f, 210.0f + i);
    }

    // setup rotations
    for (index i = 0; i < numEntts; ++i)
        rotQuats[i] = { 0,0,0,1 };

    // setup uniform scales
    for (index i = 0; i < numEntts; ++i)
        scales[i] = 1.0f;


    // ---------------------------------------------

    // generate a name for each sphere entity
    std::string enttsNames[numEntts];

    for (int i = 0; const EntityID id : enttsIDs)
        enttsNames[i++] = "sphere_" + std::to_string(id);

    // ---------------------------------------------

    // prepare some materials
    const MaterialID defaultMatId  = g_MaterialMgr.GetMatIdByName("gigachad");
    const MaterialID ground04MatId = g_MaterialMgr.GetMatIdByName("ground04");
    const MaterialID catMatID      = g_MaterialMgr.GetMatIdByName("cat");
    const MaterialID brickMatId    = g_MaterialMgr.GetMatIdByName("brick_01");

    // prepare world bounding boxes
    BoundingBox localBox = model.GetModelAABB();
    cvector<BoundingBox> worldBoxes(numEntts);

    for (index i = 0; i < numEntts; ++i)
    {
        const XMVECTOR vPos = XMLoadFloat3(&positions[i]);
        localBox.Transform(worldBoxes[i], scales[i], rotQuats[i], vPos);
    }

    //
    // add and setup components of entities
    //
    mgr.AddModelComponent(ids, model.GetId(), numEntts);
    mgr.AddNameComponent(ids, enttsNames, numEntts);
    mgr.AddRenderingComponent(ids, numEntts);
    mgr.AddTransformComponent(ids, numEntts, positions, rotQuats, scales);
    mgr.AddBoundingComponent(ids, numEntts, localBox, worldBoxes.data());

    // specify material for some entities
    mgr.AddMaterialComponent(enttsIDs[0], catMatID);
    mgr.AddMaterialComponent(enttsIDs[1], brickMatId);

    for (index i = 2; i < numEntts-3; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], defaultMatId);

    for (index i = numEntts-3; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIDs[i], ground04MatId);

#if ATTACH_SPHERES_TO_QT
    // bind these entities to the quad tree
    mgr.AttachEnttsToQuadTree(enttsIDs.data(), enttsIDs.size());
#endif
}

//---------------------------------------------------------
// Desc:  create and setup cylinder entities
//---------------------------------------------------------
void CreateCylinders(ECS::EntityMgr& mgr, const Model& model)
{
    LogDbg(LOG, "create cylinders entities");

    constexpr size numEntts = 10;
    const cvector<EntityID> enttsIDs = mgr.CreateEntities(numEntts);
    const EntityID* ids = enttsIDs.data();

    // ---------------------------------------------------------
    // setup transform data for entities

    XMFLOAT3 positions[numEntts];
    XMVECTOR dirQuats[numEntts];
    float scales[numEntts];

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
        scales[i] = 1.0f;

    // generate names for the entities
    std::string names[numEntts];
    for (int i = 0; const EntityID id : enttsIDs)
        names[i++] = "cylinder_" + std::to_string(id);

  
    // material for cylinders
    const MaterialID brickMatID = g_MaterialMgr.GetMatIdByName("brick_01");

    // setup boundings 
    BoundingBox localBox = model.GetModelAABB();
    BoundingBox worldBoxes[numEntts];

    for (index i = 0; i < numEntts; ++i)
    {
        XMVECTOR vPos = XMLoadFloat3(&positions[i]);
        localBox.Transform(worldBoxes[i], scales[i], dirQuats[i], vPos);
    }

    //
    // add and setup components
    //
    mgr.AddTransformComponent(ids, numEntts, positions, dirQuats, scales);
    mgr.AddMaterialComponent(ids, numEntts, brickMatID);
    mgr.AddNameComponent(ids, names, numEntts);
    mgr.AddRenderingComponent(ids, numEntts);
    mgr.AddModelComponent(ids, model.GetId(), numEntts);

    mgr.AddBoundingComponent(ids, numEntts, localBox, worldBoxes);

#if ATTACH_CYLINDERS_TO_QT
    mgr.AttachEnttsToQuadTree(ids, numEntts);
#endif
}

//---------------------------------------------------------
// Desc:  generate trees entities at random places and based on input model
//---------------------------------------------------------
void CreateTreesEntities(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const char* modelName,
    const uint numEntts,
    const float scale,
    const XMVECTOR rotQuat)
{
    using namespace DirectX;

    if (StrHelper::IsEmpty(modelName))
    {
        LogErr(LOG, "empty model name");
        return;
    }
    if (numEntts == 0)
    {
        LogErr(LOG, "num of entities == 0, why do you call me?");
        return;
    }

    LogDbg(LOG, "create trees based on model: %s", modelName);

    // find a model which will be used as trees
    const Model& tree = g_ModelMgr.GetModelByName(modelName);
    if (tree.GetId() == INVALID_MODEL_ID)
    {
        LogErr(LOG, "no model by name: %s", modelName);
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
    Terrain& terrain = g_ModelMgr.GetTerrain();

    const float     maxPosHeight        = 150;      // max height where tree can appear
    const float     range               = (float)terrain.heightMap_.GetWidth();
    constexpr float gndSlopeMaxAngle    = DEG_TO_RAD(45);
    constexpr float offsetUnderGnd      = 0.25f;
    constexpr uint8 minNatureDensity    = 1;

    //
    // generate RANDOM position for each tree
    //
    for (uint i = 0; i < numEntts; ++i)
    {
        XMFLOAT3& pos    = positions[i];
        float slopeAngle = 0;
        uint8 density    = 0;
        bool bSharpSlope = true;
        bool bLowDensity = true;

        do
        {
            // regenerate position each time when we got at area with no/low
            // nature density or angle of ground surface is too sharp
            do
            {
                pos.x = RandF(0, range-5);
                pos.z = RandF(0, range-5);

                slopeAngle = terrain.GetSlopeAngleAtPoint(pos.x, pos.z);
                density    = terrain.GetNatureDensityAtPoint(pos.x, pos.z);

                bSharpSlope = (slopeAngle > gndSlopeMaxAngle);
                bLowDensity = (density < minNatureDensity);

            } while (bSharpSlope || bLowDensity);

            // now we have proper x,z coords so find height at this point
            pos.y = terrain.GetScaledInterpolatedHeightAtPoint(pos.x, pos.z) - offsetUnderGnd;

        } while (pos.y > maxPosHeight);   // limit height for trees
    }

    // calc quaternions for rotation around Y-axis
    for (uint i = 0; i < numEntts; ++i)
    {
        const float angle = DEG_TO_RAD(RandUint(0, 360));
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
        scales[i] = scale;


    // prepare data for material component
    const Core::Subset* subsets = tree.GetSubsets();
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
    mgr.AddModelComponent(enttsIds, tree.GetId(), numEntts);
    mgr.AddRenderingComponent(enttsIds, numEntts);

    for (index i = 0; i < numEntts; ++i)
        mgr.AddMaterialComponent(enttsIds[i], matsIds.data(), numSubsets);


    // rotate each tree using a quaternion
    if (rotQuat != XMVECTOR{ 0,0,0,1 })
        mgr.transformSys_.RotateLocalSpacesByQuat(enttsIds, numEntts, rotQuat);

    // rotate each tree around Y-axis by random angle
    for (uint i = 0; i < numEntts; ++i)
        mgr.transformSys_.RotateLocalSpaceByQuat(enttsIds[i], yRotationQuats[i]);


    // setup boundings 
    BoundingBox localBox = tree.GetModelAABB();
    cvector<BoundingBox> worldBoxes(numEntts);

    for (index i = 0; i < numEntts; ++i)
    {
        const XMVECTOR vPos = XMLoadFloat3(&positions[i]);
        localBox.Transform(worldBoxes[i], scales[i], rotQuat, vPos);
    }

    mgr.AddBoundingComponent(enttsIds, numEntts, localBox, worldBoxes.data());

#if ATTACH_TREES_TO_QT
    mgr.AttachEnttsToQuadTree(enttsIds, numEntts);
#endif
}

//---------------------------------------------------------
// Desc:  handler for entity name to make it proper and unique
//---------------------------------------------------------
void HandleName(
    const char* inName,
    char* outName,
    const EntityID id,
    const Model& model,
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
    const Model& model,
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
    if (model.GetId() == INVALID_MODEL_ID)
        uniformScale = 3.0f;
}

//---------------------------------------------------------
// Desc:   create and init a new building entity based on input model
//---------------------------------------------------------
EntityID CreateBuildingEntt(
    ECS::EntityMgr& mgr,
    const Model& model,
    const XMFLOAT3& pos,
    const XMVECTOR& dir,
    const XMVECTOR& rotQuat,
    float scale,
    const char* inName)
{
    assert(scale > 0);

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char enttName[MAX_LEN_ENTT_NAME]{ '\0' };
    HandleName(inName, enttName, enttId, model, mgr);

    LogDbg(LOG, "create entity: %s", enttName);

    // if for any reason we got "invalid" model (cube) prevent it to be too big or small
    if (model.GetId() == INVALID_MODEL_ID)
        scale = 3.0f;


    // setup materials ids
    const Core::Subset* subsets = model.GetSubsets();

    // add components to the entity
    mgr.AddTransformComponent(enttId, pos, dir, scale);
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, enttName);
    mgr.AddModelComponent(enttId, model.GetId());
    mgr.AddRenderingComponent(enttId);

    // setup boundings
    BoundingBox localBox = model.GetModelAABB();
    BoundingBox worldBox;
    localBox.Transform(worldBox, mgr.transformSys_.GetWorld(enttId));
    mgr.AddBoundingComponent(enttId, localBox, worldBox);


    // add material component
    const int numSubsets = model.GetNumSubsets();
    if (numSubsets > 1)
    {
        cvector<MaterialID> materialsIds(numSubsets, INVALID_MAT_ID);

        for (index i = 0; i < numSubsets; ++i)
            materialsIds[i] = subsets[i].materialId;

        // bound material to each mesh of this entity
        mgr.AddMaterialComponent(enttId, materialsIds.data(), numSubsets);
    }
    else
    {
        mgr.AddMaterialComponent(enttId, subsets[0].materialId);
    }

#if ATTACH_BUILDING_TO_QT
    mgr.AttachEnttToQuadTree(enttId);
#endif

    return enttId;
}

//---------------------------------------------------------
// Desc:   create and init a new vehicle entity based on input model
//---------------------------------------------------------
EntityID CreateVehicleEntt(
    ECS::EntityMgr& mgr,
    const Model& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float scale,
    const char* inName = nullptr)
{
    assert(scale > 0.0f);

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    // print debug msg
    LogDbg(LOG, "create %s entity", name);

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, scale);
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetId());
    mgr.AddRenderingComponent(enttId);

    // setup bounding shapes
    BoundingBox localBox = model.GetModelAABB();
    BoundingBox worldBox;

    localBox.Transform(worldBox, mgr.transformSys_.GetWorld(enttId));
    mgr.AddBoundingComponent(enttId, localBox, worldBox);


    // add material component
    const Core::Subset* subsets = model.GetSubsets();
    const size numSubsets = model.GetNumSubsets();

    if (numSubsets > 1)
    {
        MaterialID matsIds[32]{ INVALID_MAT_ID };
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

#if ATTACH_VEHICLE_TO_QT
    mgr.AttachEnttToQuadTree(enttId);
#endif
    return enttId;
}

//---------------------------------------------------------
// Desc:   create and init a new weapon entity based on input model
//---------------------------------------------------------
EntityID CreateWeaponEntt(
    ECS::EntityMgr& mgr,
    const Model& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    XMVECTOR rotQuat,
    float scale,
    const char* inName = nullptr)
{
    using namespace DirectX;
    assert(scale > 0.0f);

    const EntityID enttId   = mgr.CreateEntity();

    rotQuat = XMQuaternionNormalize(rotQuat);

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    LogDbg(LOG, "create %s entity", name);

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, scale);

    // rotate entity around itself
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetId());
    mgr.AddRenderingComponent(enttId);

    // setup bounding shapes
    XMVECTOR        vPos = XMLoadFloat3(&position);
    BoundingBox localBox = model.GetModelAABB();
    BoundingBox worldBox;

    localBox.Transform(worldBox, scale, rotQuat, vPos);
    mgr.AddBoundingComponent(enttId, localBox, worldBox);


    // add material to multiple subsets of the entt
    const Core::Subset* subsets = model.GetSubsets();
    const size numSubsets       = model.GetNumSubsets();

    if (numSubsets > 1)
    {
        MaterialID matsIds[32]{ INVALID_MAT_ID };
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

#if ATTACH_WEAPON_TO_QT
    mgr.AttachEnttToQuadTree(enttId);
#endif

    return enttId;
}

//---------------------------------------------------------
// Desc:   create and init a cube entity
//---------------------------------------------------------
EntityID CreateCubeEntt(
    ECS::EntityMgr& mgr,
    const Model& model,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const XMVECTOR& rotQuat,
    float scale,
    const char* inName = nullptr)
{
    assert(scale > 0);

    const EntityID enttId = mgr.CreateEntity();

    // setup a name
    char name[MAX_LEN_ENTT_NAME]{'\0'};
    HandleName(inName, name, enttId, model, mgr);

    LogDbg(LOG, "create %s entity", name);

    // add components to the entity
    mgr.AddTransformComponent(enttId, position, direction, scale);

    // rotate entity around itself
    mgr.transformSys_.RotateLocalSpaceByQuat(enttId, rotQuat);

    mgr.AddNameComponent(enttId, name);
    mgr.AddModelComponent(enttId, model.GetId());
    mgr.AddRenderingComponent(enttId);
    mgr.AddMaterialComponent(enttId, model.GetSubsets()[0].materialId);

    // setup boundings
    BoundingBox localBox = model.GetModelAABB();
    BoundingBox worldBox;
    XMVECTOR vPos = XMLoadFloat3(&position);

    localBox.Transform(worldBox, scale, direction, vPos);
    mgr.AddBoundingComponent(enttId, localBox, worldBox);

#if ATTACH_CUBE_TO_QT
    mgr.AttachEnttToQuadTree(enttId);
#endif

    return enttId;
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
    char modelPath[128]{ '\0' };
    char modelName[MAX_LEN_MODEL_NAME]{ '\0' };
    ModelID modelId = 0;
    ModelLoader loader;

    // open a file for models reading
    FILE* pFile = fopen(filepath, "r");
    if (!pFile)
    {
        LogFatal(LOG, "can't open file for models creation: %s", filepath);
    }

    // read and init each model
    while (fgets(buf, sizeof(buf), pFile))
    {
        // skip new lines and comments
        if (buf[0] == '\n' || buf[0] == ';')
            continue;


        int count = sscanf(buf, "%s", modelPath);
        if (count != 1)
        {
            LogErr(LOG, "can't get a model path from str buffer: %s", buf);
            continue;
        }

        // add empty model into the manager and setup its name
        Model& model = g_ModelMgr.AddEmptyModel();
        FileSys::GetFileStem(modelPath, modelName);
        g_ModelMgr.SetModelName(model.GetId(), modelName);

        const bool isLoaded = loader.Load(modelPath, &model);

        // if failed to load model...
        if (!isLoaded)
        {
            LogErr(LOG, "can't load model from file: %s", modelPath);
            LogMsg("%s Set model (%s) to default (cube)%s", YELLOW, modelName, RESET);

            const Model& invalidModel = g_ModelMgr.GetModelById(0);
            model.Copy(invalidModel);

            g_ModelMgr.SetModelName(model.GetId(), modelName);
        }

        // init vb/ib buffers
        model.InitBuffers();
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
        LogFatal(LOG, "can't open file for entities creation: %s", filePath);
    }

    int count = 0;
    char key[32];
    char buf[128]{'\0'};
    char enttName[MAX_LEN_ENTT_NAME]{'\0'};
    char matName[MAX_LEN_MAT_NAME]{'\0'};
    char archetype[32]{'\0'};

    char animSkeletonName[MAX_LEN_SKELETON_NAME]{'\0'};
    char animName[MAX_LEN_ANIMATION_NAME]{'\0'};

    EntityID enttId  = INVALID_ENTT_ID;
    XMFLOAT3 pos     = { 0,0,0 };
    XMFLOAT3 dir     = { 0,0,0 };
    
    XMVECTOR vDir    = { 0,0,1,0 };
    XMVECTOR rotQuat = { 0,0,0,1 };
    float    scale   = 1.0;
    Model*   pModel  = nullptr;

    bool bHasAnimSkeleton = false;
    bool bHasAnimName = false;
    int  bEnttVisible = true;

    cvector<MaterialID> materialIds(8, INVALID_MAT_ID);
    cvector<SubsetID>   subsetIds(8, 0);


    while (fgets(buf, sizeof(buf), pFile))
    {
        // skip new lines and comments
        if (buf[0] == '\n' || buf[0] == ';')
            continue;


        count = sscanf(buf, "%s", key);
        assert(count == 1);

        // create a new entity
        if (strcmp(key, "newentt") == 0)
        {
            ReadStr(buf, "newentt %s", enttName);

            // reset params
            pModel = nullptr;
            materialIds.clear();
            subsetIds.clear();
            rotQuat = { 0,0,0,1 };

            bHasAnimSkeleton = false;
            bHasAnimName = false;
            bEnttVisible = true;
        }

        // read in a model name
        else if (strcmp(key, "model") == 0)
        {
            char modelName[MAX_LEN_MODEL_NAME]{'\0'};
            ReadStr(buf, "model %s\n", modelName);

            pModel = &g_ModelMgr.GetModelByName(modelName);
            assert(pModel != nullptr);
        }

        // read in material (subsetId => materialId)
        else if (strcmp(key, "material") == 0)
        {
            int meshId = -1;
            count = sscanf(buf, "material %d %s\n", &meshId, matName);
            assert(count == 2);

            const MaterialID matId = g_MaterialMgr.GetMatIdByName(matName);

            if (matId == INVALID_MODEL_ID)
            {
                LogErr(LOG, "no material by name: %s", matName);
                continue;
            }

            materialIds.push_back(matId);
            subsetIds.push_back((SubsetID)meshId);
        }

        // read in position in world
        else if (strcmp(key, "pos") == 0)
        {
            ReadFloat3(buf, "pos %f %f %f\n", &pos.x);
        }

        else if (strcmp(key, "dir") == 0)
        {
            ReadFloat3(buf, "dir %f %f %f\n", &dir.x);
            vDir = { dir.x, dir.y, dir.z };
        }

        else if (strcmp(key, "rot_axis_x") == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_x %f\n", &angle);

            const XMVECTOR q = QuatRotAxis({ 1,0,0 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strcmp(key, "rot_axis_y") == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_y %f\n", &angle);

            const XMVECTOR q = QuatRotAxis({ 0,1,0 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strcmp(key, "rot_axis_z") == 0)
        {
            float angle = 1.0f;
            ReadFloat(buf, "rot_axis_z %f\n", &angle);

            const XMVECTOR q = QuatRotAxis({ 0,0,1 }, angle);
            rotQuat = QuatMul(rotQuat, q);
        }

        else if (strcmp(key, "rot_axis") == 0)
        {
            Vec4 rotAxis = { 0,0,0,0 };
            ReadFloat4(buf, "rot_axis %f %f %f %f\n", &rotAxis.x);

            const XMVECTOR axis = { rotAxis.x, rotAxis.y, rotAxis.z };
            rotQuat = QuatRotAxis(axis, rotAxis.w);
        }

        else if (strcmp(key, "rot_quat") == 0)
        {
            Vec4 q = { 0,0,0,1 };
            ReadFloat4(buf, "rot_quat %f %f %f %f\n", &q.x);
            rotQuat = { q.x, q.y, q.z, q.w };
        }

        else if (strcmp(key, "scale") == 0)
        {
            ReadFloat(buf, "scale %f\n", &scale);
     
        }

        else if (strcmp(key, "anim_skeleton") == 0)
        {
            ReadStr(buf, "anim_skeleton %s\n", animSkeletonName);
            bHasAnimSkeleton = true;
        }

        else if (strcmp(key, "anim_name") == 0)
        {
            ReadStr(buf, "anim_name %s\n", animName);
            bHasAnimName = true;
        }

        else if (strcmp(key, "is_visible") == 0)
        {
            ReadInt(buf, "is_visible %d", &bEnttVisible);
        }

        else if (strcmp(key, "archetype") == 0)
        {
            ReadStr(buf, "archetype %s", archetype);

            // make sure that we have any model for this entity
            assert(pModel != nullptr);

            EntityID enttId = 0;

            // create entity: building / vehicle / weapon / etc.
            if (strcmp(archetype, "building") == 0)
            {
                enttId = CreateBuildingEntt(enttMgr, *pModel, pos, vDir, rotQuat, scale, enttName);
            }

            else if (strcmp(archetype, "vehicle") == 0)
            {
                enttId = CreateVehicleEntt(enttMgr, *pModel, pos, vDir, rotQuat, scale, enttName);
            }

            else if (strcmp(archetype, "weapon") == 0)
            {
                enttId = CreateWeaponEntt(enttMgr, *pModel, pos, vDir, rotQuat, scale, enttName);
            }

            else if (strcmp(archetype, "cube") == 0)
            {
                enttId = CreateCubeEntt(enttMgr, *pModel, pos, vDir, rotQuat, scale, enttName);
            }

            else
            {
                LogErr(LOG, "can't create entity (%s): unknown archetype (%s)", enttName, archetype);
            }


            // setup animation for this entity (if we have any)
            if (bHasAnimSkeleton && bHasAnimName)
            {
                AnimSkeleton&        skeleton = g_AnimationMgr.GetSkeleton(animSkeletonName);
                const AnimationID      animId = skeleton.GetAnimationIdx(animName);
                const AnimationClip& animClip = skeleton.GetAnimation(animId);

                enttMgr.AddAnimationComponent(enttId, skeleton.id_, animId, animClip.GetEndTime());
            }


            // setup visibility for this entity
            if (!bEnttVisible)
                enttMgr.RemoveComponent(enttId, ECS::RenderedComponent);


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
    LogDbg(LOG, "generate nature: trees, bushes, etc.");

    char buf[128]{ '\0' };
    NatureParams params;

    // open config file
    FILE* pFile = fopen(filename, "r");
    if (!pFile)
    {
        LogFatal(LOG, "can't open a config file for nature generation: %s", filename);
    }

    while (fgets(buf, sizeof(buf), pFile))
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

        else
            ReadTreesParams(buf, params);
    }

    if (pFile)
        fclose(pFile);

    LogDbg(LOG, "nature initialization is finished\n");
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
        LogFatal(LOG, "can't open a file: %s", filepath);
    }

    // check if we opened a valid file
    fgets(buf, sizeof(buf), pFile);

    if (strncmp(buf, "animations", 10) != 0)
    {
        LogFatal(LOG, "invalid file for animations: %s", filepath);
    }

    // read in each path to animation and load this animation
    while (!feof(pFile))
    {
        count = fscanf(pFile, "%s", buf);
        assert(count == 1);

        animLoader.Load(buf);
    }


    const TimeDurationMs dur = GetTimePoint() - start;
    LogMsg("------------------------------------------------");
    LogMsg("loading of animations took:  %f sec", dur.count() * 0.001f);
    LogMsg("------------------------------------------------\n");
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

#endif

    //--------------------------------
    
    LoadAnimations(animationsFilepath);
    LoadEntities(entitiesFilepath, enttMgr);

#if 1
    //---------------------------------

    Model& cube         = g_ModelMgr.GetModelByName("basic_cube");
    Model& sphere       = g_ModelMgr.GetModelByName("basic_sphere");
    Model& cylinder     = g_ModelMgr.GetModelByName("basic_cylinder");
    Model& treeSpruce   = g_ModelMgr.GetModelByName("tree_spruce");
    Model& treePine     = g_ModelMgr.GetModelByName("tree_pine");


    const DirectX::BoundingBox& treePineAABB   = treePine.GetModelAABB();
    const DirectX::BoundingBox& treeSpruceAABB = treeSpruce.GetModelAABB();

    //---------------------------------

    const float treePineWidth       = treePineAABB.Extents.x * 3.5f * 0.8f * 0.95f;
    const float treePineHeight      = treePineAABB.Extents.z * 2.0f;

    const float treeSpruceWidth     = treeSpruceAABB.Extents.z * 2.0f;
    const float treeSpruceHeight    = treeSpruceAABB.Extents.y * 2.0f * 1.05f;

    const bool originAtBottom       = true;
    const float rotatePineAroundX   = -90.0f;
    const float rotateSpruceAroundX = 0.0f;

    //---------------------------------

    ModelsCreator creator;

    // create lod models
    const ModelID idTreePineLod1   = creator.CreatePlaneLod(treePineWidth, treePineHeight);
    const ModelID idTreeSpruceLod1 = creator.CreatePlaneLod(treeSpruceWidth, treeSpruceHeight);

    const ModelID idTreePineLod2   = creator.CreatePlaneLod(treePineWidth, treePineHeight);
    const ModelID idTreeSpruceLod2 = creator.CreatePlaneLod(treeSpruceWidth, treeSpruceHeight);

    // get lod models
    Model& treePineLod1   = g_ModelMgr.GetModelById(idTreePineLod1);
    Model& treeSpruceLod1 = g_ModelMgr.GetModelById(idTreeSpruceLod1);

    Model& treePineLod2   = g_ModelMgr.GetModelById(idTreePineLod2);
    Model& treeSpruceLod2 = g_ModelMgr.GetModelById(idTreeSpruceLod2);

    // get materials for lods
    const MaterialID treePineLod1MatId   = g_MaterialMgr.GetMatIdByName("tree_pine_lod_1");
    const MaterialID treeSpruceLod1MatId = g_MaterialMgr.GetMatIdByName("tree_spruce_lod_1");

    const MaterialID treePineLod2MatId   = g_MaterialMgr.GetMatIdByName("tree_pine_lod_2");
    const MaterialID treeSpruceLod2MatId = g_MaterialMgr.GetMatIdByName("tree_spruce_lod_2");

    // setup lod models
    treePineLod1.SetMaterialForSubset(0, treePineLod1MatId);
    treePineLod1.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    treeSpruceLod1.SetMaterialForSubset(0, treeSpruceLod1MatId);
    treeSpruceLod1.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    treePineLod2.SetMaterialForSubset(0, treePineLod2MatId);
    treePineLod2.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    treeSpruceLod2.SetMaterialForSubset(0, treeSpruceLod2MatId);
    treeSpruceLod2.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // bind LODs
    treeSpruce.SetLod(LOD_1, idTreeSpruceLod1);
    treeSpruce.SetLodDistance(LOD_1, 50);

    treePine.SetLod(LOD_1, idTreePineLod1);
    treePine.SetLodDistance(LOD_1, 50);

    treeSpruce.SetLod(LOD_2, idTreeSpruceLod2);
    treeSpruce.SetLodDistance(LOD_2, 100);

    treePine.SetLod(LOD_2, idTreePineLod2);
    treePine.SetLodDistance(LOD_2, 100);

    CreateNature(natureFilepath, enttMgr, render);
#endif
}

//---------------------------------------------------------
// Desc:   create and setup a terrain
// Args:   - mgr:          ECS entities manager
//         - cfgFilepath:  a path to terrain config file
//---------------------------------------------------------
void CreateTerrain(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const char* cfgFilepath)
{
    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogFatal(LOG, "LOL, your terrain config's path is empty");
    }

    TerrainInitializer trnInitializer;
    trnInitializer.Init(cfgFilepath);

    Core::Terrain& terrain = g_ModelMgr.GetTerrain();
    Core::TerrainConfig  terrainCfg;


    // load terrain configs (we do it again since we need some params here)
    if (!terrain.LoadSetupFile(cfgFilepath, terrainCfg))
    {
        LogFatal(LOG, "can't load terrain's configs");
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
    mat.SetTexture(eTexType(1), terrain.texture_.GetId());
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

    mat.SetTexture(TEX_TYPE_DIFFUSE_ROUGHNESS, terrain.detailMap_.GetId());
    //mat.SetTexture(TEX_TYPE_LIGHTMAP,          terrain.lightmap_.id);

    terrain.materialID_ = mat.id;

    // update constant buffer to hold terrain's material colors
    render.UpdateCbTerrainMaterial(mat.ambient, mat.diffuse, mat.specular, mat.reflect);

    // create and setup a terrain entity
    const EntityID enttId = mgr.CreateEntity();

    // setup bounding params
    const Vec3 c = terrain.GetAABB().MidPoint();  // center
    const Vec3 e = terrain.GetAABB().Extents();
    const DirectX::BoundingBox aabb = { ToFloat3(c), ToFloat3(e) };

    mgr.AddTransformComponent(enttId);
    mgr.AddNameComponent     (enttId, "terrain_geomipmap");
    mgr.AddBoundingComponent (enttId, aabb, aabb);
    mgr.AddMaterialComponent (enttId, terrain.materialID_);

    LogErr(LOG, "TEMP");
    terrain.GetLodMgr().SetDistanceToLOD(2, 300);
}

//---------------------------------------------------------
// Desc:   generate and setup different assets/entities
// Args:   - pDevice:  a ptr to DirectX11 device
//         - mgr:      ECS entity manager
//---------------------------------------------------------
void GenerateEntities(ECS::EntityMgr& mgr, Render::CRender& render)
{
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
    Model& cube        = g_ModelMgr.GetModelById(cubeID);
    Model& sphere      = g_ModelMgr.GetModelById(sphereID);
    Model& cylinder    = g_ModelMgr.GetModelById(cylinderID);

    // set names for some models
    g_ModelMgr.SetModelName(cube.GetId(), "basic_cube");
    g_ModelMgr.SetModelName(sphere.GetId(), "basic_sphere");
    g_ModelMgr.SetModelName(cylinder.GetId(), "basic_cylinder");

    // setup LODs for some models
    sphere.SetLod(LOD_1, cube.GetId());
    sphere.SetLod(LOD_2, cylinder.GetId());

    sphere.SetLodDistance(LOD_1, 10);
    sphere.SetLodDistance(LOD_2, 20);

    // manual setup of some models
    cube.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("box01"));
    sphere.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("gigachad"));
    cylinder.SetMaterialForSubset(0, g_MaterialMgr.GetMatIdByName("cat"));


    // create and setup entities with models  
    CreateSpheres(mgr, sphere);
    //CreateCylinders(mgr, cylinder);
}

//---------------------------------------------------------
// Desc:  create some 2D sprites
//---------------------------------------------------------
void GameInitializer::Create2dSprites(
    const char* cfgFilepath,
    ECS::EntityMgr& mgr,
    const Render::CRender& render)
{
    Sprite2dInitializer initializer;

    initializer.Init(
        cfgFilepath,
        mgr,
        render.GetD3D().GetWindowWidth(),
        render.GetD3D().GetWindowHeight());
}

//---------------------------------------------------------
// initialize skybox, skyplane, skydome
//---------------------------------------------------------
void CreateSkyStuff(const char* cfgFilepath, ECS::EntityMgr& enttMgr)
{
    SkyInitializer skyInit;
    skyInit.Init(cfgFilepath, enttMgr);
}

//---------------------------------------------------------
// Desc:   initialize all the entities on the scene
//---------------------------------------------------------
bool GameInitializer::InitEntities(
    ECS::EntityMgr& mgr,
    Render::CRender& render,
    const Core::EngineConfigs& cfgs,
    const GameInitPaths& initPaths)
{
    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("------------------------------------------------------------");
    LogMsg("                  INITIALIZATION: MODELS                    ");
    LogMsg("------------------------------------------------------------\n");
    SetConsoleColor(RESET);

    try
    {
        InitMaterials(initPaths.materialsFilepath);

        CreateTerrain(mgr, render, initPaths.terrainFilepath);

        Terrain& terrain = g_ModelMgr.GetTerrain();
        Rect3d worldBox = terrain.GetAABB();

        // init a quad tree
        const float maxWorldHeight = 160;
        const int quadTreeDepth = 6;
        worldBox.y1 = Max(worldBox.y1, maxWorldHeight);
        mgr.quadTree_.Init(worldBox, quadTreeDepth);

        // generate some entities based on geometric primitives
        GenerateEntities(mgr, render);

        Create2dSprites(initPaths.sprites2dFilepath, mgr, render);

        LoadModelsAndEntities(
            initPaths.modelsFilepath,
            initPaths.animationsFilepath,
            initPaths.entitiesFilepath,
            initPaths.natureGenFilepath,
            mgr,
            render,
            cfgs);

        CreateSkyStuff(initPaths.skyFilepath, mgr);
    }
    catch (const std::out_of_range& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "went out of range");
        return false;
    }
    catch (const std::bad_alloc& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't alloc mem for an element");
        return false;
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't init models");
        return false;
    }

    return true;
}

} // namespace Game
