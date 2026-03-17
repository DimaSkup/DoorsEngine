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

#include <bit_flags.h>

// components (ECS)
#include "../Components/Transform.h"
#include "../Components/Movement.h"
#include "../Components/Model.h"
#include "../Components/Rendered.h"
#include "../Components/Name.h"
#include "../Components/Material.h"   
#include "../Components/Light.h"
#include "../Components/Bounding.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Hierarchy.h"
#include "../Components/Inventory.h"
#include "../Components/ParticleEmitter.h"
#include "../Components/animation.h"
#include "../Components/Sprite.h"

// systems (ECS)
#include "../Systems/TransformSystem.h"
#include "../Systems/MoveSystem.h"
#include "../Systems/ModelSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/NameSystem.h"
#include "../Systems/MaterialSystem.h"
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

// quad tree related stuff
#include "../QuadTree/quad_tree.h"
#include "../QuadTree/scene_object.h"


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
    EntityMgr(const EntityMgr&)             = delete;
    EntityMgr(EntityMgr&&)                  = delete;
    EntityMgr& operator=(const EntityMgr&)  = delete;
    EntityMgr& operator=(EntityMgr&&)       = delete;


    // public creation/destroyment API
    cvector<EntityID>   CreateEntities(const int newEnttsCount);
    void                DestroyEntities(const EntityID* ids, const size numEntts);

    EntityID            CreateEntity();
    EntityID            CreateEntity(const char* enttName);

    void                Update(const float gameTime, const float dt);
    void                PushEvent(const Event& e);

    void                RemoveComponent(const EntityID id, eComponentType component);

    // quad tree functions...
    size                GetNumSceneObjects() const;
    ::QuadTree&         GetQuadTree();
    void                AttachEnttToQuadTree(const EntityID id);
    void                AttachEnttsToQuadTree(const EntityID* ids, const size count);
    void                UpdateQuadTreeMembership(const EntityID id);
    void                UpdateQuadTreeMembership(const EntityID* ids, const size count);

    // debugging functions...
    void DumpEntts()        const;
    void DumpSceneObjects() const;

    // =============================================================================
    // PUBLIC METHODS: ADD COMPONENTS 
    // =============================================================================

    // add TRANSFORM component
    void AddTransformComponent(
        const EntityID& enttID,
        const XMFLOAT3& position = { 0,0,0 },
        const XMVECTOR& direction = { 0,0,1,1 },
        const float scale = 1.0f);

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
    void AddMaterialComponent(const EntityID enttId, const MaterialID matId);

    void AddMaterialComponent(
        const EntityID* enttsIds,
        const size numEntts,
        const MaterialID matId);

    void AddMaterialComponent(
        const EntityID enttId,
        const MaterialID* materialsIds,
        const size numMeshes);

    // add LIGHT component
    void AddLightComponent(const EntityID id, const DirLight& initData);
    void AddLightComponent(const EntityID id, const PointLight& initData);
    void AddLightComponent(const EntityID id, const SpotLight& initData);

    // add BOUNDING component
    void AddBoundingComponent(
        const EntityID id,
        const DirectX::BoundingSphere& localSphere,
        const DirectX::BoundingSphere& worldSphere);

    void AddBoundingComponent(
        const EntityID id,
        const DirectX::BoundingBox& localBox,
        const DirectX::BoundingBox& worldBox);

    void AddBoundingComponent(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingBox& localBox,
        const DirectX::BoundingBox* worldBoxes);

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
    const char*              GetComponentName(const eComponentType comp) const;

    const size               GetNumAllEntts(void) const;
    const cvector<EntityID>& GetAllEnttsIDs(void) const;

    bool                     CheckEnttExist (const EntityID id)                        const;
    bool                     CheckEnttsExist(const EntityID* ids, const size numEntts) const;

    // get entity's components info
    bool     GetAddedComponentsByEntt(const EntityID id, ECS::eComponentType* outArr, size& outNumComponents) const;
    u32Flags GetAddedComponentsByEntt(const EntityID id) const;


private:
    index GetEnttIdx(const EntityID id) const;

    void CreateQuadTreeObjects(const EntityID* ids, const size numEntts);

    void SetEnttHasComponent(const EntityID id, const eComponentType comp);

    void SetEnttsHaveComponent(
        const EntityID* ids,
        const size numEntts,
        const eComponentType compType);

public:

    // public data...
    QuadTree                quadTree_;

    cvector<EntityID>       sceneObjectsIds_;
    cvector<SceneObject>    sceneObjects_;

    // systems...
    LightSystem             lightSys_;
    NameSystem              nameSys_;
    TransformSystem         transformSys_;
    MoveSystem              moveSys_;
    ModelSystem             modelSys_;
    RenderSystem            renderSys_;
    MaterialSystem          materialSys_;
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
    cvector<u32Flags> componentFlags_;

    // pairs [component_type => component_name]
    std::map<eComponentType, std::string> componentNames_;  

private:

    // private data...
    Event       eventsList_[MAX_NUM_EVENTS];
    int         currNumEvents_ = 0;

    static int  lastEntityID_;

    // components
    Transform       transform_;
    Movement        movement_;
    Model           modelComp_;
    Rendered        renderComp_;
    Material        materials_;
    Name            names_;
    Light           light_;
    Bounding        bounding_;
    Camera          camera_;
    Hierarchy       hierarchy_;
    Inventory       inventory_;
    ParticleEmitter particleEmitter_;
    Animations      animations_;
    Sprite          sprites_;
};


//==================================================================================
// inline functions
//==================================================================================

//---------------------------------------------------------
// return an index of entity by id
//---------------------------------------------------------
inline index EntityMgr::GetEnttIdx(const EntityID id) const
{
    const index idx = ids_.get_idx(id);
    assert(idx > 0 && idx < ids_.size());
    return idx;
}

//---------------------------------------------------------
// return a name of the component by its type
//---------------------------------------------------------
inline const char* EntityMgr::GetComponentName(const eComponentType comp) const
{
    return componentNames_.at(comp).c_str();
}

//---------------------------------------------------------
// how many entities we currently have at all?
//---------------------------------------------------------
inline const size EntityMgr::GetNumAllEntts() const
{
    return ids_.size();
}

//---------------------------------------------------------
// return an array of all the entities IDs
//---------------------------------------------------------
inline const cvector<EntityID>& EntityMgr::GetAllEnttsIDs() const
{
    return ids_;
}

//---------------------------------------------------------
// return a components bitfield of entity by id
//---------------------------------------------------------
inline u32Flags EntityMgr::GetAddedComponentsByEntt(const EntityID id) const
{
    return componentFlags_[GetEnttIdx(id)];
}

//---------------------------------------------------------
// simply check if entities exist in the manager
//---------------------------------------------------------
inline bool EntityMgr::CheckEnttExist(const EntityID id) const
{
    return ids_.binary_search(id);
}

inline bool EntityMgr::CheckEnttsExist(const EntityID* ids, const size numEntts) const
{
    return ids_.binary_search(ids, numEntts);
}

};
