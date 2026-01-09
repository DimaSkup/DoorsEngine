// ************************************************************************************
// Filename:        ModelsCreator.cpp
// Description:     implementation of the functional of the ModelsCreator class
//
// Created:         12.02.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "model_creator.h"

// common stuff
#include <Render/d3dclass.h>           // for using global pointers to DX11 device and context

// models stuff
#include "model_importer.h"
#include "geometry_generator.h"
#include "basic_model.h"
#include "model_math.h"
#include "model_loader.h"

// terrain stuff
#include "../Terrain/TerrainCreator.h"

// managers
#include "../Model/model_mgr.h"
#include "../Texture/texture_mgr.h"

#include <Render/d3dclass.h>


using namespace DirectX;


namespace Core
{
//---------------------------------------------------------
// a little cringe helper to wrap getting of DX11 device
//---------------------------------------------------------
static ID3D11Device* GetDevice()
{
    return Render::g_pDevice;
}

//---------------------------------------------------------

ModelsCreator::ModelsCreator()
{
}

//---------------------------------------------------------
// Desc:   load model data from .de3d file and init this model
//---------------------------------------------------------
ModelID ModelsCreator::CreateFromDE3D(const char* modelPath)
{
    try
    {
        ModelLoader loader;
        BasicModel model;

        // load a model from file
        const bool isLoaded = loader.Load(modelPath, &model);
        if (!isLoaded)
        {
            LogErr(LOG, "can't load model from file: %s", modelPath);
            return INVALID_MODEL_ID;
        }

        // init vertex/index buffers
        model.InitializeBuffers(GetDevice());

        // add model into the model manager
        ModelID id = id = model.id_;
        g_ModelMgr.AddModel(std::move(model));

        return id;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't load model by path: %s", modelPath);
        return INVALID_MODEL_ID;
    }

    return INVALID_MODEL_ID;
}

//---------------------------------------------------------
// Desc:    create a model by loading its
//          vertices/indices/texture data/etc. from a file
// Args:    - pDevice:  a ptr to DX11 device
//          - path:     a path to file with model
//---------------------------------------------------------
ModelID ModelsCreator::ImportFromFile(const char* path)
{
    if (StrHelper::IsEmpty(path))
    {
        LogErr(LOG, "input path is empty");
        return INVALID_MODEL_ID;
    }

    char          modelName[MAX_LEN_MODEL_NAME]{ '\0' };
    ModelImporter importer;
    BasicModel&   model = g_ModelMgr.AddEmptyModel();
    
    FileSys::GetFileStem(path, modelName);
    g_ModelMgr.SetModelName(model.id_, modelName);

    model.type_ = MODEL_TYPE_Imported;

    if (!importer.LoadFromFile(GetDevice(), &model, path))
    {
        LogErr(LOG, "can't import model from file: %s", path);
        return INVALID_MODEL_ID;
    }

    return model.id_;
}

//---------------------------------------------------------
// Desc:   create new plane model
//---------------------------------------------------------
ModelID ModelsCreator::CreatePlane(const float width, const float height)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GeneratePlane(width, height, model);

    // initialize vb/ib
    model.InitializeBuffers(GetDevice());

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    sprintf(model.name_, "plane_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Plane;

    return model.id_;
}

//---------------------------------------------------------
// Desc:  generate a "tree LOD1" model and store it into the model manager
//---------------------------------------------------------
ModelID ModelsCreator::CreateTreeLod1(
    const float planeWidth,
    const float planeHeight,
    const bool originAtBottom,
    const float rotateAroundX)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();
    const ModelID id = model.GetID();

    // generate mesh data for the model
    geoGen.GenerateTreeLod1(model, planeWidth, planeHeight, originAtBottom, rotateAroundX);

    // initialize vb/ib
    model.InitializeBuffers(GetDevice());

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    char name[MAX_LEN_MODEL_NAME]{'\0'};
    snprintf(name, MAX_LEN_MODEL_NAME, "tree_lod1_%" PRIu32, id);

    g_ModelMgr.SetModelName(id, name);
    model.type_ = MODEL_TYPE_Lod;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a cube model and store it into the model manager
//---------------------------------------------------------
ModelID ModelsCreator::CreateCube()
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    model.type_ = MODEL_TYPE_Cube;

    // generate mesh data for the model
    geoGen.GenerateCube(model);

    // initialize vb/ib
    model.InitializeBuffers(GetDevice());

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name
    ModelID cubeId = model.GetID();
    char modelName[MAX_LEN_MODEL_NAME]{'\0'};
    sprintf(modelName, "cube_%d", (int)cubeId);
    g_ModelMgr.SetModelName(cubeId, modelName);

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a sky cube (box) model
// Args:   - height:  length of cube's one side
//---------------------------------------------------------
void ModelsCreator::CreateSkyCube(const float height)
{
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    geoGen.GenerateSkyBoxForCubeMap(GetDevice(), sky, height);

    sky.SetName("sky_cube");
}

//---------------------------------------------------------
// Desc:   generate a sky sphere model (allow us to use a sky gradient)
// Args:   - radius:      sphere's radius
//         - sliceCount:  number of sphere's vertical parts
//         - stackCount:  number of spheres's horizontal parts
//---------------------------------------------------------
void ModelsCreator::CreateSkySphere(const float radius, const int sliceCount, const int stackCount)
{
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    geoGen.GenerateSkySphere(GetDevice(), sky, radius, sliceCount, stackCount);

    sky.SetName("sky_sphere");
}

//---------------------------------------------------------
// generate a new sky dome model by input params
// 
// input:  geometry params for a mesh generation;
// return: created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateSkyDome(
    const float radius,
    const int sliceCount,
    const int stackCount)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateSkyDome(radius, sliceCount, stackCount, model);
    model.InitializeBuffers(GetDevice());

    sprintf(model.name_, "sky_dome_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Sky;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a new sphere model by input params
// Args:   -params: sphere's geometry params
// Ret:    created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateSphere(const MeshSphereParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateSphere(params, model);
    model.InitializeBuffers(GetDevice());

    // setup the bounding box of the model
    const float r = params.radius_;
    DirectX::BoundingBox aabb({ 0,0,0 }, { r,r,r });
    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "sphere_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Sphere;

    return model.id_;
}

//---------------------------------------------------------
// generate a new GEOSPHERE model by input params
// 
// input:  geometry params for a mesh generation;
// return: created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateGeoSphere(const MeshGeosphereParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateGeosphere(params.radius_, params.numSubdivisions_, model);
    model.InitializeBuffers(GetDevice());

    // setup the bounding box of the model
    const float r = params.radius_;
    DirectX::BoundingBox aabb({ 0,0,0 }, { r, r, r });
    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "geo_sphere_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_GeoSphere;
    
    return model.id_;
}

//---------------------------------------------------------
// generate new cylinder model by input params
// 
// input:  geometry params for a mesh generation;
// return: created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateCylinder(const MeshCylinderParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateCylinder(params, model);
    model.InitializeBuffers(GetDevice());

    // setup the bounding box of the model
    const float r = max(params.topRadius_, params.bottomRadius_);
    const DirectX::XMFLOAT3 extents(r, 0.5f * params.height_, r);
    const DirectX::BoundingBox aabb({ 0,0,0 }, extents);

    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "cylinder_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Cylinder;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   create and setup a terrain
// Args:   - configFilepath:  path to a file with params for terrain
//---------------------------------------------------------
bool ModelsCreator::CreateTerrain(const char* configFilepath)
{
    return TerrainCreator::CreateTerrain(configFilepath);
}


} // namespace Core
