#include "TestComponents.h"
#include "TestUtils.h"
#include "HelperTypes.h"
#include "Common/Types.h"   // ECS typedefs

#include <vector>
#include <DirectXMath.h>

using namespace DirectX;
using namespace TestUtils;

// ********************************************************************************
//                          PUBLIC TESTS FUNCTIONS
// ********************************************************************************

void TestComponents::Run()
{
	Log::Print();
	Log::Print("----------------  TESTS: ECS Components -----------------", ConsoleColor::YELLOW);
	Log::Print();

	try
	{
		// here we test if all the ECS components can be correctly added to entities
		TestTransformComponent();
		TestNameComponent();
		TestMoveComponent();
		TestMeshComponent();

		TestRenderComponent();
		TestTexturedComponent();
		TestTexTransformComponent();

		TestLightComponent();
	}
	catch (EngineException& e)
	{
		Log::Error(e, true);
		throw EngineException("can't pass the test for some ECS component");
	}
}

///////////////////////////////////////////////////////////

void TestComponents::TestTransformComponent()
{
	// UNIT TEST: check behaviour of the ECS when we use the Transform component;

	try
	{
		// here we check if everything works correct about adding 
		// the Transform and WorldMatrix components to the created entities

		ECS::EntityManager mgr;
		TransformData data;

		const u32 enttsCount = 50; 
		const ECS::Transform& transformComp = mgr.GetComponentTransform();
		const ECS::WorldMatrix& worldMatComp = mgr.GetComponentWorld();

		const std::vector<EntityID> ids = mgr.CreateEntities(enttsCount);
		GetRandTransformData(enttsCount, data);
		mgr.AddTransformComponent(ids, data.positions, data.dirQuats, data.uniformScales);

		// check data from the EntityManager
		const bool enttsHaveTransform = CheckEnttsHaveComponent(mgr, ids, transformComp.type_);
		const bool enttsHaveWorldMatrix = CheckEnttsHaveComponent(mgr, ids, worldMatComp.type_);

		Assert::True(enttsHaveTransform, "some entity doesn't have the Transform component");
		Assert::True(enttsHaveWorldMatrix, "some entity doesn't have the WorldMatrix component");

		// check if stored into the Transform component data is correct
		CompareTransformData(transformComp, ids, data);

		// check if stored into the WorldMatrix component data is correct
		const bool areIDsValid = TestUtils::ContainerCompare(worldMatComp.ids_, ids);
		Assert::True(areIDsValid, "TEST: IDs data from the WorldMatrix component isn't valid");

	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test with the Transform component");
	}

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestNameComponent()
{
	// here we test functional of the EntityManager about the Name component;

	try
	{
		ECS::EntityManager mgr;
		std::vector<EntityName> names;
		std::vector<EntityID> ids;

		const u32 enttsCount = 50;
		const u32 nameLength = 10;
		const ECS::Name& component = mgr.GetComponentName();

		GetRandEnttsNames(enttsCount, nameLength, names);
		ids = mgr.CreateEntities(enttsCount);
		mgr.AddNameComponent(ids, names);


		// test if data is stored correctly in the EntityManager
		const bool enttsHaveNames = CheckEnttsHaveComponent(mgr, ids, component.type_);
		Assert::True(enttsHaveNames, "some entity doesn't have the Name component");
		
		// test if data is stored correctly in the component
		CompareNameData(component, ids, names);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test for the Name component");
	}

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestMoveComponent()
{
	// UNIT TEST: check behaviour of the ECS when we use the Move component;

	try
	{
		std::vector<EntityID> ids;
		ECS::EntityManager mgr;
		MoveData move;
		const u32 enttsCount = 20;   // create this number of new empty entts
		const ECS::Movement& component = mgr.GetComponentMovement();

		ids = mgr.CreateEntities(enttsCount);
		GetRandMoveData(enttsCount, move);
		mgr.AddMoveComponent(ids, move.translations, move.rotQuats, move.uniformScales);


		// test if data is stored correctly in the EntityManager
		const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, ids, component.type_);
		Assert::True(enttsHaveComponent, "some entity doesn't have the Move component");
		
		// check if stored into the Movement component data is correct
		CompareMoveData(component, ids, move);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test for the Move component");
	}

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestMeshComponent()
{
	// UNIT TEST: check behaviour of the ECS when we add 
	//            the mesh component to entities

	try
	{
		std::vector<EntityID> enttsIDs;
		std::vector<MeshID> meshesIDs;
		ECS::EntityManager mgr;

		const u32 enttsCount = 50;   // create this number of new empty entts
		const ECS::MeshComponent& component = mgr.GetComponentMesh();

		// generate ids for meshes and sort them (!) because it 
		// MUST BE stored in sorted order
		TestUtils::GetArrOfRandUINTs(enttsCount, meshesIDs, 0, 100000);
		std::sort(meshesIDs.begin(), meshesIDs.end());

		enttsIDs = mgr.CreateEntities(enttsCount);
		mgr.AddMeshComponent(enttsIDs, meshesIDs);


		// test if data is stored correctly in the EntityManager
		const bool enttsHaveComponent  = CheckEnttsHaveComponent(mgr, enttsIDs, component.type_);
		Assert::True(enttsHaveComponent, "some entity doesn't have the Mesh component");

		// test if data is stored correctly in the component
		CompareMeshData(mgr.meshSystem_, enttsIDs, meshesIDs);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test with the Mesh component");
	}

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestRenderComponent()
{
	// UNIT TEST: check behaviour of the ECS when we add 
	//            the render component to entities

	try
	{
		ECS::EntityManager mgr;
		RenderedData data;
		std::vector<EntityID> ids;

		const u32 enttsCount = 50;
		const ECS::Rendered& component = mgr.GetComponentRendered();

		TestUtils::GetRandRenderedData(enttsCount, data);
		ids = mgr.CreateEntities(enttsCount);
		mgr.AddRenderingComponent(ids, data.shaderTypes, data.primTopologyTypes);


		// test if data is stored correctly in the EntityManager
		const bool enttsHaveComponent  = CheckEnttsHaveComponent(mgr, ids, component.type_);
		Assert::True(enttsHaveComponent, "some entity doesn't have the Rendered component");
		
		// test if data is stored correctly in the component
		CompareRenderedData(component, ids, data);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test for the Rendered component");
	}

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestTexturedComponent()
{
	// check if we can properly set textures for entities
	// (so entity will have differ textures from its mesh textures)

	ECS::EntityManager mgr;
	std::vector<EntityID> ids;


	
	// prepare data for the component
	const std::string texDir = "data/textures/";
	const std::vector<TexPath> diffuseTexPaths =
	{
		texDir + "cat.dds",
		texDir + "fire_atlas.dds",
		texDir + "lightmap.dds",
		texDir + "WireFence.dds",
		texDir + "WoodCrate01.dds",
		texDir + "WoodCrate02.dds",
		texDir + "box01d.dds",
	};

	//TestUtils::GetRandRenderedData(enttsCount, data);
	const ptrdiff_t texTypesCount = ECS::Textured::TEXTURES_TYPES_COUNT;
	const u32 diffuseTexTypeCode = 1;
	const size enttsCount = std::ssize(diffuseTexPaths);
	ids = mgr.CreateEntities((u32)enttsCount);

	// add textured component to each entity
	for (u32 idx = 0; const EntityID id : ids)
	{
		
		std::vector<TexID> texIDsForEntt(texTypesCount, 0);
		std::vector<TexPath> texPathsForEntt(texTypesCount, "unloaded");

		// set unique diffuse texture for each entity
		texIDsForEntt[diffuseTexTypeCode] = id;
		texPathsForEntt[diffuseTexTypeCode] = diffuseTexPaths[idx++];

		mgr.AddTexturedComponent(id, texIDsForEntt, texPathsForEntt);
	}	

	// ------------------------------------------

	// test if data is stored correctly in the EntityManager
	const ECS::Textured& component = mgr.GetComponentTextured();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, ids, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the Rendered component");

	// check if entts ids are set correctly in the component
	const bool enttsIDsAreValid = ContainerCompare(component.ids_, ids);
	Assert::True(enttsIDsAreValid, "Textured component: entts ids are invalid");

	// ------------------------------------------

	std::vector<TexID> texIDsForEntt(texTypesCount, 0);
	
	// check if textures ids are set correctly in the component
	for (u32 idx = 0; const EntityID id : ids)
	{
		texIDsForEntt[diffuseTexTypeCode] = id;

		const bool texIDsAreValid = ContainerCompare(component.texIDs_[idx++], texIDsForEntt);
		Assert::True(texIDsAreValid, "Textured component: textures ids are invalid");
	}

	// ------------------------------------------
	
	std::vector<TexPath> texPathsForEntt(texTypesCount, "unloaded");

	// check if textures paths are set correctly in the component
	for (u32 idx = 0; const TexPath& path : diffuseTexPaths)
	{
		texPathsForEntt[diffuseTexTypeCode] = path;

		const bool texPathsAreValid = ContainerCompare(component.texPaths_[idx++], texPathsForEntt);
		Assert::True(texPathsAreValid, "Textured component: textures paths are invalid");
	}

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestTexTransformComponent()
{
	// UNIT TEST: check behaviour of the ECS when we use
	//            the TextureTransform component (texture animation) to entities

	try
	{
		TestTexTransformStatic();
		TestTexTransformAtlasAnimation();
		TestTexTransformRotAroundCoords();
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't pass the test for the TextureTransform component");
	}

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestTexTransformStatic()
{
	ECS::EntityManager mgr;
	std::vector<EntityID> ids;
	const u32 enttsCount = 10;

	// prepare texture transformations
	const XMMATRIX texTransform = BuildMatrix({ 10,10,10 }, { 0,0,0.01f }, { 5,5,5 });
	ECS::StaticTexTransParams params(enttsCount, texTransform);

	ids = mgr.CreateEntities(enttsCount);
	mgr.AddTextureTransformComponent(ECS::TexTransformType::STATIC, ids, params);

	// test if data is stored correctly in the EntityManager
	const ECS::TextureTransform& component = mgr.GetComponentTexTransform();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, ids, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the TextureTransform component");

	// test if data is stored correctly in the component
	CompareTexTransformationsStatic(component, ids, params);
}

///////////////////////////////////////////////////////////

void TestComponents::TestTexTransformAtlasAnimation()
{
	ECS::EntityManager mgr;
	std::vector<EntityID> ids;
	const u32 enttsCount = 10;

	// prepare texture transformations
	std::vector<u32> texRows;
	std::vector<u32> texColumns;
	std::vector<float> animDurations;
	ECS::AtlasAnimParams params;

	// prepare params of atlas textures
	GetArrOfRandUINTs(enttsCount, texRows, 1, 100);
	GetArrOfRandUINTs(enttsCount, texColumns, 1, 100);
	GetArrOfRandFloats(enttsCount ,animDurations, 0.5f, 4.0f);

	// add data using different methods
	params.Push(texRows, texColumns, animDurations);
	params.Push(10, 10, 4.0f);


	ids = mgr.CreateEntities(enttsCount);
	mgr.AddTextureTransformComponent(ECS::TexTransformType::ATLAS_ANIMATION, ids, params);

	// test if data is stored correctly in the EntityManager
	const ECS::TextureTransform& component = mgr.GetComponentTexTransform();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, ids, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the TextureTransform component");

	// test if data is stored correctly in the component
	CompareTexTransformationsAtlasAnimations(component, ids, params);
}

///////////////////////////////////////////////////////////

void TestComponents::TestTexTransformRotAroundCoords()
{

}

///////////////////////////////////////////////////////////

void TestComponents::TestLightComponent()
{
	// test if we can correctly add the Light component to the entities

	TestDirLights();
	TestPointLights();
	TestSpotLights();
	
	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void TestComponents::TestDirLights()
{
	// test if we can correctly add the Light component 
	// (to be exact: directional lights) to the ECS entities

	const u32 enttsCount = 3;   // how many light sources do we want to create?
	ECS::EntityManager mgr;
	ECS::DirLightsInitParams dirLightsParams;

	std::vector<EntityID> dirLightsIds = mgr.CreateEntities(enttsCount);
	TestUtils::GetRandDirLightsData(enttsCount, dirLightsParams);
	mgr.AddLightComponent(dirLightsIds, dirLightsParams);

	// test if data is stored correctly in the EntityManager
	const ECS::Light& component = mgr.GetComponentLight();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, dirLightsIds, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the Light component");

	// test if data is stored correctly in the Light component
	TestUtils::CheckDirLightsProps(component, dirLightsIds, dirLightsParams);
}

///////////////////////////////////////////////////////////

void TestComponents::TestPointLights()
{
	// test if we can correctly add the Light component 
	// (to be exact: point lights) to the ECS entities

	const u32 enttsCount = 3;   // how many light sources do we want to create?
	ECS::EntityManager mgr;
	ECS::PointLightsInitParams pointLightsParams;

	std::vector<EntityID> pointLightsIds = mgr.CreateEntities(enttsCount);
	TestUtils::GetRandPointLightsData(enttsCount, pointLightsParams);
	mgr.AddLightComponent(pointLightsIds, pointLightsParams);

	// test if data is stored correctly in the EntityManager
	const ECS::Light& component = mgr.GetComponentLight();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, pointLightsIds, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the Light component");

	// test if data is stored correctly in the Light component
	TestUtils::CheckPointLightsProps(component, pointLightsIds, pointLightsParams);
}

///////////////////////////////////////////////////////////

void TestComponents::TestSpotLights()
{
	// test if we can correctly add the Light component 
	// (to be exact: spot lights) to the ECS entities

	const u32 enttsCount = 3;   // how many light sources do we want to create?
	ECS::EntityManager mgr;
	ECS::SpotLightsInitParams spotLightsParams;

	std::vector<EntityID> spotLightsIds = mgr.CreateEntities(enttsCount);
	TestUtils::GetRandSpotLightsData(enttsCount, spotLightsParams);
	mgr.AddLightComponent(spotLightsIds, spotLightsParams);

	// test if data is stored correctly in the EntityManager
	const ECS::Light& component = mgr.GetComponentLight();
	const bool enttsHaveComponent = CheckEnttsHaveComponent(mgr, spotLightsIds, component.type_);
	Assert::True(enttsHaveComponent, "some entity doesn't have the Light component");

	// test if data is stored correctly in the Light component
	TestUtils::CheckSpotLightsProps(component, spotLightsIds, spotLightsParams);
}

///////////////////////////////////////////////////////////