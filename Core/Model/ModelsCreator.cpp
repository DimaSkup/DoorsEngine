// ************************************************************************************
// Filename:        ModelsCreator.cpp
// Description:     implementation of the functional of the ModelsCreator class
//
// Created:         12.02.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "ModelsCreator.h"

#include "ModelMath.h"
#include "../Engine/Settings.h"

#include "GeometryGenerator.h"
#include "../Model/ModelLoader.h"
#include "../Model/BasicModel.h"
#include "../Terrain/Terrain.h"
#include "ModelImporter.h"

#include "../Model/ModelMgr.h"
#include "../Texture/TextureMgr.h"

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

    BasicModel& model = g_ModelMgr.AddEmptyModel();

    const char* filepath = "models/skull/skull.txt";
    ReadSkullMeshFromFile(model, filepath);

    // initialize vb/ib
    model.InitializeBuffers(pDevice);

    // setup the bounding box of the model
    model.ComputeSubsetsAABB();
    model.ComputeModelAABB();

    sprintf(model.name_, "skull_%ud", model.GetID());
    model.type_ = eModelType::Skull;

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
    Vertex3dTerrain* vertices,
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

// --------------------------------------------------------
// Desc:   create DirectX texture from the image's input raw data
// Args:   - name:      a name for texture identification
//         - data:      actual pixels data (one element per channel)
//         - width:     the texture width
//         - height:    the texture height
//         - bpp:       bits per pixel (24 or 32)
//         - mipMapped: defines if we will generate mipmaps of not
// Ret:    an ID of create texture (for details look at TextureMgr)
// --------------------------------------------------------
TexID CreateTextureFromRawData(
    ID3D11Device* pDevice,
    const char* name,
    const uint8* data,
    const uint width,
    const uint height,
    const int bpp,
    const bool mipMapped)
{
    uint8* dstImage = nullptr;

    try
    {
        // check input params
        CAssert::True(!StrHelper::IsEmpty(name),          "input name is empty");
        CAssert::True(data,                               "input ptr to image data array == nullptr");
        CAssert::True(width && !(width & (width - 1)),    "input width must be a power of 2");
        CAssert::True(height && !(height & (height - 1)), "input height must be a power of 2");
        CAssert::True(bpp == 24 || bpp == 32,             "input number of bits per pixel must be equal to 24 or 32");

        // if we got image with 24 bits per pixel we have to convert it into 32 bits
        // since DirectX11 cannot create 24 bits images
        if (bpp == 24)
        {
            const uint bytesPerPixel = 4;
            const uint numPixels     = width * height;
            const uint sizeInBytes   = numPixels * bytesPerPixel;

            dstImage = new uint8[sizeInBytes]{ 0 };

            // copy data from the generated 24bits texture map into the 32bits image buffer
            for (int i = 0, i1 = 0, i2 = 0; i < (int)numPixels; i++, i1 = i*4, i2 = i*3)
            {
                // convert from RGB to RGBA
                dstImage[i1 + 0] = data[i2 + 0];        // R
                dstImage[i1 + 1] = data[i2 + 1];        // G
                dstImage[i1 + 2] = data[i2 + 2];        // B
                dstImage[i1 + 3] = 255;                 // A (255 because we use uint8)
            }

            // create a DirectX texture
            Texture texture(pDevice, name, dstImage, width, height, mipMapped);

            // move texture into the textures manager and get an ID of the texture
            return g_TextureMgr.Add(name, std::move(texture));
        }

        // we already have a 32 bits image so just create a DirectX texture
        else
        {
            const uint bytesPerPixel = 4;
            const uint numPixels     = width * height;
            const uint sizeInBytes   = numPixels * bytesPerPixel;

            // create a DirectX texture
            Texture texture(pDevice, name, data, width, height, mipMapped);

            // move texture into the textures manager and get an ID of the texture
            return g_TextureMgr.Add(name, std::move(texture));
        }
    }
    catch (std::bad_alloc& e)
    {
        SafeDeleteArr(dstImage);
        sprintf(g_String, "can't allocate memory for the terrain texture: %s", name);
        LogErr(e.what());
        LogErr(g_String);
        return INVALID_TEXTURE_ID;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        SafeDeleteArr(dstImage);
        return INVALID_TEXTURE_ID;
    }
}

///////////////////////////////////////////////////////////

bool ModelsCreator::CreateTerrainFromHeightmap(
    ID3D11Device* pDevice,
    const char* configFilename)
{
    if (StrHelper::IsEmpty(configFilename))
    {
        LogErr("intput path to terrain config file is empty!");
        return false;
    }

    Terrain&          terrain = g_ModelMgr.GetTerrain();
    GeometryGenerator geoGen;
    TerrainConfig     terrainCfg;

    // load from the file meta-info about the terrain 
    terrain.LoadSetupFile(configFilename, terrainCfg);

    const int width = terrainCfg.width;
    const int depth = terrainCfg.depth;

    // generate terrain flat grid's vertices and indices by input params
    geoGen.GenerateTerrainFlatGrid(width, depth, width + 1, depth + 1, terrain);

    // generate a height map or load it from file
    bool hasHeights = false;

    if (terrainCfg.generateHeights)
    {
        // we generate height maps only relatively the terrain's size
        const int generatedHeightMapSize = width + 1;

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
        const int heightMapSize = width;
        hasHeights = terrain.LoadHeightMap(terrainCfg.pathHeightMap, heightMapSize);
    }

    // set heights for the terrain at particular positions
    if (hasHeights)
    {
        for (int i = 0; i < terrain.GetNumVertices(); ++i)
        {
            XMFLOAT3& pos = terrain.vertices_[i].position;
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z);
        }
    }

 

    // generate new texture map 
    if (terrainCfg.generateTextureMap)
    {
        // load tiles which will be used to generate texture map for terrain
        terrain.LoadTile(LOWEST_TILE,   terrainCfg.pathLowestTile);
        terrain.LoadTile(LOW_TILE,      terrainCfg.pathLowTile);
        terrain.LoadTile(HIGH_TILE,     terrainCfg.pathHighTile);
        terrain.LoadTile(HIGHEST_TILE,  terrainCfg.pathHighestTile);

        // generate texture map based on loaded tiles and height map
        terrain.GenerateTextureMap(terrainCfg.textureMapSize);
        terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);
    }

    // load texture map from file
    else
    {
        terrain.LoadTextureMap(terrainCfg.pathTextureMap);
    }
    

    // load detail map from the file
    terrain.LoadDetailMap(terrainCfg.pathDetailMap);

    //
    // Create DirectX textures for the terrain
    //
    constexpr bool mipMapped = false;
    const Image&   tileMap   = terrain.texture_;
    const Image&   detailMap = terrain.detailMap_;

    const TexID tileMapTexId = CreateTextureFromRawData(
        pDevice,
        "terrain_tile_map",
        tileMap.GetData(),
        tileMap.GetWidth(),
        tileMap.GetHeight(),
        tileMap.GetBPP(),
        mipMapped);

    const TexID detailMapTexId = CreateTextureFromRawData(
        pDevice,
        "terrain_detail_map",
        detailMap.GetData(),
        detailMap.GetWidth(),
        detailMap.GetHeight(),
        detailMap.GetBPP(),
        mipMapped);

    // set the texture's ID
    terrain.texture_.SetID(tileMapTexId);
    terrain.detailMap_.SetID(detailMapTexId);

    ComputeAveragedNormals(
        terrain.vertices_,
        terrain.indices_,
        terrain.numVertices_,
        terrain.numIndices_);

    // initialize vertex/index buffers
    terrain.InitBuffers(
        pDevice,
        terrain.vertices_,
        terrain.indices_,
        terrain.numVertices_,
        terrain.numIndices_);

    // compute the bounding box of the terrain
    const DirectX::XMFLOAT3 center  = { 0,0,0 };
    const DirectX::XMFLOAT3 extents = { (float)width, 1.0f, (float)depth };

    // setup axis-aligned bounding box
    terrain.SetAABB(center, extents);

    // release CPU copy of the vertices/indices data since we've already created buffers on GPU 
    terrain.ClearMemory();

    return true;
}

} // namespace Core
