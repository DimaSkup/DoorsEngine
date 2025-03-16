// =================================================================================
// Filename: LightSystem.cpp
// =================================================================================
#include "LightSystem.h"

#include "../Common/MathHelper.h"
#include "../Common/log.h"
#include "../Common/Assert.h"
#include "../Common/Utils.h"
#include "../Common/MathHelper.h"
//#include <format>

using namespace DirectX;


namespace ECS 
{

LightSystem::LightSystem(Light* pLightComponent) : pLightComponent_(pLightComponent)
{
	Assert::NotNullptr(pLightComponent, "input ptr to the light component == nullptr");
}

LightSystem::~LightSystem()
{
}


// =================================================================================
// private API: common helpers
// =================================================================================
std::string GenerateMsgNoEntity(const EntityID id, const std::string& lightTypename)
{
    return { "there is no " + lightTypename + " light entity by ID: " + std::to_string(id) };
}

///////////////////////////////////////////////////////////

std::string GenerateMsgUnknownProperty(
    const EntityID id,
    const int propertyCode,
    const std::string& lightTypename)
{
    return {
        "unknown type of " + lightTypename +
        " light property (entt id: " + std::to_string(id) +
        ", type: " + std::to_string(propertyCode) + ")" };
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
	const std::vector<EntityID>& ids,
	DirLightsInitParams& params)
{
	// add new directional light sources

	//CheckInputParams(ids, params);
	
	Light& comp = *pLightComponent_;
	DirLights& lights = GetDirLights();

	// normalize all the directions before storing
	for (XMFLOAT3& dir : params.directions)
		dir = DirectX::XMFloat3Normalize(dir);

	// execute sorted insertion of new records into the data arrays of the component
	for (const EntityID& id : ids)
	{
		const index idx = Utils::GetPosForID(comp.ids_, id);
		Utils::InsertAtPos(comp.ids_, idx, id);
		Utils::InsertAtPos(comp.types_, idx, LightTypes::DIRECTIONAL);
		Utils::InsertAtPos(comp.isActive_, idx, true);
	}

	// add ids and lights data into the light container
	for (index i = 0; i < std::ssize(ids); ++i)
	{
		const index pos = Utils::GetPosForID(lights.ids_, ids[i]);

		Utils::InsertAtPos(lights.ids_, pos, ids[i]);
		Utils::InsertAtPos(lights.data_, pos, DirLight(
			params.ambients[i], 
			params.diffuses[i], 
			params.speculars[i], 
			params.directions[i]));
	}
}

///////////////////////////////////////////////////////////

void LightSystem::AddPointLights(
	const std::vector<EntityID>& ids,
	PointLightsInitParams& params)
{
	// create a new point light sources

	Assert::True(CheckCanAddRecords(ids), "can't add point lights: there is already a record with some input entity ID");
	Assert::True(std::ssize(ids) == std::ssize(params.ambients), "the number of ids != the number of data");

	Light& comp = *pLightComponent_;
	PointLights& lights = GetPointLights();

	// execute sorted insertion of new IDs into the component
	for (const EntityID& id : ids)
	{
		const index idx = Utils::GetPosForID(comp.ids_, id);
		Utils::InsertAtPos(comp.ids_, idx, id);
		Utils::InsertAtPos(comp.types_, idx, LightTypes::POINT);
		Utils::InsertAtPos(comp.isActive_, idx, true);
	}

	// add ids and lights data into the light container
	for (u32 idx = 0; const EntityID & id : ids)
	{
		const index pos = Utils::GetPosForID(lights.ids_, id);

		Utils::InsertAtPos(lights.ids_,  pos, id);
		Utils::InsertAtPos(lights.data_, pos, PointLight(
			params.ambients[idx],
			params.diffuses[idx],
			params.speculars[idx],
			params.positions[idx],
			params.ranges[idx],
			params.attenuations[idx]));

		++idx;
	}
}

///////////////////////////////////////////////////////////

void LightSystem::AddSpotLights(
	const std::vector<EntityID>& ids,
	SpotLightsInitParams& params)
{
	// create a new spotlight source

	Assert::True(CheckCanAddRecords(ids), "can't add point lights: there is already a record with some input entity ID");

	Light& comp = *pLightComponent_;
	SpotLights& lights = GetSpotLights();

	// normalize all the directions before storing
	for (XMFLOAT3& dir : params.directions)
		dir = DirectX::XMFloat3Normalize(dir);

	// execute sorted insertion of new IDs into the component
	for (const EntityID& id : ids)
	{
		const index idx = Utils::GetPosForID(comp.ids_, id);
		Utils::InsertAtPos(comp.ids_, idx, id);
		Utils::InsertAtPos(comp.types_, idx, LightTypes::SPOT);
		Utils::InsertAtPos(comp.isActive_, idx, true);
	}

	// add ids and lights data into the light container
	for (u32 idx = 0; const EntityID & id : ids)
	{
		const index pos = Utils::GetPosForID(lights.ids_, id);

		Utils::InsertAtPos(lights.ids_, pos, id);
		Utils::InsertAtPos(lights.data_, pos, SpotLight(
			params.ambients[idx],
			params.diffuses[idx],
			params.speculars[idx],
			params.positions[idx],
			params.ranges[idx],
			params.directions[idx],
			params.spotExponents[idx],
			params.attenuations[idx]));

		++idx;
	}
}


// =================================================================================
// public API: get/set directed light properties
// =================================================================================
bool LightSystem::GetDirectedLightData(EntityID id, ECS::DirLight& outDirLight) const
{
    // out:    data of a point light source entity by ID
    // return: true - if there is such an entity by ID; false - in another case;

    DirLights& lights = GetDirLights();
    const index idx = GetIdxByID(lights.ids_, id);

    // if we didn't find any entt by input ID
    if (idx == -1)
    {
        Log::Error(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

    outDirLight = lights.data_[idx];
    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetDirLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	DirLights& lights = GetDirLights();
	const index idx   = Utils::GetIdxInSortedArr(lights.ids_, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (id == -1)
    {
        Log::Error(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

	DirLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::AMBIENT:
		{
			light.ambient_ = value;
			break;
		}
		case LightProps::DIFFUSE:
		{
			light.diffuse_ = value;
			break;
		}
		case LightProps::SPECULAR:
		{
			light.specular_ = value;
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "directed"));
            return false;
		}
	}

    return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetDirLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT3& value)
{
	DirLights& lights = GetDirLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);

    // if there is no entity by such ID we cannot update any data so return false
    if (id == -1)
    {
        Log::Error(GenerateMsgNoEntity(id, "directed"));
        return false;
    }

	// maybe there will be more props of XMFLOAT3 type so...
	switch (prop)
	{
		case LightProps::DIRECTION:
		{
			lights.data_[idx].direction_ = DirectX::XMFloat3Normalize(value);
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "directed"));
            return false;
		}
	}

    return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetDirLightProp(const EntityID id, const LightProps prop)
{
	// get a property of the directed light entity by ID

    DirLights& lights = GetDirLights();
    const index idx = GetIdxByID(lights.ids_, id);

    if (idx == -1)
    {
        // return invalid data since we didn't find any entity by input ID
        Log::Error(GenerateMsgNoEntity(id, "directed"));
        return GetLightPropInvalidData();
    }

    DirLight& light = lights.data_[idx];

    switch (prop)
    {
        case AMBIENT:
        {
            return light.ambient_;
        }
        case DIFFUSE:
        {
            return light.diffuse_;
        }
        case SPECULAR:
        {
            return light.specular_;
        }
        case DIRECTION:
        {
            const XMFLOAT3& dir = light.direction_;
            return { dir.x, dir.y, dir.z, 0.0f };
        }
        default:
        {
            Log::Error(GenerateMsgUnknownProperty(id, prop, "directed"));
            return GetLightPropInvalidData();
        }
    }
}


// =================================================================================
// public API: get/set point light properties
// =================================================================================
bool LightSystem::GetPointLightData(
	const EntityID* ids,
	ECS::PointLight* outData,
	const int numEntts) const
{
	// get point light data by the input array of entities IDs

	Assert::True((ids != nullptr) && (outData != nullptr) && (numEntts > 0), "invalid input arguments");

	std::vector<index> idxs;
	PointLights& lights = GetPointLights();
	Utils::GetIdxsInSortedArr(lights.ids_, ids, numEntts, idxs);

	for (int i = 0; i < numEntts; ++i)
		outData[i] = lights.data_[idxs[i]];

	return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::GetPointLightData(const EntityID id, ECS::PointLight& outPointLight) const
{
	// out:    data of a point light source entity by ID
	// return: true - if there is such an entity by ID; false - in another case;

	PointLights& lights = GetPointLights();
	const index idx = GetIdxByID(lights.ids_, id);

	// if we didn't find any entt by input ID
	if (idx == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "point"));
		return false;
	}

	outPointLight = lights.data_[idx];
	return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetPointLightProp(const EntityID id, const LightProps prop)
{
	// get a property of the point light entity by ID

	PointLights& lights = GetPointLights();
	const index idx = GetIdxByID(lights.ids_, id);
	
	if (idx == -1)
	{
		// return invalid data since we didn't find any entity by input ID
        Log::Error(GenerateMsgNoEntity(id, "point"));
		return GetLightPropInvalidData();
	}

	PointLight& light = lights.data_[idx];

	switch (prop)
	{
		case AMBIENT:
		{
			return light.ambient_;
		}
		case DIFFUSE:
		{
			return light.diffuse_;
		}
		case SPECULAR:
		{
			return light.specular_;
		}
		case POSITION:
		{
			const XMFLOAT3& pos = light.position_;
			return { pos.x, pos.y, pos.z, 1.0f };
		}
		case RANGE:
		{
			// return the range value in each component of XMFLOAT4
			const float r = light.range_;
			return { r, r, r, r };               
		}
		case ATTENUATION:
		{
			const XMFLOAT3& att = light.att_;
			return { att.x, att.y, att.z, 1.0f };
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "point"));
            return GetLightPropInvalidData();
		}
	}
}
///////////////////////////////////////////////////////////

bool LightSystem::SetPointLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	PointLights& lights = GetPointLights();
	const index idx = GetIdxByID(lights.ids_, id);

	// if there is no entity by such ID we cannot update any data so return false
	if (id == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "point"));
		return false;
	}
		
	PointLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::AMBIENT:
		{
			light.ambient_ = value;
			break;
		}
		case LightProps::DIFFUSE:
		{
			light.diffuse_ = value;
			break;
		}
		case LightProps::SPECULAR:
		{
			light.specular_ = value;
			break;
		}
		case LightProps::POSITION:
		{
			light.position_ = { value.x, value.y, value.z };
			break;
		}
		case LightProps::ATTENUATION:
		{
			light.att_ = { value.x, value.y, value.z };
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "point"));
            return false;
		}
	}

	// we successfully updated some property of the light entity
	return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetPointLightProp(
	const EntityID id,
	const LightProps prop,
	const float value)
{
	PointLights& lights = GetPointLights();
	const index idx = GetIdxByID(lights.ids_, id);

	// if there is no entity by such ID we cannot update any data so return false
	if (id == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "point"));
		return false;
	}

	PointLight& light = lights.data_[idx];
		
	// maybe there will be more props of float type so...
	switch (prop)
	{
		case LightProps::RANGE:
		{
			light.range_ = value;
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "point"));
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
	const index idx = GetIdxByID(lights.ids_, id);

	// if we didn't find any entt by input ID
	if (idx == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "spot"));
		return false;
	}

	outSpotlight = lights.data_[idx];
	return true;
}

///////////////////////////////////////////////////////////

XMFLOAT4 LightSystem::GetSpotLightProp(const EntityID id, const LightProps prop)
{
	// get a property of the spotlight entity by ID

	SpotLights& lights = GetSpotLights();
	const index idx = GetIdxByID(lights.ids_, id);

	if (idx == -1)
	{
		// return invalid data since we didn't find any entity by input ID
        Log::Error(GenerateMsgNoEntity(id, "spot"));
		return GetLightPropInvalidData();
	}

	SpotLight& light = lights.data_[idx];

	switch (prop)
	{
		case AMBIENT:
		{
			return light.ambient_;
		}
		case DIFFUSE:
		{
			return light.diffuse_;
		}
		case SPECULAR:
		{
			return light.specular_;
		}
		case POSITION:
		{
			const XMFLOAT3& pos = light.position_;
			return { pos.x, pos.y, pos.z, 1.0f };
		}
		case DIRECTION:
		{
			const XMFLOAT3& dir = light.direction_;
			return { dir.x, dir.y, dir.z, 0.0f };
		}
		case RANGE:
		{
			// return the range value in each component of XMFLOAT4
			const float r = light.range_;
			return { r, r, r, r };
		}
		case ATTENUATION:
		{
			const XMFLOAT3& att = light.att_;
			return { att.x, att.y, att.z, 0.0f };
		}
		case SPOT_EXP:  
		{
			// spot exponent : light intensity fallof(for control the spotlight cone)
			const float exp = light.spot_;
			return { exp, exp, exp, exp };
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "spot"));
            return GetLightPropInvalidData();
		}
	}
}

///////////////////////////////////////////////////////////

bool LightSystem::SetSpotLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	SpotLights& lights = GetSpotLights();
	const index idx = GetIdxByID(lights.ids_, id);

	// if there is no entity by such ID we cannot update any data so return false
	if (id == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "spot"));
		return false;
	}

	SpotLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::AMBIENT:
		{
			light.ambient_ = value;
			break;
		}
		case LightProps::DIFFUSE:
		{
			light.diffuse_ = value;
			break;
		}
		case LightProps::SPECULAR:
		{
			light.specular_ = value;
			break;
		}
		case LightProps::POSITION:
		{
			light.position_ = { value.x, value.y, value.z };
			break;
		}
		case LightProps::DIRECTION:
		{
			// normalize and store the direction 
			// (input: x,y,z - roll pitch yaw; BUT we need to store it as yaw pitch roll)
            light.direction_ = DirectX::XMFloat3Normalize({ value.x, value.y, value.z });
			//DirectX::XMStoreFloat3(&light.direction_, DirectX::XMVector3Normalize({ value.x, value.y, value.z }));
			break;
		}
		case LightProps::ATTENUATION:
		{
			light.att_ = { value.x, value.y, value.z };
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "spot"));
            return false;
		}
	}

	// we successfully updated some property of the light entity
	return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetSpotLightProp(
	const EntityID id,
	const LightProps prop,
	const float value)
{
	SpotLights& lights = GetSpotLights();
	const index idx = GetIdxByID(lights.ids_, id);
	
	// if there is no entity by such ID we cannot update any data so return false
	if (id == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "spot"));
		return false;
	}

	SpotLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::RANGE:
		{
			light.range_ = value;
			break;
		}
		case LightProps::SPOT_EXP:
		{
			light.spot_ = value;
			break;
		}
		default:
		{
            Log::Error(GenerateMsgUnknownProperty(id, prop, "spot"));
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
		SetDirLightProp(dirLights.ids_[idx], ECS::LightProps::DIRECTION, dir);
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

	SetPointLightProp(pointLights.ids_[0], LightProps::POSITION, {x, y, z, 1.0f});
}

///////////////////////////////////////////////////////////

void LightSystem::UpdateFlashlight(const XMFLOAT3& pos, const XMFLOAT3& dir)
{
	SpotLights& spotLights = GetSpotLights();
	SpotLight& flashlight = spotLights.data_[0];

	// the spotlight takes on the camera position and is aimed in the same direction 
	// the camera is looking. In this way, it looks like we are holding a flashlight
	flashlight.position_ = pos;
	flashlight.direction_ = dir;
}

///////////////////////////////////////////////////////////

bool LightSystem::SetLightIsActive(const EntityID id, const bool state)
{
	// turn on/off the light source by ID

	const index idx = GetIdxByID(id);

	if (idx == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "directed/point/spot"));
		return false;
	}

	pLightComponent_->isActive_[idx] = state;
	return true;
}

///////////////////////////////////////////////////////////

bool LightSystem::IsLightActive(const EntityID id)
{
	// is a light source entity by ID activated?
	
	const index idx = GetIdxByID(id);

	if (idx == -1)
	{
        Log::Error(GenerateMsgNoEntity(id, "directed/point/spot"));
		return false;
	}

	return pLightComponent_->isActive_[idx];
}


// =================================================================================
// public API:  query 
// =================================================================================

bool LightSystem::GetPointLightsPositionAndRange(
	const EntityID* ids,
	ECS::PosAndRange* outData,
	const int numEntts) const
{
	// out: position and range of point light sources by input IDs;
	// NOTE: outData is supposed to be already allocated to size of numEntts

	Assert::True((ids != nullptr) && (outData != nullptr) && (numEntts > 0), "invalid input data");

	Light& comp = *pLightComponent_;
	const PointLight* lights = GetPointLights().data_.data();
	std::vector<index> idxs;

	Utils::GetIdxsInSortedArr(comp.ids_, ids, numEntts, idxs);

	for (int i = 0; i < numEntts; ++i)
	{
		outData[i].position = lights[i].position_;
		outData[i].range = lights[i].range_;
	}

	return true;
}

///////////////////////////////////////////////////////////

void* LightSystem::operator new(size_t i)
{
	if (void* ptr = _aligned_malloc(i, 16))
	{
		return ptr;
	}

	Log::Error("can't allocate the memory for object");
	throw std::bad_alloc{};
}

void LightSystem::operator delete(void* p)
{
	_aligned_free(p);
}


// =================================================================================
// private helpers
// =================================================================================
bool LightSystem::CheckCanAddRecords(const std::vector<EntityID>& ids)
{
	// check if we can add records by IDs
	bool canAddRecords = true;

	for (const EntityID& id : ids)
		canAddRecords &= (!Utils::BinarySearch(pLightComponent_->ids_, id));

	return canAddRecords;
}

///////////////////////////////////////////////////////////

void LightSystem::CheckInputParams(const std::vector<EntityID>& ids, DirLightsInitParams& params)
{
	// here we check if input entts ids and light params are valid

	const size numInputLights = std::ssize(ids);

	Assert::True(CheckCanAddRecords(ids), "can't add directional lights: there is already a record with some input entity ID");
	Assert::True(numInputLights == std::ssize(params.ambients), "wrong number of ambients params");
	Assert::True(numInputLights == std::ssize(params.diffuses), "wrong number of diffuses params");
	Assert::True(numInputLights == std::ssize(params.speculars), "wrong number of speculars params");
	Assert::True(numInputLights == std::ssize(params.directions), "wrong number of directions params");
}

///////////////////////////////////////////////////////////

void LightSystem::CheckIdExist(const EntityID id, const std::string& errorMsg)
{
	// check if there is such an ID in the component;
	// if there is no such ID we throw an exception with errorMsg;

	bool compHasLightByID = Utils::BinarySearch(pLightComponent_->ids_, id);
	Assert::True(compHasLightByID, errorMsg);
}

};


