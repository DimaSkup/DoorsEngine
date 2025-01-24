// =================================================================================
// Filename: LightSystem.cpp
// =================================================================================
#include "LightSystem.h"

#include "../Common/MathHelper.h"
#include "../Common/log.h"
#include "../Common/Assert.h"
#include "../Common/Utils.h"
#include "../Common/MathHelper.h"

using namespace Utils;
using namespace DirectX;

namespace ECS 
{

LightSystem::LightSystem(Light* pLightComponent)
{
	Assert::NotNullptr(pLightComponent, "input ptr to the light component == nullptr");
	pLightComponent_ = pLightComponent;
}

LightSystem::~LightSystem()
{
}



// =================================================================================
// public API: creation
// =================================================================================

void LightSystem::AddDirLights(
	const std::vector<EntityID>& ids,
	DirLightsInitParams& params)
{
	// add new directional light sources

	CheckInputParams(ids, params);
	
	Light& comp = *pLightComponent_;
	DirLights& lights = GetDirLights();

	// normalize all the directions before storing
	for (XMFLOAT3& dir : params.directions)
		dir = DirectX::XMFloat3Normalize(dir);

	// execute sorted insertion of new records into the data arrays of the component
	for (const EntityID& id : ids)
		InsertAtPos(comp.ids_, GetPosForID(comp.ids_, id), id);

	// add ids and lights data into the light container
	for (u32 idx = 0; const EntityID& id : ids)
	{
		const ptrdiff_t pos = GetPosForID(lights.ids_, id);

		InsertAtPos(lights.ids_, pos, id);
		InsertAtPos(lights.data_, pos, DirLight(
			params.ambients[idx], 
			params.diffuses[idx], 
			params.speculars[idx], 
			params.directions[idx]));
		
		++idx;
	}
}

///////////////////////////////////////////////////////////

void LightSystem::AddPointLights(
	const std::vector<EntityID>& ids,
	PointLightsInitParams& params)
{
	// create a new point light sources

	Assert::True(CheckCanAddRecords(ids), "can't add point lights: there is already a record with some input entity ID");

	Light& comp = *pLightComponent_;
	PointLights& lights = GetPointLights();

	// execute sorted insertion of new IDs into the component
	for (const EntityID& id : ids)
		InsertAtPos(comp.ids_, GetPosForID(comp.ids_, id), id);

	// add ids and lights data into the light container
	for (u32 idx = 0; const EntityID & id : ids)
	{
		const ptrdiff_t pos = GetPosForID(lights.ids_, id);

		InsertAtPos(lights.ids_,  pos, id);
		InsertAtPos(lights.data_, pos, PointLight(
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
		InsertAtPos(comp.ids_, GetPosForID(comp.ids_, id), id);

	// add ids and lights data into the light container
	for (u32 idx = 0; const EntityID & id : ids)
	{
		const ptrdiff_t pos = GetPosForID(lights.ids_, id);

		InsertAtPos(lights.ids_, pos, id);
		InsertAtPos(lights.data_, pos, SpotLight(
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
// public API: modification
// =================================================================================

void LightSystem::SetDirLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	DirLights& lights = GetDirLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
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
			throw LIB_Exception("unknown type of directed light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetDirLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT3& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	DirLights& lights = GetDirLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);

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
			throw LIB_Exception("unknown type of directed light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetPointLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	PointLights& lights = GetPointLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
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
		default:
		{
			throw LIB_Exception("unknown type of point light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetPointLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT3& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	PointLights& lights = GetPointLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
	PointLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::POSITION:
		{
			light.position_ = value;
			break;
		}
		case LightProps::ATTENUATION:
		{
			light.att_ = value;
			break;
		}
		default:
		{
			throw LIB_Exception("unknown type of point light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetPointLightProp(
	const EntityID id,
	const LightProps prop,
	const float value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	PointLights& lights = GetPointLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);

	// maybe there will be more props of float type so...
	switch (prop)
	{
		case LightProps::RANGE:
		{
			lights.data_[idx].range_ = value;
			break;
		}
		default:
		{
			throw LIB_Exception("unknown type of point light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetSpotLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT4& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	SpotLights& lights = GetSpotLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
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
		default:
		{
			throw LIB_Exception("unknown type of spot light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetSpotLightProp(
	const EntityID id,
	const LightProps prop,
	const XMFLOAT3& value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	SpotLights& lights = GetSpotLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
	SpotLight& light = lights.data_[idx];

	switch (prop)
	{
		case LightProps::POSITION:
		{
			light.position_ = value;
			break;
		}
		case LightProps::DIRECTION:
		{
			light.direction_ = DirectX::XMFloat3Normalize(value);
			break;
		}
		case LightProps::ATTENUATION:
		{
			light.att_ = value;
			break;
		}
		default:
		{
			throw LIB_Exception("unknown type of spot light property: " + std::to_string(prop));
		}
	}
}

///////////////////////////////////////////////////////////

void LightSystem::SetSpotLightProp(
	const EntityID id,
	const LightProps prop,
	const float value)
{
	CheckIdExist(id, "there is no light source by id: " + std::to_string(id));

	SpotLights& lights = GetSpotLights();
	const ptrdiff_t idx = Utils::GetIdxInSortedArr(lights.ids_, id);
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
			throw LIB_Exception("unknown type of spot light property: " + std::to_string(prop));
		}
	}
}




// =================================================================================
// public API: updating of light sources
// =================================================================================

void LightSystem::Update(
	const float deltaTime,
	const float totalGameTime)
{
	UpdateDirLights(deltaTime, totalGameTime);
	UpdatePointLights(deltaTime, totalGameTime);
}

///////////////////////////////////////////////////////////

void LightSystem::UpdateDirLights(
	const float deltaTime,
	const float totalGameTime)
{
	// circle sun light over the land surface

	DirLights& dirLights = GetDirLights();
	size numDirLights = GetLightsNum(ECS::LightTypes::DIRECTIONAL);

	float x = 30.0f * cosf(0.2f * totalGameTime);
	float y = -0.57735f;
	float z = 30.0f * sinf(0.2f * totalGameTime);

	for (ptrdiff_t idx = 0; idx < numDirLights; ++idx)
	{
		SetDirLightProp(dirLights.ids_[idx], ECS::LightProps::DIRECTION, {x,y,z});
	}
}

///////////////////////////////////////////////////////////

void LightSystem::UpdatePointLights(
	const float deltaTime,
	const float totalGameTime)
{
	// update point light props (position, color, etc.)

	PointLights& pointLights = GetPointLights();
	size numPointLights = GetLightsNum(LightTypes::POINT);

	float x = 30.0f * cosf(0.2f * totalGameTime);
	float y = 3;
	float z = 30.0f * sinf(0.2f * totalGameTime);


	SetPointLightProp(pointLights.ids_[0], LightProps::POSITION, { x,y,z });
}

///////////////////////////////////////////////////////////

void LightSystem::UpdateSpotLights(const XMFLOAT3& pos, const XMFLOAT3& dir)
{
	SpotLights& spotLights = GetSpotLights();
	SpotLight& flashlight = spotLights.data_[0];

	// the spotlight takes on the camera position and is aimed in the same direction 
	// the camera is looking. In this way, it looks like we are holding a flashlight
	flashlight.position_ = pos;
	flashlight.direction_ = dir;
}



// =================================================================================
// public API:  query for directional light sources
// =================================================================================

const ptrdiff_t LightSystem::GetLightsNum(const LightTypes type) const
{
	switch (type)
	{
		case DIRECTIONAL:
			return std::ssize(pLightComponent_->dirLights_.ids_);
		case POINT:
			return std::ssize(pLightComponent_->pointLights_.ids_);
		case SPOT:
			return std::ssize(pLightComponent_->spotLights_.ids_);
		default:
			return 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////

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
		canAddRecords &= (!BinarySearch(pLightComponent_->ids_, id));

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


