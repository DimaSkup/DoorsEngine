// =================================================================================
// Filename: LightSystem.cpp
// =================================================================================
#include "../Common/pch.h"
#include "LightSystem.h"

#pragma warning (disable : 4996)
using namespace DirectX;


namespace ECS
{

//---------------------------------------------------------
// static arrays for internal purposes
//---------------------------------------------------------
static cvector<EntityID> s_Ids;
static cvector<index>    s_Idxs;


//---------------------------------------------------------
// Desc:  light system constructo/destructor
//---------------------------------------------------------
LightSystem::LightSystem(Light* pLightComponent, TransformSystem* pTransformSys) :
    pLightComp_(pLightComponent),
    pTransformSys_(pTransformSys)
{
    CAssert::NotNullptr(pLightComponent, "ptr to LIGHT component == NULL");
    CAssert::NotNullptr(pTransformSys,   "ptr to TRANSFORM system == NULL");
}

LightSystem::~LightSystem()
{
}

// =================================================================================
// private API: common helpers
// =================================================================================
const char* GenerateMsgNoEntity(const EntityID id, const char* lightTypename)
{
    sprintf(g_String, "there is no %s light entity by ID: %" PRIu32, lightTypename, id);
    return g_String;
}

//---------------------------------------------------------

const char* GenerateMsgUnknownProperty(
    const EntityID id,
    const int propertyCode,
    const char* lightTypename)
{
    sprintf(g_String, "unknown type of %s light property (entt id: %" PRIu32 ", type: propertyCode: % d)", lightTypename, id, propertyCode);
    return g_String;
}

//---------------------------------------------------------
// if we ask for wrong light entity of
// wrong property we just return this data
//---------------------------------------------------------
inline XMFLOAT4 GetLightPropInvalidData()
{
    return { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
}

// =================================================================================
// public API: creation
// =================================================================================

//---------------------------------------------------------
// Desc:   bind a directed light source to a single input entity by ID
// Args:   - id:        entity identifier
//         - initData:  initial params for directed light source
//---------------------------------------------------------
void LightSystem::AddDirLight(const EntityID id, const DirLight& initData)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity ID == 0");
        return;
    }

    Light& comp = *pLightComp_;
    index idx = comp.ids.get_insert_idx(id);

    // exec sorted insertion of a new record into each data array
    comp.ids.insert_before(idx, id);
    comp.types.insert_before(idx, LightType::DIRECTED);
    comp.isActive.insert_before(idx, true);

    // add data into the lights container
    DirLights& lights = GetDirLights();

    idx = lights.ids.get_insert_idx(id);
    lights.ids.insert_before(idx, id);
    lights.data.insert_before(idx, initData);
}

//---------------------------------------------------------
// Desc:   bind a point light source to a single input entity by ID
// Args:   - id:        entity identifier
//         - initData:  initial params for point light source
//---------------------------------------------------------
void LightSystem::AddPointLight(const EntityID id, const PointLight& initData)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity ID == 0");
        return;
    }

    Light& comp = *pLightComp_;
    index idx = comp.ids.get_insert_idx(id);

    // exec sorted insertion of a new record into each data array
    comp.ids.insert_before(idx, id);
    comp.types.insert_before(idx, LightType::POINT);
    comp.isActive.insert_before(idx, true);

    // add data into the lights container
    PointLights& lights = GetPointLights();

    idx = lights.ids.get_insert_idx(id);
    lights.ids.insert_before(idx, id);
    lights.data.insert_before(idx, initData);
}

//---------------------------------------------------------
// Desc:   bind a spotlight source to a single input entity by ID
// Args:   - id:        entity identifier
//         - initData:  initial params for spotlight source
//---------------------------------------------------------
void LightSystem::AddSpotLight(const EntityID id, const SpotLight& initData)
{
    if (id == INVALID_ENTITY_ID)
    {
        LogErr(LOG, "input entity ID == 0");
        return;
    }

    Light& comp = *pLightComp_;
    index idx = comp.ids.get_insert_idx(id);

    // exec sorted insertion of a new record into each data array
    comp.ids.insert_before(idx, id);
    comp.types.insert_before(idx, LightType::SPOT);
    comp.isActive.insert_before(idx, true);

    // add data into the lights container
    SpotLights& lights = GetSpotLights();

    idx = lights.ids.get_insert_idx(id);
    lights.ids.insert_before(idx, id);
    lights.data.insert_before(idx, initData);
}

// =================================================================================
// public API: get/set directed light properties
// =================================================================================

//---------------------------------------------------------
// out:    data of a point light source entity by ID
// return: true - if there is such an entity by ID; false - in another case;
//---------------------------------------------------------
bool LightSystem::GetDirectedLightData(EntityID id, ECS::DirLight& outData) const
{
    const index idx = GetDirLightIdx(id);

    // if we didn't find any entt by input Id
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    outData = GetDirLights().data[idx];
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetDirLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& value)
{
    const index idx = GetDirLightIdx(id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    DirLight& L = GetDirLights().data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    L.ambient  = value; break;
        case LightProp::DIFFUSE:    L.diffuse  = value; break;
        case LightProp::SPECULAR:   L.specular = value; break;

        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, prop, "directed"));
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
// setup a property of a directed light by id
//---------------------------------------------------------
bool LightSystem::SetDirLightProp(
    const EntityID id,
    const LightProp propType,
    const XMFLOAT3& val)
{
    const index idx = GetDirLightIdx(id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    if (propType != LightProp::DIRECTION)
    {
        LogErr(LOG, GenerateMsgUnknownProperty(id, propType, "directed"));
        return false;
    }

    // light direction is stored in the Transform component
    pTransformSys_->SetDirection(id, XMVECTOR{ val.x, val.y, val.z, 1.0f });
    return true;
}

//---------------------------------------------------------
// get a property (by input type) of the directed light entity by ID 
//---------------------------------------------------------
XMFLOAT4 LightSystem::GetDirLightProp(const EntityID id, const LightProp prop)
{
    const index idx = GetDirLightIdx(id);

    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(LOG, GenerateMsgNoEntity(id, "directed"));
        return GetLightPropInvalidData();
    }

    DirLight& L = GetDirLights().data[idx];

    switch (prop)
    {
        case AMBIENT:       return L.ambient;
        case DIFFUSE:       return L.diffuse;
        case SPECULAR:      return L.specular;

        case DIRECTION:
        {
            // get light direction from the Transform component
            XMFLOAT4 dir;
            DirectX::XMStoreFloat4(&dir, pTransformSys_->GetDirectionVec(id));
            return dir;
        }
        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, prop, "directed"));
            return GetLightPropInvalidData();
        }
    }
}


// =================================================================================
// public API: get/set point light properties
// =================================================================================

//---------------------------------------------------------
// get point light data by the input array of entities IDs
//---------------------------------------------------------
void LightSystem::GetPointLightsData(
    const EntityID* ids,
    const size numEntts,
    cvector<ECS::PointLight>& outData,
    cvector<XMFLOAT3>& outPositions) const
{
    if (!ids)
    {
        LogErr(LOG, "input IDs arr == NULL");
        return;
    }

    if (numEntts == 0)
    {
        outData.resize(0);
        outPositions.resize(0);
        return;
    }


    pLightComp_->ids.get_idxs(ids, numEntts, s_Idxs);
    s_Ids.resize(numEntts);

    // get only ACTIVE lights
    size numActive = 0;

    for (const index idx : s_Idxs)
    {
        s_Ids[numActive] = pLightComp_->ids[idx];
        numActive       += pLightComp_->isActive[idx];
    }

    outData.resize(numActive);


    // get data of lights
    PointLights& lights = GetPointLights();
    lights.ids.get_idxs(s_Ids.data(), numActive, s_Idxs);

    for (int i = 0; const index idx : s_Idxs)
        outData[i++] = lights.data[idx];

    // get point light positions (are stored separatedly in the Transform component)
    pTransformSys_->GetPositions(s_Ids.data(), numActive, outPositions);
}

//---------------------------------------------------------
// get spotlight sources data by input entities ids
//---------------------------------------------------------
void LightSystem::GetSpotLightsData(
    const EntityID* ids,
    const size numEntts,
    cvector<ECS::SpotLight>& outData,
    cvector<XMFLOAT3>& outPositions,
    cvector<XMFLOAT3>& outDirections) const
{
    if (!ids)
    {
        LogErr(LOG, "input IDs arr == NULL");
        return;
    }

    if (numEntts == 0)
    {
        outData.resize(0);
        outPositions.resize(0);
        return;
    }

    pLightComp_->ids.get_idxs(ids, numEntts, s_Idxs);
    s_Ids.resize(numEntts);

    // get only ACTIVE lights
    size numActive = 0;

    for (const index idx : s_Idxs)
    {
        s_Ids[numActive] = pLightComp_->ids[idx];
        numActive       += pLightComp_->isActive[idx];
    }
    
    outData.resize(numActive);


    // get data of lights
    SpotLights& lights = GetSpotLights();
    lights.ids.get_idxs(s_Ids.data(), numActive, s_Idxs);

    for (int i = 0; const index idx : s_Idxs)
        outData[i++] = lights.data[idx];

    // get spotlights positions and directions
    pTransformSys_->GetPositionsAndDirections(s_Ids.data(), numActive, outPositions, outDirections);
}

//---------------------------------------------------------
// out:    data of a point light source entity by ID
// return: true - if there is such an entity by ID; false - in another case;
//---------------------------------------------------------
bool LightSystem::GetPointLightData(const EntityID id, ECS::PointLight& outData) const
{
    const index idx = GetPointLightIdx(id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "point"));
        return false;
    }

    outData = GetPointLights().data[idx];
    return true;
}

//---------------------------------------------------------
// get a property of the point light entity by ID
//---------------------------------------------------------
XMFLOAT4 LightSystem::GetPointLightProp(const EntityID id, const LightProp propType)
{
    const index idx = GetPointLightIdx(id);
    
    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(LOG, GenerateMsgNoEntity(id, "point"));
        return GetLightPropInvalidData();
    }

    PointLight& L = GetPointLights().data[idx];

    switch (propType)
    {
        case AMBIENT:       return L.ambient;
        case DIFFUSE:       return L.diffuse;
        case SPECULAR:      return L.specular;

        // get a light position from the Transform component
        case POSITION:      return pTransformSys_->GetPositionFloat4(id);

        // return the range value in each component of XMFLOAT4
        case RANGE:         return { L.range, L.range, L.range, L.range };

        case ATTENUATION:   return { L.att.x, L.att.y, L.att.z, 1.0f };

        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, propType, "point"));
            return GetLightPropInvalidData();
        }
    }
}

//---------------------------------------------------------
// set a property of the point light entity by ID
//---------------------------------------------------------
bool LightSystem::SetPointLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& value)
{
    const index idx = GetPointLightIdx(id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "point"));
        return false;
    }
        
    PointLight& L = GetPointLights().data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    L.ambient  = value; break;
        case LightProp::DIFFUSE:    L.diffuse  = value; break;
        case LightProp::SPECULAR:   L.specular = value; break;

        // store light position into the Transform component
        case LightProp::POSITION:
        {
            pTransformSys_->SetPosition(id, XMFLOAT3{ value.x, value.y, value.z });
            break;
        }
        case LightProp::ATTENUATION:
        {
            L.att = { value.x, value.y, value.z };
            break;
        }
        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, prop, "point"));
            return false;
        }
    }

    // we successfully updated some property of the light entity
    return true;
}

//---------------------------------------------------------
// set a property of the point light entity by ID
//---------------------------------------------------------
bool LightSystem::SetPointLightProp(
    const EntityID id,
    const LightProp propType,
    const float value)
{
    const index idx = GetPointLightIdx(id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "point"));
        return false;
    }

    if (propType != LightProp::RANGE)
    {
        LogErr(LOG, GenerateMsgUnknownProperty(id, propType, "point"));
        return false;
    }

    // setup range of point light
    GetPointLights().data[idx].range = value;
    return true;
}


// =================================================================================
// public API: get/set spotlight properties
// =================================================================================

//---------------------------------------------------------
// out:    spotlight data of entity by ID
// return: true - if there is such an entity by ID; false - in another case;
//---------------------------------------------------------
bool LightSystem::GetSpotLightData(const EntityID id, ECS::SpotLight& outData) const
{
    const index idx = GetSpotLightIdx(id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    outData = GetSpotLights().data[idx];
    return true;
}

//---------------------------------------------------------
// get a property of a spotlight entity by ID
//---------------------------------------------------------
XMFLOAT4 LightSystem::GetSpotLightProp(const EntityID id, const LightProp prop)
{
    const index idx = GetSpotLightIdx(id);

    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        LogErr(LOG, GenerateMsgNoEntity(id, "spot"));
        return GetLightPropInvalidData();
    }

    SpotLight& L = GetSpotLights().data[idx];

    switch (prop)
    {
        case AMBIENT:       return L.ambient;
        case DIFFUSE:       return L.diffuse;
        case SPECULAR:      return L.specular;

        case POSITION:
        {
            // get a light position from the Transform component
            return pTransformSys_->GetPositionFloat4(id);
        }
        case DIRECTION:
        {
            // get light direction from the Transform component
            const XMFLOAT3 dir = pTransformSys_->GetDirection(id);
            return { dir.x, dir.y, dir.z, 0.0f };
        }
        case RANGE:
        {
            // return the range value in each component of XMFLOAT4
            return { L.range, L.range, L.range, L.range };
        }
        case ATTENUATION:
        {
            return { L.att.x, L.att.y, L.att.z, 0.0f };
        }
        case SPOT_EXP:  
        {
            // spot exponent : light intensity fallof (for control the spotlight cone)
            return { L.spot, L.spot, L.spot, L.spot };
        }
        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, prop, "spot"));
            return GetLightPropInvalidData();
        }
    }
}

//---------------------------------------------------------
// setup a property of a spotlight entity by id
//---------------------------------------------------------
bool LightSystem::SetSpotLightProp(
    const EntityID id,
    const LightProp prop,
    const XMFLOAT4& val)
{
    const index idx = GetSpotLightIdx(id);

    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    SpotLight& L = GetSpotLights().data[idx];

    switch (prop)
    {
        case LightProp::AMBIENT:    L.ambient  = val; break;
        case LightProp::DIFFUSE:    L.diffuse  = val; break;
        case LightProp::SPECULAR:   L.specular = val; break;

        case LightProp::POSITION:
        {
            // store light position into the Transform component
            pTransformSys_->SetPosition(id, { val.x, val.y, val.z });
            break;
        }
        case LightProp::DIRECTION:
        {
            // store light direction into the Transform component
            pTransformSys_->SetDirection(id, { val.x, val.y, val.z, 1 });
            break;
        }
        case LightProp::ATTENUATION:
        {
            L.att = { val.x, val.y, val.z };
            break;
        }
        default:
        {
            LogErr(LOG, GenerateMsgUnknownProperty(id, prop, "spot"));
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
// setup a property of a spotlight entity by id
//---------------------------------------------------------
bool LightSystem::SetSpotLightProp(
    const EntityID id,
    const LightProp propType,
    const float value)
{
    const index idx = GetSpotLightIdx(id);
    
    // if there is no entity by such ID we cannot update any data so return false
    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "spot"));
        return false;
    }

    SpotLight& L = GetSpotLights().data[idx];

    switch (propType)
    {
        case LightProp::RANGE:      L.range = value; break;
        case LightProp::SPOT_EXP:   L.spot  = value; break;

        default:
            LogErr(LOG, GenerateMsgUnknownProperty(id, propType, "spot"));
            return false;
    }

    return true;
}

//---------------------------------------------------------
// turn on/off a light source by ID
//---------------------------------------------------------
bool LightSystem::SetLightIsActive(const EntityID id, const bool onOff)
{
    const index idx = GetIdxById(id);

    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "directed/point/spot"));
        return false;
    }

    pLightComp_->isActive[idx] = onOff;
    return true;
}

//---------------------------------------------------------
// is a light source entity by ID currently active?
//---------------------------------------------------------
bool LightSystem::IsLightActive(const EntityID id)
{
    const index idx = GetIdxById(id);

    if (idx == -1)
    {
        LogErr(LOG, GenerateMsgNoEntity(id, "directed/point/spot"));
        return false;
    }

    return pLightComp_->isActive[idx];
}


// =================================================================================
// public API:  query 
// =================================================================================

//---------------------------------------------------------
// out: position and range of point light sources by input IDs;
// NOTE: outData is supposed to be already allocated to size of numEntts
//---------------------------------------------------------
bool LightSystem::GetPointLightsPositionAndRange(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions,
    cvector<float>& outRanges) const
{
    if (!ids)
    {
        LogErr(LOG, "input IDs arr == NULL");
        return false;
    }
    if (numEntts <= 0)
    {
        LogErr(LOG, "input number of entities must be > 0");
        return false;
    }


    // get position of each point light by ID
    pTransformSys_->GetPositions(ids, numEntts, outPositions);

    // get range of each point light by ID
    const cvector<PointLight>& lights = GetPointLights().data;
    pLightComp_->ids.get_idxs(ids, numEntts, s_Idxs);

    outRanges.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
        outRanges[i++] = lights[idx].range;

    return true;
}

};  // namespace ECS
