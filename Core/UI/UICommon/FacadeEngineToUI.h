// =================================================================================
// Filename:     FacadeEngineToUI.h
// 
// Description:  concrete implementation of the IFacadeEngineToUI interface
// 
// Created:      31.12.24  by DimaSkup
// =================================================================================
#pragma once
#include "IFacadeEngineToUI.h"

#include "../../Render/CGraphics.h"
#include "Entity/EntityMgr.h"            // from the ECS module
#include <Render/CRender.h>              // from the Render module
#include <Terrain/TerrainGeomipmapped.h>
#include <d3d11.h>


namespace UI
{

class FacadeEngineToUI : public IFacadeEngineToUI
{
private:
    ID3D11DeviceContext*  pContext_      = nullptr;
    Render::CRender*      pRender_       = nullptr;
    ECS::EntityMgr*       pEnttMgr_      = nullptr;
    Core::CGraphics*      pGraphics_     = nullptr;
    Core::TerrainGeomip*  pTerrain_      = nullptr;

public:
    FacadeEngineToUI(
        ID3D11DeviceContext* pContext,
        Render::CRender* pRender,
        ECS::EntityMgr* pEntityMgr,
        Core::CGraphics* pGraphics,
        Core::TerrainGeomip* pTerrain);


    virtual ModelID GetModelIdByName(const char* name) override;
    virtual const cvector<ModelName>* GetModelsNamesArrPtr() const override;


    // =============================================================================
    // Graphics control
    // =============================================================================
    virtual void  SetAntiAliasingType(const uint8 type)       override;
    virtual uint8 GetAntiAliasingType()                 const override;

    //
    // get camera info
    //
    virtual void GetCameraViewAndProj(const EntityID camEnttID, float* view, float* proj) override;
    virtual void FocusCameraOnEntity (const EntityID id)                                   override;


    // =============================================================================
    // for the entity editor
    // =============================================================================
    virtual EntityID CreateEntity() override;

    virtual bool GetEntityAddedComponentsNames(const EntityID id, cvector<std::string>& componentsNames)              const override;
    virtual bool GetEntityAddedComponentsTypes(const EntityID id, cvector<eEnttComponentType>& componentTypes)        const override;

    virtual bool AddNameComponent     (const EntityID id, const char* name)                                                 override;
    virtual bool AddTransformComponent(const EntityID id, const Vec3& pos, const Vec3& dir, const float scale)              override;
    virtual bool AddModelComponent    (const EntityID enttID, const uint32_t modelID)                                       override;
    virtual bool AddRenderedComponent (const EntityID enttID)                                                               override;
    virtual bool AddBoundingComponent (const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)            override;

    virtual bool     GetAllEnttsIDs   (const EntityID*& outPtrToEnttsIDsArr, int& outNumEntts)                        const override;
    virtual EntityID GetEnttIdByName  (const char* name)                                                              const override;
    virtual bool     GetEnttNameById  (const EntityID id, std::string& outName)                                       const override;

    virtual bool GetEnttTransformData (const EntityID id, Vec3& pos, Vec3& dir, Vec4& rotQuat, float& uniScale)       const override;
    virtual bool GetEnttWorldMatrix   (const EntityID id, DirectX::XMMATRIX& outMat)                                  const override;

    virtual Vec3  GetEnttPosition     (const EntityID id)                                                             const override;
    virtual Vec3  GetEnttDirection    (const EntityID id)                                                             const override;
    virtual Vec4  GetEnttRotQuat      (const EntityID id)                                                             const override;
    virtual float GetEnttScale        (const EntityID id)                                                             const override;

    virtual bool SetEnttPosition      (const EntityID entityID, const Vec3& pos)                                            override;
    virtual bool SetEnttDirection     (const EntityID entityID, const Vec3& dir)                                            override;
    virtual bool SetEnttUniScale      (const EntityID entityID, const float scale)                                          override;

    virtual bool RotateEnttByQuat     (const EntityID id, const Vec4& rotQuat) override;

    virtual bool GetEnttLightType     (const EntityID id, int& lightType)                                             const override;

    // =============================================================================
    // get all the data of light entity by ID
    // =============================================================================
    virtual bool GetEnttDirectedLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular) override;

    virtual bool GetEnttPointLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range,
        bool& isActive) override;

    virtual bool GetEnttSpotLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range,
        float& spotExponent) override;

    virtual bool SetLightActive  (const EntityID id, const bool state)       override;
    virtual bool GetLightIsActive(const EntityID id)                   const override;


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
    virtual bool SetSkyTexture(const int idx, const TexID textureID)    override;


    // =============================================================================
    // weather control
    // =============================================================================
    virtual bool GetFogData(ColorRGB& fogColor, float& fogStart, float& fogRange, bool& fogEnabled) override;

    virtual bool SetFogStart  (const float start)      override;
    virtual bool SetFogRange  (const float range)      override;
    virtual bool SetFogEnabled(const bool enabled)     override;
    virtual bool SetFogColor  (const ColorRGB& color)  override;
    
    virtual void  SetWeatherParam(const eWeatherParam param, const float val)       override;
    virtual float GetWeatherParam(const eWeatherParam param)                  const override;


    // =============================================================================
    // for debugging
    // =============================================================================
    virtual bool SwitchDebugState(const int debugType)                                        override;
    virtual void SwitchRenderDebugShape(const eRenderDebugShapes shapeType, const bool state) override;

    virtual void EnableDepthPrepass(const bool state)                            override;
    virtual bool IsEnabledDepthPrepass()                                   const override;

    virtual void EnableDepthVisualize(const bool state)                          override;
    virtual bool IsEnabledDepthVisualize()                                 const override;

    virtual void EnablePostFxs(const bool state)                                 override;
    virtual bool IsPostFxsEnabled()                                        const override;

    virtual void  GetPostFxsQueue  (const void** queue)                          override;
    virtual uint8 GetNumUsedPostFxs()                                      const override;
    virtual void  PushPostFx       (const uint16 type)                           override;
    virtual void  RemovePostFx     (const uint8 orderNum)                        override;
    virtual void  SetPostFxParam   (const uint16 param, const float value)       override;
    virtual float GetPostFxParam   (const uint16 param)                    const override;

    // =============================================================================
    // for assets: models/textures/sounds/etc.
    // =============================================================================
    virtual bool ImportModelFromFile(const char* filePath, const char* modelName)                   const override;
     
    virtual bool LoadTextureFromFile  (const char* path)                                            const override;
    virtual bool ReloadTextureFromFile(const TexID id, const char* path)                            const override;

    virtual bool GetModelsNamesList (cvector<std::string>& names)                                   const override;
    virtual bool GetTextureIdByIdx  (const index idx, TexID& outTextureID)                          const override;
    virtual bool GetMaterialIdByIdx (const index idx, MaterialID& outMatID)                         const override;
    virtual bool GetMaterialNameById(const MaterialID id, char* outName, const int nameMaxLength)   const override;
    virtual bool GetMaterialTexIds  (const MaterialID id, TexID* outTexIds)                         const override;
    virtual bool GetTextureNameById (const TexID id, TexName& outName)                              const override;
    virtual bool GetNumMaterials    (size& numMaterials)                                            const override;

    virtual bool GetMaterialDataById(const MaterialID id, MaterialData& outData)                    const override;

    virtual uint         GetNumRenderStates (const eMaterialPropGroup type)                         const override;
    virtual const char** GetRenderStateNames(const eMaterialPropGroup type)                         const override;

    virtual bool SetMaterialRenderState(
        const MaterialID id,
        const uint32 stateIdx,
        const eMaterialPropGroup type) const override;

    virtual bool SetMaterialColorData(
        const MaterialID id,
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,                      // vec3(specular_color) + float(specular_power)
        const Vec4& reflect) override;

    virtual bool SetTerrainMaterialColors(
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,                      // vec3(specular_color) + float(glossiness)
        const Vec4& reflect) override;


    // get SRV (shader resource view)
    virtual bool GetArrTexturesSRVs(ID3D11ShaderResourceView**& outTexViews, size& outNumViews)             const override;
    virtual bool GetTexViewsByIds  (TexID* texIds, ID3D11ShaderResourceView** outTexViews, size numTexTypes) const override;

    virtual bool SetEnttMaterial(
        const EntityID enttID,
        const SubsetID subsetID,
        const MaterialID matID) override;

    
    virtual bool InitMaterialBigIcon(const uint width, const uint height) const override;

    virtual bool InitMaterialsIcons(
        const size numIcons,
        const int iconWidth,
        const int iconHeight) override;

    virtual bool RenderMaterialsIcons()                                                       const override;
    virtual bool RenderMaterialBigIconById(const MaterialID matID, const float yAxisRotation)       override;


    virtual bool GetShadersIdAndName(cvector<ShaderData>& outData)                    override;
    virtual bool SetMaterialShaderId(const MaterialID matId, const ShaderID shaderId) override;


    //-------------------------------------------
    // model preview configuration (for model editor, or model screenshot tool)
    //-------------------------------------------
    virtual bool InitModelFrameBuf(const uint width, const uint height)                  const override;
    virtual bool RenderModelFrameBuf()                                                         override;

    virtual void  SetModelPreviewParam(const eModelPreviewParams param, const float val)       override;
    virtual float GetModelPreviewParam(const eModelPreviewParams param)                  const override;

    virtual ID3D11ShaderResourceView* GetModelFrameBufView()                             const override;

    virtual void CreateEmptyTexAtlas(
        const uint elemWidth,
        const uint elemHeight,
        const uint numElems) override;

    virtual void PushTexIntoAtlas(ID3D11ShaderResourceView* pSRV) override;
    virtual void SaveTexAtlasToFile(const char* filename) override;
    virtual void ClearTexAtlasMemory() override;

    virtual void SaveTexToFile(
        const char* filename,
        ID3D11ShaderResourceView* pSRV,
        const DXGI_FORMAT targetFormat) override;

    // =============================================================================
    // PARTICLES
    // =============================================================================
    virtual bool SetParticlesColor      (const EntityID id, const ColorRGB& c)         override;
    virtual bool SetParticlesExternForce(const EntityID id, const Vec3& force)         override;
    virtual bool SetParticlesSpawnRate  (const EntityID id, const int num)             override;
    virtual bool SetParticlesLifespanMs (const EntityID id, const int milliseconds)    override;
    virtual bool SetParticlesMass       (const EntityID id, const float mass)          override;
    virtual bool SetParticlesSize       (const EntityID id, const float size)          override;
    virtual bool SetParticlesFriction   (const EntityID id, const float airResistance) override;

    virtual bool GetEnttParticleEmitterData(
        const EntityID id,
        ColorRGB& color,
        Vec3& externForce,
        int& spawnRate,
        int& lifespanMs,
        float& mass,
        float& size,
        float& friction) override;


    // =============================================================================
    // TERRAIN
    // =============================================================================
    virtual int  GetTerrainNumMaxLOD()                                   const override;
    virtual int  GetTerrainDistanceToLOD(const int lod)                  const override;
    virtual bool SetTerrainDistanceToLOD(const int lod, const int dist)        override;


    // =============================================================================
    // GRASS
    // =============================================================================
    virtual float GetGrassDistFullSize()                const override;
    virtual float GetGrassDistVisible()                 const override;

    virtual bool SetGrassDistFullSize(const float dist)       override;
    virtual bool SetGrassDistVisible (const float dist)       override;

};

} // namespace UI
