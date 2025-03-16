////////////////////////////////////////////////////////////////////////////////////////////////
// Filename:      LightSystem.h
// Description:   an ECS system to handle light sources
//
// Created:       22.08.24
////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Components/Light.h"
#include "../Common/Utils.h"

#include <DirectXMath.h>
#include <vector>
#include <type_traits>



namespace ECS 
{

class LightSystem
{
public:
	LightSystem(Light* pLightComponent);
	~LightSystem();

	// restrict a copying of this class instance 
	LightSystem(const LightSystem& obj) = delete;
	LightSystem& operator=(const LightSystem& obj) = delete;


	//
	// Public creation API
	//
	void AddDirLights  (const std::vector<EntityID>& ids, DirLightsInitParams& params);
	void AddPointLights(const std::vector<EntityID>& ids, PointLightsInitParams& params);
	void AddSpotLights (const std::vector<EntityID>& ids, SpotLightsInitParams& params);
		
	//
	// Public update API
	//
	void Update           (const float deltaTime, const float totalGameTime);
	void UpdateDirLights  (const float deltaTime, const float totalGameTime);
	void UpdatePointLights(const float deltaTime, const float totalGameTime);
	void UpdateFlashlight (const XMFLOAT3& pos, const XMFLOAT3& dir);

	// get/set light active state
	bool SetLightIsActive(const EntityID id, const bool state);
	bool IsLightActive   (const EntityID id);
	
	//
	// get/set light properties
	//
	XMFLOAT4 GetDirLightProp  (const EntityID id, const LightProps prop);
	XMFLOAT4 GetPointLightProp(const EntityID id, const LightProps prop);
	XMFLOAT4 GetSpotLightProp (const EntityID id, const LightProps prop);

	bool SetDirLightProp      (const EntityID id, const LightProps prop, const XMFLOAT4& val);
	bool SetDirLightProp      (const EntityID id, const LightProps prop, const XMFLOAT3& val);

	bool SetPointLightProp    (const EntityID id, const LightProps prop, const XMFLOAT4& val);
	bool SetPointLightProp    (const EntityID id, const LightProps prop, const float val);

	bool SetSpotLightProp     (const EntityID id, const LightProps prop, const XMFLOAT4& val);
	bool SetSpotLightProp     (const EntityID id, const LightProps prop, const float val);


	//
	// Public query API 
	//
	inline index GetIdxByID(const EntityID id) const
	{
		// return valid idx if there is an entity by such ID;
		// or return -1 if there is no such entity;
		const std::vector<EntityID>& ids = pLightComponent_->ids_;
		return (Utils::BinarySearch(ids, id)) ? Utils::GetIdxInSortedArr(ids, id) : -1;
	}

	inline index GetIdxByID(const std::vector<EntityID>& ids, const EntityID id) const
	{
		// return valid idx if there is an entity by such ID;
		// or return -1 if there is no such entity;
		return (Utils::BinarySearch(ids, id)) ? Utils::GetIdxInSortedArr(ids, id) : -1;
	}

	inline bool IsLightSource(const EntityID id)        const { return Utils::BinarySearch(pLightComponent_->ids_, id); }
	inline bool IsDirLight(const EntityID id)           const { return Utils::BinarySearch(pLightComponent_->dirLights_.ids_, id); }
	inline bool IsPointLight(const EntityID id)         const { return Utils::BinarySearch(pLightComponent_->pointLights_.ids_, id); }
	inline bool IsSpotLight(const EntityID id)          const { return Utils::BinarySearch(pLightComponent_->spotLights_.ids_, id); }

	inline DirLights&   GetDirLights()                  const { return pLightComponent_->dirLights_; }
	inline PointLights& GetPointLights()                const { return pLightComponent_->pointLights_; }
	inline SpotLights&  GetSpotLights()                 const { return pLightComponent_->spotLights_; }
	inline LightTypes   GetLightType(const EntityID id) const { return pLightComponent_->types_[GetIdxByID(id)]; }


	bool GetPointLightData(const EntityID* ids,	ECS::PointLight* outData, const int numEntts) const;

    bool GetDirectedLightData(const EntityID id, ECS::DirLight& outDirLight) const;
	bool GetPointLightData(const EntityID id, ECS::PointLight& outPointLight) const;
	bool GetSpotLightData (const EntityID id, ECS::SpotLight& outSpotlight) const;

	bool GetPointLightsPositionAndRange(
		const EntityID* ids,
		ECS::PosAndRange* outData, 
		const int numEntts) const;

	const size GetNumDirLights()   const { return std::ssize(pLightComponent_->dirLights_.ids_); }
	const size GetNumPointLights() const { return std::ssize(pLightComponent_->pointLights_.ids_); }
	const size GetNumSpotLights()  const { return std::ssize(pLightComponent_->spotLights_.ids_); }

	// memory allocation
	void* operator new(size_t i);
	void operator delete(void* p);

private:
	//XMFLOAT4 GetLightPropInvalidData();
	bool CheckCanAddRecords(const std::vector<EntityID>& ids);
	void CheckInputParams(const std::vector<EntityID>& ids, DirLightsInitParams& params);
	void CheckIdExist(const EntityID id, const std::string& errorMsg);

	Light* pLightComponent_ = nullptr;
};

}; // namespace ECS
