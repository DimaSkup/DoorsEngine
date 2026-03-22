// ************************************************************************************
// Filename:        ModelsCreator.cpp
// Description:     implementation of the functional of the ModelsCreator class
//
// Created:         12.02.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "model_creator.h"

// models stuff
#include "model_loader.h"
#include "model_importer.h"
#include "geometry_generator.h"
#include "model.h"

// managers
#include "../Model/model_mgr.h"


namespace Core
{

//---------------------------------------------------------
// Desc:   load model data from .de3d file and init this model
//---------------------------------------------------------
ModelID ModelsCreator::CreateFromDE3D(const char* modelPath)
{
    try
    {
        ModelLoader loader;
        Model model;

        // load a model from file
        if (!loader.Load(modelPath, &model))
        {
            LogErr(LOG, "can't load model from a file: %s", modelPath);
            return INVALID_MODEL_ID;
        }

        // init vertex/index buffers
        model.InitBuffers();

        // add model into the model manager
        ModelID id = model.GetId();
        g_ModelMgr.AddModel(std::move(model));

        return id;
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
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
        LogErr(LOG, "empty path");
        return INVALID_MODEL_ID;
    }

    ModelImporter importer;
    Model&   model = g_ModelMgr.AddEmptyModel();
    
    FileSys::GetFileStem(path, g_String);
    g_ModelMgr.SetModelName(model.GetId(), g_String);

    if (!importer.LoadFromFile(&model, path))
    {
        LogErr(LOG, "can't import a model from file: %s", path);
        return INVALID_MODEL_ID;
    }

    return model.GetId();
}

//---------------------------------------------------------
// Desc:   create new plane model
//---------------------------------------------------------
ModelID ModelsCreator::CreatePlane(const float width, const float height)
{
    GeometryGenerator geoGen;
    Model& model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    // setup name
    sprintf(g_String, "plane_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry
    geoGen.GeneratePlane(width, height, model);

    model.InitBuffers();
    model.ComputeBoundings();
    
    return id;
}

//---------------------------------------------------------
// Desc:  create a model which will serve us as a generated grass
//---------------------------------------------------------
ModelID ModelsCreator::CreateGrassModel(const float planeW, const float planeH)
{
    GeometryGenerator geoGen;
    Model&    model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    // setup name
    g_ModelMgr.SetModelName(id, "generated_grass");

    // generate geometry and init vb/ib
    geoGen.GenerateGrass(model, planeW, planeH);
    model.InitBuffers();

    // manually setup boundings
    const float halfW = 0.5f * planeW;

    Vec3 minP{ -halfW, 0,      -halfW };
    Vec3 maxP{ +halfW, planeH, +halfW };

    Vec3 c = (maxP + minP) * 0.5f;   // center 
    Vec3 e = (maxP - minP) * 0.5f;   // extents

    DirectX::BoundingBox    bbox({c.x, c.y, c.z}, {e.x, e.y, e.z});
    DirectX::BoundingSphere sphere(bbox.Center, Vec3Length(e));

    model.SetModelBoundSphere(sphere);
    model.SetModelAABB(bbox);
    model.SetSubsetAABB(0, bbox);

    return id;
}

//---------------------------------------------------------
// Desc:  generate a plane lod model (just a point for billboard)
//        it may be used as a lod for trees, bushes, plants, etc.
//---------------------------------------------------------
ModelID ModelsCreator::CreatePlaneLod(const float planeW, const float planeH)
{
    GeometryGenerator geoGen;
    Model&     model = g_ModelMgr.AddEmptyModel();
    const ModelID id = model.GetId();

    // setup name
    sprintf(g_String, "tree_lod1_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry and init vb/ib
    geoGen.GeneratePlaneLod(model, planeW, planeH);
    model.InitBuffers();

    // manually setup boundings
    const float halfW = 0.5f * planeW;

    Vec3 minP{ -halfW, 0,      -halfW };
    Vec3 maxP{ +halfW, planeH, +halfW };

    Vec3 c = (maxP + minP) * 0.5f;   // center 
    Vec3 e = (maxP - minP) * 0.5f;   // extents

    DirectX::BoundingBox    bbox({c.x, c.y, c.z}, {e.x, e.y, e.z});
    DirectX::BoundingSphere sphere(bbox.Center, Vec3Length(e));

    model.SetModelBoundSphere(sphere);
    model.SetModelAABB(bbox);
    model.SetSubsetAABB(0, bbox);

    return id;
}

//---------------------------------------------------------
// Desc:  generate a "tree LOD1" model and store it into the model manager
//---------------------------------------------------------
ModelID ModelsCreator::CreateTreeLod1(
    const float planeW,                   // width of the plane
    const float planeH,                   // height of the plane
    const bool originAtBottom,
    const float rotateAroundX)
{
    GeometryGenerator geoGen;
    Model&     model = g_ModelMgr.AddEmptyModel();
    const ModelID id = model.GetId();

    // setup name
    sprintf(g_String, "tree_lod1_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry
    geoGen.GenerateTreeLod1(model, planeW, planeH, originAtBottom, rotateAroundX);

    model.InitBuffers();
    model.ComputeBoundings();

    return id;
}

//---------------------------------------------------------
// Desc:   generate a cube model and store it into the model manager
//---------------------------------------------------------
ModelID ModelsCreator::CreateCube()
{
    GeometryGenerator geoGen;
    Model&     model = g_ModelMgr.AddEmptyModel();
    const ModelID id = model.GetId();

    // setup name
    sprintf(g_String, "cube_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry
    geoGen.GenerateCube(model);

    model.InitBuffers();
    model.ComputeBoundings();

    return id;
}

//---------------------------------------------------------
// Desc:   generate a sky cube (box) model
// Args:   - height:  length of cube's one side
//---------------------------------------------------------
void ModelsCreator::CreateSkyCube(const float height)
{
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    geoGen.GenerateSkyBoxForCubeMap(sky, height);
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

    geoGen.GenerateSkySphere(sky, radius, sliceCount, stackCount);
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
    Model& model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    sprintf(g_String, "sky_dome_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry and init vb/ib
    geoGen.GenerateSkyDome(radius, sliceCount, stackCount, model);
    model.InitBuffers();

    return id;
}

//---------------------------------------------------------
// Desc:   generate a new sphere model by input params
// Args:   -params: sphere's geometry params
// Ret:    created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateSphere(const MeshSphereParams& params)
{
    GeometryGenerator geoGen;
    Model& model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    // setup name
    sprintf(g_String, "sphere_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry and init vb/ib
    geoGen.GenerateSphere(params, model);
    model.InitBuffers();

    // manually setup boundings
    DirectX::BoundingSphere sphere({ 0,0,0 }, params.radius);
    DirectX::BoundingBox bbox;
    DirectX::BoundingBox::CreateFromSphere(bbox, sphere);

    model.SetModelBoundSphere(sphere);
    model.SetModelAABB(bbox);
    model.SetSubsetAABB(0, bbox);

    return id;
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
    Model& model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    // setup name
    sprintf(g_String, "geo_sphere_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry and init vb/ib
    geoGen.GenerateGeosphere(params.radius, params.numSubdivisions, model);
    model.InitBuffers();

    // manually setup boundings
    DirectX::BoundingSphere sphere({ 0,0,0 }, params.radius);
    DirectX::BoundingBox bbox;
    DirectX::BoundingBox::CreateFromSphere(bbox, sphere);

    model.SetModelBoundSphere(sphere);
    model.SetModelAABB(bbox);
    model.SetSubsetAABB(0, bbox);

    return id;
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
    Model& model = g_ModelMgr.AddEmptyModel();
    const ModelID  id = model.GetId();

    // setup name
    sprintf(g_String, "cylinder_%" PRIu32, id);
    g_ModelMgr.SetModelName(id, g_String);

    // generate geometry and init vb/ib
    geoGen.GenerateCylinder(params, model);
    model.InitBuffers();

    // manually setup boundings
    const float r = Max(params.topRadius, params.bottomRadius);
    const Vec3  e = { r, 0.5f * params.height, r };     // extents

    const DirectX::BoundingBox    box({ 0,0,0 }, { e.x, e.y, e.z });
    const DirectX::BoundingSphere sphere(box.Center, Vec3Length(e));

    model.SetModelBoundSphere(sphere);
    model.SetModelAABB(box);
    model.SetSubsetAABB(0, box);

    return id;
}

} // namespace Core
