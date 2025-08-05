////////////////////////////////////////////////////////////////////////////////////////////////
// Filename:      LightSystem.h
// Description:   an ECS system to handle light sources
//
// Created:       22.08.24
////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Components/Light.h"
#include "../Systems/TransformSystem.h"

namespace ECS 
{

class LightSystem
{
public:
    LightSystem(Light* pLightComponent, TransformSystem* pTransformSys);
    ~LightSystem();

    // restrict a copying of this class instance 
    LightSystem(const LightSystem& obj) = delete;
    LightSystem& operator=(const LightSystem& obj) = delete;


    //
    // Public creation API
    //
    void AddDirLights  (const EntityID* ids, const size numEntts, DirLightsInitParams& params);
    void AddPointLights(const EntityID* ids, const size numEntts, PointLightsInitParams& params);
    void AddSpotLights (const EntityID* ids, const size numEntts, SpotLightsInitParams& params);

    void AddPointLight(const EntityID id, const PointLight& initData);

    //
    // Public update API
    //
    void Update           (const float deltaTime, const float totalGameTime);
    void UpdateDirLights  (const float deltaTime, const float totalGameTime);
    void UpdatePointLights(const float deltaTime, const float totalGameTime);
    void UpdateFlashlight (const EntityID id, const XMFLOAT3& pos, const XMFLOAT3& dir);

    // get/set light active state
    bool SetLightIsActive(const EntityID id, const bool state);
    bool IsLightActive   (const EntityID id);
    
    //
    // get/set light properties
    //
    XMFLOAT4 GetDirLightProp  (const EntityID id, const LightProp prop);
    XMFLOAT4 GetPointLightProp(const EntityID id, const LightProp prop);
    XMFLOAT4 GetSpotLightProp (const EntityID id, const LightProp prop);

    bool SetDirLightProp      (const EntityID id, const LightProp prop, const XMFLOAT4& val);
    bool SetDirLightProp      (const EntityID id, const LightProp prop, const XMFLOAT3& val);

    bool SetPointLightProp    (const EntityID id, const LightProp prop, const XMFLOAT4& val);
    bool SetPointLightProp    (const EntityID id, const LightProp prop, const float val);

    bool SetSpotLightProp     (const EntityID id, const LightProp prop, const XMFLOAT4& val);
    bool SetSpotLightProp     (const EntityID id, const LightProp prop, const float val);


    //
    // Public query API 
    //
    inline index GetIdxByID(const EntityID id) const
    {
        // return valid idx if there is an entity by such ID;
        // or return -1 if there is no such entity;
        return pLightComponent_->ids.get_idx(id);
    }

    inline index GetIdxByID(const cvector<EntityID>& ids, const EntityID id) const
    {
        // return valid idx if there is an entity by such ID;
        // or return -1 if there is no such entity;
        const index idx = ids.get_idx(id);
        return (ids[idx] == id) ? idx : -1;
    }

    inline bool IsLightSource(const EntityID id)        const { return pLightComponent_->ids.binary_search(id); }
    inline bool IsDirLight   (const EntityID id)        const { return pLightComponent_->dirLights.ids.binary_search(id); }
    inline bool IsPointLight (const EntityID id)        const { return pLightComponent_->pointLights.ids.binary_search(id); }
    inline bool IsSpotLight  (const EntityID id)        const { return pLightComponent_->spotLights.ids.binary_search(id); }

    inline DirLights&   GetDirLights()                  const { return pLightComponent_->dirLights; }
    inline PointLights& GetPointLights()                const { return pLightComponent_->pointLights; }
    inline SpotLights&  GetSpotLights()                 const { return pLightComponent_->spotLights; }
    inline LightType    GetLightType(const EntityID id) const { return pLightComponent_->types[GetIdxByID(id)]; }

    // get data for multiple entities
    void GetPointLightsData(
        const EntityID* ids,
        const size numEntts,
        cvector<ECS::PointLight>& outData,
        cvector<XMFLOAT3>& outPositions) const;

    void GetSpotLightsData(
        const EntityID* ids,
        const size numEntts,
        cvector<ECS::SpotLight>& outData,
        cvector<XMFLOAT3>& outPositions,
        cvector<XMFLOAT3>& outDirections) const;

    // get data for a single entity
    bool GetDirectedLightData(const EntityID id, ECS::DirLight& outDirLight)     const;
    bool GetPointLightData   (const EntityID id, ECS::PointLight& outPointLight) const;
    bool GetSpotLightData    (const EntityID id, ECS::SpotLight& outSpotlight)   const;

    bool GetPointLightsPositionAndRange(
        const EntityID* ids,
        const size numEntts,
        cvector<XMFLOAT3>& outPositions,
        cvector<float>& outRanges) const;

    const size GetNumDirLights()   const { return pLightComponent_->dirLights.ids.size(); }
    const size GetNumPointLights() const { return pLightComponent_->pointLights.ids.size(); }
    const size GetNumSpotLights()  const { return pLightComponent_->spotLights.ids.size(); }

    // memory allocation
    void* operator new(size_t i);
    void operator delete(void* p);

private:
    Light*           pLightComponent_ = nullptr;
    TransformSystem* pTransformSys_ = nullptr;
};

}; // namespace ECS
