// **********************************************************************************
// Filename:     EntityMgr.h
// Description:
// 
// Created:
// **********************************************************************************

#pragma once


#include "../Common/Types.h"
#include "../Common/log.h"

// components (ECS)
#include "../Components/Transform.h"
#include "../Components/WorldMatrix.h"
#include "../Components/Movement.h"
#include "../Components/Model.h"
#include "../Components/Rendered.h"
#include "../Components/Name.h"
#include "../Components/Textured.h"          // if entity has the Textured component it means that this entt has own textures set which is different from the meshes textures
#include "../Components/TextureTransform.h"
#include "../Components/Light.h"
#include "../Components/RenderStates.h"
#include "../Components/Bounding.h"
#include "../Components/Camera.h"

// systems (ECS)
#include "../Systems/TransformSystem.h"
#include "../Systems/MoveSystem.h"
#include "../Systems/ModelSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/NameSystem.h"
#include "../Systems/TexturesSystem.h"
#include "../Systems/TextureTransformSystem.h"
#include "../Systems/LightSystem.h"
#include "../Systems/RenderStatesSystem.h"
#include "../Systems/BoundingSystem.h"
#include "../Systems/CameraSystem.h"

#include <set> 

namespace ECS
{

class EntityMgr final
{
public:
	EntityMgr();
	~EntityMgr();

	// restrict any copying of instances of this class
	EntityMgr(const EntityMgr&) = delete;
	EntityMgr(EntityMgr&&) = delete;
	EntityMgr& operator=(const EntityMgr&) = delete;
	EntityMgr& operator=(EntityMgr&&) = delete;

	void SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList);

	// public serialization / deserialization API
	bool Serialize(const std::string& dataFilepath);
	bool Deserialize(const std::string& dataFilepath);

	// public creation/destroyment API
	std::vector<EntityID> CreateEntities(const int newEnttsCount);
	void DestroyEntities(const std::vector<EntityID>& enttsIDs);

	EntityID CreateEntity();
	//void DestroyEntity(const EntityName& enttName);

	void Update(const float totalGameTime, const float deltaTime);


	// ------------------------------------------------------------------------
	// add TRANSFORM component API

	void AddTransformComponent(
		const EntityID& enttID,
		const XMFLOAT3& position = { 0,0,0 },
		const XMVECTOR& dirQuat = { 0,0,0,1 },  // no rotation by default
		const float uniformScale = 1.0f);

	void AddTransformComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT3>& positions,
		const std::vector<XMVECTOR>& dirQuats,
		const std::vector<float>& uniformScales);

	// ------------------------------------
	// add RENDERED component API

	void AddMoveComponent(
		const EntityID& enttID,
		const XMFLOAT3& translation,
		const XMVECTOR& rotationQuat,
		const float uniformScaleFactor);

	void AddMoveComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT3>& translations,
		const std::vector<XMVECTOR>& rotationQuats,
		const std::vector<float>& uniformScaleFactors);

	// ------------------------------------
	// add NAME component API

	void AddNameComponent(
		const EntityID& enttID,
		const EntityName& enttName);

	void AddNameComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<EntityName>& enttsNames);

	// ------------------------------------
	// add MODEL component API

	void AddModelComponent(
		const EntityID enttID,
		const ModelID modelID);

	void AddModelComponent(
		const std::vector<EntityID> enttsIDs,
		const ModelID modelID);

	void AddModelComponent(
		const std::vector<EntityID>& enttID,
		const std::vector<ModelID>& modelsIDs);

	// ------------------------------------
	// add RENDERED component API

	void AddRenderingComponent(
		const EntityID id,
		const ECS::RenderShaderType shaderType = RenderShaderType::LIGHT_SHADER,
		const D3D11_PRIMITIVE_TOPOLOGY topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void AddRenderingComponent(
		const std::vector<EntityID>& enttsIDs,
		const RenderShaderType renderShaderType = RenderShaderType::LIGHT_SHADER,
		const D3D11_PRIMITIVE_TOPOLOGY topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void AddRenderingComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<RenderShaderType>& renderShadersTypes,
		const std::vector<D3D11_PRIMITIVE_TOPOLOGY>& topologyTypes);

	// ------------------------------------
	// add TEXTURED component API

	void AddTexturedComponent(
		const EntityID enttID,
		const TexID* texIDs,
		const size numTextures,
		const int submeshID);

	// ------------------------------------
	// add TEXTURE TRANSFORM component API
	
	void AddTextureTransformComponent(
		const TexTransformType type,
		const EntityID enttID,
		const TexTransformInitParams& params);

	void AddTextureTransformComponent(
		const TexTransformType type,
		const std::vector<EntityID>& ids,
		const TexTransformInitParams& params);

	// ------------------------------------
	// add LIGHT component API

	void AddLightComponent(const std::vector<EntityID>& ids, DirLightsInitParams& params);
	void AddLightComponent(const std::vector<EntityID>& ids, PointLightsInitParams& params);
	void AddLightComponent(const std::vector<EntityID>& ids, SpotLightsInitParams& params);

	// ------------------------------------
	// add RENDER STATES component API

	void AddRenderStatesComponent(const EntityID id);
	void AddRenderStatesComponent(const std::vector<EntityID>& ids);

	// ------------------------------------
	// add BOUNDING component API

	void AddBoundingComponent(               // takes only one entt with only one subset (mesh)
		const EntityID id,
		const BoundingType type,
		const DirectX::BoundingBox& aabb);

#if 0
	void AddBoundingComponent(
		const EntityID id,
		const size numSubsets,               // the number of submeshes of this entity
		const BoundingType* types,           // AABB type per mesh
		const DirectX::BoundingBox* AABBs);  // AABB per mesh
#endif


	void AddBoundingComponent(
		const EntityID* ids,
		const size numEntts,
		const size numSubsets,               // the number of entt's meshes (the num of AABBs)
		const BoundingType* types,           // AABB type per mesh
		const DirectX::BoundingBox* AABBs);  // AABB per mesh

	// ------------------------------------
	// add CAMERA component

	void AddCameraComponent(
		const EntityID id,
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& proj);


	// ------------------------------------
	// components SETTERS API

	void SetEnttHasComponent(
		const EntityID id,
		const ComponentType compType);

	void SetEnttsHaveComponent(
		const EntityID* ids,
		const size numEntts,
		const ComponentType compType);

	void SetEnttsHaveComponents(
		const std::vector<EntityID>& ids,
		const std::vector<ComponentType> compTypes);

	// ---------------------------------------------------------------------------
	// public QUERY API

	inline const Transform& GetComponentTransform() const { return transform_; }
	inline const Movement& GetComponentMovement()   const { return movement_; }
	inline const WorldMatrix& GetComponentWorld()   const { return world_; }
	inline const Model& GetComponentModel()         const { return modelComponent_; }
	inline const Name& GetComponentName()           const { return names_; }
	inline const Rendered& GetComponentRendered()   const { return renderComponent_; }
	inline const Textured& GetComponentTextured()   const { return textureComponent_; }
	inline const TextureTransform& GetComponentTexTransform() const { return texTransform_; }
	inline const Light& GetComponentLight()         const { return light_; }
	inline const Bounding& GetComponentBounding()   const { return bounding_; }

	inline const std::map<ComponentType, ComponentID>& GetMapCompTypeToName() {	return componentTypeToName_; }
	inline const std::vector<EntityID>& GetAllEnttsIDs() const { return ids_; }

	void CheckEnttsHaveComponents(
		const EntityID* ids,
		const size numEntts,
		const std::vector<ComponentType>& componentsTypes,
		std::vector<bool>& outHasComponent);

	void GetComponentHashesByIDs(
		const std::vector<EntityID>& ids,
		std::vector<ComponentsHash>& componentFlags);

	void GetEnttsByComponents(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<ComponentType> compTypes,
		std::vector<EntityID>& outFilteredEntts);

	void GetEnttsByComponent(
		const ComponentType componentType,
		std::vector<EntityID>& outIDs);

	bool CheckEnttExist(const EntityID id);
	bool CheckEnttsExist(const EntityID* ids, const size numEntts);

	inline WorldMatrix& GetWorldComponent() { return world_; }

	ComponentsHash GetHashByComponent(const ComponentType component);
	ComponentsHash GetHashByComponents(const std::vector<ComponentType>& components);

private:

	void GetDataIdxsByIDs(
		const EntityID* ids,
		const size numEntts,
		std::vector<index>& outDataIdxs);

	bool CheckEnttsByDataIdxsHaveComponent(
		const std::vector<index>& enttsDataIdxs,
		const ComponentType componentType);


public:
	static const u32 ENTT_MGR_SERIALIZE_DATA_BLOCK_MARKER = 1000;


	// SYSTEMS
	LightSystem            lightSystem_;
	NameSystem             nameSystem_;
	TransformSystem        transformSystem_;
	MoveSystem             moveSystem_;
	ModelSystem            modelSystem_;
	RenderSystem           renderSystem_;
	TexturesSystem         texturesSystem_;
	TextureTransformSystem texTransformSystem_;
	RenderStatesSystem     renderStatesSystem_;
	BoundingSystem         boundingSystem_;
	CameraSystem           cameraSystem_;
	

	// "ID" of an entity is just a numeral index
	std::vector<EntityID> ids_;

	// bit flags for every component, indicating whether this object "has it"
	std::vector<ComponentsHash> componentHashes_;

	// pairs ['component_type' => 'component_name']
	std::map<ComponentType, ComponentID> componentTypeToName_;  

private:

	static int       lastEntityID_;

	// COMPONENTS
	Transform        transform_;
	Movement         movement_;
	Model            modelComponent_;
	WorldMatrix      world_;
	Rendered         renderComponent_;
	Textured         textureComponent_;
	Name             names_;
	TextureTransform texTransform_;
	Light            light_;
	RenderStates     renderStates_;
	Bounding         bounding_;
	Camera           camera_;
};

};