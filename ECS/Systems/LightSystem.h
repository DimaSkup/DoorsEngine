////////////////////////////////////////////////////////////////////////////////////////////////
// Filename:      LightSystem.h
// Description:   an ECS system to handle light sources
//
// Created:       16.04.22 (moved into ECS at 22.08.24)
////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once


#include "../Components/Light.h"

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


	// Public creation API
	void AddDirLights  (const std::vector<EntityID>& ids, DirLightsInitParams& params);
	void AddPointLights(const std::vector<EntityID>& ids, PointLightsInitParams& params);
	void AddSpotLights (const std::vector<EntityID>& ids, SpotLightsInitParams& params);
		
	// Public update API
	void Update(const float deltaTime, const float totalGameTime);
	void UpdateDirLights(const float deltaTime, const float totalGameTime);
	void UpdatePointLights(const float deltaTime, const float totalGameTime);
	void UpdateSpotLights(const XMFLOAT3& pos, const XMFLOAT3& dir);
	
	// Public modificators API
	void SetDirLightProp(const EntityID id, const LightProps prop, const XMFLOAT4& val);
	void SetDirLightProp(const EntityID id, const LightProps prop, const XMFLOAT3& val);

	void SetPointLightProp(const EntityID id, const LightProps prop, const XMFLOAT4& val);
	void SetPointLightProp(const EntityID id, const LightProps prop, const XMFLOAT3& val);
	void SetPointLightProp(const EntityID id, const LightProps prop, const float val);

	void SetSpotLightProp(const EntityID id, const LightProps prop, const XMFLOAT4& val);
	void SetSpotLightProp(const EntityID id, const LightProps prop, const XMFLOAT3& val);
	void SetSpotLightProp(const EntityID id, const LightProps prop, const float val);


	// Public query API 
	inline DirLights&   GetDirLights()   const { return pLightComponent_->dirLights_; }
	inline PointLights& GetPointLights() const { return pLightComponent_->pointLights_; }
	inline SpotLights&  GetSpotLights()  const { return pLightComponent_->spotLights_; }
	
	const ptrdiff_t GetLightsNum(const LightTypes type) const;

	// memory allocation
	void* operator new(size_t i);
	void operator delete(void* p);

private:
	bool CheckCanAddRecords(const std::vector<EntityID>& ids);
	void CheckInputParams(const std::vector<EntityID>& ids, DirLightsInitParams& params);
	void CheckIdExist(const EntityID id, const std::string& errorMsg);

	Light* pLightComponent_ = nullptr;
};

}; // namespace ECS