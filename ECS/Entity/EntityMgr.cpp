#include "EntityMgr.h"

#include "EntityMgrSerializer.h"
#include "EntityMgrDeserializer.h"

#include <cassert>
#include <algorithm>
#include <unordered_map>

//#include <cctype>
//#include <random>
#include <sstream>
#include <format>

namespace ECS
{

// after creation of each new entity this value is increased by 1
int EntityMgr::lastEntityID_ = 1;


EntityMgr::EntityMgr() :
    nameSystem_         { &names_ },
    moveSystem_         { &transform_, &movement_ },
    modelSystem_        { &modelComponent_ },
    renderSystem_       { &renderComponent_ },
    texturesSystem_     { &textureComponent_ },
    texTransformSystem_ { &texTransform_ },
    lightSystem_        { &light_ },
    renderStatesSystem_ { &renderStates_ },
    boundingSystem_     { &bounding_ },
    cameraSystem_       { &camera_ }
{
    Log::Debug("start of entity mgr init");

    constexpr int reserveMemForEnttsCount = 100;

    transformSystem_.Initialize(&transform_);

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
        { TexturedComponent,            "Textured" },
        { TextureTransformComponent,    "Texture transform" },
        { LightComponent,               "Light" },
        { RenderStatesComponent,        "Render states" },
        { BoundingComponent,            "Bounding" },
        { CameraComponent,              "Camera" },
    };

    // add "invalid" entity with ID == 0
    ids_.push_back(INVALID_ENTITY_ID);
    componentHashes_.push_back(0);

    Log::Debug("entity mgr is initialized");
}

///////////////////////////////////////////////////////////

EntityMgr::~EntityMgr()
{
    Log::Debug();
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

std::string GetErrMsg(const std::string& prefix, const EntityID* ids, const size numEntts)
{
    return prefix + GetEnttsIDsAsString(ids, numEntts);
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

cvector<EntityID> EntityMgr::CreateEntities(const int newEnttsCount)
{
    // create batch of new empty entities, generate for each entity 
    // unique ID and set that it hasn't any component by default;
    //
    // return: SORTED array of IDs of just created entities;

    Assert::True(newEnttsCount > 0, "new entitites count cannot be <= 0");

    cvector<EntityID> generatedIDs(newEnttsCount, INVALID_ENTITY_ID);

    // "generate" consistent IDs
    for (EntityID& id : generatedIDs)
        id = lastEntityID_++;

    // append ids and hashes of entities
    ids_.append_vector(generatedIDs);
    componentHashes_.append_vector(cvector<ComponentHash>(newEnttsCount, 0));

    return generatedIDs;
}

///////////////////////////////////////////////////////////

void EntityMgr::DestroyEntities(const EntityID* ids, const size numEntts)
{
    //Assert::True(ids != nullptr, "input ptr to enitites IDs arr == nullptr");
    //Assert::True(numEntts > 0, "input number of entts must be > 0");

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
}

// *********************************************************************************
// 
//                              ADD COMPONENTS 
// 
// *********************************************************************************

#pragma region AddComponentsAPI

void EntityMgr::SetEnttHasComponent(
    const EntityID id,
    const ComponentType compType)
{
    // set that an entity by ID has such a component 
    const index idx = ids_.get_idx(id);
    componentHashes_[idx] |= (1 << compType);
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponent(
    const EntityID* ids,
    const size numEntts,
    const ComponentType compType)
{
    // add the same component to each input entt

    cvector<index> idxs;
    ids_.get_idxs(ids, numEntts, idxs);

    // generate hash mask by input component type
    ComponentHash hashMask = 1 << compType;

    for (const index idx : idxs)
        componentHashes_[idx] |= hashMask;
}

///////////////////////////////////////////////////////////

void EntityMgr::SetEnttsHaveComponents(
    const EntityID* ids,
    const cvector<ComponentType>& compTypes,
    const size numEntts)
{
    // add an arr of components to each input entt

    // get data idx of each input entt
    cvector<index> idxs;
    ids_.get_idxs(ids, numEntts, idxs);

    // generate hash mask by input component types
    ComponentHash hashMask = 0;
    for (const ComponentType type : compTypes)
        hashMask |= (1 << type);

    for (const index idx : idxs)
        componentHashes_[idx] |= hashMask;
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(
    const EntityID& id,
    const EntityName& name)
{
    // add the Name component to a single entity in terms of arrays
    AddNameComponent(&id, &name, 1);
}

///////////////////////////////////////////////////////////

void EntityMgr::AddNameComponent(
    const EntityID* ids,
    const EntityName* names,
    const size numEntts)
{
    // add the Name component to all the input entities
    // so each entity will have its own name
    try
    {
        nameSystem_.AddRecords(ids, names, numEntts);
        SetEnttsHaveComponent(ids, numEntts, NameComponent);
    }
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add a name component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add transform component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add move component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add model component to entts: ";
        errMsg += GetEnttsIDsAsString(enttsIDs, numEntts);
        errMsg += "\nmodel_id: " + std::to_string(modelID);

        Log::Error(e);
        Log::Error(errMsg);
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
        Assert::NotEmpty(enttsIDs.empty(), "the array of entities IDs is empty");
        Assert::NotEmpty(modelsIDs.empty(), "the array of models IDs is empty");

        modelSystem_.AddRecords(enttsIDs, modelsIDs);
        SetEnttsHaveComponent(enttsIDs.data(), std::ssize(enttsIDs), ModelComponent);
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

        using enum ComponentType;
        SetEnttsHaveComponents(ids, { RenderedComponent, RenderStatesComponent }, numEntts);
    }
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add rendering component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

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
    catch (LIB_Exception& e)
    {
        Log::Error(e);
        Log::Error(std::format("can't add textured component to entt (id: {}; entt_name: {})", enttID, nameSystem_.GetNameById(enttID)));
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add texture transform component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add Light component (directional lights) component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add Light component (point lights) component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        std::string errMsg;
        errMsg += "can't add Light component (spot lights) component to entts: ";
        errMsg += GetEnttsIDsAsString(ids, numEntts);

        Log::Error(e);
        Log::Error(errMsg);
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
    catch (LIB_Exception& e)
    {
        Log::Error(e);
        Log::Error(GetErrMsg("can't add render states component to entts: ", ids, numEntts));
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
    }
    catch (LIB_Exception& e)
    {
        Log::Error(e);
        Log::Error(GetErrMsg("can't add bounding component to entts: ", ids, numEntts));
    }
}

///////////////////////////////////////////////////////////

void EntityMgr::AddBoundingComponent(
    const EntityID* ids,
    const DirectX::BoundingSphere* spheres,
    const size numEntts)
{
    // add bounding spheres to each input entity by ID (input arrays are supposed to be equal)
    try
    {
        boundingSystem_.Add(ids, numEntts, spheres);
        SetEnttsHaveComponent(ids, numEntts, BoundingComponent);
    }
    catch (LIB_Exception& e)
    {
        Log::Error(e);
        Log::Error(GetErrMsg("can't add bounding component to entts: ", ids, numEntts));
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
    catch (LIB_Exception& e)
    {
        Log::Error(e);
        Log::Error("can't add the camera component to entt: " + std::to_string(id));
    }
}

#pragma endregion



// ************************************************************************************
// 
//                               PRIVATE HELPERS
// 
// ************************************************************************************


ComponentHash EntityMgr::GetHashByComponent(const ComponentType component)
{
    u32 bitmask = 0;
    return (bitmask |= (1 << component));
}

///////////////////////////////////////////////////////////

#if 0
ComponentHash EntityMgr::GetHashByComponents(
    const std::vector<ComponentType>& components)
{
    // generate and return a hash by input components

    u32 bitmask = 0;

    for (const ComponentType comp : components)
        bitmask |= (1 << comp);

    return bitmask;
}
#endif

///////////////////////////////////////////////////////////

void EntityMgr::GetComponentNamesByEntity(
    const EntityID id,
    cvector<std::string>& names)
{
    const index idx = ids_.get_idx(id);

    for (int i = 0; i < ComponentType::NUM_COMPONENTS; ++i)
    {
        //names.push_back
    }
}



} // namespace ECS
