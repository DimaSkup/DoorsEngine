// =================================================================================
// Filename:     InitializeGraphics.cpp
// Description:  there are functions for initialization of DirectX
//               and graphics parts of the engine;
//
// Created:      02.12.22
// =================================================================================
#include "InitializeGraphics.h"

#include "../Common/FileSystemPaths.h"
#include "../Common/Assert.h"
#include "../Common/MathHelper.h"
#include "../Common/StringHelper.h"
#include "Common/LIB_Exception.h"    // ECS exception

#include "../Model/ModelExporter.h"
#include "../Model/ModelLoader.h"
#include "../Model/ModelMath.h"
#include "../Model/ModelStorage.h"

#include "../Engine/ProjectSaver.h"

#include "InitGraphicsHelperDataTypes.h"

//#include "../Model/SkyModel.h"
#include "SetupModels.h"

#include <filesystem>

#define IMPORT_EXPORT_MODE true


using namespace DirectX;
namespace fs = std::filesystem;

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

#if 0
		// create and init scene elements
		result = InitializeModels(
			d3d.GetDevice(),
			d3d.GetDeviceContext(),
			entityMgr, 
			settings, 
			d3d.GetScreenDepth());
		Assert::True(result, "can't initialize models");
#endif

#if 1
		// init all the light source on the scene
		result = InitializeLightSources(entityMgr, settings);
		Assert::True(result, "can't initialize light sources");
#endif

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
	BasicCamera& cameraForRenderToTexture,
	DirectX::XMMATRIX& baseViewMatrix,      // is used for 2D rendering
	ECS::EntityMgr& enttMgr,
	const Settings& settings)
{
	try
	{
		const float nearZ       = settings.GetFloat("NEAR_Z");
		const float farZ        = settings.GetFloat("FAR_Z");
		const float fovInRad    = settings.GetFloat("FOV_IN_RAD");         // field of view in radians
		const float speed       = settings.GetFloat("CAMERA_SPEED");       // camera movement speed
		const float sensitivity = settings.GetFloat("CAMERA_SENSITIVITY"); // camera rotation speed

		const SIZE windowedSize = d3d.GetWindowedWndSize();
		const SIZE fullscreenSize = d3d.GetFullscreenWndSize();


		// 1. initialize the editor camera
		BasicCamera::CameraInitParams editorCamParams(
			(int)windowedSize.cx,
			(int)windowedSize.cy,
			nearZ, farZ, fovInRad, speed, sensitivity);

		// we need to have the proper aspect ratio for our editor camera
		// to prevent scene objects distortion
		//editorCamParams.aspectRatio_ = (float)fullscreenSize.cx / (float)fullscreenSize.cy;

		editorCamera.Initialize(editorCamParams);
		editorCamera.SetPosition(-15, 1, 5);

		// --------------------------------------------

		// 2. initialize the game camera
		

		BasicCamera::CameraInitParams gameCamParams(
			(int)fullscreenSize.cx,
			(int)fullscreenSize.cy,
			nearZ, farZ, fovInRad, speed, sensitivity);

		gameCamera.Initialize(gameCamParams);
		gameCamera.SetPosition(0, 2, -3);

		// --------------------------------------------

		// initialize view matrices of the cameras
		editorCamera.UpdateViewMatrix();
		gameCamera.UpdateViewMatrix();

		// initialize a base view matrix with the camera for 2D user interface rendering
		baseViewMatrix = editorCamera.GetViewMatrix();

		// create and setup an editor camera entity
		EntityID id = enttMgr.CreateEntity();
		enttMgr.AddCameraComponent(id, editorCamera.GetViewMatrix(), editorCamera.GetProjectionMatrix());
		enttMgr.AddNameComponent(id, "editor_camera");
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

void CreateSpheres(ID3D11Device* pDevice, ECS::EntityMgr& mgr, const BasicModel& model)
{
	//
	// create and setup spheres entities
	//

	Log::Debug();

	const u32 spheresCount = 10;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(spheresCount);

	// ---------------------------------------------------------
	// setup transform data for entities

	TransformData transform;

	transform.positions.reserve(spheresCount);
	transform.dirQuats.resize(spheresCount, {0,0,0,1});   // no rotation
	transform.uniformScales.resize(spheresCount, 1.0f);

	// make two rows of the spheres
	for (u32 idx = 0; idx < spheresCount/2; ++idx)
	{
		transform.positions.emplace_back(-5.0f, 5.0f, 10.0f * idx);
		transform.positions.emplace_back(+5.0f, 5.0f, 10.0f * idx);
	}
		
	mgr.AddTransformComponent(
		enttsIDs,
		transform.positions,
		transform.dirQuats,
		transform.uniformScales);


	// ---------------------------------------------------------
	// setup movement for the spheres

	MovementData movement;

	movement.translations.resize(spheresCount, { 0,0,0 });
	movement.rotQuats.resize(spheresCount, { 0,0,0,1 });  // XMQuaternionRotationRollPitchYaw(0, 0.001f, 0));
	movement.uniformScales.resize(spheresCount, 1.0f);
	movement.uniformScales[0] = 1.0f;

	mgr.AddMoveComponent(
		enttsIDs,
		movement.translations,
		movement.rotQuats,
		movement.uniformScales);

	transform.Clear();
	movement.Clear();	

	// ---------------------------------------------
	
	// generate a name for each sphere entity
	std::vector<EntityName> enttsNames(spheresCount, "sphere_");

	for (int i = 0; const EntityID& id : enttsIDs)
		enttsNames[i++] += std::to_string(id);

	// ---------------------------------------------
	// create and setup a model 

	TextureMgr* pTexMgr = TextureMgr::Get();
	ModelsCreator modelCreator;

	//const MeshSphereParams params;
	//BasicModel& model = modelCreator.CreateSphere(pDevice, params);
	//const TexPath gigachadTexPath = g_TexDirPath + "/gigachad.dds";

	//const TexID texID = pTexMgr->LoadFromFile(gigachadTexPath);
	//model.SetTexture(0, TexType::DIFFUSE, texID);

	// ---------------------------------------------

	mgr.AddModelComponent(enttsIDs, model.GetID());
	mgr.AddNameComponent(enttsIDs, enttsNames);
	mgr.AddRenderingComponent(enttsIDs);

	// ---------------------------------------------

	const size numEntts = std::ssize(enttsIDs);
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

void CreateSkyBox(ID3D11Device* pDevice, ECS::EntityMgr& mgr, SkyModel& sky)
{
	// create and setup the sky entity

	TexPath skyTexPath;
	int textureIdx = 0;
	XMFLOAT3 colorCenter;
	XMFLOAT3 colorApex;

	std::ifstream fin(g_ModelsDirPath + "sky_config.txt");

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
	TexID skyMapID = texMgr.LoadFromFile(g_TexDirPath + skyTexPath);

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

void CreateCylinders(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	//
	// create and setup cylinders entities
	//

	Log::Debug();

	const int cylindersCount = 10;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(cylindersCount);

	// ---------------------------------------------------------
	// setup transform data for entities

	TransformData transform;

	transform.positions.reserve(cylindersCount);
	transform.dirQuats.resize(cylindersCount, { 0,0,0,1 });  // no rotation
	transform.uniformScales.resize(cylindersCount, 1.0f);

	// make two rows of the cylinders
	for (int idx = 0; idx < cylindersCount / 2; ++idx)
	{
		transform.positions.emplace_back(-5.0f, 2.0f, 10.0f * idx);
		transform.positions.emplace_back(+5.0f, 2.0f, 10.0f * idx);
	}

	mgr.AddTransformComponent(
		enttsIDs,
		transform.positions,
		transform.dirQuats,
		transform.uniformScales);

	transform.Clear();

	// ---------------------------------------------------------
	// generate and set names for the entities

	std::vector<std::string> names(cylindersCount, "cylinder_");

	for (int i = 0; const EntityID id : enttsIDs)
		names[i++] += std::to_string(id);

	mgr.AddNameComponent(enttsIDs, names);
	names.clear();

	// ---------------------------------------------------------

	TextureMgr* pTexMgr = TextureMgr::Get();

	// set a default texture for the cylinder mesh
	const TexPath brickTexPath = g_TexDirPath + "brick01.dds";
	const TexID brickTexID = pTexMgr->LoadFromFile(brickTexPath);
	//model.SetTexture(0, TexType::DIFFUSE, brickTexID);

	// ---------------------------------------------------------

	mgr.AddModelComponent(enttsIDs, model.GetID());
	mgr.AddRenderingComponent(enttsIDs);

	TexID textures[22]{ 0 };
	textures[TexType::DIFFUSE] = brickTexID;

	for (const EntityID id : enttsIDs)
		mgr.AddTexturedComponent(id, textures, 22, 0);

	const size numEntts = std::ssize(enttsIDs);
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

void CreateCubes(ID3D11Device* pDevice, ECS::EntityMgr& mgr, const BasicModel& model)
{
	//
	// create and setup cubes entities
	//

	try
	{
		Log::Debug();
		const int numCubes = 6;

		const std::vector<EntityName> enttsNames =
		{
			"cat",
			"fireflame",
			"wireFence",
			"woodCrate01",
			"woodCrate02",
			"box01",
		};

		const std::vector<EntityID> enttsIDs = mgr.CreateEntities(numCubes);

		Assert::True(std::ssize(enttsNames) == numCubes, "num of names != num of cubes");

		// make a map: 'entt_name' => 'entity_id'
		std::map<EntityName, EntityID> enttsNameToID;

		for (int i = 0; i < numCubes; ++i)
			enttsNameToID.insert({ enttsNames[i], enttsIDs[i] });


		// ---------------------------------------------------------
		// setup textures for the cubes

		const std::string texKeys[numCubes] =
		{
			"cat",
			"fireAtlas",
			"wireFence",
			"woodCrate01",
			"woodCrate02",
			"box01",
		};

		const TexPath texFilenames[numCubes] =
		{
			"cat.dds",
			"fire_atlas.dds",
			"WireFence.dds",
			"WoodCrate01.dds",
			"WoodCrate02.dds",
			"box01d.dds",
		};

		TextureMgr& texMgr = *TextureMgr::Get();
		TexID texIDs[numCubes];  // just setup only diffuse texture for each cube

		// load textures
		for (int i = 0; i < numCubes; ++i)
			texIDs[i] = texMgr.LoadFromFile(g_TexDirPath + texFilenames[i]);

		// ---------------------------------------------

		std::map<std::string, TexID> keysToTexIDs;

		for (int i = 0; i < numCubes; ++i)
			keysToTexIDs.insert({ texKeys[i], texIDs[i] });

		// ---------------------------------------------------------
		// prepare position/rotations/scales

		const std::vector<XMVECTOR> dirQuats(numCubes, { 0,0,0,1 }); // no rotation
		const std::vector<float> uniformScales(numCubes, 1.0f);
		std::vector<XMFLOAT3> positions(numCubes);

		for (index i = 0; i < numCubes; ++i)
			positions[i] = { -15.0f, 1.0f, 2.0f * i };

		positions[2] = { -7, -0.3f, 0 };


		// ---------------------------------------------------------
		// prepare textures transformations

		ECS::AtlasAnimParams atlasTexAnimParams;
		ECS::RotationAroundCoordParams rotAroundCoordsParams;

		atlasTexAnimParams.Push(15, 8, 4);
		rotAroundCoordsParams.Push(0.5f, 0.5f, 0.1f);

		// ---------------------------------------------------------
		// setup the cubes entities

		mgr.AddTransformComponent(enttsIDs, positions, dirQuats, uniformScales);
		mgr.AddNameComponent(enttsIDs, enttsNames);
		mgr.AddModelComponent(enttsIDs, model.GetID());

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
			ECS::TexTransformType::ATLAS_ANIMATION,
			enttsNameToID.at("fireflame"),
			atlasTexAnimParams);

		mgr.AddTextureTransformComponent(
			ECS::TexTransformType::ROTATION_AROUND_TEX_COORD,
			enttsNameToID.at("cat"),
			rotAroundCoordsParams);

		mgr.AddRenderingComponent(enttsIDs);

		// setup blending params of the entities
		using enum ECS::RSTypes;
		mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("wireFence"), { ALPHA_CLIPPING, CULL_NONE });

		mgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("cat"), ADDING);
		//enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate01"), ADDING);
		//enttMgr.renderStatesSystem_.UpdateStates(enttsNameToID.at("woodCrate02"), MULTIPLYING);


		const size numEntts = std::ssize(enttsIDs);
		const size numSubsets = 1;                    // each cube has only one mesh
		const ECS::BoundingType boundTypes[1] = { ECS::BoundingType::BOUND_BOX };

		// add bounding component to each entity
		mgr.AddBoundingComponent(
			enttsIDs.data(),
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

void CreateTerrain(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	//
	// create and setup terrain elements
	//

	Log::Debug();


	// create and setup a terrain entity
	const EntityID enttID = mgr.CreateEntity();

	mgr.AddTransformComponent(enttID, { 0, 0, 0 });
	mgr.AddNameComponent(enttID, model.GetName());
	mgr.AddModelComponent(enttID, model.GetID());

	// setup a transformation for the terrain's texture
	ECS::StaticTexTransParams terrainTexTransform;
	terrainTexTransform.Push(DirectX::XMMatrixScaling(50, 50, 0));
	mgr.AddTextureTransformComponent(ECS::TexTransformType::STATIC, enttID, { terrainTexTransform });

	mgr.AddRenderingComponent(enttID);

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
}

///////////////////////////////////////////////////////////

void CreateWater(ID3D11Device* pDevice, ECS::EntityMgr& mgr)
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

void CreateSkull(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	// create and setup a skull entity

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
		Log::Error(e, true);
		Log::Error("can't create a skull model");
	}
}

///////////////////////////////////////////////////////////

void CreatePlanes(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	Log::Debug();

	try 
	{

	const UINT planesCount = 2;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(planesCount);

	// ---------------------------------------------------------
	// setup transform data for entities

	TransformData transform;

	transform.positions.resize(planesCount, { 0,0,0 });
	transform.dirQuats.resize(planesCount, { 0,0,0,1 });  // no rotation
	transform.uniformScales.resize(planesCount, 1.0f);

	transform.positions[0] = { 4, 1.5f, 6 };
	transform.positions[1] = { 2, 1.5f, 6 };

	mgr.AddTransformComponent(
		enttsIDs,
		transform.positions,
		transform.dirQuats,
		transform.uniformScales);

	// ---------------------------------------------------------
	// setup names for the entities

	std::vector<std::string> names(planesCount, "plane_");

	for (int i = 0; const EntityID id : enttsIDs)
		names[i++] += std::to_string(id);

	// ---------------------------------------------------------
	// setup meshes of the entities

	TextureMgr& texMgr = *TextureMgr::Get();

	const int numTexPaths = 2;
	const TexPath texPaths[numTexPaths] =
	{
		"data/textures/brick01.dds",
		"data/textures/sprite01.tga",
	};

	TexID texIDs[numTexPaths];

	// create textures
	for (int i = 0; i < numTexPaths; ++i)
		texIDs[i] = texMgr.LoadFromFile(texPaths[i]);

	//model.SetTexture(0, TexType::DIFFUSE, texIDs[0]);

	// setup texture for each plane mesh
	//for (int idx = 0; const MeshID meshID : meshesIDs)
	//	pMeshStorage->SetTextureForMeshByID(meshID, aiTextureType_DIFFUSE, texIDs[idx++]);
	

	mgr.AddModelComponent(enttsIDs, model.GetID());
	mgr.AddNameComponent(enttsIDs, names);
	mgr.AddRenderingComponent(enttsIDs);

	// ---------------------------------------------------------
	// setup render states of the entities
	using enum ECS::RSTypes;

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

inline float GetHeightOfGeneratedTerrainAtPoint(const float x, const float z)
{
	return 0.01f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

///////////////////////////////////////////////////////////

void CreateTrees(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	Log::Debug();

	using enum ECS::RSTypes;
	const int treesCount = 50;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(treesCount);

	// ---------------------------------------------

	std::vector<DirectX::XMFLOAT3> positions(treesCount);
	std::vector<DirectX::XMVECTOR> quats(treesCount);

	// place in center of the world
	positions[0] = { 0,0,0 };

	// generate positions
	for (XMFLOAT3& pos : positions)
	{
		pos.x = MathHelper::RandF(-150, 150);
		pos.z = MathHelper::RandF(-150, 150);
		pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);
	}

	// generate direction quats
	for (int i = 0; i < treesCount; ++i)
	{
		float angleX = DirectX::XM_PIDIV2;
		float angleY = MathHelper::RandF(0, XM_PIDIV2);
		float angleZ = 0;

		quats[i] = DirectX::XMQuaternionRotationRollPitchYaw(angleX, angleY, angleZ);
	}

	// generate name for each tree
	std::vector<EntityName> names(treesCount, "tree_");

	for (int i = 0; EntityName& name : names)
		name += std::to_string(enttsIDs[i++]);

	// ---------------------------------------------

	const float enttsUniformScale = 0.01f;

	mgr.AddTransformComponent(
		enttsIDs,
		positions, 
		quats,
		std::vector<float>(treesCount, enttsUniformScale));   // apply the same scale to each tree entity

	mgr.AddNameComponent(enttsIDs, names);
	mgr.AddModelComponent(enttsIDs, model.GetID());
	mgr.AddRenderingComponent(enttsIDs);

	// apply alpha_clipping and cull_none to each tree entt
	mgr.renderStatesSystem_.UpdateStates(enttsIDs, { ALPHA_CLIPPING, CULL_NONE });

	//const DirectX::XMMATRIX world = mgr.transformSystem_.GetWorldMatrixOfEntt(enttI)
	const size numSubsets = model.GetNumSubsets();
	const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);
	//std::vector<DirectX::BoundingBox> origAABBs(model.GetSubsetsAABB(), model.GetSubsetsAABB() + model.GetNumSubsets());

	// add bounding component to each entity
	mgr.AddBoundingComponent(
		enttsIDs.data(),
		treesCount,
		numSubsets,
		boundTypes.data(),
		model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreatePowerLine(ID3D11Device* pDevice, ECS::EntityMgr& mgr, const BasicModel& model)
{
	// create and setup a power lines entts

	Log::Debug();

	// create and setup an entity
	const int numPWTowers = 3;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(numPWTowers);

	// generate a position for each entt
	std::vector<XMFLOAT3> positions =
	{
		{ -200, 0, 8 },
		{ 0,    0, 8 },
		{ +200, 0, 8 },
	};

	for (XMFLOAT3& pos : positions)
		pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);


	// generate a name for each entt
	std::vector<std::string> names(numPWTowers, "power_hw_tower_");

	for (int i = 0; const EntityID id : enttsIDs)
		names[i++] += std::to_string(id);


	// apply the same rotation and scale for each entt
	DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, 0, 0);
	const std::vector<XMVECTOR> quats(numPWTowers, rotQuat);
	const std::vector<float> scales(numPWTowers, 0.5f);        

	mgr.AddTransformComponent(enttsIDs, positions, quats, scales);
	mgr.AddNameComponent(enttsIDs, names);
	mgr.AddModelComponent(enttsIDs, model.GetID());
	mgr.AddRenderingComponent(enttsIDs);

	// add bounding component to each subset of this entity
	const size numEntts = numPWTowers;
	const size numSubsets = model.GetNumSubsets();
	const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

	mgr.AddBoundingComponent(
		enttsIDs.data(),
		numEntts,
		numSubsets,
		boundTypes.data(),
		model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateBarrel(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr, 
	const BasicModel& model)
{
	// create and setup a barrel entity

	const EntityID enttID = mgr.CreateEntity();

	mgr.AddTransformComponent(enttID, { 0, 2, -5 });
	mgr.AddNameComponent(enttID, model.GetName());
	mgr.AddModelComponent(enttID, model.GetID());
	mgr.AddRenderingComponent(enttID);

	const size numEntts = 1;
	const size numSubsets = model.GetNumSubsets();
	const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

	mgr.AddBoundingComponent(
		&enttID,
		numEntts,
		numSubsets,
		boundTypes.data(),
		model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateNanoSuit(
	ID3D11Device* pDevice,
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	// create and setup a nanosuit entity

	Log::Debug();

	const EntityID enttID = mgr.CreateEntity();

	float posY = GetHeightOfGeneratedTerrainAtPoint(20, 8);
	DirectX::XMFLOAT3 pos = { 20, posY, 8 };
	DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYaw(0, DirectX::XM_PIDIV2, 0);

	mgr.AddTransformComponent(enttID, pos, rotQuat, {0.5f});
	mgr.AddNameComponent(enttID, model.GetName());
	mgr.AddModelComponent(enttID, model.GetID());
	mgr.AddRenderingComponent(enttID);

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
}

///////////////////////////////////////////////////////////

void CreateAK(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	Log::Debug();

	const EntityID enttID = mgr.CreateEntity();

	mgr.AddTransformComponent(enttID, { 10, 2, 6 }, { 0,0,0,1 }, { 5 });
	mgr.AddNameComponent(enttID, "aks74");
	mgr.AddModelComponent(enttID, model.GetID());
	mgr.AddRenderingComponent(enttID);

	// add bounding component to each subset of this entity
	const size numEntts = 1;
	const size numSubsets = model.GetNumSubsets();
	const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

	mgr.AddBoundingComponent(
		&enttID,
		1,
		numSubsets,
		boundTypes.data(),
		model.GetSubsetsAABB());      // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreateHouse(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	// create and setup a house entity

	Log::Debug();

	// create and setup an entity
	const EntityID enttID = mgr.CreateEntity();

	const float posY = GetHeightOfGeneratedTerrainAtPoint(-30, 14) + 2.5f;
	const XMFLOAT3 pos{ -30, posY, 14 };
	const XMVECTOR rotVec { DirectX::XM_PIDIV2, 0,0 };
	const XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYawFromVector(rotVec);

	mgr.AddTransformComponent(enttID, pos, quat);
	mgr.AddNameComponent(enttID, model.GetName());
	mgr.AddModelComponent(enttID, model.GetID());
	mgr.AddRenderingComponent({ enttID });

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
}

///////////////////////////////////////////////////////////

void CreateHouse2(
	ID3D11Device* pDevice, 
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{

	// create and setup a house entity

	Log::Debug();


	const DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XM_PIDIV2, DirectX::XM_PIDIV2, 0);

	const EntityID enttID = mgr.CreateEntity();

	const float posY = GetHeightOfGeneratedTerrainAtPoint(-20, -40);
	const XMFLOAT3 pos{ -20, posY, -40 };
	
	mgr.AddTransformComponent(enttID, pos, quat, {0.01f});
	mgr.AddNameComponent(enttID, "blockpost");
	mgr.AddModelComponent(enttID, model.GetID());

	mgr.AddRenderingComponent({ enttID });

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
}

///////////////////////////////////////////////////////////

void CreateRocks(
	ID3D11Device* pDevice,
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	// create and setup rock entities

	const int numRocks = 20;
	const std::vector<EntityID> enttsIDs = mgr.CreateEntities(numRocks);

	std::vector<XMFLOAT3> positions(numRocks);
	std::vector<XMVECTOR> quats(numRocks, { 0,0,0,1 });       // apply the same rotation to each entt
	std::vector<float>    scales(numRocks, 1);                // apply the same scale to each entt
	std::vector<std::string> names(numRocks, model.GetName() + "_");


	// generate positions for entts
	for (XMFLOAT3& pos : positions)
	{
		pos.x = MathHelper::RandF(-250.0f, 250.0f);
		pos.z = MathHelper::RandF(-250.0f, 250.0f);
		pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);
	}

	// generate names for entts
	for (int i = 0; std::string & name : names)
		name += std::to_string(enttsIDs[i++]);

	mgr.AddTransformComponent(enttsIDs, positions, quats, scales);
	mgr.AddNameComponent(enttsIDs, names);
	mgr.AddModelComponent(enttsIDs, model.GetID());

	mgr.AddRenderingComponent(enttsIDs);

	// add bounding component to each subset of this entity
	const size numEntts = numRocks;
	const size numSubsets = model.GetNumSubsets();
	const std::vector<ECS::BoundingType> boundTypes(numSubsets, ECS::BoundingType::BOUND_BOX);

	mgr.AddBoundingComponent(
		enttsIDs.data(),
		numEntts,
		numSubsets,
		boundTypes.data(),
		model.GetSubsetsAABB());             // AABB data (center, extents)
}

///////////////////////////////////////////////////////////

void CreatePillar(
	ID3D11Device* pDevice,
	ECS::EntityMgr& mgr,
	const BasicModel& model)
{
	const EntityID enttID = mgr.CreateEntity();

	XMFLOAT3 pos;
	pos.x = MathHelper::RandF(-250, 250);
	pos.z = MathHelper::RandF(-250, 250);
	pos.y = GetHeightOfGeneratedTerrainAtPoint(pos.x, pos.z);

	mgr.AddTransformComponent(enttID, pos, { 0,0,0,1 }, 1.0f);
	mgr.AddNameComponent(enttID, "pillar_" + std::to_string(enttID));
	mgr.AddModelComponent(enttID, model.GetID());

	mgr.AddRenderingComponent(enttID);

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
}

///////////////////////////////////////////////////////////

void CreateM3D(
	ID3D11Device* pDevice,
	ECS::EntityMgr& mgr,
	const std::string& filepath,
	const DirectX::XMFLOAT3& pos,
	const DirectX::XMFLOAT3& dir = {0,0,0})
{
	


#if 0

	bool useAlphaClipping = false;

	for (int i = 0; i < model.numSubsets_; ++i)
		useAlphaClipping |= model.meshes_.subsets_[i].alphaClip_;

	if (useAlphaClipping)
	{
		using enum ECS::RSTypes;
		mgr.renderStatesSystem_.UpdateStates(enttID, { ALPHA_CLIPPING, CULL_NONE });
	}
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
	Log::Print("------------------------------------------------------------", ConsoleColor::YELLOW);
	Log::Print("                 INITIALIZATION: MODELS                     ", ConsoleColor::YELLOW);
	Log::Print("------------------------------------------------------------", ConsoleColor::YELLOW);

	Log::Debug();

	try
	{

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

#endif
		ModelsCreator creator;
		ModelStorage& storage = *ModelStorage::Get();
		
		// NOTE: the bounding line box model must be created first of all, before all the other models
		const ModelID boundingBoxID = creator.CreateBoundingLineBox(pDevice);
	
#if 1
		
#else
		ProjectSaver saverLoader;
		saverLoader.LoadModels(pDevice);

		const ModelID treePineID = 2;
		const ModelID nanosuitID = 3;
		const ModelID powerHVTowerID = 4;
		const ModelID rockID = 5;
		const ModelID pillar1ID = 6;
		const ModelID pillar2ID = 7;
		const ModelID pillar5ID = 8;
		const ModelID pillar6ID = 9;
		const ModelID stalkerHouse1ID = 10;
		const ModelID stalkerHouse2ID = 11;
		const ModelID ak47ID = 12;

		// generated models
		const ModelID planeID = 13;
		const ModelID skullID = 14;
		const ModelID cylinderID = creator.CreateSkyDome(pDevice, 2, 30, 30); // 15;
		const ModelID cubeID = 16;
		const ModelID terrainID = 17;
		const ModelID sphereID = 18;
;
#endif

		
		MeshSphereParams skyDomeSphereParams(2000, 30, 30);
		creator.CreateSkyCube(pDevice, 2000);
		//creator.CreateSkySphere(pDevice, 2000, 30, 30);
		
		//const ModelID skyBoxID = creator.CreateSphere(pDevice, skyDomeSphereParams);
		SkyModel& skyBox                = storage.GetSky();


		const ModelID terrainID = creator.CreateGeneratedTerrain(pDevice, 500, 500, 501, 501);
		BasicModel& terrain = storage.GetModelByID(terrainID);
		SetupTerrain(terrain);
		CreateTerrain(pDevice, mgr, terrain);
#if 0
		// paths for import
		const std::string m3dDirPath = g_ModelsDirPath + "ModelsM3D/";
		const std::string pathRock = m3dDirPath + "rock.m3d";
		const std::string pathPillar1 = m3dDirPath + "pillar1.m3d";
		const std::string pathPillar2 = m3dDirPath + "pillar2.m3d";
		const std::string pathPillar5 = m3dDirPath + "pillar5.m3d";
		const std::string pathPillar6 = m3dDirPath + "pillar6.m3d";

#endif
		const std::string treePinePath     = g_ModelsDirPath + "trees/FBX format/tree_conifer_macedonian_pine.fbx";
		const std::string powerHVTowerPath = g_ModelsDirPath + "power_line/Power_HV_Tower.FBX";
		const std::string barrelPath       = g_ModelsDirPath + "Barrel1/Barrel1.obj";
#if 0
		const std::string nanosuitPath = g_ModelsDirPath + "nanosuit/nanosuit.obj";
		
		const std::string stalkerHouseSmallPath = g_ModelsDirPath + "stalker/stalker-house/source/SmallHouse.fbx";
		const std::string stalkerHouseAbandonedPath = g_ModelsDirPath + "stalker/abandoned-house-20/source/StalkerAbandonedHouse.fbx";
		const std::string ak47Path = g_ModelsDirPath + "stalker/aks-74_game_ready/scene.gltf";

		
#endif
		


		// IMPORT a model from file by path
		const ModelID treePineID     = creator.ImportFromFile(pDevice, treePinePath);
		const ModelID powerHVTowerID = creator.ImportFromFile(pDevice, powerHVTowerPath);
		const ModelID barrelID       = creator.ImportFromFile(pDevice, barrelPath);
#if 0
		const ModelID nanosuitID = creator.ImportFromFile(pDevice, nanosuitPath);
		
		const ModelID rockID = creator.ImportFromFile(pDevice, pathRock);
		const ModelID pillar1ID = creator.ImportFromFile(pDevice, pathPillar1);
		const ModelID pillar2ID = creator.ImportFromFile(pDevice, pathPillar2);
		const ModelID pillar5ID = creator.ImportFromFile(pDevice, pathPillar5);
		const ModelID pillar6ID = creator.ImportFromFile(pDevice, pathPillar6);
		const ModelID stalkerHouse1ID = creator.ImportFromFile(pDevice, stalkerHouseSmallPath);
		const ModelID stalkerHouse2ID = creator.ImportFromFile(pDevice, stalkerHouseAbandonedPath);
		const ModelID ak47ID = creator.ImportFromFile(pDevice, ak47Path);

		// generated models
		const MeshCylinderParams cylParams;
		const MeshSphereParams sphereParams;

		const ModelID planeID = creator.CreatePlane(pDevice);
		const ModelID skullID = creator.CreateSkull(pDevice);
#endif
		const ModelID cubeID = creator.CreateCube(pDevice);
#if 0		
		const ModelID cylinderID = creator.CreateCylinder(pDevice, cylParams);
		const ModelID terrainID = creator.CreateGeneratedTerrain(pDevice, 500, 500, 501, 501);
		const ModelID sphereID = creator.CreateSphere(pDevice, sphereParams);


#endif
		// models
		BasicModel& tree         = storage.GetModelByID(treePineID);
		BasicModel& cube         = storage.GetModelByID(cubeID);
		BasicModel& powerHVTower = storage.GetModelByID(powerHVTowerID);
		BasicModel& barrel       = storage.GetModelByID(barrelID);
#if 0
		BasicModel& skull = storage.GetModelByID(skullID);
		BasicModel& cylinder = storage.GetModelByID(cylinderID);
		BasicModel& nanosuit = storage.GetModelByID(nanosuitID);
		
		BasicModel& rock = storage.GetModelByID(rockID);
		BasicModel& pillar1 = storage.GetModelByID(pillar1ID);
		BasicModel& pillar2 = storage.GetModelByID(pillar2ID);
		BasicModel& pillar5 = storage.GetModelByID(pillar5ID);
		BasicModel& pillar6 = storage.GetModelByID(pillar6ID);
		BasicModel& stalkerHouse1 = storage.GetModelByID(stalkerHouse1ID);
		BasicModel& stalkerHouse2 = storage.GetModelByID(stalkerHouse2ID);
		BasicModel& ak47 = storage.GetModelByID(ak47ID);
		BasicModel& terrain = storage.GetModelByID(terrainID);
		BasicModel& sphere = storage.GetModelByID(sphereID);


		// manual setup of some models
		SetupTerrain(terrain);

#endif
		SetupTree(tree);
		SetupPowerLine(powerHVTower);

#if 0
		
		SetupStalkerSmallHouse(stalkerHouse1);
		SetupStalkerAbandonedHouse(stalkerHouse2);
		SetupSphere(sphere);

		// create entts
		CreateCylinders(pDevice, mgr, cylinder);
		CreateSkull(pDevice, mgr, skull);
		CreateNanoSuit(pDevice, mgr, nanosuit);
		
		CreateRocks(pDevice, mgr, rock);
		CreatePillar(pDevice, mgr, pillar1);
		CreatePillar(pDevice, mgr, pillar2);
		CreatePillar(pDevice, mgr, pillar5);
		CreatePillar(pDevice, mgr, pillar6);
		CreateHouse(pDevice, mgr, stalkerHouse1);
		CreateHouse2(pDevice, mgr, stalkerHouse2);
		CreateAK(pDevice, mgr, ak47);
		CreateTerrain(pDevice, mgr, terrain);
		CreateSpheres(pDevice, mgr, sphere);
#endif
		CreateCubes(pDevice, mgr, cube);
		CreateTrees(pDevice, mgr, tree);
		CreateSkyBox(pDevice, mgr, skyBox);
		CreatePowerLine(pDevice, mgr, powerHVTower);
		CreateBarrel(pDevice, mgr, barrel);
		//CreateWater(pDevice, entityMgr);
		//CreatePlanes(pDevice, mgr);

#if 0

		CreateM3D(pDevice, mgr, m3d_dir_path + "base.m3d", { 0,2,0 });
		CreateM3D(pDevice, mgr, m3d_dir_path + "stairs.m3d", { 0,-0.5f,-12.0f }, { 0, XM_PIDIV2, 0 });
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

bool InitializeGraphics::InitializeSprites(const UINT screenWidth,
	const UINT screenHeight)
{
	Log::Debug();

	const UINT crosshairWidth = 25;
	const UINT crosshairHeight = crosshairWidth;
	const char* animatedSpriteSetupFilename{ "data/models/sprite_data_01.txt" };
	const char* crosshairSpriteSetupFilename{ "data/models/sprite_crosshair.txt" };

	////////////////////////////////////////////////

#if 0

	// initialize an animated sprite
	pGameObj = pRenderableGameObjCreator_->Create2DSprite(animatedSpriteSetupFilename,
		"animated_sprite",
		{ 0, 500 },
		screenWidth, screenHeight);


	////////////////////////////////////////////////
	// compute a crosshair's center location
	POINT renderCrossAt{ screenWidth / 2 - crosshairWidth, screenHeight / 2 - crosshairHeight };

	// initialize a crosshair
	pGameObj = pRenderableGameObjCreator_->Create2DSprite(crosshairSpriteSetupFilename,
		"sprite_crosshair",
		renderCrossAt,
		screenWidth, screenHeight);
#endif

	return true;

}

/////////////////////////////////////////////////

bool InitializeGraphics::InitializeLightSources(
	ECS::EntityMgr& mgr,
	const Settings& settings)
{
	// this function initializes all the light sources on the scene

	const int numDirLights = 3;
	const int numPointLights = 10;
	const int numSpotLights = 1;

	// -----------------------------------------------------------------------------
	//                 DIRECTIONAL LIGHTS: SETUP AND CREATE
	// -----------------------------------------------------------------------------

	ECS::DirLightsInitParams dirLightsParams;

	dirLightsParams.ambients = 
	{
		{0.3f, 0.3f, 0.3f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};

	dirLightsParams.diffuses = 
	{
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.2f, 0.2f, 0.2f, 1.0f},
		{0.2f, 0.2f, 0.2f, 1.0f}
	};

	dirLightsParams.speculars =
	{
		{0.3f, 0.3f, 0.3f, 1.0f},
		{0.25f, 0.25f, 0.25f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};

	dirLightsParams.directions =
	{
		{0.57735f, -0.9f, 0.57735f},
		{-0.57735f, -0.57735f, 0.57735f},
		{0.0f, -0.707f, -0.707f}
	};

	// create directional light entities and add a light component to them
	std::vector<EntityID> dirLightsIds = mgr.CreateEntities(numDirLights);
	mgr.AddLightComponent(dirLightsIds,	dirLightsParams);
	mgr.AddNameComponent(dirLightsIds, { "dir_light_1", "dir_light_2", "dir_light_3" });
	

	// -----------------------------------------------------------------------------
	//                    POINT LIGHTS: SETUP AND CREATE
	// -----------------------------------------------------------------------------

	ECS::PointLightsInitParams pointLightsParams;

	// setup point light which is moving over the surface
	pointLightsParams.ambients.resize(numPointLights, { 0,0,0,1 });
	pointLightsParams.diffuses.resize(numPointLights, { 0.7f, 0.7f, 0.7f, 1.0f });
	pointLightsParams.speculars.resize(numPointLights, { 0.1f, 0.1f, 0.1f, 1.0f });
	pointLightsParams.attenuations.resize(numPointLights, { 0, 0.1f, 100 });
	pointLightsParams.ranges.resize(numPointLights, 60);
	pointLightsParams.positions.resize(numPointLights);

	// generate positions for the point light sources
	for (XMFLOAT3& pos : pointLightsParams.positions)
	{
		pos.x = MathHelper::RandF(-100, 100);
		pos.y = 4.0f;
		pos.z = MathHelper::RandF(-100, 100);
	}

	// generate diffuse colors for point lights
	for (int i = 1; i < numPointLights; ++i)
	{
		XMFLOAT4& diffColor = pointLightsParams.diffuses[i];
		XMFLOAT4& specColor = pointLightsParams.speculars[i];

		diffColor = MathHelper::RandColorRGBA();
		specColor = diffColor;

		diffColor.x *= 0.6f;
		diffColor.y *= 0.6f;
		diffColor.z *= 0.6f;

		specColor.x *= 0.1f;
		specColor.y *= 0.1f;
		specColor.z *= 0.1f;
	}

	// create point light entities and add a light component to them
	std::vector<EntityID> pointLightsIds = mgr.CreateEntities(numPointLights);

	std::vector<std::string> names(numPointLights);

	// generate name for each point light src
	for (int i = 0; const EntityID id : pointLightsIds)
		names[i++] = "point_light_" + std::to_string(i);

	
	mgr.AddLightComponent(pointLightsIds, pointLightsParams);
	mgr.AddNameComponent(pointLightsIds, names);



	// -----------------------------------------------------------------------------
	//                   SPOT LIGHTS: SETUP AND CREATE
	// -----------------------------------------------------------------------------

	ECS::SpotLightsInitParams spotLightsParams;

	spotLightsParams.ambients.resize(numSpotLights, { 0.01f, 0.01f, 0.01f, 1.0f });
	spotLightsParams.diffuses.resize(numSpotLights, { 0.1f, 0.1f, 0.1f, 1.0f });
	spotLightsParams.speculars.resize(numSpotLights, { 0, 0, 0, 1 });

	spotLightsParams.positions.resize(numSpotLights, { 0, 0, 0 });
	spotLightsParams.directions.resize(numSpotLights, { 0, 0, 0 });
	spotLightsParams.attenuations.resize(numSpotLights, { 0.0f, 0.01f, 0.0f });

	spotLightsParams.ranges.resize(numSpotLights, 100);
	spotLightsParams.spotExponents.resize(numSpotLights, 96);

	// create spot light entities and add a light component to them
	std::vector<EntityID> spotLightsIds = mgr.CreateEntities(numSpotLights);

	mgr.AddLightComponent(spotLightsIds, spotLightsParams);
	mgr.AddNameComponent(spotLightsIds.front(), "flashlight");


	return true;
}