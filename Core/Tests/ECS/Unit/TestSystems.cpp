// *********************************************************************************
// Filename:       TestSystems.cpp
// Description:    implementation of tests for each system of the ECS;
// 
// Created:        13.06.24
// *********************************************************************************
#include "TestSystems.h"
#include "TestUtils.h"
#include "../Common/MathHelper.h"

using namespace DirectX;
using namespace TestUtils;

void TestSystems::Run()
{
	Log::Print();
	Log::Print("-------------  TESTS: ECS SYSTEMS  ---------------", ConsoleColor::YELLOW);
	Log::Print();

	try
	{
		srand((u32)time(NULL));

		TestSerialDeserial();
		TestMoveSysUpdating();
		TestTexTransformSysUpdating();
	}
	catch (EngineException& e)
	{
		Log::Error(e, false);
		Log::Error("TEST SYSTEMS: some test doesn't pass");
		exit(-1);
	}
}

///////////////////////////////////////////////////////////

void TestSystems::TestSerialDeserial()
{
	// here we test ECS systems for correct SEPARATE
	// serialization and deserialization of data from the ECS components

	try
	{
		TestTransformSysSerialDeserial();
		TestNameSysSerialDeserial();
		TestMoveSysSerialDeserial();
		TestMeshSysSerialDeserial();
		TestRenderedSysSerialDeserial();
	}
	catch (EngineException& e)
	{
		Log::Error(e, false);
		throw EngineException("TEST SYSTEMS: serialization/deserialization of some system works incorectly");
	}
}


// *********************************************************************************
//                            UPDATING TEST HELPERS
// *********************************************************************************

void TestSystems::TestMoveSysUpdating()
{
	// test updating functional of the ECS MoveSystem

	const u32 enttsCount = 1;
	ECS::EntityManager mgr;
	TransformData transform;
	MoveData move;
	std::vector<EntityID> ids;

	ids = mgr.CreateEntities(enttsCount);

	transform.positions.emplace_back(0.0f, 0.0f, 0.0f);
	transform.dirQuats.push_back({ 0,0,0,0 });
	transform.uniformScales.emplace_back(1.0f);

	move.translations.emplace_back(10.0f, 0.0f, 0.0f);
	move.rotQuats.push_back(XMQuaternionRotationRollPitchYaw(0, 0.001f, 0));
	move.uniformScales.emplace_back(1.0f);
	//GetRandTransformData(enttsCount, transform);
	//GetRandMoveData(enttsCount, move);

	mgr.AddTransformComponent(ids, transform.positions, transform.dirQuats, transform.uniformScales);
	mgr.AddMoveComponent(ids, move.translations, move.rotQuats, move.uniformScales);

	mgr.Update(100.0f, 1.0f);

	const ECS::Transform& transComp = mgr.GetComponentTransform();
	//const ECS::Movement& moveComp = mgr.GetComponentMovement();
	const ECS::WorldMatrix& worldComp = mgr.GetComponentWorld();

	XMFLOAT4 expectedTransAndUniScale = { 10.0f, 0.0f, 0.0f, 1.0f };
	XMMATRIX updatedWorldMat = worldComp.worlds_[0];

	XMVECTOR nonNormQuat = XMQuaternionRotationRollPitchYaw(0, 0.001f, 0);
	XMVECTOR normQuat = XMQuaternionNormalize(nonNormQuat);

	XMMATRIX expectedWorldMat = DirectX::XMMatrixAffineTransformation(
		{ 1,1,1 },      // scaling
		{ 0,0,0 },      // rotation origin
		nonNormQuat,    // rotation quat
		{ 10, 0, 0 });  // translation

	bool isTranslatedProperly = (transComp.posAndUniformScale_[0] == expectedTransAndUniScale);
	Assert::True(isTranslatedProperly, "wrong translation");

	Log::Print("\tPASSED");
}

// --------------------------------------------------------

void TestSystems::TestTexTransformSysUpdating()
{
	Log::Print("\tPASSED");
}

// *********************************************************************************
//                    SERIALIZATION / DESERIALIZATION HELPERS
// *********************************************************************************

void TestSystems::TestTransformSysSerialDeserial()
{
	// test the serialization and deserialization of data 
	// from the Transform component

	const std::string filepath = "test_serialization.bin";
	const u32 enttsCount = 10;
	std::vector<EntityID> ids;
	ECS::EntityManager origMgr;
	ECS::EntityManager deserMgr;
	TransformData data;

	// create entities and add the Trasnform component to them
	ids = origMgr.CreateEntities(enttsCount);

	GetRandTransformData(enttsCount, data);
	origMgr.AddTransformComponent(ids, data.positions, data.dirQuats, data.uniformScales);

	// serialize and deserialize transform data
	// and then check if deserialized data is correct
	SysSerialDeserialHelper(filepath, ECS::TransformComponent, origMgr, deserMgr);
	CompareTransformData(deserMgr.GetComponentTransform(), ids, data);

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestSystems::TestNameSysSerialDeserial()
{
	// test the serialization and deserialization of data
	// from the Name component using the NameSystem

	const std::string filepath = "test_serialization.bin";
	const u32 enttsCount = 50;
	const u32 nameLength = 10;
	ECS::EntityManager origMgr;
	ECS::EntityManager deserMgr;
	std::vector<EntityID> ids;
	std::vector<EntityName> names;
	
	GetRandEnttsNames(enttsCount, nameLength, names);

	ids = origMgr.CreateEntities(enttsCount);
	origMgr.AddNameComponent(ids, names);

	// serialize and deserialize names data
	// and then check if deserialized data is correct
	SysSerialDeserialHelper(filepath, ECS::NameComponent, origMgr, deserMgr);
	CompareNameData(deserMgr.GetComponentName(), ids, names);

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestSystems::TestMoveSysSerialDeserial()
{
	// test the serialization and deserialization of data 
	// from the Move component using the MoveSystem

	const std::string filepath = "test_serialization.bin";
	const u32 enttsCount = 10;
	ECS::EntityManager origMgr;
	ECS::EntityManager deserMgr;
	std::vector<EntityID> ids;
	MoveData data;

	// create entities and add the Movement component to them
	ids = origMgr.CreateEntities(enttsCount);
	GetRandMoveData(enttsCount, data);
	origMgr.AddMoveComponent(ids, data.translations, data.rotQuats, data.uniformScales);

	// serialize and deserialize movement data
	// and then check if deserialized data is correct
	SysSerialDeserialHelper(filepath, ECS::MoveComponent, origMgr, deserMgr);
	CompareMoveData(deserMgr.GetComponentMovement(), ids, data);

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestSystems::TestMeshSysSerialDeserial()
{
	// test the serialization and deserialization of data 
	// from the Mesh component

	const std::string filepath = "test_serialization.bin";
	const u32 enttsCount = 4;

	std::vector<EntityID> enttsIDs;
	std::vector<MeshID> meshesIDs;
	ECS::EntityManager origMgr;
	ECS::EntityManager deserMgr;

	// generate ids for meshes and sort them (!) because it 
	// MUST BE stored in sorted order
	TestUtils::GetArrOfRandUINTs(enttsCount, meshesIDs, 0, 100000);
	std::sort(meshesIDs.begin(), meshesIDs.end());

	enttsIDs = origMgr.CreateEntities(enttsCount);
	origMgr.AddMeshComponent(enttsIDs, meshesIDs);

	// store current data of the Mesh component before serialization 
	// so later we will use it for comparison with deserialized data
	//const ECS::MeshComponent& comp = origMgr.GetComponentMesh();
	//const auto origEntityToMeshes = comp.enttToMeshes_;
	//const auto origMeshToEntts = comp.meshToEntts_;

	// serialize and deserialize Mesh component data
	// and then check if deserialized data is correct
	SysSerialDeserialHelper(filepath, ECS::MeshComp, origMgr, deserMgr);
	CompareMeshData(deserMgr.meshSystem_, enttsIDs, meshesIDs);

	Log::Print("\tPASSED");
}

///////////////////////////////////////////////////////////

void TestSystems::TestRenderedSysSerialDeserial()
{
	// test the serialization and deserialization of data 
	// from the Rendered component

	const std::string filepath = "test_serialization.bin";
	const u32 enttsCount = 10;
	RenderedData data;
	ECS::EntityManager origMgr;
	ECS::EntityManager deserMgr;
	std::vector<EntityID> ids;

	GetRandRenderedData(enttsCount, data);
	ids = origMgr.CreateEntities(enttsCount);
	origMgr.AddRenderingComponent(ids, data.shaderTypes, data.primTopologyTypes);

	// serialize and deserialize Rendered component data
	// and then check if deserialized data is correct
	SysSerialDeserialHelper(filepath, ECS::RenderedComponent, origMgr, deserMgr);
	CompareRenderedData(deserMgr.GetComponentRendered(), ids, data);

	
	Log::Print("\tPASSED");
}