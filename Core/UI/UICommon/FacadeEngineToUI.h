// =================================================================================
// Filename:     FacadeEngineToUI.h
// 
// Description:  concrete implementation of the IFacadeEngineToUI interface
// 
// Created:      31.12.24  by DimaSkup
// =================================================================================
#pragma once
#include "IFacadeEngineToUI.h"

#include "../../Camera/Camera.h"
#include "Entity/EntityMgr.h"           // from the ECS module
#include "Render.h"                     // from the Render module

#include <d3d11.h>


namespace UI
{

class FacadeEngineToUI : public IFacadeEngineToUI
{
public:
    using EntityID = uint32_t;
    

private:
    ID3D11DeviceContext* pContext_      = nullptr;
    Render::Render*      pRender_       = nullptr;
    ECS::EntityMgr*      pEntityMgr_    = nullptr;
    //Core::TextureMgr*    pTextureMgr_   = nullptr;
    //Core::ModelMgr*  pModelStorage_ = nullptr;
    Core::Camera*        pEditorCamera_ = nullptr;

public:
    FacadeEngineToUI(
        ID3D11DeviceContext* pContext,
        Render::Render* pRender,
        ECS::EntityMgr* pEntityMgr,
        //Core::TextureMgr* pTextureMgr,
        //Core::ModelMgr* pModelStorage,
        Core::Camera* pEditorCamera);


    virtual ModelID GetModelIdByName(const std::string& name) override;

    // 
    // for using the textures manager
    //
    virtual bool GetShaderResourceViewByTexID(const uint32_t textureID, ID3D11ShaderResourceView*& pSRV) override;


    //
    // get camera info
    //
    virtual void GetCameraViewAndProj(const EntityID camEnttID, float* view, float* proj) override;
    virtual void FocusCameraOnEntity(const EntityID id)                                   override;


    // =============================================================================
    // for the entity editor
    // =============================================================================
    virtual EntityID CreateEntity() override;

    virtual bool GetEntityAddedComponentsNames(const EntityID id, cvector<std::string>& componentsNames) const override;

    virtual bool AddNameComponent     (const EntityID id, const std::string& name)                                          override;
    virtual bool AddTransformComponent(const EntityID id, const Vec3& pos, const Vec3& direction, const float uniformScale) override;
    virtual bool AddModelComponent    (const EntityID enttID, const uint32_t modelID)                                       override;
    virtual bool AddRenderedComponent (const EntityID enttID)                                                               override;
    virtual bool AddBoundingComponent (const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)            override;

    virtual bool     GetAllEnttsIDs   (const EntityID*& outPtrToEnttsIDsArr, int& outNumEntts)                        const override;
    virtual EntityID GetEnttIDByName  (const std::string& name)                                                       const override;
    virtual bool     GetEnttNameByID  (const EntityID id, std::string& outName)                                       const override;

    // extract entities with particular component
    virtual bool GetEnttsOfModelType  (const EntityID*& enttsIDs, int& numEntts)                                            override;
    virtual bool GetEnttsOfCameraType (const EntityID*& enttsIDs, int& numEntts)                                            override;
    virtual bool GetEnttsOfLightType  (const EntityID*& enttsIDs, int& numEntts)                                            override;

    virtual bool GetEnttTransformData (const EntityID id, Vec3& pos, Vec4& rotQuat, float& uniScale)                  const override;
    virtual bool GetEnttWorldMatrix   (const EntityID id, DirectX::XMMATRIX& outMat)                                  const override;

    virtual Vec3 GetEnttPosition    (const EntityID id)                                                               const override;
    virtual Vec4 GetEnttRotationQuat(const EntityID id)                                                               const override;
    virtual float GetEnttScale      (const EntityID id)                                                               const override;

    virtual bool SetEnttPosition    (const EntityID entityID, const Vec3& pos)                                              override;
    virtual bool SetEnttRotationQuat(const EntityID entityID, const Vec4& rotationQuat)                                     override;
    virtual bool SetEnttUniScale    (const EntityID entityID, const float scale)                                            override;

    virtual bool IsEnttLightSource(const EntityID id, int& lightType)                                                 const override;

    // =============================================================================
    // get all the data of light entity by ID
    // =============================================================================
    virtual bool GetEnttDirectedLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& direction) override;

    virtual bool GetEnttPointLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& position,
        float& range,
        Vec3& attenuation) override;

    virtual bool GetEnttSpotLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& position,
        float& range,
        Vec3& direction,
        float& spotExponent,
        Vec3& attenuation) override;

    // =============================================================================
    // set/get directed light props
    // =============================================================================
    virtual bool SetDirectedLightAmbient  (const EntityID id, const ColorRGBA& rgba)    override;
    virtual bool SetDirectedLightDiffuse  (const EntityID id, const ColorRGBA& rgba)    override;
    virtual bool SetDirectedLightSpecular (const EntityID id, const ColorRGBA& rgba)    override;
    virtual bool SetDirectedLightDirection(const EntityID id, const Vec3& dir)          override;

    virtual ColorRGBA GetDirectedLightAmbient  (const EntityID id)                const override;
    virtual ColorRGBA GetDirectedLightDiffuse  (const EntityID id)                const override;
    virtual ColorRGBA GetDirectedLightSpecular (const EntityID id)                const override;
    virtual Vec3      GetDirectedLightDirection(const EntityID id)                const override;

    // =============================================================================
    // set/get point light props
    // =============================================================================
    virtual bool SetPointLightAmbient    (const EntityID id, const ColorRGBA& color)  override;
    virtual bool SetPointLightDiffuse    (const EntityID id, const ColorRGBA& color)  override;
    virtual bool SetPointLightSpecular   (const EntityID id, const ColorRGBA& color)  override;
    virtual bool SetPointLightPos        (const EntityID id, const Vec3& position)    override;
    virtual bool SetPointLightRange      (const EntityID id, const float range)       override;
    virtual bool SetPointLightAttenuation(const EntityID id, const Vec3& attenuation) override;

    virtual ColorRGBA GetPointLightAmbient    (const EntityID id)               const override;
    virtual ColorRGBA GetPointLightDiffuse    (const EntityID id)               const override;
    virtual ColorRGBA GetPointLightSpecular   (const EntityID id)               const override;
    virtual Vec3      GetPointLightPos        (const EntityID id)               const override;
    virtual float     GetPointLightRange      (const EntityID id)               const override;
    virtual Vec3      GetPointLightAttenuation(const EntityID id)               const override;

    // =============================================================================
    // set/get spotlight props
    // =============================================================================
    virtual bool SetSpotLightAmbient     (const EntityID id, const ColorRGBA& color) override;
    virtual bool SetSpotLightDiffuse     (const EntityID id, const ColorRGBA& color) override;
    virtual bool SetSpotLightSpecular    (const EntityID id, const ColorRGBA& color) override;
    virtual bool SetSpotLightPos         (const EntityID id, const Vec3& pos)        override;
    virtual bool SetSpotLightDirection   (const EntityID id, const Vec3& direction)  override;
    virtual bool SetSpotLightAttenuation (const EntityID id, const Vec3& att)        override;
    virtual bool SetSpotLightRange       (const EntityID id, const float range)      override;
    virtual bool SetSpotLightSpotExponent(const EntityID id, const float spotExp)    override;

    virtual ColorRGBA GetSpotLightAmbient     (const EntityID id)              const override;
    virtual ColorRGBA GetSpotLightDiffuse     (const EntityID id)              const override;
    virtual ColorRGBA GetSpotLightSpecular    (const EntityID id)              const override;
    virtual Vec3      GetSpotLightPos         (const EntityID id)              const override;
    virtual Vec3      GetSpotLightDirection   (const EntityID id)              const override;
    virtual Vec3      GetSpotLightAttenuation (const EntityID id)              const override;
    virtual float     GetSpotLightRange       (const EntityID id)              const override;
    virtual float     GetSpotLightSpotExponent(const EntityID id)              const override;


    // =============================================================================
    // for the sky editor
    // =============================================================================
    virtual bool GetSkyData(
        const uint32_t skyEnttID,
        ColorRGB& center,
        ColorRGB& apex,
        Vec3& offset) override;

    virtual bool SetSkyColorCenter(const ColorRGB& color)               override;
    virtual bool SetSkyColorApex(const ColorRGB& color)                 override;
    virtual bool SetSkyOffset(const Vec3& offset)                       override;
    virtual bool SetSkyTexture(const int idx, const uint32_t textureID) override;


    // =============================================================================
    // for the fog editor
    // =============================================================================
    virtual bool GetFogData(ColorRGB& fogColor, float& fogStart, float& fogRange) override;
    virtual bool SetFogParams(const ColorRGB& color, const float start, const float range) override;


    // =============================================================================
    // for debugging
    // =============================================================================
    virtual bool SwitchDebugState(const int debugType) override;


    // =============================================================================
    // for assets manager
    // =============================================================================
    virtual bool GetAssetsNamesList(cvector<std::string>& names) override;
};

} // namespace UI
