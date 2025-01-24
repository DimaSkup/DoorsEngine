#include "TestEntityMgr.h"
#include "TestUtils.h"

#include "Entity/EntityManager.h"
#include "../Engine/EngineException.h"
#include "../Engine/log.h"


using namespace TestUtils;

void TestEntityMgr::TestEntitiesCreation()
{
	// UNIT TEST: check if we can correctly create some amount of empty entities 
	//            inside the entity manager

	ECS::EntityManager mgr;
	const u32 newEnttsCount = 20;
	const std::vector<EntityID>& ids = mgr.CreateEntities(newEnttsCount);

	// test if such entities are correctly stored in the EntityManager
	Assert::True(mgr.CheckEnttsByIDsExist(ids), "the CheckEnttsByIDsExist() doens't work correctly");

	Log::Print("\t\tPASSED");
}

///////////////////////////////////////////////////////////

void CheckDeserialEnttMgrData(
	const ECS::EntityManager& mgr,
	const std::vector<EntityID>& origIDs,
	const std::vector<ComponentsHash>& origFlags)
{
	// check if deserialized data of the EntityManager (ids, flags, etc.)
	// is equal to the input origin data

	const bool areIDsCorrect   = ContainerCompare(mgr.ids_, origIDs);
	const bool areFlagsCorrect = ContainerCompare(mgr.componentHashes_, origFlags);

	Assert::True(areIDsCorrect,   "TEST ENTITY MANAGER: deserialized entities IDs data isn't correct");
	Assert::True(areFlagsCorrect, "TEST ENTITY MANAGER: deserialized component flags data isn't correct");
}

///////////////////////////////////////////////////////////

void TestEntityMgr::TestSerialDeserial()
{
	// test the EntityManager for correct serialization/deserialization 
	// of its own data (all the entities IDs / related to entts components / etc.)

	const std::string filepath = "test_entity_mgr_serialization.bin";
	const u32 enttsCount = 50;                        // how many entts will we create
	const std::vector<MeshID> meshesIDs{ 1,2,3,4,5 };  // like each entity has these meshes
	std::vector<EntityID> enttsIDs;
	std::vector<EntityName> names;

	ECS::EntityManager origMgr;     // entity mgr which will contain the origin data
	ECS::EntityManager deserMgr;    // entity mgr which will contain deserialized data
	TransformData transform;
	MoveData move;
	RenderedData rendered;
	

	// prepare data for the components
	GetRandTransformData(enttsCount, transform);
	GetRandEnttsNames(enttsCount, 10, names);
	GetRandMoveData(enttsCount, move);
	GetRandRenderedData(enttsCount, rendered);

	// create entities and add components to them
	enttsIDs = origMgr.CreateEntities(enttsCount);

	origMgr.AddTransformComponent(enttsIDs, transform.positions, transform.dirQuats, transform.uniformScales);
	origMgr.AddNameComponent(enttsIDs, names);
	origMgr.AddMoveComponent(enttsIDs, move.translations, move.rotQuats, move.uniformScales);
	origMgr.AddMeshComponent(enttsIDs, meshesIDs);
	origMgr.AddRenderingComponent(enttsIDs, rendered.shaderTypes, rendered.primTopologyTypes);

	// ---------------------------------------------

	// store some origin data before serialization so later we'll use it for testing
	const std::vector<ComponentsHash> origFlags = origMgr.componentHashes_;
	
	// serialize and deserialize data from the EntityManager and Components
	origMgr.Serialize(filepath);         
	deserMgr.Deserialize(filepath);   // deserialize data into the NEW entity manager
	RemoveFile(filepath);

	// ---------------------------------------------

	// TODO: instead of separate checking of each component, make a 
	//       method for complete comparison of two entity managers

	// check if all the deserialized data is correct 
	CheckDeserialEnttMgrData(deserMgr, enttsIDs, origFlags);
	CompareTransformData(deserMgr.GetComponentTransform(), enttsIDs, transform);
	CompareMoveData(deserMgr.GetComponentMovement(), enttsIDs, move);
	CompareNameData(deserMgr.GetComponentName(), enttsIDs, names);
	CompareMeshData(deserMgr.meshSystem_, enttsIDs, meshesIDs);
	CompareRenderedData(deserMgr.GetComponentRendered(), enttsIDs, rendered);

	Log::Print("\t\tPASSED");
}
