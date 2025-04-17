// **********************************************************************************
// Filename:     EntityMgr.h
// Description:
// 
// Created:
// **********************************************************************************

#pragma once

// components (ECS)
#include "../Components/Transform.h"
#include "../Components/Movement.h"
#include "../Components/Model.h"
#include "../Components/Rendered.h"
#include "../Components/Name.h"
#include "../Components/Material.h"   
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
#include "../Systems/MaterialSystem.h"
#include "../Systems/TextureTransformSystem.h"
#include "../Systems/LightSystem.h"
#include "../Systems/RenderStatesSystem.h"
#include "../Systems/BoundingSystem.h"
#include "../Systems/CameraSystem.h"


namespace ECS
{

class EntityMgr
{
public:
    EntityMgr();
    ~EntityMgr();

    // restrict any copying of instances of this class
    EntityMgr(const EntityMgr&) = delete;
    EntityMgr(EntityMgr&&) = delete;
    EntityMgr& operator=(const EntityMgr&) = delete;
    EntityMgr& operator=(EntityMgr&&) = delete;

    void SetupLogger(FILE* pFile);

    // public serialization / deserialization API
    bool Serialize(const std::string& dataFilepath);
    bool Deserialize(const std::string& dataFilepath);

    // public creation/destroyment API
    cvector<EntityID> CreateEntities(const int newEnttsCount);
    void DestroyEntities(const EntityID* ids, const size numEntts);

    EntityID CreateEntity();
    //void DestroyEntity(const EntityName& enttName);

    void Update(const float totalGameTime, const float deltaTime);


    // =============================================================================
    // PUBLIC METHODS: ADD COMPONENTS 
    // =============================================================================

    // add TRANSFORM component
    void AddTransformComponent(
        const EntityID& enttID,
        const XMFLOAT3& position = { 0,0,0 },
        const XMVECTOR& dirQuat = { 0,0,0,1 },  // no rotation by default
        const float uniformScale = 1.0f);

    void AddTransformComponent(
        const EntityID* ids,
        const size numEntts,
        const XMFLOAT3* positions,
        const XMVECTOR* dirQuats,
        const float* uniformScales);


    // add MOVEMENT component
    void AddMoveComponent(
        const EntityID& enttID,
        const XMFLOAT3& translation,
        const XMVECTOR& rotationQuat,
        const float uniformScaleFactor);

    void AddMoveComponent(
        const EntityID* ids,
        const XMFLOAT3* translations,
        const XMVECTOR* rotationQuats,
        const float* uniformScaleFactors,
        const size numEntts);

  
    // add NAME component
    void AddNameComponent(
        const EntityID& id,
        const EntityName& name);

    void AddNameComponent(
        const EntityID* ids,
        const EntityName* names,
        const size numEntts);

    
    // add MODEL component
    void AddModelComponent(
        const EntityID enttID,
        const ModelID modelID);

    void AddModelComponent(
        const EntityID* enttsIDs,
        const ModelID modelID,
        const size numEntts);

    void AddModelComponent(
        const EntityID* enttsIDs,
        const ModelID* modelsIDs,
        const size numEntts);


    // add RENDER component
    void AddRenderingComponent(const EntityID id, const RenderInitParams& params);

    void AddRenderingComponent(
        const EntityID* ids,
        const size numEntts,
        const RenderInitParams& params);

    void AddRenderingComponent(
        const EntityID* ids,
        const size numEntts,
        const RenderInitParams* params);

  
    // add MATERIAL component
    void AddMaterialComponent(
        const EntityID enttID,
        const MaterialID* materialsIDs,
        const size numSubmeshes,
        const bool areMaterialsMeshBased);


    // add TEXTURE TRANSFORM component
    void AddTextureTransformComponent(
        const EntityID enttID,
        const TexTransformType type,
        const TexTransformInitParams& params);

    void AddTextureTransformComponent(
        const EntityID* ids,
        const size numEntts,
        const TexTransformType type,
        const TexTransformInitParams& params);


    // add LIGHT component
    void AddLightComponent(const EntityID* ids, const size numEntts, DirLightsInitParams& params);
    void AddLightComponent(const EntityID* ids, const size numEntts, PointLightsInitParams& params);
    void AddLightComponent(const EntityID* ids, const size numEntts, SpotLightsInitParams& params);


    // add RENDER STATES component
    void AddRenderStatesComponent(const EntityID id);
    void AddRenderStatesComponent(const EntityID* ids, const size numEntts);


    // add BOUNDING component
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

    void AddBoundingComponent(
        const EntityID* ids,
        const DirectX::BoundingSphere* boundingSpheres,
        const size numEntts);


    // add CAMERA component
    void AddCameraComponent(
        const EntityID id,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj);


    // =============================================================================
    // public API: QUERY
    // =============================================================================
    inline const Transform&         GetComponentTransform()     const { return transform_; }
    inline const Movement&          GetComponentMovement()      const { return movement_; }
    inline const Model&             GetComponentModel()         const { return modelComponent_; }
    inline const Name&              GetComponentName()          const { return names_; }
    inline const Rendered&          GetComponentRendered()      const { return renderComponent_; }
    inline const Material&          GetComponentMaterial()      const { return materialComponent_;; }
    inline const TextureTransform&  GetComponentTexTransform()  const { return texTransform_; }
    inline const Light&             GetComponentLight()         const { return light_; }
    inline const Bounding&          GetComponentBounding()      const { return bounding_; }

    inline const std::map<eComponentType, ComponentName> GetMapCompTypeToName() { return componentTypeToName_; }

    inline const size      GetNumAllEntts() const { return ids_.size(); }
    inline const EntityID* GetAllEnttsIDs() const { return ids_.data(); }

    bool GetComponentNamesByEntt(const EntityID id, cvector<std::string>& names) const;
    bool GetComponentTypesByEntt(const EntityID id, cvector<uint8_t>& types)     const;

    inline bool CheckEnttExist(const EntityID id)                         const { return ids_.binary_search(id); }
    inline bool CheckEnttsExist(const EntityID* ids, const size numEntts) const { return ids_.binary_search(ids, numEntts); }

private:
    ComponentHash GetHashByComponent(const eComponentType component);
 
    // common setters: components
    void SetEnttHasComponent(
        const EntityID id,
        const eComponentType compType);

    void SetEnttsHaveComponent(
        const EntityID* ids,
        const size numEntts,
        const eComponentType compType);

    void SetEnttsHaveComponents(
        const EntityID* ids,
        const cvector<eComponentType>& compTypes,
        const size numEntts);



public:
    static const u32 ENTT_MGR_SERIALIZE_DATA_BLOCK_MARKER = 1000;

    // SYSTEMS
    LightSystem            lightSystem_;
    NameSystem             nameSystem_;
    TransformSystem        transformSystem_;
    MoveSystem             moveSystem_;
    ModelSystem            modelSystem_;
    RenderSystem           renderSystem_;
    MaterialSystem         materialSystem_;
    TextureTransformSystem texTransformSystem_;
    RenderStatesSystem     renderStatesSystem_;
    BoundingSystem         boundingSystem_;
    CameraSystem           cameraSystem_;
    

    // "ID" of an entity is just a numeral index
    cvector<EntityID> ids_;

    // bit flags for every component, indicating whether this object "has it"
    cvector<ComponentHash> componentHashes_;

    std::map<eComponentType, ComponentName> componentTypeToName_;  


private:

    static int       lastEntityID_;

    // COMPONENTS
    Transform        transform_;
    Movement         movement_;
    Model            modelComponent_;
    Rendered         renderComponent_;
    Material         materialComponent_;
    Name             names_;
    TextureTransform texTransform_;
    Light            light_;
    RenderStates     renderStates_;
    Bounding         bounding_;
    Camera           camera_;
};

};
