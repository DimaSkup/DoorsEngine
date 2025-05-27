// ************************************************************************************
// Filename:        ModelsCreator.cpp
// Description:     implementation of the functional of the ModelsCreator class
//
// Created:         12.02.24
// ************************************************************************************
#include "ModelsCreator.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/MathHelper.h>
#include <CoreCommon/FileSystem.h>
#include "ModelMath.h"
#include "../Engine/Settings.h"

#include "GeometryGenerator.h"
#include "../Model/ModelLoader.h"
#include "../Model/BasicModel.h"
#include "../Model/BruteForceTerrain.h"
#include "ModelImporter.h"

#include "../Model/ModelMgr.h"

using namespace DirectX;


namespace Core
{

ModelsCreator::ModelsCreator()
{
}

// ************************************************************************************

ModelID ModelsCreator::CreateFromDE3D(ID3D11Device* pDevice, const char* modelPath)
{
    // load model's data from a file in the INTERNAL format .de3d

    char relativePath[256]{ '\0' };             // path relative to the working directory
    strcat(relativePath, g_RelPathAssetsDir);
    strcat(relativePath, modelPath);

    try
    {
        ModelLoader loader;
        BasicModel model;

        // load a model from file
        const bool isLoaded = loader.Load(relativePath, model);
        if (!isLoaded)
        {
            sprintf(g_String, "can't load model from file: %s", relativePath);
            LogErr(g_String);
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
        sprintf(g_String, "can't load model by path: %s", relativePath);
        LogErr(g_String);
        return INVALID_MODEL_ID;
    }
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::ImportFromFile(
    ID3D11Device* pDevice,
    const char* modelPath)
{
    // create a model by loading its vertices/indices/texture data/etc. from a file
    
    try
    {
        ModelImporter importer;
        BasicModel& model = g_ModelMgr.AddEmptyModel();

        // set a name and type for the model
        FileSys::GetFileStem(modelPath, g_String);

        model.SetName(g_String);
        model.type_ = eModelType::Imported;


        // import model from a file by path
        importer.LoadFromFile(pDevice, model, modelPath);

        // initialize vb/ib
        model.InitializeBuffers(pDevice);
        
        model.ComputeSubsetsAABB();
        model.ComputeModelAABB();

        return model.id_;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        sprintf(g_String, "can't import a model from file: %s", modelPath);
        LogErr(g_String);
        return INVALID_MODEL_ID;
    }
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::Create(ID3D11Device* pDevice, const eModelType type)
{
    // create new BASIC model by input type and use default params;

#if 0
    switch (type)
    {
        case MeshType::Plane:
        {
            return CreatePlane(pDevice);
        }
        case MeshType::Cube:
        {
            return CreateCube(pDevice);
        }
        case MeshType::Skull:
        {
            return CreateSkull(pDevice);
        }
        case MeshType::Pyramid:
        {
            return CreatePyramid(pDevice);
        }
        case MeshType::Sphere:
        {
            return CreateSphere(pDevice);
        }
        case MeshType::Cylinder:
        {
            Mesh::MeshCylinderParams defaultCylParams;
            return CreateCylinder(pDevice, defaultCylParams);
        }
        default:
        {
            throw EngineException("Unknown mesh type");
        }
    }
#endif

    return 0;
}


// ************************************************************************************
//                                 HELPERS API
// ************************************************************************************
ModelID ModelsCreator::CreatePlane(
    ID3D11Device* pDevice,
    const float width,
    const float height)
{
    // create new plane model

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GeneratePlane(width, height, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    sprintf(model.name_, "plane_%ud", model.GetID());
    model.type_ = eModelType::Plane;

    return model.id_;
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateBoundingLineBox(ID3D11Device* pDevice)
{
    // create a line box which will be used to visualise bounding boxes (AABB)

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GenerateLineBox(model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    model.SetName("line_box");
    model.type_ = eModelType::LineBox;

    return model.id_;
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateCube(ID3D11Device* pDevice)
{
    // THIS FUNCTION creates a cube model and stores it into the storage;

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();
    
    // generate mesh data for the model
    geoGen.GenerateCube(model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    sprintf(model.name_, "cube_%ud", model.GetID());
    model.type_ = eModelType::Cube;

    return model.id_;
}

///////////////////////////////////////////////////////////

void ModelsCreator::CreateSkyCube(ID3D11Device* pDevice, const float height)
{
    // create a sky cube model which can be used to render the sky
    
    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    // generate mesh data for the sky model and init its buffers
    geoGen.GenerateSkyBoxForCubeMap(pDevice, sky, height);

    sky.SetName("sky_cube");
}

///////////////////////////////////////////////////////////


void ModelsCreator::CreateSkySphere(ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount)
{
    // create a sky sphere model which can be used to render the sky 
    // and also is used for the sky gradient

    GeometryGenerator geoGen;
    SkyModel& sky = g_ModelMgr.GetSky();

    // generate mesh data for the sky model and init its buffers
    geoGen.GenerateSkySphere(pDevice, sky, radius, sliceCount, stackCount);

    sky.SetName("sky_sphere");
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateWater(
    ID3D11Device* pDevice,
    const float width,
    const float depth)
{
#if 0
    // create a default water plane mesh;
    // return: plane mesh ID

    GeometryGenerator geoGen;
    MeshRawData data;

    geoGen.GeneratePlaneMesh(width, depth, data);

    data.name_ = "water";
    data.path_ = "data/models/default/water.txt";

    // -----------------------------------

    // specify a default material for the water
    MeshMaterial& mat = data.material_;

    mat.ambient_  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
    mat.diffuse_  = XMFLOAT4(0.137f, 0.42f, 0.556f, 0.5f);
    mat.specular_ = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
    mat.reflect_  = XMFLOAT4(.5f, .5f, .5f, 1);

    // -----------------------------------

    // set a default water texture for the mesh
    const TexPath waterDiffTexPath = "data/textures/water2.dds";
    const TexID waterDiffTexID = g_TextureMgr.LoadFromFile(waterDiffTexPath);

    data.texIDs_[aiTextureType_DIFFUSE] = waterDiffTexID;

    // store the mesh and return its ID
    return MeshStorage::Get()->CreateMeshWithRawData(pDevice, data);
#endif
    return 0;
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateSkyDome(
    ID3D11Device* pDevice,
    const float radius,
    const int sliceCount,
    const int stackCount)
{
    // generate a new sky dome model by input params
    // 
    // input:  geometry params for a mesh generation;
    // return: created model ID

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GenerateSkyDome(radius, sliceCount, stackCount, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    // setup name and type
    sprintf(model.name_, "sky_dome_%ud", model.GetID());
    model.type_ = eModelType::Sky;

    return model.id_;
}


///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateSphere(
    ID3D11Device* pDevice,
    const MeshSphereParams& params)
{
    // generate a new sphere model by input params
    // 
    // input:  geometry params for a mesh generation;
    // return: created model ID

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GenerateSphere(params, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    // setup the bounding box of the model
    const float r = params.radius_;
    DirectX::BoundingBox aabb;
    DirectX::XMStoreFloat3(&aabb.Center, { 0,0,0 });
    DirectX::XMStoreFloat3(&aabb.Extents, { r, r, r });

    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "sphere_%ud", model.GetID());
    model.type_ = eModelType::Sphere;

    return model.id_;
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateGeoSphere(
    ID3D11Device* pDevice,
    const MeshGeosphereParams& params)
{
    // generate a new GEOSPHERE model by input params
    // 
    // input:  geometry params for a mesh generation;
    // return: created model ID

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate mesh data for the model
    geoGen.GenerateGeosphere(params.radius_, params.numSubdivisions_, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    // setup the bounding box of the model
    const float r = params.radius_;
    DirectX::BoundingBox aabb;
    DirectX::XMStoreFloat3(&aabb.Center, { 0,0,0 });
    DirectX::XMStoreFloat3(&aabb.Extents, { r, r, r });

    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "geo_sphere_%ud", model.GetID());
    model.type_ = eModelType::GeoSphere;
    
    return model.id_;
}


///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateSkull(ID3D11Device* pDevice)
{
#if 0
    BasicModel& model = ModelStorage::Get()->AddEmptyModel();

    const std::string filepath = "models/skull/skull.txt";
    ReadSkullMeshFromFile(model, filepath);

    //ModelMath modelMath;
    //modelMath.CalculateModelVectors(model.vertices_, model.numVertices_, true);

    // specify a material for the skull
    Material& mat = model.materials_[0];
    mat.ambient_  = { 1, 1, 1, 1 };
    mat.diffuse_  = { 0, 1, 1, 1 };
    mat.specular_ = { 0.8f, 0.8f, 0.8f, 128.0f };

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    model.name_ = "skull_" + std::to_string(model.GetID());
    model.type_ = eModelType::Skull;

    return model.id_;
#endif

    return INVALID_MODEL_ID;
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

        for (u32 i = 0; i < model.numVertices_; ++i)
        {
            XMFLOAT3& pos = model.vertices_[i].position;
            XMFLOAT3& norm = model.vertices_[i].normal;

            fscanf(pFile, "%f %f %f", &pos.x, &pos.y, &pos.z);
            fscanf(pFile, "%f %f %f", &norm.x, &norm.y, &norm.z);
        }

        // skip separators
        fscanf(pFile, "%s%s%s", &buf, &buf, &buf);

        // read in indices
        for (u32 i = 0; i < model.numIndices_; ++i)
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
///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateCylinder(
    ID3D11Device* pDevice,
    const MeshCylinderParams& params)
{
    // generate new cylinder model by input params
    // 
    // input:  (if passed NULL then default) geometry params for a mesh generation;
    // return: created model ID

    GeometryGenerator geoGen;
    BasicModel& model = g_ModelMgr.AddEmptyModel();

    // generate geometry of cylinder by input params
    geoGen.GenerateCylinder(params, model);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    // setup the bounding box of the model
    const float r = max(params.topRadius_, params.bottomRadius_);
    DirectX::BoundingBox aabb;
    DirectX::XMStoreFloat3(&aabb.Center, { 0,0,0 });
    DirectX::XMStoreFloat3(&aabb.Extents, { r, 0.5f * params.height_, r });

    model.SetSubsetAABB(0, aabb);
    model.SetModelAABB(aabb);

    // setup name and type
    sprintf(model.name_, "cylinder_%ud", model.GetID());
    model.type_ = eModelType::Cylinder;

    return model.id_;
}

///////////////////////////////////////////////////////////

void GenerateHeightsForTerrainGrid(BasicModel& grid)
{
    // generate height for the input grid by some particular function;
    // (there can be several different types of height generation)

    Vertex3D* vertices = grid.vertices_;

#if 1
    for (u32 i = 0; i < grid.numVertices_; ++i)
    {
        DirectX::XMFLOAT3& pos = vertices[i].position;

        // a function for making hills for the terrain
        pos.y = 0.1f * (pos.z * sinf(0.1f * pos.x) + pos.x * cosf(0.1f * pos.z));

        // get hill normal
        // n = (-df/dx, 1, -df/dz)
        DirectX::XMVECTOR normalVec{
           -0.03f * pos.z * cosf(0.1f * pos.x) - 0.3f * cosf(0.1f * pos.z),
           1.0f,
           -0.3f * sinf(0.1f * pos.x) + 0.03f * pos.x * sinf(0.1f * pos.z) };

        normalVec = DirectX::XMVector3Normalize(normalVec);
        DirectX::XMStoreFloat3(&vertices[i].normal, normalVec);
    }

#elif 1

    // generate heights for the grid
    float m = 100.0f;
    float n = 100.0f;
    const float sin_step = DirectX::XM_PI / m * 3.0f;
    const float cos_step = DirectX::XM_PI / n * 5.0f;
    float valForSin = 0.0f;
    float valForCos = 0.0f;

    for (UINT i = 0; i < m; ++i)
    {
        valForSin = 0.0f;
        for (UINT j = 0; j < n; ++j)
        {
            const UINT idx = i * n + j;
            grid.vertices[idx].position.y = 30 * (sinf(valForSin) - cosf(valForCos));

            valForSin += sin_step;
        }
        valForCos += cos_step;
    }

#endif

    // after creation of heights we compute tangent for each vertex
    GeometryGenerator geoGen;
    geoGen.ComputeTangents(grid.vertices_, grid.indices_, grid.numIndices_);
}

///////////////////////////////////////////////////////////

void ComputeAveragedNormals(
    Vertex3D* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // compute normal-vectors for input vertices with vertex normal averaging
    //                  n0 + n1 + n2 + n3
    //        Navg = -----------------------
    //               || n0 + n1 + n2 + n3 ||

    // check input params
    if (!vertices || !indices)
    {
        LogErr("input arr of vertices/indices == nullptr");
        return;
    }

    if ((numVertices <= 0) || (numIndices <= 0))
    {
        LogErr("input number of vertices/indices must be > 0");
        return;
    }

    // for each triangle
    for (int i = 0; i < numIndices / 3; ++i)
    {
        // indices of the ith triangle 
        int baseIdx = i * 3;
        UINT i0 = indices[baseIdx + 0];
        UINT i1 = indices[baseIdx + 1];
        UINT i2 = indices[baseIdx + 2];

        // positions of vertices of ith triangle stored as XMVECTOR
        XMVECTOR v0 = DirectX::XMLoadFloat3(&vertices[i0].position);
        XMVECTOR v1 = DirectX::XMLoadFloat3(&vertices[i1].position);
        XMVECTOR v2 = DirectX::XMLoadFloat3(&vertices[i2].position);

        // compute face normal
        XMVECTOR e0 = v1 - v0;
        XMVECTOR e1 = v2 - v0;
        XMVECTOR normalVec = DirectX::XMVector3Cross(e0, e1);
        XMFLOAT3 faceNormal;
        DirectX::XMStoreFloat3(&faceNormal, normalVec);

        // this triangle shares the following three vertices, 
        // so add this face normal into the average of these vertex normals
        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }

    // for each vertex v, we have summed the face normals of all
    // the triangles that share v, so now we just need to normalize
    for (int i = 0; i < numVertices; ++i)
        vertices[i].normal = DirectX::XMFloat3Normalize(vertices[i].normal);
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateTerrainFromHeightmap(
    ID3D11Device* pDevice,
    const char* setupFilename)
{
    if (!setupFilename || setupFilename[0] == '\0')
    {
        LogErr("intput path to setup file is empty!");
        return INVALID_MODEL_ID;
    }

    BasicModel&       model = g_ModelMgr.AddEmptyModel();
    GeometryGenerator geoGen;
    BruteForceTerrain terrain;
    TerrainConfig     terrainCfg;

    // load from the file meta-info about the terrain 
    terrain.LoadSetupFile(setupFilename, terrainCfg);

    // generate terrain flat grid's vertices and indices by input params
    geoGen.GenerateFlatGrid(
        terrain.GetWidth(),
        terrain.GetDepth(),
        terrain.GetWidth() + 1,     // num of quads (cells count) by X 
        terrain.GetDepth() + 1,     // num of quads (cells count) by Z
        model);


    // generate a height map or load it from file
    bool hasHeights = false;

    if (terrainCfg.generateHeights)
    {
        // we generate height maps only relatively the terrain's size
        const int generatedHeightMapSize = terrainCfg.width + 1;

        // use "Fault formation" algorithm for height generation
        if (terrainCfg.useGenFaultFormation)
        {
            hasHeights = terrain.GenHeightFaultFormation(
                generatedHeightMapSize,
                terrainCfg.numIterations,
                terrainCfg.minDelta,
                terrainCfg.maxDelta,
                terrainCfg.filter);
        }

        // use "midpoint displacement" for height generation
        else
        {
            // bigger value makes smother terrain
            const float roughness = terrainCfg.roughness;
            hasHeights = terrain.GenHeightMidpointDisplacement(generatedHeightMapSize, roughness);
        }

        // also save this generated height map into the file
        terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);
    }
    else
    {
        // load a height map from the file
        const int heightMapSize = terrain.GetWidth();
        hasHeights = terrain.LoadHeightMap(terrainCfg.pathHeightMap, heightMapSize);
    }

    // set heights for the terrain at particular positions
    if (hasHeights)
    {
        for (int i = 0; i < model.GetNumVertices(); ++i)
        {
            XMFLOAT3& pos = model.vertices_[i].position;
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z);
        }
    }

    // load tiles which will be used to generate texture map for terrain
    terrain.LoadTile(LOWEST_TILE,   terrainCfg.pathLowestTile);
    terrain.LoadTile(LOW_TILE,      terrainCfg.pathLowTile);
    terrain.LoadTile(HIGH_TILE,     terrainCfg.pathHighTile);
    terrain.LoadTile(HIGHEST_TILE,  terrainCfg.pathHighestTile);

    // generate texture map based on loaded tiles and height map
    const TexID textureMap = terrain.GenerateTextureMap(pDevice, terrainCfg.textureMapSize);
    terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);


    ComputeAveragedNormals(model.vertices_, model.indices_, model.numVertices_, model.numIndices_);

    // initialize vertex/index buffers
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    return model.id_;
}

///////////////////////////////////////////////////////////

ModelID ModelsCreator::CreateGeneratedTerrain(
    ID3D11Device* pDevice,
    const int terrainWidth,
    const int terrainDepth,
    const int verticesCountByX,
    const int verticesCountByZ)
{
    // create a terrain grid with generated heights

    BasicModel& model = g_ModelMgr.AddEmptyModel();
    GeometryGenerator geoGen;

    // generate terrain flat grid's vertices and indices by input params
    geoGen.GenerateFlatGrid(
        terrainWidth,
        terrainDepth,
        verticesCountByX,     // num of quads (cells count) by X 
        verticesCountByZ,     // num of quads (cells count) by Z
        model);

    // generate height for each vertex of the terrain grid
    GenerateHeightsForTerrainGrid(model);

    // compute normal vector for each triangle in the grid
    //ComputeAveragedNormals(model.vertices_, model.indices_, model.numVertices_, model.numIndices_);

#if 0
    // setup a material for the single mesh (subset) of the model
    Material& mat = model.materials_[0];

    mat.ambient_  = XMFLOAT4(1, 1, 1, 1);
    mat.diffuse_  = XMFLOAT4(1, 1, 1, 1);
    mat.specular_ = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
#endif
    //mat.ambient_  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);   // like grass
    //mat.diffuse_  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
    
    // ----------------------------------- 

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    // setup name and type
    sprintf(model.name_, "terrain_gen_%d_%d", terrainWidth, terrainDepth);
    model.type_ = eModelType::Terrain;

    return model.id_;
}

} // namespace Core
