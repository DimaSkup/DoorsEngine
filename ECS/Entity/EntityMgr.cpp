#include "../Common/pch.h"
#include "EntityMgr.h"

#pragma warning (disable : 4996)

namespace ECS
{

// after creation of each new entity this value is increased by 1
int EntityMgr::lastEntityID_ = 1;


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
    renderStatesSystem_ { &renderStates_ },
    boundingSystem_     { &bounding_ },
    cameraSystem_       { &camera_, &transformSystem_ },
    hierarchySystem_    { &hierarchy_, &transformSystem_ },
    playerSystem_       { &transformSystem_, &cameraSystem_, &hierarchySystem_ }
   
{
    LogDbg("start of entity mgr init");

    constexpr int reserveMemForEnttsCount = 100;

    

    ids_.reserve(reserveMemForEnttsCount);
    componentHashes_.reserve(reserveMemForEnttsCount);

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

    LogDbg("entity mgr is initialized");
}

///////////////////////////////////////////////////////////

EntityMgr::~EntityMgr()
{
    LogDbg("ECS destroyment");
}

///////////////////////////////////////////////////////////

std::string GetEnttsIDsAsString(
    const EntityID* ids,
    const size numEntts,
    const std::string& glue = ", ")
{
    // join all the input ids into a single str and separate it with glue

    std::stringstream ss;

    for (index i = 0; i < numEntts; ++i)
        ss << ids[i] << glue;

    return ss.str();
}

///////////////////////////////////////////////////////////

inline std::string GetErrMsg(const std::string& prefix, const EntityID* ids, const size numEntts)
{
    // return a string: "prefix + arr_of_ids"
    return prefix + GetEnttsIDsAsString(ids, numEntts);
}

///////////////////////////////////////////////////////////

inline std::string GetErrMsgNoEntt(const EntityID id)
{
    return "there is no entity by ID: " + std::to_string(id);
}

///////////////////////////////////////////////////////////

void EntityMgr::SetupLogger(FILE* pFile)
{
    // setup a file for writing log msgs into it;
    // also setup a list which will be filled with log messages;
    
    LogDbg("logger is setup successfully");
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

EntityID EntityMgr::CreateEntity(const char* enttName)
{
    // create a new entity and set a name for it
    // return: id of created entity or 0

    if (!enttName || enttName[0] == '\0')
    {
        LogErr("input name for entity is empty!");
        return INVALID_ENTITY_ID;
    }

    const EntityID id = CreateEntity();
    AddNameComponent(id, enttName);

    return id;
}

///////////////////////////////////////////////////////////

cvector<EntityID> EntityMgr::CreateEntities(const int newEnttsCount)
{
    // create batch of new empty entities, generate for each entity 
    // unique ID and set that it hasn't any component by default;
    //
    // return: SORTED array of IDs of just created entities;

    CAssert::True(newEnttsCount > 0, "new entitites count cannot be <= 0");

    cvector<EntityID> generatedIDs(newEnttsCount, INVALID_ENTITY_ID);

    // "generate" consistent IDs
    for (EntityID& id : generatedIDs)
        id = lastEntityID_++;

    // append ids and hashes of entities
    ids_.append_vector(generatedIDs);
    componentHashes_.append_vector(cvector<ComponentBitfield>(newEnttsCount, 0));

    return generatedIDs;
}

///////////////////////////////////////////////////////////

void EntityMgr::DestroyEntities(const EntityID* ids, const size numEntts)
{
    //CAssert::True(ids != nullptr, "input ptr to enitites IDs arr == nullptr");
    //CAssert::True(numEntts > 0, "input number of entts must be > 0");

    assert("TODO: IMPLEMENT IT!" && 0);
}


#pragma endregion

// ************************************************************************************
//                          PUBLIC UPDATING FUNCTIONS
// ************************************************************************************

void EntityMgr::Update(const float totalGameTime, const float deltaTime)
{
    //moveSystem_.UpdateAllMoves(deltaTime, transformSystem_);
    texTransformSystem_.UpdateAllTextrureAnimations(totalGameTime, deltaTime);
    lightSystem_.Update(deltaTime, totalGameTime);

    cvector<EntityID> ids(16);

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
                hierarchySystem_.GetChildrenArr(id, ids);
                ids.push_back(id);

                // adjust position for entt and its children
                transformSystem_.AdjustPositions(ids.data(), ids.size(), { offset.x, offset.y, offset.z });

#if 0
                EntityID playerID = playerSystem_.GetPlayerID();
                XMFLOAT3 t = { e.x, e.y, e.z };
                XMFLOAT3 playerOldPos = playerSystem_.GetPosition();

                cvector<EntityID> ids;
                hierarchySystem_.GetChildrenArr(playerID, ids);
                ids.push_back(playerID);

                XMFLOAT3 offset = { t.x-playerOldPos.x, t.y - playerOldPos.y, t.z - playerOldPos.z};

                transformSystem_.AdjustPositions(ids.data(), ids.size(), { offset.x, offset.y, offset.z });
#endif
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

    // we handled all the events so clear the list of event
    events_.clear();

}

void EntityMgr::AddEvent(const Event& e)
{
    events_.push_back(e);
}

// *********************************************************************************
// 
//                              ADD COMPONENTS 
// 
// *********************************************************************************

#pragma region AddComponentsAPI

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
        sprintf(g_String, "can't add transform component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddMoveComponent(
    const EntityID& id,
    const XMFLOAT3& translation,
    const XMVECTOR& rotationQuat,
    const float uniformScaleFactor)
{
    // add the Move component to a single entity
    AddMoveComponent(&id, &translation, &rotationQuat, &uniformScaleFactor, 1);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddMoveComponent(
    const EntityID* ids,
    const XMFLOAT3* translations,
    const XMVECTOR* rotationQuats,
    const float* uniformScaleFactors,
    const size numEntts)
{
    // add the Move component to all the input entities;
    // and setup entities movement using input data arrays

    try
    {
        moveSystem_.AddRecords(ids, translations, rotationQuats, uniformScaleFactors, numEntts);
        SetEnttsHaveComponent(ids, numEntts, MoveComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add move component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddModelComponent(
    const EntityID enttID,
    const ModelID modelID)
{
    // add the Model component: relate a single entity ID to a single modelID
    AddModelComponent(&enttID, modelID, 1);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddModelComponent(
    const EntityID* enttsIDs,
    const ModelID modelID,
    const size numEntts)
{
    // add the Model component to each input entity by ID in terms of arrays;
    // here we relate the same model to each input entt

    try
    {
        modelSystem_.AddRecords(enttsIDs, modelID, numEntts);
        SetEnttsHaveComponent(enttsIDs, numEntts, ModelComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add model component to entts: %s; \nmodel ID: %ud", GetEnttsIDsAsString(enttsIDs, numEntts).c_str(), modelID);
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

#if 0
void EntityMgr::AddModelComponent(
    const EntityID* enttsIDs,
    const ModelID* modelsIDs,
    const size numEntts)
{
    // add ModelComponent to each entity by its ID; 
    // and bind to each input entity a model by respective idx (one to one)

    try
    {
        CAssert::NotEmpty(enttsIDs.empty(), "the array of entities IDs is empty");
        CAssert::NotEmpty(modelsIDs.empty(), "the array of models IDs is empty");

        modelSystem_.AddRecords(enttsIDs, modelsIDs);
        SetEnttsHaveComponent(enttsIDs.data(), std::ssize(enttsIDs), ModelComponent);
    }
    catch (EngineException& e)
    {
        std::string errMsg;
        errMsg += "can't add model component to entts: ";
        errMsg += Utils::GetEnttsIDsAsString(enttsIDs.data(), (int)enttsIDs.size());

        LogErr(e);
        LogErr(errMsg);
    }
}

#endif

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
    const EntityID id,
    const RenderInitParams& params)
{
    // add rendering component to a single entity by ID
    AddRenderingComponent(&id, 1, &params);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
    const EntityID* ids,
    const size numEntts,
    const RenderInitParams& params)
{
    // add the Rendered component to each input entity by ID
    // and setup them with the same rendering params 

    const cvector<RenderInitParams> paramsArr(numEntts, params);
    AddRenderingComponent(ids, numEntts, paramsArr.data());
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderingComponent(
    const EntityID* ids,
    const size numEntts,
    const RenderInitParams* params)
{
    // add RenderComponent to each entity by its ID; 
    // so these entities will be rendered onto the screen

    try
    {
        renderSystem_.AddRecords(ids, params, numEntts);
        renderStatesSystem_.AddWithDefaultStates(ids, numEntts);

        using enum eComponentType;
        SetEnttsHaveComponents(ids, { RenderedComponent, RenderStatesComponent }, numEntts);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add rendering component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddMaterialComponent(
    const EntityID enttID,
    const MaterialID* materialsIDs,
    const size numSubmeshes,
    const bool areMaterialsMeshBased)             
{
    // add material component for input entity by ID;
    //
    // in: materialsIDs          -- arr of material IDs (each material ID will be related to a single submesh of the entity)
    //     numSubmeshes          -- how many meshes does input entity have
    //     areMaterialsMeshBased -- defines if all the materials IDs are the same as materials IDs of the related model

    try
    {
        sprintf(g_String, "no entity by ID: %ud", enttID);
        CAssert::True(CheckEnttExist(enttID), g_String);

        materialSystem_.AddRecord(enttID, materialsIDs, numSubmeshes, areMaterialsMeshBased);
        SetEnttHasComponent(enttID, MaterialComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add Material component to entt (id: %ud; name: %s)", enttID, nameSystem_.GetNameById(enttID).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTextureTransformComponent(
    const EntityID id,
    const TexTransformType type,
    const TexTransformInitParams& params)
{
    // add texture transformation to a signle entity by ID
    AddTextureTransformComponent(&id, 1, type, params);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddTextureTransformComponent(
    const EntityID* ids,
    const size numEntts,
    const TexTransformType type,
    const TexTransformInitParams& params)
{
    // set texture transformation of input type for each input entity by ID
    //
    // in:   type     -- what kind of texture transformation we want to apply?
    //       params -- struct of arrays of texture transformations params according to the input type

    try
    {
        texTransformSystem_.AddTexTransformation(ids, numEntts, type, params);
        SetEnttsHaveComponent(ids, numEntts, TextureTransformComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add texture transform component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddLightComponent(
    const EntityID* ids,
    const size numEntts,
    DirLightsInitParams& params)
{
    // add light component (directed light) to each input entity by ID 
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

///////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////

void EntityMgr::AddLightComponent(
    const EntityID* ids,
    const size numEntts,
    SpotLightsInitParams& params)
{
    // add light component to each input entity by ID;
    // 
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

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderStatesComponent(const EntityID id)
{
    // add the RenderStates component to the input entt and setup it with DEFAULT states
    AddRenderStatesComponent(&id, 1);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddRenderStatesComponent(const EntityID* ids, const size numEntts)
{
    // add the RenderStates component to the input entts and 
    // setup each with DEFAULT states
    try
    {
        renderStatesSystem_.AddWithDefaultStates(ids, numEntts);
        SetEnttsHaveComponent(ids, numEntts, RenderStatesComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add render states component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddBoundingComponent(
    const EntityID id,
    const BoundingType type,
    const DirectX::BoundingBox& aabb)
{
    // add bounding component to a single entity
    try
    {
        boundingSystem_.Add(id, type, aabb);
        SetEnttHasComponent(id, BoundingComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add bounding component to entts: %ud", id);
        LogErr(e);
        LogErr(g_String);
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
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add bounding component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddBoundingComponent(
    const EntityID* ids,
    const DirectX::BoundingSphere* spheres,
    const size numEntts)
{
    assert(0 && "FIXME");
    // add bounding spheres to each input entity by ID (input arrays are supposed to be equal)
    try
    {
        //boundingSystem_.Add(ids, numEntts, spheres);
        //SetEnttsHaveComponent(ids, numEntts, BoundingComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add bounding component to entts: %s", GetEnttsIDsAsString(ids, numEntts).c_str());
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddCameraComponent(const EntityID id, const CameraData& data)
{
    // add a camera component to the entity by input ID
    try
    {
        cameraSystem_.AddRecord(id, data);
        SetEnttHasComponent(id, CameraComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add camera component to entts: %ud", id);
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddPlayerComponent(const EntityID id)
{
    // add a player component to the entity by input ID
    try
    {
        playerSystem_.SetPlayer(id);
        SetEnttHasComponent(id, PlayerComponent);
    }
    catch (EngineException& e)
    {
        sprintf(g_String, "can't add a player component to entt: %ud", id);
        LogErr(e);
        LogErr(g_String);
    }
}

#pragma endregion


// ************************************************************************************
//                               PRIVATE HELPERS
// ************************************************************************************

ComponentBitfield EntityMgr::GetHashByComponent(const eComponentType component)
{
    ComponentBitfield bitmask = 0;
    return (bitmask |= (1 << component));
}

///////////////////////////////////////////////////////////

#if 0
ComponentHash EntityMgr::GetHashByComponents(
    const std::vector<eComponentType>& components)
{
    // generate and return a hash by input components

    u32 bitmask = 0;

    for (const eComponentType comp : components)
        bitmask |= (1 << comp);

    return bitmask;
}
#endif

///////////////////////////////////////////////////////////

bool EntityMgr::GetComponentNamesByEntt(const EntityID id, cvector<std::string>& names) const
{
    // out:    names array of components which are added to entity by ID;
    // return: false if there is no entity by ID

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

///////////////////////////////////////////////////////////

bool EntityMgr::GetComponentTypesByEntt(const EntityID id, cvector<uint8_t>& types) const
{
    // out:    names array of component types which are added to entity by ID;
    // return: false if there is no entity by ID

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
