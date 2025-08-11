#include "../Common/pch.h"
#include "EntityMgr.h"

#pragma warning (disable : 4996)

namespace ECS
{

// after creation of each new entity this value is increased by 1
int EntityMgr::lastEntityID_ = 1;

// static array of entities IDs for internal purposes
cvector<EntityID> s_Ids;


EntityMgr::EntityMgr() :
    hierarchy_{},
    nameSystem_         { &names_ },
    transformSystem_    { &transform_ },
    moveSystem_         { &transform_, &movement_ },
    modelSystem_        { &modelComponent_ },
    renderSystem_       { &renderComponent_ },
    materialSystem_     { &materialComponent_, &nameSystem_ },
    texTransformSystem_ { &texTransform_ },
    lightSystem_        { &light_, &transformSystem_ },
    //renderStatesSystem_ { &renderStates_ },
    boundingSystem_     { &bounding_ },
    cameraSystem_       { &camera_, &transformSystem_ },
    hierarchySystem_    { &hierarchy_, &transformSystem_ },
    playerSystem_       { &transformSystem_, &cameraSystem_, &hierarchySystem_ },
    particleSystem_     { &transformSystem_ }
{
    LogDbg(LOG, "start of entity mgr init");

    constexpr int reserveMemoryForEnttsCount = 100;

    ids_.reserve(reserveMemoryForEnttsCount);
    componentHashes_.reserve(reserveMemoryForEnttsCount);

    // make pairs ['component_type' => 'component_name']
    componentTypeToName_ =
    {
        { TransformComponent,           "Transform" },
        { NameComponent,                "Name" },
        { MoveComponent,                "Movement" },
        { ModelComponent,               "Model" },
        { RenderedComponent,            "Rendered" },
        { MaterialComponent,            "Textured" },
        { TextureTransformComponent,    "Texture transform" },
        { LightComponent,               "Light" },
        { RenderStatesComponent,        "Render states" },
        { BoundingComponent,            "Bounding" },
        { CameraComponent,              "Camera" },
    };

    // add "invalid" entity with ID == 0
    ids_.push_back(INVALID_ENTITY_ID);
    componentHashes_.push_back(0);

    LogDbg(LOG, "entity mgr is initialized");
}

///////////////////////////////////////////////////////////

EntityMgr::~EntityMgr()
{
    LogDbg(LOG, "ECS destroyment");
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

inline std::string GetErrMsgNoEntt(const EntityID id)
{
    return "there is no entity by ID: " + std::to_string(id);
}


// ************************************************************************************
//                 PUBLIC SERIALIZATION / DESERIALIZATION API
// ************************************************************************************

bool EntityMgr::Serialize(const std::string& dataFilepath)
{
    return false;
}

///////////////////////////////////////////////////////////

bool EntityMgr::Deserialize(const std::string& dataFilepath)
{
    return false;
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
    componentHashes_.push_back(0);

    return id;
}

//---------------------------------------------------------
// Desc:   create a new entity and set a name for it
// Ret:    id of created entity or 0 (in case if we didn't manage to create)
//---------------------------------------------------------
EntityID EntityMgr::CreateEntity(const char* enttName)
{
    if (!enttName || enttName[0] == '\0')
    {
        LogErr("input name for entity is empty!");
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

    // append ids and hashes of entities
    ids_.append_vector(s_Ids);
    componentHashes_.append_vector(cvector<ComponentBitfield>(newEnttsCount, 0));

    return s_Ids;
}

///////////////////////////////////////////////////////////

void EntityMgr::DestroyEntities(const EntityID* ids, const size numEntts)
{
    //CAssert::True(ids != nullptr, "input ptr to enitites IDs arr == nullptr");
    //CAssert::True(numEntts > 0, "input number of entts must be > 0");

    assert("TODO: IMPLEMENT IT!" && 0);
}


// ************************************************************************************
//                          PUBLIC UPDATING FUNCTIONS
// ************************************************************************************

void EntityMgr::Update(const float totalGameTime, const float deltaTime)
{
    //moveSystem_.UpdateAllMoves(deltaTime, transformSystem_);
    texTransformSystem_.UpdateAllTextrureAnimations(totalGameTime, deltaTime);
    lightSystem_.Update(deltaTime, totalGameTime);

    
    for (const Event& e : events_)
    {
        switch (e.type)
        {
            case EVENT_TRANSLATE:
            {
                // compute offset for translation
                EntityID id = e.enttID;
                XMFLOAT3 prevPos = transformSystem_.GetPosition(id);
                XMFLOAT3 offset = { e.x-prevPos.x, e.y-prevPos.y, e.z-prevPos.z };

                // make an arr of entt and its children's ids
                hierarchySystem_.GetChildrenArr(id, s_Ids);
                s_Ids.push_back(id);

                // adjust position for entt and its children
                transformSystem_.AdjustPositions(s_Ids.data(), s_Ids.size(), { offset.x, offset.y, offset.z });
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
                playerSystem_.SetIsRunning(e.x);
                break;
            }
            case EVENT_PLAYER_JUMP:
            {
                playerSystem_.Move(ePlayerState::JUMP);
                break;
            }
            case EVENT_PLAYER_MOVE_FORWARD:
            {
                playerSystem_.Move(ePlayerState::MOVE_FORWARD);
                break;
            }
            case EVENT_PLAYER_MOVE_BACK:
            {
                playerSystem_.Move(ePlayerState::MOVE_BACK);
                break;
            }
            case EVENT_PLAYER_MOVE_RIGHT:
            {
                playerSystem_.Move(ePlayerState::MOVE_RIGHT);
                break;
            }
            case EVENT_PLAYER_MOVE_LEFT:
            {
                playerSystem_.Move(ePlayerState::MOVE_LEFT);
                break;
            }
            case EVENT_PLAYER_MOVE_UP:
            {
                playerSystem_.Move(ePlayerState::MOVE_UP);
                break;
            }
            case EVENT_PLAYER_MOVE_DOWN:
            {
                playerSystem_.Move(ePlayerState::MOVE_DOWN);
                break;
            }
        }
    }


    playerSystem_.Update(deltaTime);
    particleSystem_.Update(deltaTime);

    // we handled all the events so clear the list of events
    events_.clear();
}

//---------------------------------------------------------
// Desc:   add a new event into the events list
//---------------------------------------------------------
void EntityMgr::AddEvent(const Event& e)
{
    events_.push_back(e);
}

// *********************************************************************************
// 
//                              ADD COMPONENTS 
// 
// *********************************************************************************

void EntityMgr::SetEnttHasComponent(
    const EntityID id,
    const eComponentType compType)
{
    // set that an entity by ID has such a component 
    const index idx = ids_.get_idx(id);
    componentHashes_[idx] |= (1 << compType);
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponent(
    const EntityID* ids,
    const size numEntts,
    const eComponentType compType)
{
    // add the same component to each input entt

    cvector<index> idxs;
    ids_.get_idxs(ids, numEntts, idxs);

    // generate hash mask by input component type
    ComponentBitfield hashMask = 1 << compType;

    for (const index idx : idxs)
        componentHashes_[idx] |= hashMask;
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponents(
    const EntityID* ids,
    const cvector<eComponentType>& compTypes,
    const size numEntts)
{
    // add an arr of components to each input entt

    // get data idx of each input entt
    cvector<index> idxs;
    ids_.get_idxs(ids, numEntts, idxs);

    // generate hash mask by input component types
    ComponentBitfield hashMask = 0;
    for (const eComponentType type : compTypes)
        hashMask |= (1 << type);

    for (const index idx : idxs)
        componentHashes_[idx] |= hashMask;
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(const EntityID& id, const std::string& name)
{
    // add the Name component to a single entity in terms of arrays
    AddNameComponent(&id, &name, 1);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(
    const EntityID* ids,
    const std::string* names,
    const size numEntts)
{
    // add the Name component to all the input entities
    // so each entity will have its own name
    try
    {
        nameSystem_.AddRecords(ids, names, numEntts);
        SetEnttsHaveComponent(ids, numEntts, NameComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(GetErrMsg("can't add a name component to entts: ", ids, numEntts).c_str());
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
    AddTransformComponent(&enttID, 1, &position, &dirQuat, &uniformScale);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTransformComponent(
    const EntityID* ids,
    const size numEntts,
    const XMFLOAT3* positions,
    const XMVECTOR* dirQuats,
    const float* uniformScales)
{
    // add transform component to all the input entities
    try
    {
        transformSystem_.AddRecords(ids, positions, dirQuats, uniformScales, numEntts);
        SetEnttsHaveComponents(ids, { TransformComponent }, numEntts);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add transform component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add the Move component to a single entity
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
// Desc:  add the Move component to all the input entities;
//        and setup entities movement using input data arrays

void EntityMgr::AddMoveComponent(
    const EntityID* ids,
    const XMFLOAT3* translations,
    const XMVECTOR* rotationQuats,
    const float* uniformScaleFactors,
    const size numEntts)
{
    try
    {
        moveSystem_.AddRecords(ids, translations, rotationQuats, uniformScaleFactors, numEntts);
        SetEnttsHaveComponent(ids, numEntts, MoveComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add move component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add the Model component: relate a single entity ID to a single modelID
//---------------------------------------------------------
void EntityMgr::AddModelComponent(
    const EntityID enttID,
    const ModelID modelID)
{
    
    AddModelComponent(&enttID, modelID, 1);
}

//---------------------------------------------------------
// Desc:   add the Model component to each input entity by ID in terms of arrays;
//         here we relate the same model to each input entt
//---------------------------------------------------------
void EntityMgr::AddModelComponent(
    const EntityID* enttsIDs,
    const ModelID modelID,
    const size numEntts)
{
    try
    {
        modelSystem_.AddRecords(enttsIDs, modelID, numEntts);
        SetEnttsHaveComponent(enttsIDs, numEntts, ModelComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add model component to entts: %s; \nmodel ID: %ud", GetEnttsIDsAsString(enttsIDs, numEntts).c_str(), modelID);
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
        renderSystem_.AddRecords(ids, numEntts);
        //renderStatesSystem_.AddWithDefaultStates(ids, numEntts);

        using enum eComponentType;
        SetEnttsHaveComponents(ids, { RenderedComponent, RenderStatesComponent }, numEntts);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add rendering component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
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
// Desc:  add material component for input entity by ID;
// Args:  - materialsIDs:          arr of material IDs (each material ID will be related to a single submesh of the entity)
//        - numSubmeshes:          how many meshes does input entity have
//        - areMaterialsMeshBased: defines if all the materials IDs are the same as materials IDs of the related model
//---------------------------------------------------------
void EntityMgr::AddMaterialComponent(
    const EntityID enttId,
    const MaterialID* materialsIds,
    const size numSubmeshes)             
{
    try
    {
        if (!CheckEnttExist(enttId))
        {
            LogErr(LOG, "there is no entity by ID: %ld", enttId);
            return;
        }
        
        materialSystem_.AddRecord(enttId, materialsIds, numSubmeshes);
        SetEnttHasComponent(enttId, MaterialComponent);
    }
    catch (EngineException& e)
    {
        const char* name = nameSystem_.GetNameById(enttId);

        LogErr(e);
        LogErr(LOG, "can't add Material component to entt (id: %ld; name: %s)", enttId, name);
    }
}

//---------------------------------------------------------
// Desc:   add texture transformation to a signle entity by ID
//---------------------------------------------------------
void EntityMgr::AddTextureTransformComponent(
    const EntityID id,
    const TexTransformType type,
    const TexTransformInitParams& params)
{
    AddTextureTransformComponent(&id, 1, type, params);
}

//---------------------------------------------------------
// Desc:   set texture transformation of input type for each input entity by ID
// Args:   - type:   what kind of texture transformation we want to apply?
//         - params: struct of arrays of texture transformations params according to the input type
//---------------------------------------------------------
void EntityMgr::AddTextureTransformComponent(
    const EntityID* ids,
    const size numEntts,
    const TexTransformType type,
    const TexTransformInitParams& params)
{
    try
    {
        texTransformSystem_.AddTexTransformation(ids, numEntts, type, params);
        SetEnttsHaveComponent(ids, numEntts, TextureTransformComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add texture transform component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add light component (directed light) to each input entity by ID 
//---------------------------------------------------------
void EntityMgr::AddLightComponent(
    const EntityID* ids,
    const size numEntts,
    DirLightsInitParams& params)
{
    try
    {
        lightSystem_.AddDirLights(ids, numEntts, params);
        SetEnttsHaveComponent(ids, numEntts, LightComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(GetErrMsg("can't add Light component (directional lights) to entts: ", ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add light component (point light) to each input entity by ID 
//---------------------------------------------------------
void EntityMgr::AddLightComponent(
    const EntityID* ids,
    const size numEntts,
    PointLightsInitParams& params)
{
    try
    {
        lightSystem_.AddPointLights(ids, numEntts, params);
        SetEnttsHaveComponent(ids, numEntts, LightComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(GetErrMsg("can't add Light component (point lights) to entts: ", ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add light component (point light) to a single input entity by ID
//---------------------------------------------------------
void EntityMgr::AddLightComponent(const EntityID id, const PointLight& initData)
{
    lightSystem_.AddPointLight(id, initData);
    SetEnttHasComponent(id, LightComponent);
}

//---------------------------------------------------------
// Desc:   add light component (spotlight) to each input entity by ID 
//---------------------------------------------------------
void EntityMgr::AddLightComponent(
    const EntityID* ids,
    const size numEntts,
    SpotLightsInitParams& params)
{
    try
    {
        lightSystem_.AddSpotLights(ids, numEntts, params);
        SetEnttsHaveComponent(ids, numEntts, LightComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(GetErrMsg("can't add Light component (spot lights) to entts: ", ids, numEntts).c_str());
    }
}

//---------------------------------------------------------
// Desc:   add bounding component to a single entity
//         so it will have some bounding shape
//---------------------------------------------------------
void EntityMgr::AddBoundingComponent(
    const EntityID id,
    const BoundingType type,
    const DirectX::BoundingBox& aabb)
{
    try
    {
        boundingSystem_.Add(id, type, aabb);
        SetEnttHasComponent(id, BoundingComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add bounding component to entts: %ud", id);
    }
}

//---------------------------------------------------------
// Desc:   apply the same set of AABBs to each input entity
//         (for instance: we have 100 the same trees so each will have the same set of AABBs) 
//---------------------------------------------------------
void EntityMgr::AddBoundingComponent(
    const EntityID* ids,
    const size numEntts,
    const size numSubsets,              // the number of entt's meshes (the num of AABBs)
    const BoundingType* types,          // AABB type per mesh
    const DirectX::BoundingBox* AABBs)  // AABB per mesh
{
    try
    {
        boundingSystem_.Add(ids, numEntts, numSubsets, types, AABBs);
        SetEnttsHaveComponent(ids, numEntts, BoundingComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, "can't add bounding component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
    }
}

//---------------------------------------------------------
// Desc:   add a camera component to the entity by input ID
//---------------------------------------------------------
void EntityMgr::AddCameraComponent(const EntityID id, const CameraData& data)
{
    try
    {
        cameraSystem_.AddRecord(id, data);
        SetEnttHasComponent(id, CameraComponent);
    }
    catch (EngineException& e)
    {
        LogErr(LOG, "can't add camera component to entts: %ud", id);
        LogErr(e);
    }
}

//---------------------------------------------------------
// Desc:   add a player component to the entity by input ID
//---------------------------------------------------------
void EntityMgr::AddPlayerComponent(const EntityID id)
{
    try
    {
        playerSystem_.SetPlayer(id);
        SetEnttHasComponent(id, PlayerComponent);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't add a player component to entt: %ud", id);
    }
}

//---------------------------------------------------------
// Desc:   add a particle emitter to the entity by input ID
// Args:   - id:  entity identifier
//---------------------------------------------------------
void EntityMgr::AddParticleEmitterComponent(const EntityID id)
{
    particleSystem_.AddEmitter(id);
    SetEnttHasComponent(id, ParticlesComponent);
}


// ************************************************************************************
//                               PRIVATE HELPERS
// ************************************************************************************

ComponentBitfield EntityMgr::GetHashByComponent(const eComponentType component)
{
    ComponentBitfield bitmask = 0;
    return (bitmask |= (1 << component));
}

//---------------------------------------------------------
// out:    names array of components which are added to entity by ID;
// return: false if there is no entity by ID
//---------------------------------------------------------
bool EntityMgr::GetComponentNamesByEntt(const EntityID id, cvector<std::string>& names) const
{
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    if (!exist)
    {
        LogErr(GetErrMsgNoEntt(id).c_str());
        return false;
    }

    // get a bitfield where each bit is define if such component is added to entity or not
    const ComponentBitfield hash = componentHashes_[idx];

    for (int i = 0; i < eComponentType::NUM_COMPONENTS; ++i)
    {
        if (hash & (1 << i))
            names.push_back(componentTypeToName_.at(eComponentType(i)));
    }

    return true;
}

//---------------------------------------------------------
// out:    names array of component types which are added to entity by ID;
// return: false if there is no entity by ID
//---------------------------------------------------------
bool EntityMgr::GetComponentTypesByEntt(const EntityID id, cvector<uint8_t>& types) const
{
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    if (!exist)
    {
        LogErr(GetErrMsgNoEntt(id).c_str());
        return false;
    }

    // get a bitfield where each bit is define if such component is added to entity or not
    const ComponentBitfield hash = componentHashes_[idx];
    types.reserve(4);

    for (int i = 0; i < (int)eComponentType::NUM_COMPONENTS; ++i)
    {
        if (hash & (1 << i))
            types.push_back((uint8_t)i);
    }

    return true;
}

} // namespace ECS
