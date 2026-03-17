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
    void AddDirLight  (const EntityID id, const DirLight& initData);
    void AddPointLight(const EntityID id, const PointLight& initData);
    void AddSpotLight (const EntityID id, const SpotLight& initData);

    //
    // Public update API
    //

    // get/set light active state
    bool SetLightIsActive(const EntityID id, const bool onOff);
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
    index GetIdxById        (const EntityID id) const;
    index GetDirLightIdx    (const EntityID id) const;
    index GetPointLightIdx  (const EntityID id) const;
    index GetSpotLightIdx   (const EntityID id) const;
  
    bool IsLightSource      (const EntityID id) const;
    bool IsDirLight         (const EntityID id) const;
    bool IsPointLight       (const EntityID id) const;
    bool IsSpotLight        (const EntityID id) const;

    DirLights&   GetDirLights()                  const;
    PointLights& GetPointLights()                const;
    SpotLights&  GetSpotLights()                 const;
    LightType    GetLightType(const EntityID id) const;

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
    bool GetDirectedLightData(const EntityID id, ECS::DirLight&   outData)  const;
    bool GetPointLightData   (const EntityID id, ECS::PointLight& outData) const;
    bool GetSpotLightData    (const EntityID id, ECS::SpotLight&  outData) const;

    bool GetPointLightsPositionAndRange(
        const EntityID* ids,
        const size numEntts,
        cvector<XMFLOAT3>& outPositions,
        cvector<float>& outRanges) const;

    size GetNumDirLights()   const;
    size GetNumPointLights() const;
    size GetNumSpotLights()  const;

private:
    Light*           pLightComp_ = nullptr;
    TransformSystem* pTransformSys_ = nullptr;
};


//==================================================================================
// inline functions
//==================================================================================

//---------------------------------------------------------
// return valid idx if there is an entity by such ID;
// or return -1 if there is no such entity;
//---------------------------------------------------------
inline index LightSystem::GetIdxById(const EntityID id) const
{
    const index idx = pLightComp_->ids.get_idx(id);

    if (pLightComp_->ids.is_valid_index(idx))
        return idx;

    return -1;
}

//---------------------------------------------------------
// get index of particular light type withing its specific structure
//---------------------------------------------------------
inline index GetIdxFromArrById(const cvector<EntityID>& ids, const EntityID id)
{
    const index idx = ids.get_idx(id);

    if (ids.is_valid_index(idx))
        return idx;

    return -1;
}

inline index LightSystem::GetDirLightIdx(const EntityID id) const
{
    return GetIdxFromArrById(pLightComp_->dirLights.ids, id);
}

inline index LightSystem::GetPointLightIdx(const EntityID id) const
{
    return GetIdxFromArrById(pLightComp_->pointLights.ids, id);
}

inline index LightSystem::GetSpotLightIdx(const EntityID id) const
{
    return GetIdxFromArrById(pLightComp_->spotLights.ids, id);
}

//---------------------------------------------------------
// return a counter of point light sources on the scene
//---------------------------------------------------------
inline size LightSystem::GetNumDirLights(void) const
{
    return pLightComp_->dirLights.ids.size();
}

inline size LightSystem::GetNumPointLights(void) const
{
    return pLightComp_->pointLights.ids.size();
}

inline size LightSystem::GetNumSpotLights(void) const
{
    return pLightComp_->spotLights.ids.size();
}

//---------------------------------------------------------
// check if entity by id represents particular light type
//---------------------------------------------------------
inline bool LightSystem::IsLightSource(const EntityID id) const
{
    return pLightComp_->ids.binary_search(id);
}

inline bool LightSystem::IsDirLight(const EntityID id) const
{
    return pLightComp_->dirLights.ids.binary_search(id);
}

inline bool LightSystem::IsPointLight(const EntityID id) const
{
    return pLightComp_->pointLights.ids.binary_search(id);
}

inline bool LightSystem::IsSpotLight(const EntityID id) const
{
    return pLightComp_->spotLights.ids.binary_search(id);
}

//---------------------------------------------------------
// return dataset of particular light type
//---------------------------------------------------------
inline DirLights& LightSystem::GetDirLights() const
{
    return pLightComp_->dirLights;
}

inline PointLights& LightSystem::GetPointLights() const
{
    return pLightComp_->pointLights;
}

inline SpotLights& LightSystem::GetSpotLights() const
{
    return pLightComp_->spotLights;
}

//---------------------------------------------------------
// get light type of entity by id
//---------------------------------------------------------
inline LightType LightSystem::GetLightType(const EntityID id) const
{
    return pLightComp_->types[GetIdxById(id)];
}

}; // namespace
