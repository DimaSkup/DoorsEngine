// =================================================================================
// Filename: LightSystem.cpp
// =================================================================================
#include "../Common/pch.h"
#include "LightSystem.h"

#pragma warning (disable : 4996)
using namespace DirectX;


namespace ECS 
{

LightSystem::LightSystem(Light* pLightComponent, TransformSystem* pTransformSys) :
    pLightComponent_(pLightComponent),
    pTransformSys_(pTransformSys)
{
    CAssert::NotNullptr(pLightComponent, "input ptr to the light component == nullptr");
    CAssert::NotNullptr(pTransformSys,   "input ptr to the Transform system == nullptr");
}

LightSystem::~LightSystem()
{
}


// =================================================================================
// private API: common helpers
// =================================================================================
const char* GenerateMsgNoEntity(const EntityID id, const char* lightTypename)
{
    sprintf(g_String, "there is no %s light entity by ID: %ud", lightTypename, id);
    return g_String;
}

///////////////////////////////////////////////////////////

const char* GenerateMsgUnknownProperty(
    const EntityID id,
    const int propertyCode,
    const char* lightTypename)
{
    sprintf(g_String, "unknown type of %s light property (entt id: %ud, type: propertyCode: %d)", lightTypename, id, propertyCode);
    return g_String;
}

///////////////////////////////////////////////////////////

XMFLOAT4 GetLightPropInvalidData()
{
    // if we ask for wrong light entity of wrong property we just return this data
    return { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
}


// =================================================================================
// public API: creation
// =================================================================================
void LightSystem::AddDirLights(
    const EntityID* ids,
    const size numEntts,
    DirLightsInitParams& params)
{
    // add new directional light sources

    CAssert::True(ids,          "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0, "input number of entitites must be > 0");
    
    Light& comp = *pLightComponent_;
    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of new records into the data arrays of the component
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.types.insert_before(idxs[i], LightType::DIRECTIONAL);

    for (index i = 0; i < numEntts; ++i)
        comp.isActive.insert_before(idxs[i], true);

    // ------------------------------------------

    // add ids and lights data into the light container
    DirLights& lights = GetDirLights();

    lights.ids.get_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        lights.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        lights.data.insert_before(idxs[i], params.data[i]);
}

//---------------------------------------------------------
// Desc:   bind a point light source to each input entity by ID
// Args:   - ids:       arr of entities identifiers
//         - numEntts:  how many entities we have here
//         - params:    SOA of initial params for each input entity
//---------------------------------------------------------
void LightSystem::AddPointLights(
    const EntityID* ids,
    const size numEntts,
    PointLightsInitParams& params)
{
    CAssert::True((ids != nullptr) && (numEntts > 0), "invalid input args");

    Light& comp = *pLightComponent_;
    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of new records into the data arrays of the component
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.types.insert_before(idxs[i], LightType::POINT);

    for (index i = 0; i < numEntts; ++i)
        comp.isActive.insert_before(idxs[i], true);

    // ------------------------------------------

    // add ids and lights data into the light container
    PointLights& lights = GetPointLights();

    lights.ids.get_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        lights.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        lights.data.insert_before(idxs[i], params.data[i]);
}

//---------------------------------------------------------
// Desc:   bind a point light source to a single input entity by ID
// Args:   - id:        entity identifier
//         - initData:  initial params for point light source
//---------------------------------------------------------
void LightSystem::AddPointLight(const EntityID id, const PointLight& initData)
{
    Light& comp = *pLightComponent_;
    index idx = comp.ids.get_insert_idx(id);

    // exec sorted insertion of a new record into each data array
    comp.ids.insert_before(idx, id);
    comp.types.insert_before(idx, LightType::POINT);
    comp.isActive.insert_before(idx, true);

    // add data into the point lights container
    PointLights& lights = GetPointLights();

    idx = lights.ids.get_insert_idx(id);
    lights.ids.insert_before(idx, id);
    lights.data.insert_before(idx, initData);
}

///////////////////////////////////////////////////////////

void LightSystem::AddSpotLights(
    const EntityID* ids,
    const size numEntts,
    SpotLightsInitParams& params)
{
    // create a new spotlight source

    CAssert::True((ids != nullptr) && (numEntts > 0), "invalid input args");

    Light& comp = *pLightComponent_;
    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of new records into the data arrays of the component
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.types.insert_before(idxs[i], LightType::SPOT);

    for (index i = 0; i < numEntts; ++i)
        comp.isActive.insert_before(idxs[i], true);

    // ------------------------------------------

    SpotLights& lights = GetSpotLights();

    lights.ids.get_idxs(ids, numEntts, idxs);

    // exec some index correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of data
    for (index i = 0; i < numEntts; ++i)
        lights.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        lights.data.insert_before(idxs[i], params.data[i]);
}


// =================================================================================
// public API: get/set directed light properties
// =================================================================================
bool LightSystem::GetDirectedLightData(EntityID id, ECS::DirLight& outDirLight) const
{
    // out:    data of a point light source entity by ID
    // return: true - if there is such an entity by ID; false - in another case;

    DirLights& lights = GetDirLights();
    const index idx = GetIdxByID(lights.ids, id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    outDirLight = lights.data[idx];
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetDirLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& value)
{
    DirLights& lights = GetDirLights();
    const index idx   = GetIdxByID(lights.ids, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    DirLight& light = lights.data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    light.ambient  = value; break;
        case LightProp::DIFFUSE:    light.diffuse  = value; break;
        case LightProp::SPECULAR:   light.specular = value; break;

        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "directed"));
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetDirLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT3& val)
{
    DirLights& lights = GetDirLights();
    const index idx = GetIdxByID(lights.ids, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    // maybe there will be more props of XMFLOAT3 type so...
    switch (prop)
    {
        case LightProp::DIRECTION:
        {
            // light direction is stored in the Transform component
            pTransformSys_->SetDirection(id, XMVECTOR{ val.x, val.y, val.z, 1.0f });
            break;
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "directed"));
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetDirLightProp(const EntityID id, const LightProp prop)
{
    // get a property of the directed light entity by ID

    DirLights& lights = GetDirLights();
    const index idx = GetIdxByID(lights.ids, id);

    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(GenerateMsgNoEntity(id, "directed"));
        return GetLightPropInvalidData();
    }

    DirLight& light = lights.data[idx];

    switch (prop)
    {
        case AMBIENT:       return light.ambient;
        case DIFFUSE:       return light.diffuse;
        case SPECULAR:      return light.specular;

        case DIRECTION:
        {
            // get light direction from the Transform component
            XMFLOAT4 dir;
            DirectX::XMStoreFloat4(&dir, pTransformSys_->GetDirectionVec(id));
            return dir;
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "directed"));
            return GetLightPropInvalidData();
        }
    }
}


// =================================================================================
// public API: get/set point light properties
// =================================================================================
void LightSystem::GetPointLightsData(
    const EntityID* ids,
    const size numEntts,
    cvector<ECS::PointLight>& outData,
    cvector<XMFLOAT3>& outPositions) const
{
    // get point light data by the input array of entities IDs

    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,   "input number of entities must be > 0");

    cvector<index> idxs;
    PointLights& lights = GetPointLights();
    lights.ids.get_idxs(ids, numEntts, idxs);

    // get point lights data by indices
    outData.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outData[i++] = lights.data[idx];

    // get point light positions (are stored in the Transform component)
    pTransformSys_->GetPositions(ids, numEntts, outPositions);
}

///////////////////////////////////////////////////////////

void LightSystem::GetSpotLightsData(
    const EntityID* ids,
    const size numEntts,
    cvector<ECS::SpotLight>& outData,
    cvector<XMFLOAT3>& outPositions,
    cvector<XMFLOAT3>& outDirections) const
{
    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    outData.resize(numEntts);

    cvector<index> idxs;
    SpotLights& lights = GetSpotLights();
    lights.ids.get_idxs(ids, numEntts, idxs);

    // get spotlights by idxs
    for (int i = 0; const index idx : idxs)
        outData[i++] = lights.data[idx];

    // get spotlights positions and directions
    pTransformSys_->GetPositionsAndDirections(ids, numEntts, outPositions, outDirections);
}

///////////////////////////////////////////////////////////

bool LightSystem::GetPointLightData(const EntityID id, ECS::PointLight& outPointLight) const
{
    // out:    data of a point light source entity by ID
    // return: true - if there is such an entity by ID; false - in another case;

    PointLights& lights = GetPointLights();
    const index idx = GetIdxByID(lights.ids, id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "point"));
        return false;
    }

    outPointLight = lights.data[idx];
    return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetPointLightProp(const EntityID id, const LightProp prop)
{
    // get a property of the point light entity by ID

    PointLights& lights = GetPointLights();
    const index idx = GetIdxByID(lights.ids, id);
    
    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(GenerateMsgNoEntity(id, "point"));
        return GetLightPropInvalidData();
    }

    PointLight& light = lights.data[idx];

    switch (prop)
    {
        case AMBIENT:       return light.ambient;
        case DIFFUSE:       return light.diffuse;
        case SPECULAR:      return light.specular;

        // get a light position from the Transform component
        case POSITION:
            return pTransformSys_->GetPositionFloat4(id);

        // return the range value in each component of XMFLOAT4
        case RANGE:
        {
            const float r = light.range;
            return { r, r, r, r };               
        }
        case ATTENUATION:
        {
            const XMFLOAT3& att = light.att;
            return { att.x, att.y, att.z, 1.0f };
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "point"));
            return GetLightPropInvalidData();
        }
    }
}
///////////////////////////////////////////////////////////

bool LightSystem::SetPointLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& value)
{
    PointLights& lights = GetPointLights();
    const index idx     = GetIdxByID(lights.ids, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "point"));
        return false;
    }
        
    PointLight& light = lights.data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    light.ambient  = value; break;
        case LightProp::DIFFUSE:    light.diffuse  = value; break;
        case LightProp::SPECULAR:   light.specular = value; break;

        // store light position into the Transform component
        case LightProp::POSITION:
        {
            pTransformSys_->SetPosition(id, XMFLOAT3{ value.x, value.y, value.z });
            break;
        }
        case LightProp::ATTENUATION:
        {
            light.att = { value.x, value.y, value.z };
            break;
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "point"));
            return false;
        }
    }

    // we successfully updated some property of the light entity
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetPointLightProp(
    const EntityID id,
    const LightProp prop,
    const float value)
{
    PointLights& lights = GetPointLights();
    const index idx = GetIdxByID(lights.ids, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "point"));
        return false;
    }

    PointLight& light = lights.data[idx];
        
    // maybe there will be more props of float type so...
    switch (prop)
    {
        case LightProp::RANGE:
            light.range = value;
            break;

        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "point"));
            return false;
        }
    }

    // we successfully updated some property of the light entity
    return true;
}


// =================================================================================
// public API: get/set spotlight properties
// =================================================================================
bool LightSystem::GetSpotLightData(const EntityID id, ECS::SpotLight& outSpotlight) const
{
    // out:    spotlight data of entity by ID
    // return: true - if there is such an entity by ID; false - in another case;

    SpotLights& lights = GetSpotLights();
    const index idx    = GetIdxByID(lights.ids, id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    outSpotlight = lights.data[idx];
    return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetSpotLightProp(const EntityID id, const LightProp prop)
{
    // get a property of the spotlight entity by ID

    SpotLights& lights = GetSpotLights();
    const index idx    = GetIdxByID(lights.ids, id);

    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(GenerateMsgNoEntity(id, "spot"));
        return GetLightPropInvalidData();
    }

    SpotLight& light = lights.data[idx];

    switch (prop)
    {
        case AMBIENT:       return light.ambient;
        case DIFFUSE:       return light.diffuse;
        case SPECULAR:      return light.specular;

        // get a light position from the Transform component
        case POSITION:
            return pTransformSys_->GetPositionFloat4(id);

        case DIRECTION:
        {
            // get light direction from the Transform component
            const XMFLOAT3 dir = pTransformSys_->GetDirection(id);
            return XMFLOAT4{ dir.x, dir.y, dir.z, 0.0f };
        }
        case RANGE:
        {
            // return the range value in each component of XMFLOAT4
            const float r = light.range;
            return { r, r, r, r };
        }
        case ATTENUATION:
        {
            const XMFLOAT3& att = light.att;
            return { att.x, att.y, att.z, 0.0f };
        }
        case SPOT_EXP:  
        {
            // spot exponent : light intensity fallof(for control the spotlight cone)
            const float exp = light.spot;
            return { exp, exp, exp, exp };
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "spot"));
            return GetLightPropInvalidData();
        }
    }
}

///////////////////////////////////////////////////////////

bool LightSystem::SetSpotLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& val)
{
    SpotLights& lights = GetSpotLights();
    const index idx    = GetIdxByID(lights.ids, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    SpotLight& light = lights.data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    light.ambient  = val; break;
        case LightProp::DIFFUSE:    light.diffuse  = val; break;
        case LightProp::SPECULAR:   light.specular = val; break;

        case LightProp::POSITION:
        {
            // store light position into the Transform component
            pTransformSys_->SetPosition(id, XMFLOAT3{ val.x, val.y, val.z });
            break;
        }
        case LightProp::DIRECTION:
        {
            // store light direction into the Transform component
            pTransformSys_->SetDirection(id, { val.x, val.y, val.z, 1.0f });
            break;
        }
        case LightProp::ATTENUATION:
        {
            light.att = { val.x, val.y, val.z };
            break;
        }
        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "spot"));
            return false;
        }
    }

    // we successfully updated some property of the light entity
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetSpotLightProp(
    const EntityID id,
    const LightProp prop,
    const float value)
{
    SpotLights& lights = GetSpotLights();
    const index idx = GetIdxByID(lights.ids, id);
    
    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    SpotLight& light = lights.data[idx];

    switch (prop)
    {
        case LightProp::RANGE:      light.range = value; break;
        case LightProp::SPOT_EXP:   light.spot  = value; break;

        default:
        {
            LogErr(GenerateMsgUnknownProperty(id, prop, "spot"));
            return false;
        }
    }

    // we successfully updated some property of the light entity
    return true;
}


// =================================================================================
// public API: updating of light sources
// =================================================================================

void LightSystem::Update(
    const float deltaTime,
    const float totalGameTime)
{
    //if (GetNumDirLights() > 0)
    //	UpdateDirLights(deltaTime, totalGameTime);

    //if (GetNumPointLights() > 0)
        //UpdatePointLights(deltaTime, totalGameTime);
}

///////////////////////////////////////////////////////////

void LightSystem::UpdateDirLights(
    const float deltaTime,
    const float totalGameTime)
{
    // circle sun light over the land surface

    DirLights& dirLights = GetDirLights();

    DirectX::XMFLOAT3 dir;
    dir.x = 30.0f * cosf(0.2f * totalGameTime);
    dir.y = -0.57735f;
    dir.z = 30.0f * sinf(0.2f * totalGameTime);

    for (index idx = 0; idx < GetNumDirLights(); ++idx)
    {
        SetDirLightProp(dirLights.ids[idx], ECS::LightProp::DIRECTION, dir);
    }
}

///////////////////////////////////////////////////////////

void LightSystem::UpdatePointLights(const float deltaTime, const float totalGameTime)
{
    // update point light props (position, color, etc.)

    PointLights& pointLights = GetPointLights();

    float x = 30.0f * cosf(0.2f * totalGameTime);
    float y = 3;
    float z = 30.0f * sinf(0.2f * totalGameTime);

    SetPointLightProp(pointLights.ids[0], LightProp::POSITION, {x, y, z, 1.0f});
}

///////////////////////////////////////////////////////////

void LightSystem::UpdateFlashlight(const EntityID id, const XMFLOAT3& pos, const XMFLOAT3& dir)
{
    // the spotlight takes on the camera position and is aimed in the same direction 
    // the camera is looking. In this way, it looks like we are holding a flashlight

    pTransformSys_->SetPosition(id, pos);
    pTransformSys_->SetDirection(id, XMVECTOR{ dir.x, dir.y, dir.z, 1.0f });
}

///////////////////////////////////////////////////////////

bool LightSystem::SetLightIsActive(const EntityID id, const bool state)
{
    // turn on/off the light source by ID

    const index idx = GetIdxByID(id);

    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "directed/point/spot"));
        return false;
    }

    pLightComponent_->isActive[idx] = state;
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::IsLightActive(const EntityID id)
{
    // is a light source entity by ID activated?
    
    const index idx = GetIdxByID(id);

    if (idx == -1)
    {
        LogErr(GenerateMsgNoEntity(id, "directed/point/spot"));
        return false;
    }

    return pLightComponent_->isActive[idx];
}


// =================================================================================
// public API:  query 
// =================================================================================

bool LightSystem::GetPointLightsPositionAndRange(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions,
    cvector<float>& outRanges) const
{
    // out: position and range of point light sources by input IDs;
    // NOTE: outData is supposed to be already allocated to size of numEntts

    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts,       "input number of entities must be > 0");

   
    // get position of each point light by ID
    pTransformSys_->GetPositions(ids, numEntts, outPositions);

    // get range of each point light by ID
    const cvector<PointLight>& lights = GetPointLights().data;
    cvector<index> idxs;
    pLightComponent_->ids.get_idxs(ids, numEntts, idxs);

    outRanges.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outRanges[i++] = lights[idx].range;

    return true;
}

};  // namespace ECS
