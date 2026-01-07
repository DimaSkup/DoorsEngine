/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: EntityMgr.cpp

    Desc:     Entity-Component-System (ECS) main manager

\**********************************************************************************/
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
#include "../Components/Bounding.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Hierarchy.h"
#include "../Components/Inventory.h"
#include "../Components/animation.h"
#include "../Components/Sprite.h"

// systems (ECS)
#include "../Systems/TransformSystem.h"
#include "../Systems/MoveSystem.h"
#include "../Systems/ModelSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/NameSystem.h"
#include "../Systems/MaterialSystem.h"
#include "../Systems/TextureTransformSystem.h"
#include "../Systems/LightSystem.h"
#include "../Systems/BoundingSystem.h"
#include "../Systems/CameraSystem.h"
#include "../Systems/PlayerSystem.h"
#include "../Systems/HierarchySystem.h"
#include "../Systems/InventorySystem.h"
#include "../Systems/ParticleSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/SpriteSystem.h"

// events (ECS)
#include "../Events/IEvent.h"


namespace ECS
{

//---------------------------------------------------------
// some internal constants
//---------------------------------------------------------
constexpr int MAX_NUM_EVENTS = 64;


//---------------------------------------------------------
// Class name:  EntityMgr
//---------------------------------------------------------
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


    bool                Serialize(const std::string& dataFilepath);
    bool                Deserialize(const std::string& dataFilepath);

    // public creation/destroyment API
    cvector<EntityID>   CreateEntities(const int newEnttsCount);
    void                DestroyEntities(const EntityID* ids, const size numEntts);

    EntityID            CreateEntity();
    EntityID            CreateEntity(const char* enttName);
    //void DestroyEntity(const EntityName& enttName);

    void                Update(const float totalGameTime, const float deltaTime);
    void                PushEvent(const Event& e);

    void                RemoveComponent(const EntityID id, eComponentType component);


    // =============================================================================
    // PUBLIC METHODS: ADD COMPONENTS 
    // =============================================================================

    // add TRANSFORM component
    void AddTransformComponent(
        const EntityID& enttID,
        const XMFLOAT3& position = { 0,0,0 },
        const XMVECTOR& direction = { 0,0,0,1 },  // no rotation by default
        const float uniformScale = 1.0f);

    void AddTransformComponent(
        const EntityID* ids,
        const size numEntts,
        const XMFLOAT3* positions,
        const XMVECTOR* directions,
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
    bool AddNameComponent(const EntityID& id, const char* name);
    void AddNameComponent(const EntityID* ids, const std::string* names, const size numEntts);

    
    // add MODEL component
    void AddModelComponent(
        const EntityID enttID,
        const ModelID modelID);

    void AddModelComponent(
        const EntityID* enttsIDs,
        const ModelID modelID,
        const size numEntts);

    // add RENDER component
    void AddRenderingComponent(const EntityID id);
    void AddRenderingComponent(const EntityID* ids, const size numEntts);
  
    // add MATERIAL component
    void AddMaterialComponent(
        const EntityID enttId,
        const MaterialID matId);

    void AddMaterialComponent(
        const EntityID enttID,
        const MaterialID* materialsIDs,
        const size numSubmeshes);


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
    void AddLightComponent(const EntityID id, const DirLight& initData);
    void AddLightComponent(const EntityID id, const PointLight& initData);
    void AddLightComponent(const EntityID id, const SpotLight& initData);

    // add BOUNDING component
    void AddBoundingComponent(const EntityID id, const DirectX::BoundingSphere& sphere);
    void AddBoundingComponent(const EntityID id, const DirectX::BoundingBox& aabb);

    void AddBoundingComponent(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingBox& aabb);

    void AddCameraComponent         (const EntityID id, const CameraData& data);
    void AddPlayerComponent         (const EntityID id);
    void AddParticleEmitterComponent(const EntityID id);
    void AddInventoryComponent      (const EntityID id);

    void AddAnimationComponent(
        const EntityID enttId,
        const SkeletonID skeletonId,
        const AnimationID animId,
        const float animEndTime);

    void AddSpriteComponent(
        const EntityID enttId,
        const TexID spriteTexId,
        const uint16 leftPos,
        const uint16 topPos,
        const uint16 width,
        const uint16 height);


    // =============================================================================
    // public API: QUERY
    // =============================================================================
    inline const Transform&         GetComponentTransform()     const { return transform_; }
    inline const Movement&          GetComponentMovement()      const { return movement_; }
    inline const Model&             GetComponentModel()         const { return modelComp_; }
    inline const Name&              GetComponentName()          const { return names_; }
    inline const Rendered&          GetComponentRendered()      const { return renderComp_; }
    inline const Material&          GetComponentMaterial()      const { return materials_;; }
    inline const TextureTransform&  GetComponentTexTransform()  const { return texTransform_; }
    inline const Light&             GetComponentLight()         const { return light_; }
    inline const Bounding&          GetComponentBounding()      const { return bounding_; }

    inline const std::map<eComponentType, std::string>& GetMapCompTypeToName() { return componentTypeToName_; }

    inline const size               GetNumAllEntts()            const { return ids_.size(); }
    inline const cvector<EntityID>& GetAllEnttsIDs()            const { return ids_; }

    bool                            GetComponentNamesByEntt(const EntityID id, cvector<std::string>& names) const;
    bool                            GetComponentTypesByEntt(const EntityID id, cvector<uint8_t>& types)     const;

    inline bool                     CheckEnttExist (const EntityID id)                        const { return ids_.binary_search(id); }
    inline bool                     CheckEnttsExist(const EntityID* ids, const size numEntts) const { return ids_.binary_search(ids, numEntts); }

private:
    ComponentBitfield GetHashByComponent(const eComponentType component);
 
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

    // SYSTEMS
    LightSystem             lightSys_;
    NameSystem              nameSys_;
    TransformSystem         transformSys_;
    MoveSystem              moveSys_;
    ModelSystem             modelSys_;
    RenderSystem            renderSys_;
    MaterialSystem          materialSys_;
    TextureTransformSystem  texTransformSys_;
    BoundingSystem          boundingSys_;
    CameraSystem            cameraSys_;
    PlayerSystem            playerSys_;
    HierarchySystem         hierarchySys_;
    ParticleSystem          particleSys_;
    InventorySystem         inventorySys_;
    AnimationSystem         animationSys_;
    SpriteSystem            spriteSys_;
    
    // "ID" of an entity is just a numeral index
    cvector<EntityID> ids_;

    // bit flags for every component, indicating whether this object "has it"
    cvector<ComponentBitfield> componentHashes_;

    std::map<eComponentType, std::string> componentTypeToName_;  

private:
    Event eventsList_[MAX_NUM_EVENTS];
    int currNumEvents_ = 0;

    static int       lastEntityID_;

    // COMPONENTS
    Transform        transform_;
    Movement         movement_;
    Model            modelComp_;
    Rendered         renderComp_;
    Material         materials_;
    Name             names_;
    TextureTransform texTransform_;
    Light            light_;
    Bounding         bounding_;
    Camera           camera_;
    Hierarchy        hierarchy_;
    Inventory        inventory_;
    Animations       animations_;
    Sprite           sprites_;
};

};
