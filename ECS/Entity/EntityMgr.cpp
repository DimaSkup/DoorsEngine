#include "EntityMgr.h"

#include "../Common/vector.h"
#include "../Common/Utils.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"


#include "EntityMgrSerializer.h"
#include "EntityMgrDeserializer.h"

#include <cassert>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include <cctype>
#include <random>

using namespace Utils;

namespace ECS
{

// after creation of each new entity this value is increased by 1
int EntityMgr::lastEntityID_ = 1;


EntityMgr::EntityMgr() :
	nameSystem_ {&names_},
	transformSystem_{ &transform_, &world_ },
	moveSystem_{ &transform_, &world_, &movement_ },
	modelSystem_{ &modelComponent_ },
	renderSystem_{ &renderComponent_ },
	texturesSystem_{ &textureComponent_ },
	texTransformSystem_ { &texTransform_ },
	lightSystem_{ &light_ },
	renderStatesSystem_{ &renderStates_ },
	boundingSystem_ { &bounding_ },
	cameraSystem_{ &camera_ }
{
	Log::Debug("start of entity mgr init");


	const u32 reserveMemForEnttsCount = 100;

	ids_.reserve(reserveMemForEnttsCount);
	componentHashes_.reserve(reserveMemForEnttsCount);

	// make pairs ['component_type' => 'component_name']
	componentTypeToName_ =
	{
		{ TransformComponent, "Transform" },
		{ NameComponent, "Name" },
		{ MoveComponent, "Movement" },
		{ ModelComponent, "Model" },
		{ RenderedComponent, "Rendered" },
		{ WorldMatrixComponent, "WorldMatrix" }
	};

	// add "invalid" entity with ID == 0
	ids_.push_back(INVALID_ENTITY_ID);
	componentHashes_.push_back(0);

	Log::Debug("entity mgr is initialized");
}

EntityMgr::~EntityMgr()
{
	Log::Debug();
}

///////////////////////////////////////////////////////////

void EntityMgr::SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList)
{ 
	// setup a file for writing log msgs into it;
	// also setup a list which will be filled with log messages;
	Log::Setup(pFile, pMsgsList); 
	Log::Debug("logger is setup successfully");
}


// ************************************************************************************
//                 PUBLIC SERIALIZATION / DESERIALIZATION API
// ************************************************************************************

bool EntityMgr::Serialize(const std::string& dataFilepath)
{
	std::ofstream fout;

	try
	{
		fout.open(dataFilepath, std::ios::binary);
		Assert::True(fout.is_open(), "can't open a file for serialization: " + dataFilepath);

		const int numComponents = ComponentType::LAST_COMPONENT_TYPE;
		EntityMgrSerializer serializer;
		DataHeader header(numComponents);

		// write empty data header at the beginning of the file
		serializer.WriteDataHeader(fout, header);

		// serialize data from the components
		transformSystem_.Serialize(fout, header.GetDataBlockPos(TransformComponent));
		moveSystem_.Serialize(fout, header.GetDataBlockPos(MoveComponent));
		nameSystem_.Serialize(fout, header.GetDataBlockPos(NameComponent));
		modelSystem_.Serialize(fout, header.GetDataBlockPos(ModelComponent));
		renderSystem_.Serialize(fout, header.GetDataBlockPos(RenderedComponent));

		// serialize entities ids, components hashes, etc.
		serializer.SerializeEnttMgrData(
			fout,
			ids_.data(),
			componentHashes_.data(),
			static_cast<u32>(std::ssize(ids_)),
			EntityMgr::ENTT_MGR_SERIALIZE_DATA_BLOCK_MARKER);

		// go at the beginning of the file and update the header
		// (write a start position of each component data block)
		serializer.WriteDataHeader(fout, header);

		Log::Debug("data from the ECS has been saved successfully into the file: " + dataFilepath);
	}
	catch (LIB_Exception& e)
	{
		fout.close();
		Log::Error(e, false);
		Log::Error("can't serialize data into a file: " + dataFilepath);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

bool EntityMgr::Deserialize(const std::string& dataFilepath)
{
	try
	{
		EntityMgrDeserializer deser;

		std::ifstream fin(dataFilepath, std::ios::binary);
		Assert::True(fin.is_open(), "can't open a file for deserialization: " + dataFilepath);

		u32 recordsCount = 0;

		// read in the number of header records
		fin >> recordsCount;

		// read in data header
		DataHeader header(recordsCount);
		deser.ReadDataHeader(fin, header);

		// print out header records ['data_block_marker' => 'data_block_pos']
		for (u32 i = 0; i < recordsCount; ++i)
		{
			const DataHeaderRecord& record = header.records_[i];
			Log::Print(
				std::to_string(record.dataBlockMarker)  
				+ " => " +
				std::to_string(record.dataBlockPos));
		}


		// deserialize EntityMgr data: entities IDs, component flags, etc.
		EntityID* idsTempBuff = nullptr;
		ComponentsHash* hashesTempBuff = nullptr;
		u32 idsCount = 0;

		deser.DeserializeEnttMgrData(
			fin, 
			&idsTempBuff,
			&hashesTempBuff,
			idsCount,
			EntityMgr::ENTT_MGR_SERIALIZE_DATA_BLOCK_MARKER);

		// insert deserialized data into the entity manager
		for (u32 i = 0; i < idsCount; ++i)
		{
			const EntityID enttID = idsTempBuff[i];
			const index insertAt = Utils::GetPosForID(ids_, enttID);

			// sort insertion of ID and HASH
			Utils::InsertAtPos(ids_, insertAt, enttID);
			Utils::InsertAtPos(componentHashes_, insertAt, hashesTempBuff[i]);
		}

		// deserialize components data
		transformSystem_.Deserialize(fin, header.GetDataBlockPos(TransformComponent));
		nameSystem_.Deserialize(fin, header.GetDataBlockPos(NameComponent));
		modelSystem_.Deserialize(fin, header.GetDataBlockPos(ModelComponent));
		moveSystem_.Deserialize(fin, header.GetDataBlockPos(MoveComponent));
		renderSystem_.Deserialize(fin, header.GetDataBlockPos(RenderedComponent));

		fin.close();
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, false);
		Log::Error("can't deserialize data from the file: " + dataFilepath);
		return false;
	}

	return true;
}



// ************************************************************************************
//                     PUBLIC CREATION/DESTROYMENT API
// ************************************************************************************

#pragma region PublicCreationDestroymentAPI

EntityID EntityMgr::CreateEntity()
{
	// create a new empty entity;
	// return: its ID

	EntityID id = lastEntityID_++;

	ids_.push_back(id);
	componentHashes_.push_back(0);

	return id;
}

///////////////////////////////////////////////////////////

std::vector<EntityID> EntityMgr::CreateEntities(const int newEnttsCount)
{
	// create batch of new empty entities, generate for each entity 
	// unique ID and set that it hasn't any component by default;
	//
	// return: SORTED array of IDs of just created entities;

	Assert::True(newEnttsCount > 0, "new entitites count cannot be <= 0");

	std::vector<EntityID> generatedIDs(newEnttsCount, INVALID_ENTITY_ID);
	
	// "generate" consistent IDs
	for (EntityID& id : generatedIDs)
		id = lastEntityID_++;

	// append ids and hashes of entities
	ids_.insert(ids_.end(), generatedIDs.begin(), generatedIDs.end());
	componentHashes_.insert(componentHashes_.end(), newEnttsCount, 0);

	return generatedIDs;
}

///////////////////////////////////////////////////////////

void EntityMgr::DestroyEntities(const std::vector<EntityID>& enttsIDs)
{
	assert("TODO: IMPLEMENT IT!" && 0);
}


#pragma endregion

// ************************************************************************************
//                          PUBLIC UPDATING FUNCTIONS
// ************************************************************************************

void EntityMgr::Update(const float totalGameTime, const float deltaTime)
{
	moveSystem_.UpdateAllMoves(deltaTime, transformSystem_);
	texTransformSystem_.UpdateAllTextrureAnimations(totalGameTime, deltaTime);
	lightSystem_.Update(deltaTime, totalGameTime);
}

// *********************************************************************************
// 
//                              ADD COMPONENTS 
// 
// *********************************************************************************

#pragma region AddComponentsAPI

void EntityMgr::GetEnttsByComponents(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<ComponentType> compTypes,
	std::vector<EntityID>& outFilteredEntts)
{
	ComponentsHash bitmask = 0;
	std::vector<ComponentsHash> hashes;

	// create a bitmask
	for (const ComponentType& type : compTypes)
		bitmask |= (1 << type);


	// get all the component flags of input entities
	GetComponentHashesByIDs(enttsIDs, hashes);

	outFilteredEntts.reserve(enttsIDs.size());

	// if the entity has such set of components we store this entity ID
	for (index idx = 0; idx < std::ssize(enttsIDs); ++idx)
	{
		if (bitmask == (hashes[idx] & bitmask))
			outFilteredEntts.push_back(enttsIDs[idx]);
	}

	outFilteredEntts.shrink_to_fit();
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttHasComponent(
	const EntityID id, 
	const ComponentType compType)
{
	// set that an entity by ID has such a component 
	const index idx = Utils::GetIdxInSortedArr(ids_, id);
	componentHashes_[idx] |= (1 << compType);
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponent(
	const EntityID* ids,
	const size numEntts,
	const ComponentType compType)
{
	// add the same component to each input entt
	
	std::vector<index> idxs;
	GetDataIdxsByIDs(ids, numEntts, idxs);
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponents(
	const std::vector<EntityID>& ids,
	const std::vector<ComponentType> compTypes)
{
	// add an arr of components to each input entt

	// get data idx of each input entt
	std::vector<index> idxs;
	GetDataIdxsByIDs(ids.data(), std::ssize(ids), idxs);

	// generate hash mask by input component types
	ComponentsHash hashMask = 0;
	for (const ComponentType type : compTypes)
		hashMask |= (1 << type);

	for (const index idx : idxs)
		componentHashes_[idx] |= hashMask;
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(
	const EntityID& enttID,
	const EntityName& enttName)
{
	// add the Name component to a single entity in terms of arrays
	AddNameComponent(
		std::vector<EntityID>{enttID},
		std::vector<EntityName>{enttName});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(
	const std::vector<EntityID>& ids,
	const std::vector<EntityName>& names)
{	
	// add the Name component to all the input entities
	// so each entity will have its own name
	try
	{

		Assert::NotEmpty(ids.empty(), "array of entities IDs is empty");
		Assert::True(CheckArrSizesEqual(ids , names), "count of entities IDs and names must be equal");

		nameSystem_.AddRecords(ids, names);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), NameComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add name component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTransformComponent(
	const EntityID& enttID,
	const XMFLOAT3& position,
	const XMVECTOR& dirQuat,
	const float uniformScale)
{
	// add the Transform component to a single entity in terms of arrays
	AddTransformComponent(
		std::vector<EntityID>{enttID},
		std::vector<XMFLOAT3>{position},
		std::vector<XMVECTOR>{dirQuat},
		std::vector<float>{uniformScale});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTransformComponent(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& positions,
	const std::vector<XMVECTOR>& dirQuats,
	const std::vector<float>& uniformScales)
{
	// add transform component to all the input entities

	try
	{
		using enum ComponentType;
	
		transformSystem_.AddRecords(ids, positions, dirQuats, uniformScales);
		SetEnttsHaveComponents(ids, { TransformComponent, WorldMatrixComponent });
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add transform component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddMoveComponent(
	const EntityID& enttID,
	const XMFLOAT3& translation,
	const XMVECTOR& rotationQuat,
	const float uniformScaleFactor)
{
	// add the Move component to a single entity in terms of arrays
	AddMoveComponent(
		std::vector<EntityID>{enttID},
		std::vector<XMFLOAT3>{translation},
		std::vector<XMVECTOR>{rotationQuat},
		std::vector<float>{uniformScaleFactor});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddMoveComponent(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& translations,
	const std::vector<XMVECTOR>& rotationQuats,
	const std::vector<float>& uniformScaleFactors)
{
	// add the Move component to all the input entities;
	// and setup entities movement using input data arrays

	try
	{
		moveSystem_.AddRecords(
			ids,
			translations,
			rotationQuats,
			uniformScaleFactors);

		SetEnttsHaveComponent(ids.data(), std::ssize(ids), MoveComponent);

	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add move component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddModelComponent(
	const EntityID enttID,
	const ModelID modelID)
{
	// add the Model component to a single entity by ID in terms of arrays;

	AddModelComponent(
		std::vector<EntityID>{enttID},
		std::vector<ModelID>{modelID});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddModelComponent(
	const std::vector<EntityID> enttsIDs,
	const ModelID modelID)
{
	// add the Model component to each input entity by ID in terms of arrays;
	// here we add the same model to each input entt

	AddModelComponent(enttsIDs, std::vector<ModelID>(enttsIDs.size(), modelID));
}

///////////////////////////////////////////////////////////

void EntityMgr::AddModelComponent(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<ModelID>& modelsIDs)
{
	// add ModelComponent to each entity by its ID; 
	// and bind to each input entity a model by respective idx (one to one)

	try
	{
		Assert::NotEmpty(enttsIDs.empty(), "the array of entities IDs is empty");
		Assert::NotEmpty(modelsIDs.empty(), "the array of models IDs is empty");

		modelSystem_.AddRecords(enttsIDs, modelsIDs);
		SetEnttsHaveComponent(enttsIDs.data(), std::ssize(enttsIDs), ModelComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add model component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(enttsIDs.data(), (int)enttsIDs.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
	const EntityID id,
	const ECS::RenderShaderType shaderType,
	const D3D11_PRIMITIVE_TOPOLOGY topologyType)
{
	AddRenderingComponent(
		std::vector<EntityID>{id},
		std::vector<ECS::RenderShaderType>{shaderType},
		std::vector<D3D11_PRIMITIVE_TOPOLOGY>{topologyType});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
	const std::vector<EntityID>& enttsIDs,
	const ECS::RenderShaderType renderShaderType,
	const D3D11_PRIMITIVE_TOPOLOGY topologyType)
{
	// add the Rendered component to each input entity by ID
	// and setup them with the same rendering params 

	AddRenderingComponent(
		enttsIDs,
		std::vector<ECS::RenderShaderType>(enttsIDs.size(), renderShaderType),
		std::vector<D3D11_PRIMITIVE_TOPOLOGY>(enttsIDs.size(), topologyType));
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
	const std::vector<EntityID>& ids,
	const std::vector<ECS::RenderShaderType>& shadersTypes,
	const std::vector<D3D11_PRIMITIVE_TOPOLOGY>& topologyTypes)
{
	// add RenderComponent to each entity by its ID; 
	// so these entities will be rendered onto the screen

	try
	{
		renderSystem_.AddRecords(ids, shadersTypes, topologyTypes);
		renderStatesSystem_.AddWithDefaultStates(ids);

		using enum ComponentType;
		SetEnttsHaveComponents(ids, { RenderedComponent, RenderStatesComponent });
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add rendering component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTexturedComponent(
	const EntityID enttID,
	const TexID* texIDs,     // set of tex ids for the entity submesh
	const size numTextures,  // expected to be 22 (the num of textures types)
	const int submeshID)     // to which submesh we bind these textures
{
	// add Textured component to each input entity by its ID
	// and set that this entity has own textures set 
	// (which is different from its mesh's textures set)

	try
	{
		Assert::True(CheckEnttExist(enttID), "no entity by ID: " + std::to_string(enttID));

		texturesSystem_.AddRecord(enttID, texIDs, numTextures, submeshID);
		SetEnttHasComponent(enttID, TexturedComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::stringstream ss;
		ss << "can't add textured component to entt: ";
		ss << enttID << "(" << nameSystem_.GetNameById(enttID) << ")";

		Log::Error(e);
		Log::Error(ss.str());
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTextureTransformComponent(
	const TexTransformType type,
	const EntityID id,
	const TexTransformInitParams& params)
{
	AddTextureTransformComponent(type, std::vector<EntityID>{id}, params);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTextureTransformComponent(
	const TexTransformType type,
	const std::vector<EntityID>& ids,
	const TexTransformInitParams& inParams)
{
	// set texture transformation of input type for each input entity by ID
	//
	// in:   type     -- what kind of texture transformation we want to apply?
	//       inParams -- struct of arrays of texture transformations params according to the input type

	try
	{
		texTransformSystem_.AddTexTransformation(type, ids, inParams);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), TextureTransformComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add texture transform component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddLightComponent(
	const std::vector<EntityID>& ids,
	DirLightsInitParams& params)
{
	try
	{
		Assert::NotEmpty(ids.empty(), "the array of entities IDs is empty");
		Assert::True(CheckEnttsExist(ids.data(), std::ssize(ids)), "there is no such entts in the mgr");

		lightSystem_.AddDirLights(ids, params);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), LightComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add Light component (directional lights) component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddLightComponent(
	const std::vector<EntityID>& ids,
	PointLightsInitParams& params)
{
	try
	{
		Assert::NotEmpty(ids.empty(), "the array of entities IDs is empty");
		Assert::True(CheckEnttsExist(ids.data(), std::ssize(ids)), "there is no such entts in the mgr");

		size numPointLights = std::ssize(ids);

		// dummy data for point light sources
		const std::vector<XMVECTOR> dirQuats(numPointLights, { 0,0,0,1 }); // no rotation
		const std::vector<float> uniformScales(numPointLights, 1.0f);

		lightSystem_.AddPointLights(ids, params);
		transformSystem_.AddRecords(ids, params.positions, dirQuats, uniformScales);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), LightComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add Light component (point lights) component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddLightComponent(
	const std::vector<EntityID>& ids,
	SpotLightsInitParams& params)
{
	try
	{
		Assert::NotEmpty(ids.empty(), "the array of entities IDs is empty");
		Assert::True(CheckEnttsExist(ids.data(), std::ssize(ids)), "there is no such entts in the mgr");

		lightSystem_.AddSpotLights(ids, params);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), LightComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add Light component (spot lights) component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderStatesComponent(const EntityID id)
{
	// add the RenderStates component to the input entt and setup it with default states
	AddRenderStatesComponent(std::vector<EntityID>{id});
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderStatesComponent(const std::vector<EntityID>& ids)
{
	// add the RenderStates component to the input entts and 
	// setup each with default states

	try
	{
		Assert::NotEmpty(ids.empty(), "the input arr of entities IDs is empty");
		Assert::True(CheckEnttsExist(ids.data(), std::ssize(ids)), "the entity mgr doesn't have an entity by some of input ids");

		renderStatesSystem_.AddWithDefaultStates(ids);
		SetEnttsHaveComponent(ids.data(), std::ssize(ids), RenderStatesComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add render states component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids.data(), (int)ids.size());

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddBoundingComponent(               
	const EntityID id,
	const BoundingType type,
	const DirectX::BoundingBox& aabb)
{
	// takes only one entt with only one subset (mesh)

	try
	{
		boundingSystem_.Add(id, type, aabb);
		SetEnttHasComponent(id, BoundingComponent);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't add bounding component to entts: " + std::to_string(id));
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddBoundingComponent(
	const EntityID* ids,
	const size numEntts,
	const size numSubsets,              // the number of entt's meshes (the num of AABBs)
	const BoundingType* types,          // AABB type per mesh
	const DirectX::BoundingBox* AABBs)  // AABB per mesh
{
	// apply the same set of AABBs to each input entity
	// (for instance: we have 100 the same trees so each will have the same set of AABBs)

	try
	{
		boundingSystem_.Add(ids, numEntts, numSubsets, types, AABBs);
		SetEnttsHaveComponent(ids, numEntts, BoundingComponent);

		//std::vector<DirectX::XMMATRIX> worlds;
		//transformSystem_.GetWorldMatricesOfEntts(ids, numEntts, worlds);
		//boundingSystem_.Update(ids, worlds.data(), numEntts, std::ssize(worlds));
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		std::string errMsg;
		errMsg += "can't add bounding component to entts: ";
		errMsg += Utils::GetEnttsIDsAsString(ids, (int)numEntts);

		Log::Error(e);
		Log::Error(errMsg);
	}
}

///////////////////////////////////////////////////////////

void EntityMgr::AddCameraComponent(
	const EntityID id,
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& proj)
{
	// add a camera component to the entity by input ID

	try
	{
		cameraSystem_.AddRecord(id, view, proj);
		SetEnttHasComponent(id, CameraComponent);
	}
	catch (const std::out_of_range& e)
	{
		throw LIB_Exception(e.what());
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't add the camera component to entt: " + std::to_string(id));
	}
}

#pragma endregion




// ====================================================================================
//                            PUBLIC QUERY API
// ====================================================================================

#pragma region PublicQueryAPI

bool EntityMgr::CheckEnttExist(const EntityID id)
{
	// check if entity by id exists in the manager;
	return std::binary_search(ids_.begin(), ids_.end(), id);
}

///////////////////////////////////////////////////////////

bool EntityMgr::CheckEnttsExist(const EntityID* ids, const size numEntts)
{
	// check by ID if each entity from the input array is created;
	// return: true  -- if all the entities from the input arr exists in the manager
	//         false -- if some entity from the input arr doesn't exist

	bool allExist = true;
	const auto beg = ids_.begin();
	const auto end = ids_.end();

	for (index i = 0; i < numEntts; ++i)
		allExist &= std::binary_search(beg, end, ids[i]);
	
	return allExist;
}

///////////////////////////////////////////////////////////

void EntityMgr::CheckEnttsHaveComponents(
	const EntityID* ids,
	const size numEntts,
	const std::vector<ComponentType>& componentsTypes,
	std::vector<bool>& outHasComponent)
{
	// out: arr of boolean flags which defines if input entt has such set of components

	Assert::True(ids && (numEntts > 0) && !componentsTypes.empty(), "wrong input data");

	
	// get data idxs of each input entt
	std::vector<index> idxs;
	GetDataIdxsByIDs(ids, numEntts, idxs);

	const u32 bitmask = GetHashByComponents(componentsTypes);
	outHasComponent.resize(numEntts);

	for (int i = 0; const index hashIdx : idxs)
		outHasComponent[i++] = (bitmask == (componentHashes_[hashIdx] & bitmask));
}

///////////////////////////////////////////////////////////

void EntityMgr::GetComponentHashesByIDs(
	const std::vector<EntityID>& ids,
	std::vector<ComponentsHash>& outHashes)
{
	// get component hashes (bitmasks) of entities by ID

	// get data idxs of each input entt
	std::vector<index> idxs;
	GetDataIdxsByIDs(ids.data(), std::ssize(ids), idxs);

	// get components hashes by idxs
	outHashes.resize(ids.size());

	for (int i = 0; const index idx : idxs)
		outHashes[i++] = componentHashes_[idx];
}

///////////////////////////////////////////////////////////

void EntityMgr::GetEnttsByComponent(
	const ComponentType componentType,
	std::vector<EntityID>& outIDs)
{
	// get IDs of entities which have such component;

	switch (componentType)
	{
		case ComponentType::TransformComponent:
		{
			outIDs = transform_.ids_;
			return;
		}
		case ComponentType::WorldMatrixComponent:
		{
			transformSystem_.GetAllEnttsIDsFromWorldMatrixComponent(outIDs);
			return;
		}
		case ComponentType::MoveComponent:
		{
			outIDs = movement_.ids_;
			return;
		}
		case ComponentType::ModelComponent:
		{
			const size enttsCount = std::ssize(modelComponent_.enttToModel_);
			outIDs.resize(enttsCount);

			for (int i = 0; const auto& it : modelComponent_.enttToModel_)
			{
				const EntityID id = it.first;
				outIDs[i++] = id;
			}
			return;
		}
		case ComponentType::RenderedComponent:
		{
			outIDs = renderComponent_.ids_;
			return;
		}
		default:
		{
			Log::Error("Unknown component type: " + std::to_string(componentType));
			throw LIB_Exception("can't get IDs of entities which have such component: " + std::to_string(componentType));
		}
	}
}

#pragma endregion




// ************************************************************************************
// 
//                               PRIVATE HELPERS
// 
// ************************************************************************************


void EntityMgr::GetDataIdxsByIDs(
	const EntityID* ids,
	const size numEntts,
	std::vector<index>& outDataIdxs)
{
	// in:  array of entities IDs
	// out: array of data idxs of these entts

	// check if these entities exist
	Assert::True(CheckEnttsExist(ids, numEntts), "there is no entity by some input ID");

	// get data idx into array for each ID
	Utils::GetIdxsInSortedArr(ids_, ids, numEntts, outDataIdxs);
}

///////////////////////////////////////////////////////////

bool EntityMgr::CheckEnttsByDataIdxsHaveComponent(
	const std::vector<index>& enttsDataIdxs,
	const ComponentType componentType)
{
	// check if all the input entts by data idxs have this particular component

	const u32 bitmask = GetHashByComponent(componentType);
	bool haveComponent = true;

	for (const index idx : enttsDataIdxs)
		haveComponent &= (bool)(componentHashes_[idx] & bitmask);

	return haveComponent;
}

///////////////////////////////////////////////////////////

ComponentsHash EntityMgr::GetHashByComponent(const ComponentType component)
{
	u32 bitmask = 0;
	return (bitmask |= (1 << component));
}

///////////////////////////////////////////////////////////

ComponentsHash EntityMgr::GetHashByComponents(
	const std::vector<ComponentType>& components)
{
	// generate and return a hash by input components

	u32 bitmask = 0;
	
	for (const ComponentType comp : components)
		bitmask |= (1 << comp);

	return bitmask;
}

}