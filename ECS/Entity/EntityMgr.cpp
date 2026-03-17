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
#include "../Common/pch.h"
#include "EntityMgr.h"
#include <bit_flags_functions.h>

#pragma warning (disable : 4996)


namespace ECS
{

// after creation of each new entity this value is increased by 1
int EntityMgr::lastEntityID_ = 1;

// static array of entities IDs for internal purposes
static cvector<EntityID> s_Ids;
static cvector<index>    s_Idxs;


//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
EntityMgr::EntityMgr() :
    hierarchy_{},
    nameSys_         { &names_ },
    transformSys_    { &transform_ },
    moveSys_         { &transform_, &movement_ },
    modelSys_        { &modelComp_ },
    renderSys_       { &renderComp_ },
    materialSys_     { &materials_, &nameSys_ },
    lightSys_        { &light_, &transformSys_ },
    boundingSys_     { &bounding_, &transformSys_ },
    cameraSys_       { &camera_, &transformSys_ },
    hierarchySys_    { &hierarchy_, &transformSys_ },
    playerSys_       { &transformSys_, &cameraSys_, &hierarchySys_ },
    particleSys_     { &particleEmitter_, &transformSys_, &boundingSys_ },
    inventorySys_    { &inventory_ },
    animationSys_    { &animations_ },
    spriteSys_       { &sprites_ }
{
    LogDbg(LOG, "start of entity mgr init");

    constexpr int reserveMemForEntts = 4096;

    ids_.reserve(reserveMemForEntts);
    componentFlags_.reserve(reserveMemForEntts);

    sceneObjectsIds_.reserve(reserveMemForEntts);
    sceneObjects_.reserve(reserveMemForEntts);

    // make pairs ['component_type' => 'component_name']
    componentNames_ =
    {
        { NameComponent,                "Name" },
        { TransformComponent,           "Transform" },
        { MoveComponent,                "Movement" },
        { RenderedComponent,            "Rendered" },
        { ModelComponent,               "Model" },

        { CameraComponent,              "Camera" },
        { MaterialComponent,            "Textured" },
        { TextureTransformComponent,    "Texture transform" },
        { LightComponent,               "Light" },
        { BoundingComponent,            "Bounding" },

        { PlayerComponent,              "Player" },
        { ParticlesComponent,           "Particle emitter" },
        { InventoryComponent,           "Intentory" },
        { AnimationComponent,           "Animation" },
        { SpriteComponent,              "Sprite" },

        { AIComponent,                  "AI component" },
        { HealthComponent,              "Health component" },
        { DamageComponent,              "Damage component" },
        { EnemyComponent,               "Enemy component" },
        { ColliderComponent,            "Collider component" },

        { PhysicsTypeComponent,         "PhysicsType component" },
        { CollisionComponent,           "Collision component" },
    };

    if (componentNames_.size() != NUM_COMPONENTS)
        LogFatal(LOG, "you must assign enough names for components!!!");

    // add "invalid" entity with ID == 0
    ids_.push_back(INVALID_ENTITY_ID);
    componentFlags_.push_back(0);

    sceneObjectsIds_.push_back(0);
    sceneObjects_.push_back(SceneObject());

    LogDbg(LOG, "entity mgr is initialized");
}

///////////////////////////////////////////////////////////

EntityMgr::~EntityMgr()
{
    LogDbg(LOG, "ECS destroyment");

    quadTree_.Shutdown();
}

//---------------------------------------------------------
// Desc:  join all the input ids into a single str and separate it with glue
//---------------------------------------------------------
std::string GetEnttsIDsAsString(
    const EntityID* ids,
    const size numEntts,
    const std::string& glue = ", ")
{
    std::stringstream ss;

    for (index i = 0; i < numEntts; ++i)
        ss << ids[i] << glue;

    return ss.str();
}

//---------------------------------------------------------
// Desc:   return a string: "prefix + arr_of_ids"
//---------------------------------------------------------
inline std::string GetErrMsg(const std::string& prefix, const EntityID* ids, const size numEntts)
{
    return prefix + GetEnttsIDsAsString(ids, numEntts);
}

///////////////////////////////////////////////////////////

std::string GetErrMsgFailedAddComponent(const char* componentName, const EntityID id)
{
    memset(g_String, 0, 512);

    if (!StrHelper::IsEmpty(componentName))
    {
        sprintf(g_String, "no entt by id: %" PRIu32 ", so can't add a component (%s)", id, componentName);
    }
    else
    {
        sprintf(g_String, "no entt by id: %" PRIu32 ", so can't add a component", id);
    }

    return g_String;
}

///////////////////////////////////////////////////////////

std::string GetErrMsgNoEntt(const EntityID id)
{
    return "no entity by Id: " + std::to_string(id);
}

// ************************************************************************************
//                     PUBLIC CREATION/DESTROYMENT API
// ************************************************************************************

//---------------------------------------------------------
// Desc:   create a new empty entity;
// Ret:    its ID
//---------------------------------------------------------
EntityID EntityMgr::CreateEntity()
{
    EntityID id = lastEntityID_++;
    ids_.push_back(id);
    componentFlags_.push_back(0);

    return id;
}

//---------------------------------------------------------
// Desc:   create a new entity and set a name for it
// Ret:    id of created entity or 0 (in case if we didn't manage to create)
//---------------------------------------------------------
EntityID EntityMgr::CreateEntity(const char* enttName)
{
    if (StrHelper::IsEmpty(enttName))
    {
        LogErr(LOG, "empty name");
        return INVALID_ENTITY_ID;
    }

    const EntityID id = CreateEntity();
    AddNameComponent(id, enttName);

    return id;
}

//---------------------------------------------------------
// Desc:   create a batch of new empty entities, generate for each entity 
//         unique ID and set that it hasn't any component by default;
// Args:   - newEnttsCount:  how many entitties we want to create
// Ret:    SORTED array of IDs of just created entities;
//---------------------------------------------------------
cvector<EntityID> EntityMgr::CreateEntities(const int newEnttsCount)
{
    if (newEnttsCount <= 0)
    {
        LogErr(LOG, "new entitites count cannot be <= 0");
        return cvector<EntityID>();
    }

    s_Ids.resize(newEnttsCount);

    // "generate" consistent IDs
    for (EntityID& id : s_Ids)
        id = lastEntityID_++;

    // store ids and components flags of entities into the manager
    ids_.append_vector(s_Ids);

    const size newSize = componentFlags_.size() + newEnttsCount;
    componentFlags_.resize(newSize, 0);

    return s_Ids;
}

//---------------------------------------------------------
// Desc:  remove entities from the manager and remove records from components as well
//---------------------------------------------------------
void EntityMgr::DestroyEntities(const EntityID* ids, const size numEntts)
{
    //CAssert::True(ids != nullptr, "input ptr to enitites IDs arr == nullptr");
    //CAssert::True(numEntts > 0, "input number of entts must be > 0");

    assert("TODO: IMPLEMENT IT!" && 0);
}


// ************************************************************************************
//                          PUBLIC UPDATING FUNCTIONS
// ************************************************************************************

void EntityMgr::Update(const float gameTime, const float dt)
{
    // handle events
    for (int i = 0; i < currNumEvents_; ++i)
    {
        Event& e = eventsList_[i];

        switch (e.type)
        {
            case EVENT_TRANSLATE:
            {
                // make an arr of entt and all its children's ids
                hierarchySys_.GetChildrenArr(e.enttID, s_Ids);
                s_Ids.push_back(e.enttID);

                const XMFLOAT3 prevPos  = transformSys_.GetPosition(e.enttID);
                const XMFLOAT3 adjustBy = { e.x-prevPos.x, e.y-prevPos.y, e.z-prevPos.z };

                // adjust position for entt and all its children
                transformSys_.AdjustPositions(s_Ids.data(), s_Ids.size(), adjustBy);

                // update relative position (relatively to parent if we have any)
                hierarchySys_.UpdateRelativePos(e.enttID);

                // update bounding component: world AABB of entity and all its children
                boundingSys_.UpdateWorldBoundings(s_Ids.data(), s_Ids.size());

           
                 // don't update those entities which for some reason aren't in the quad tree
                for (index i = 0; i < s_Ids.size();)
                {
                    if (!sceneObjectsIds_.binary_search(s_Ids[i]))
                    {
                        // swap n pop
                        s_Ids[i] = s_Ids.back();
                        s_Ids.pop_back();
                        continue;
                    }
                    ++i;
                }

                UpdateQuadTreeMembership(s_Ids.data(), s_Ids.size());
               
                break;
            }
            case EVENT_ROTATE:
            {
                break;
            }
            case EVENT_SCALE:
            {
                break;
            }
            case EVENT_PLAYER_RUN:              // set the player is running or not
            {
                playerSys_.SetIsRunning(e.x);
                break;
            }
            case EVENT_PLAYER_JUMP:
            {
                playerSys_.Move(ePlayerState::JUMP);
                break;
            }
            case EVENT_PLAYER_MOVE_FORWARD:
            {
                playerSys_.Move(ePlayerState::MOVE_FORWARD);
                break;
            }
            case EVENT_PLAYER_MOVE_BACK:
            {
                playerSys_.Move(ePlayerState::MOVE_BACK);
                break;
            }
            case EVENT_PLAYER_MOVE_RIGHT:
            {
                playerSys_.Move(ePlayerState::MOVE_RIGHT);
                break;
            }
            case EVENT_PLAYER_MOVE_LEFT:
            {
                playerSys_.Move(ePlayerState::MOVE_LEFT);
                break;
            }
            case EVENT_PLAYER_MOVE_UP:
            {
                playerSys_.Move(ePlayerState::MOVE_UP);
                break;
            }
            case EVENT_PLAYER_MOVE_DOWN:
            {
                playerSys_.Move(ePlayerState::MOVE_DOWN);
                break;
            }
        }
    }

    // update systems
    animationSys_.Update(dt);
    playerSys_.Update(dt);
    particleSys_.Update(dt);

    // we handled all the events so reset the events list
    currNumEvents_ = 0;
}

//---------------------------------------------------------
// Desc:   add a new event into the events list
//---------------------------------------------------------
void EntityMgr::PushEvent(const Event& e)
{
    if (currNumEvents_ >= MAX_NUM_EVENTS)
    {
        LogErr(LOG, "can't push a new event (because of limit of %d events)", MAX_NUM_EVENTS);
        return;
    }

    eventsList_[currNumEvents_++] = e;
}

//---------------------------------------------------------
// Desc:  unbind a component from entity
//---------------------------------------------------------
void EntityMgr::RemoveComponent(const EntityID id, eComponentType component)
{
    bool updateBitfield = false;

    // remove a record from component
    switch (component)
    {
        case RenderedComponent:
            renderSys_.RemoveRecord(id);
            updateBitfield = true;
            break;

        default:
        {
            LogErr(LOG, "can't remove component (%d) of entt (%" PRIu32 "): there is no such component", (int)component, id);
            return;
        }
    }

    // set that an entity by id already DON'T have such a component 
    if (updateBitfield)
    {
        componentFlags_[GetEnttIdx(id)].ClearBit(component);
    }
}

//---------------------------------------------------------
// Desc:  return an instance of ECS's quad tree
//---------------------------------------------------------
::QuadTree& EntityMgr::GetQuadTree()
{
    return quadTree_;
}

//---------------------------------------------------------
//---------------------------------------------------------
size EntityMgr::GetNumSceneObjects() const
{
    return sceneObjects_.size();
}

//---------------------------------------------------------
// Desc:  attach a single input entity to quad tree spatial structure
//---------------------------------------------------------
void EntityMgr::AttachEnttToQuadTree(const EntityID id)
{
    AttachEnttsToQuadTree(&id, 1);
}

//---------------------------------------------------------
// Desc:  attach input entities to quad tree spatial structure
//---------------------------------------------------------
void EntityMgr::AttachEnttsToQuadTree(const EntityID* ids, const size numEntts)
{
    assert(ids);
    assert(numEntts > 0);

    // create new scene objects (elements of the quad tree) by input entts
    CreateQuadTreeObjects(ids, numEntts);
}

//---------------------------------------------------------
// create and insert objects into the quad tree by input entities 
//---------------------------------------------------------
void EntityMgr::CreateQuadTreeObjects(const EntityID* ids, const size numEntts)
{
    assert(ids);
    assert(numEntts > 0);

    BoundingSystem& boundSys = boundingSys_;

    for (index i = 0; i < numEntts; ++i)
    {
        const EntityID id = ids[i];

        sceneObjectsIds_.push_back(id);
        sceneObjects_.push_back(SceneObject());

        SceneObject& obj = sceneObjects_.back();

        obj.Init(
            &quadTree_,
            id,
            boundSys.GetWorldSphere(id),
            boundSys.GetWorldBoxRect3d(id));
    }
}

//---------------------------------------------------------
// update entity's (scene object) location within the quad tree
//---------------------------------------------------------
void EntityMgr::UpdateQuadTreeMembership(const EntityID id)
{
    UpdateQuadTreeMembership(&id, 1);
}

void EntityMgr::UpdateQuadTreeMembership(const EntityID* ids, const size count)
{
    assert(ids);
    assert(count > 0);

    sceneObjectsIds_.get_idxs(ids, count, s_Idxs);

    for (int i = 0; const index idx : s_Idxs)
    {
        const Sphere worldSphere = boundingSys_.GetWorldSphere(s_Ids[i]);
        const Rect3d worldBox    = boundingSys_.GetWorldBoxRect3d(s_Ids[i]);

        sceneObjects_[idx].UpdateWorldBounds(worldSphere, worldBox);

        i++;
    }
}

// *********************************************************************************
// 
//                              ADD COMPONENTS 
// 
// *********************************************************************************

//---------------------------------------------------------
// Desc:  set that an entity by ID has a component by type
//---------------------------------------------------------
void EntityMgr::SetEnttHasComponent(const EntityID id, const eComponentType comp)
{
    componentFlags_[GetEnttIdx(id)].SetBit(comp);
}

//---------------------------------------------------------
// Desc:   add the same component to each input entt
//---------------------------------------------------------
void EntityMgr::SetEnttsHaveComponent(
    const EntityID* ids,
    const size numEntts,
    const eComponentType compType)
{
    ids_.get_idxs(ids, numEntts, s_Idxs);

    // generate hash mask by input component type
    for (const index idx : s_Idxs)
        componentFlags_[idx].SetBit(compType);
}

//---------------------------------------------------------
// Desc:  add the Name component to entity by id
//---------------------------------------------------------
bool EntityMgr::AddNameComponent(const EntityID& id, const char* name)
{
    if (!nameSys_.AddRecord(id, name))
    {
        LogErr(LOG, "can't add a NAME component to entt: %" PRIu32, id);
        return false;
    }

    SetEnttHasComponent(id, NameComponent);
    return true;
}

//---------------------------------------------------------
// Desc:  add the Name component to all the input entities
//        so each entity will have its own name
//---------------------------------------------------------
void EntityMgr::AddNameComponent(
    const EntityID* ids,
    const std::string* names,
    const size numEntts)
{
    if (!nameSys_.AddRecords(ids, names, numEntts))
    {
        std::string errMsg = GetErrMsg("can't add a NAME component to entts: ", ids, numEntts);
        LogErr(LOG, errMsg.c_str());
    }

    SetEnttsHaveComponent(ids, numEntts, NameComponent);
}

//---------------------------------------------------------
// Desc:  add the Transform component to a single entity in terms of arrays
//---------------------------------------------------------
void EntityMgr::AddTransformComponent(
    const EntityID& enttId,
    const XMFLOAT3& position,
    const XMVECTOR& direction,
    const float scale)
{
    constexpr size numEntts = 1;
    AddTransformComponent(&enttId, numEntts, &position, &direction, &scale);
}

//---------------------------------------------------------
// Desc:  add transform component to each input entity by ID
// Args:  - ids:         array of entities IDs
//        - numEntts:    how many entities we will handle
//        - positions:   arr of positions for each input entity
//        - directions:  arr of directions
//        - scales:      arr of uniform scales
//---------------------------------------------------------
void EntityMgr::AddTransformComponent(
    const EntityID* ids,
    const size numEntts,
    const XMFLOAT3* positions,
    const XMVECTOR* directions,
    const float* scales)
{
    try
    {
        transformSys_.AddRecords(ids, positions, directions, scales, numEntts);
        SetEnttsHaveComponent(ids, numEntts, TransformComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't add TRANSFORM component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add a Move component to a single entity
//---------------------------------------------------------
void EntityMgr::AddMoveComponent(
    const EntityID& id,
    const XMFLOAT3& translation,
    const XMVECTOR& rotationQuat,
    const float uniformScaleFactor)
{
    AddMoveComponent(&id, &translation, &rotationQuat, &uniformScaleFactor, 1);
}

//---------------------------------------------------------
// Desc:  add a Move component to all the input entities;
//        and setup entities movement using input data arrays
//---------------------------------------------------------
void EntityMgr::AddMoveComponent(
    const EntityID* ids,
    const XMFLOAT3* translations,
    const XMVECTOR* rotationQuats,
    const float* uniformScaleFactors,
    const size numEntts)
{
    try
    {
        moveSys_.AddRecords(ids, translations, rotationQuats, uniformScaleFactors, numEntts);
        SetEnttsHaveComponent(ids, numEntts, MoveComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't add MOVE component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add a Model component: relate a single entity ID to a single modelID
//---------------------------------------------------------
void EntityMgr::AddModelComponent(
    const EntityID enttID,
    const ModelID modelID)
{
    AddModelComponent(&enttID, modelID, 1);
}

//---------------------------------------------------------
// Desc:   add a Model component to each input entity by ID in terms of arrays;
//         here we relate the same model to each input entt
//---------------------------------------------------------
void EntityMgr::AddModelComponent(
    const EntityID* enttsIDs,
    const ModelID modelID,
    const size numEntts)
{
    try
    {
        modelSys_.AddRecords(enttsIDs, modelID, numEntts);
        SetEnttsHaveComponent(enttsIDs, numEntts, ModelComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't add MODEL component to entts: %s; \nmodel ID: %" PRIu32, GetEnttsIDsAsString(enttsIDs, numEntts).c_str(), modelID);
    }
}

//---------------------------------------------------------
// Desc:   add rendering component to a single entity by ID
//---------------------------------------------------------
void EntityMgr::AddRenderingComponent(const EntityID id)
{
    AddRenderingComponent(&id, 1);
}

//---------------------------------------------------------
// Desc:   add RenderComponent to each entity by its ID; 
//         so these entities will be rendered onto the screen
//---------------------------------------------------------
void EntityMgr::AddRenderingComponent(const EntityID* ids, const size numEntts)
{
    try
    {
        renderSys_.AddRecords(ids, numEntts);
        SetEnttsHaveComponent(ids, numEntts, RenderedComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        LogErr(LOG, "can't add RENDERING component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add material component to entity which has a single mesh
// Args:   - enttId:                entity identifier
//         - matId:                 material identifier
//         - isMaterialMeshBased:   is material the same as a model (geometry) bounded to entity
//---------------------------------------------------------
void EntityMgr::AddMaterialComponent(
    const EntityID enttId,
    const MaterialID matId)
{
    AddMaterialComponent(enttId, &matId, 1);
}

//---------------------------------------------------------
// Desc:  add a material component and set THE SAME material for each input entity
//---------------------------------------------------------
void EntityMgr::AddMaterialComponent(
    const EntityID* enttsIds,
    const size numEntts,
    const MaterialID matId)
{
    assert(enttsIds);
    assert(numEntts);
    assert(matId > 0);

    for (index i = 0; i < numEntts; ++i)
        AddMaterialComponent(enttsIds[i], &matId, 1);
}

//---------------------------------------------------------
// Desc:  add material component for input entity by ID;
// Args:  - enttId:         entity identifier
//        - materialsIDs:   arr of materials ids
//                          (each material ID will be related to
//                          a single submesh of the entity)
//        - numMeshes:      how many meshes does input entity have
//---------------------------------------------------------
void EntityMgr::AddMaterialComponent(
    const EntityID enttId,
    const MaterialID* materialsIds,
    const size numMeshes)
{
    try
    {
        if (!CheckEnttExist(enttId))
        {
            LogErr(LOG, "no entity by ID: %" PRIu32, enttId);
            return;
        }
        
        materialSys_.AddRecord(enttId, materialsIds, numMeshes);
        SetEnttHasComponent(enttId, MaterialComponent);
    }
    catch (EngineException& e)
    {
        const char* enttName = nameSys_.GetNameById(enttId);

        LogErr(LOG, e.what());
        LogErr(LOG, "can't add Material component to entt (id: %d; name: %s)", (int)enttId, enttName);
    }
}

//---------------------------------------------------------
// Desc:   add a light component according to input type
//---------------------------------------------------------
void EntityMgr::AddLightComponent(const EntityID id, const DirLight& initData)
{
    lightSys_.AddDirLight(id, initData);
    SetEnttHasComponent(id, LightComponent);
}

void EntityMgr::AddLightComponent(const EntityID id, const PointLight& initData)
{
    lightSys_.AddPointLight(id, initData);
    SetEnttHasComponent(id, LightComponent);
}

void EntityMgr::AddLightComponent(const EntityID id, const SpotLight& initData)
{
    lightSys_.AddSpotLight(id, initData);
    SetEnttHasComponent(id, LightComponent);
}

//---------------------------------------------------------
// Desc:  add bounding component to a single entity
//        (adds bounding sphere)
//---------------------------------------------------------
void EntityMgr::AddBoundingComponent(
    const EntityID id,
    const DirectX::BoundingSphere& localSphere,
    const DirectX::BoundingSphere& worldSphere)
{
    if (!boundingSys_.Add(id, localSphere, worldSphere))
    {
        LogErr(LOG, "can't add BOUNDING component to entts: %" PRIu32, id);
        return;
    }

    SetEnttHasComponent(id, BoundingComponent);
}
//---------------------------------------------------------
// Desc:   add bounding component to a single entity
//         so it will have some bounding shape
//---------------------------------------------------------
void EntityMgr::AddBoundingComponent(
    const EntityID id,
    const DirectX::BoundingBox& localBox,
    const DirectX::BoundingBox& worldBox)
{
    if (!boundingSys_.Add(id, localBox, worldBox))
    {
        LogErr(LOG, "can't add BOUNDING component to entts: %" PRIu32, id);
        return;
    }

    SetEnttHasComponent(id, BoundingComponent);
}

//---------------------------------------------------------
// Desc:   apply the same set of AABBs to each input entity
//         (for instance: we have 100 the same trees so each will have the same set of AABBs) 
//---------------------------------------------------------
void EntityMgr::AddBoundingComponent(
    const EntityID* ids,
    const size numEntts,
    const DirectX::BoundingBox& localBox,
    const DirectX::BoundingBox* worldBoxes)
{
    if (!boundingSys_.Add(ids, numEntts, localBox, worldBoxes))
    {
        LogErr(LOG, "can't add BOUNDING component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        return;
    }

    SetEnttsHaveComponent(ids, numEntts, BoundingComponent);
}

//---------------------------------------------------------
// Desc:   add a camera component to the entity by input ID
//---------------------------------------------------------
void EntityMgr::AddCameraComponent(const EntityID id, const CameraData& data)
{
    try
    {
        cameraSys_.AddRecord(id, data);
        SetEnttHasComponent(id, CameraComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, "can't add CAMERA component to entt: %" PRIu32, id);
        LogErr(LOG, e.what());
    }
}

//---------------------------------------------------------
// Desc:   add a player component to the entity by input ID
//---------------------------------------------------------
void EntityMgr::AddPlayerComponent(const EntityID id)
{
    if (!CheckEnttExist(id))
    {
        LogErr(LOG, GetErrMsgFailedAddComponent("player", id).c_str());
        return;
    }

    try
    {
        playerSys_.SetPlayer(id);
        SetEnttHasComponent(id, PlayerComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, "can't add a PLAYER component to entt: %" PRIu32, id);
        LogErr(LOG, e.what());
    }
}

//---------------------------------------------------------
// Desc:   add a particle emitter to the entity by input ID
// Args:   - id:  entity identifier
//---------------------------------------------------------
void EntityMgr::AddParticleEmitterComponent(const EntityID id)
{
    if (!CheckEnttExist(id))
    {
        LogErr(LOG, GetErrMsgFailedAddComponent("PARTICLE EMITTER", id).c_str());
        return;
    }

    particleSys_.AddEmitter(id);
    SetEnttHasComponent(id, ParticlesComponent);
}

//---------------------------------------------------------
// Desc:   an entity by input id will have its own inventory
//---------------------------------------------------------
void EntityMgr::AddInventoryComponent(const EntityID id)
{
    if (!CheckEnttExist(id))
    {
        LogErr(LOG, GetErrMsgFailedAddComponent("INVENTORY", id).c_str());
        return;
    }

    inventorySys_.AddInventory(id);
    SetEnttHasComponent(id, InventoryComponent);
}

//---------------------------------------------------------
// Desc:  bind a skeleton to entity and set current animation (for model skinning)
//---------------------------------------------------------
void EntityMgr::AddAnimationComponent(
    const EntityID enttId,
    const SkeletonID skeletonId,
    const AnimationID animId,
    const float animEndTime)
{
    if (!CheckEnttExist(enttId))
    {
        LogErr(LOG, GetErrMsgFailedAddComponent("ANIMATION", enttId).c_str());
        return;
    }

    if (!animationSys_.AddRecord(enttId, skeletonId, animId, animEndTime))
    {
        LogErr(LOG, "can't add a skeleton and animation for entt: %" PRIu32, enttId);
        return;
    }

    SetEnttHasComponent(enttId, AnimationComponent);
}

//---------------------------------------------------------
// Desc:  bind a sprite component to entity by id
// Args:  - enttId:  entity identifier
//        - texId:   identifier of a texture which will be used as a 2D sprite
//        - leftPos: position of left edge of sprite
//                   in screen pixels coords (starting from upper left <0,0>)
//        - topPos:  position of top edge of sprite
//        - width:   width of the sprite in pixels
//        - height:  height of the sprite in pixels
//---------------------------------------------------------
void EntityMgr::AddSpriteComponent(
    const EntityID enttId,
    const TexID texId,
    const uint16 leftPos,
    const uint16 topPos,
    const uint16 width,
    const uint16 height)
{
    if (!CheckEnttExist(enttId))
    {
        LogErr(LOG, GetErrMsgFailedAddComponent("SPRITE", enttId).c_str());
        return;
    }

    if (!spriteSys_.AddRecord(enttId, texId, leftPos, topPos, width, height))
    {
        LogErr(LOG, "can't add a 2D sprite component for entt: %" PRIu32, enttId);
        return;
    }

    SetEnttHasComponent(enttId, SpriteComponent);
}


// ************************************************************************************
//                               PRIVATE HELPERS
// ************************************************************************************

//---------------------------------------------------------
// out:     array of components (its types) which added to entity by id
// return:  false if there is no entity by ID
//---------------------------------------------------------
bool EntityMgr::GetAddedComponentsByEntt(
    const EntityID id,
    ECS::eComponentType* outArr,
    size& outNumComponents) const
{
    assert(outArr);

    outNumComponents = 0;
    const index enttIdx = GetEnttIdx(id);

    if (enttIdx == 0)
        return false;

    const u32Flags flags = componentFlags_[enttIdx];

    // store into arr a code of each added component
    for (int i = 0; i < NUM_COMPONENTS; ++i)
    {
        if (flags.TestBit(i))
            outArr[outNumComponents++] = eComponentType(i);
    }

    return true;
}

//---------------------------------------------------------
// print dump of all the currently existed entities 
//---------------------------------------------------------
void EntityMgr::DumpEntts() const
{
    printf("Dump entities:\n");

    for (index i = 0; i < ids_.size(); ++i)
    {
        const EntityID id = ids_[i];

        printf("[%td]:  ", i);
        printf("id: %5d    ", (int)id);
        printf("name: %s\n", nameSys_.GetNameById(id));
    }
    printf("\n");
}

//---------------------------------------------------------
// print dump of all the currently existed scene objects
//---------------------------------------------------------
void EntityMgr::DumpSceneObjects() const
{
    printf("Dump scene objects (quad tree members):\n");

    for (index i = 0; i < sceneObjects_.size(); ++i)
    {
        const EntityID id = sceneObjects_[i].GetId();

        printf("[%td]:  ", i);
        printf("id: %5d    ", (int)id);
        printf("name: %s\n", nameSys_.GetNameById(id));
    }
    printf("\n");
}

} // namespace ECS
