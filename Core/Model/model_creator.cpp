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

using namespace DirectX;


namespace Core
{

ModelsCreator::ModelsCreator()
{
}

//---------------------------------------------------------
// Desc:   load model data from .de3d file and init this model
//---------------------------------------------------------
ModelID ModelsCreator::CreateFromDE3D(ID3D11Device* pDevice, const char* modelPath)
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
        model.InitializeBuffers(pDevice);

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
ModelID ModelsCreator::ImportFromFile(ID3D11Device* pDevice, const char* path)
{
    if (StrHelper::IsEmpty(path))
    {
        LogErr(LOG, "input path is empty");
        return INVALID_MODEL_ID;
    }


    ModelImporter importer;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // set a name and type for the model
    FileSys::GetFileStem(path, g_String);
    g_ModelMgr.SetModelName(model.id_, g_String);
    model.type_ = MODEL_TYPE_Imported;

    if (!importer.LoadFromFile(pDevice, &model, path))
    {
        LogErr(LOG, "can't import model from file: %s", path);
        return INVALID_MODEL_ID;
    }

    return model.id_;
}

//---------------------------------------------------------
// Desc:   create new plane model
//---------------------------------------------------------
ModelID ModelsCreator::CreatePlane(
    ID3D11Device* pDevice,
    const float width,
    const float height)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GeneratePlane(width, height, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

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
    ID3D11Device* pDevice,
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
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    char name[MAX_LEN_MODEL_NAME];
    snprintf(name, MAX_LEN_MODEL_NAME, "tree_lod1_%" PRIu32, id);

    g_ModelMgr.SetModelName(id, name);
    model.type_ = MODEL_TYPE_Lod;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a cube model and store it into the model manager
//---------------------------------------------------------
ModelID ModelsCreator::CreateCube(ID3D11Device* pDevice)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();
    
    // generate mesh data for the model
    geoGen.GenerateCube(model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    sprintf(model.name_, "cube_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Cube;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a sky cube (box) model
// Args:   - height:  length of cube's one side
//---------------------------------------------------------
void ModelsCreator::CreateSkyCube(ID3D11Device* pDevice, const float height)
{
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    geoGen.GenerateSkyBoxForCubeMap(pDevice, sky, height);

    sky.SetName("sky_cube");
}

//---------------------------------------------------------
// Desc:   generate a sky sphere model (allow us to use a sky gradient)
// Args:   - radius:      sphere's radius
//         - sliceCount:  number of sphere's vertical parts
//         - stackCount:  number of spheres's horizontal parts
//---------------------------------------------------------
void ModelsCreator::CreateSkySphere(ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount)
{
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    geoGen.GenerateSkySphere(pDevice, sky, radius, sliceCount, stackCount);

    sky.SetName("sky_sphere");
}

//---------------------------------------------------------
// generate a new sky dome model by input params
// 
// input:  geometry params for a mesh generation;
// return: created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateSkyDome(
    ID3D11Device* pDevice,
    const float radius,
    const int sliceCount,
    const int stackCount)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateSkyDome(radius, sliceCount, stackCount, model);
    model.InitializeBuffers(pDevice);

    sprintf(model.name_, "sky_dome_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Sky;

    return model.id_;
}

//---------------------------------------------------------
// Desc:   generate a new sphere model by input params
// Args:   -params: sphere's geometry params
// Ret:    created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateSphere(
    ID3D11Device* pDevice,
    const MeshSphereParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateSphere(params, model);
    model.InitializeBuffers(pDevice);

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
ModelID ModelsCreator::CreateGeoSphere(
    ID3D11Device* pDevice,
    const MeshGeosphereParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateGeosphere(params.radius_, params.numSubdivisions_, model);
    model.InitializeBuffers(pDevice);

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
//---------------------------------------------------------
ModelID ModelsCreator::CreateSkull(ID3D11Device* pDevice)
{

    BasicModel& model = g_ModelMgr.AddEmptyModel();
    const char* filepath = "models/skull/skull.txt";
    ReadSkullMeshFromFile(model, filepath);

    model.InitializeBuffers(pDevice);

    // setup the bounding box of the model
    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    sprintf(model.name_, "skull_%" PRIu32, model.GetID());
    model.type_ = MODEL_TYPE_Skull;

    return model.id_;
}

///////////////////////////////////////////////////////////

void ModelsCreator::ReadSkullMeshFromFile(BasicModel& model, const char* filepath)
{
    try
    {
        FILE* pFile = nullptr;

        if ((pFile = fopen(filepath, "r+")) == NULL)
        {
            sprintf(g_String, "can't load a skull model data file: %s", filepath);
            LogErr(g_String);
            return;
        }

        char buf[64]{ '\0' };

        // ---------------------------------

        int numVertices = 0;
        int numTriangles = 0;
        constexpr int numSubsets = 1;      // the model has only one mesh (subset)

        // read in number of vertices and triangles
        fscanf(pFile, "%s %d", buf, &numVertices);
        fscanf(pFile, "%s %d", buf, &numTriangles);
        fscanf(pFile, "%s %s %s %s", buf, buf, buf, buf);

        // allocate memory for the vertices and indices data
        model.AllocateMemory(numVertices, numTriangles * 3, numSubsets);

        for (int i = 0; i < (int)model.numVertices_; ++i)
        {
            XMFLOAT3& pos  = model.vertices_[i].position;
            XMFLOAT3& norm = model.vertices_[i].normal;

            fscanf(pFile, "%f %f %f", &pos.x, &pos.y, &pos.z);
            fscanf(pFile, "%f %f %f", &norm.x, &norm.y, &norm.z);
        }

        // skip separators
        fscanf(pFile, "%s%s%s", &buf, &buf, &buf);

        // read in indices
        for (int i = 0; i < (int)model.numIndices_; ++i)
        {
            fscanf(pFile, "%ud", &model.indices_[i]);
        }
      
        fclose(pFile);
    }
    catch (const std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for mesh skull data");
        model.~BasicModel();
    }
}

///////////////////////////////////////////////////////////

#if 0
ModelID ModelsCreator::CreatePyramid(
    ID3D11Device* pDevice,
    const Mesh::MeshPyramidParams& meshParams)
{
    // generate a new pyramid mesh and store it into the mesh storage;
    // 
    // input:  (if passed NULL then default) geometry params for a mesh generation;
    // return: ID of created mesh

    GeometryGenerator geoGen;
    Mesh::MeshData mesh;

    mesh.name = "pyramid";
    mesh.path = "data/models/default/pyramid.txt";

    // generate pyramid's vertices and indices by input params
    geoGen.GeneratePyramidMesh(
        meshParams.height,         // height of the pyramid
        meshParams.baseWidth,      // width (length by X) of one of the base side
        meshParams.baseDepth,      // depth (length by Z) of one of the base side
        mesh);

    // store the mesh into the mesh storage and return ID of this mesh
    return MeshStorage::Get()->CreateMeshWithRawData(pDevice, mesh);

}
#endif

//---------------------------------------------------------
// generate new cylinder model by input params
// 
// input:  (if passed NULL then default) geometry params for a mesh generation;
// return: created model ID
//---------------------------------------------------------
ModelID ModelsCreator::CreateCylinder(
    ID3D11Device* pDevice,
    const MeshCylinderParams& params)
{
    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    geoGen.GenerateCylinder(params, model);
    model.InitializeBuffers(pDevice);

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
// Args:   - configFilename: path to a file with params for terrain
//---------------------------------------------------------
bool ModelsCreator::CreateTerrain(const char* configFilename)
{
    return TerrainCreator::CreateTerrain(Render::g_pDevice, configFilename);
}


} // namespace Core
