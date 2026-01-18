// =================================================================================
// Filename:     FacadeEngineToUI.h
// 
// Description:  concrete implementation of the IFacadeEngineToUI interface
// 
// Created:      31.12.24  by DimaSkup
// =================================================================================
#pragma once
#include "IFacadeEngineToUI.h"

#include <Engine/engine.h>
#include <Render/CGraphics.h>            // from the Core module
#include "Entity/EntityMgr.h"            // from the ECS module
#include <Render/CRender.h>              // from the Render module
#include <Terrain/TerrainGeomipmapped.h>


namespace UI
{

class FacadeEngineToUI : public IFacadeEngineToUI
{
private:
    Core::Engine*         pEngine_       = nullptr;
    ID3D11DeviceContext*  pContext_      = nullptr;
    Render::CRender*      pRender_       = nullptr;
    ECS::EntityMgr*       pEnttMgr_      = nullptr;
    Core::CGraphics*      pGraphics_     = nullptr;
    Core::TerrainGeomip*  pTerrain_      = nullptr;

public:
    FacadeEngineToUI(
        Core::Engine* pEngine,
        ID3D11DeviceContext* pContext,
        Render::CRender* pRender,
        ECS::EntityMgr* pEntityMgr,
        Core::CGraphics* pGraphics,
        Core::TerrainGeomip* pTerrain);

    //-----------------------------------------------------

    virtual ModelID                   GetModelIdByName    (const char* name)       override;
    virtual const cvector<ModelName>* GetModelsNamesArrPtr(void)             const override;


    // =============================================================================
    // Graphics control
    // =============================================================================
    virtual void  SetAntiAliasingType(const uint8 type)       override;
    virtual uint8 GetAntiAliasingType()                 const override;

    //
    // get camera info
    //
    virtual void GetCameraViewAndProj(const EntityID camEnttId, float* outView, float* outProj) override;
    virtual void FocusCameraOnEntity (const EntityID id)                                        override;


    // =============================================================================
    // for the entity editor
    // =============================================================================
    virtual EntityID    CreateEntity(void) override;

    virtual bool        GetEnttAddedComponentsNames (const EntityID id, cvector<std::string>& componentsNames)                  const override;
    virtual bool        GetEnttAddedComponentsTypes (const EntityID id, cvector<eEnttComponentType>& componentTypes)            const override;

    // add ECS components
    virtual bool        AddNameComponent            (const EntityID id, const char* name)                                             override;
    virtual bool        AddTransformComponent       (const EntityID id, const Vec3& pos, const Vec3& dir, const float scale)          override;
    virtual bool        AddModelComponent           (const EntityID enttId, const ModelID modelId)                                    override;
    virtual bool        AddRenderedComponent        (const EntityID enttId)                                                           override;
    virtual bool        AddBoundingComponent        (const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)        override;

    // common
    virtual const cvector<EntityID>* GetAllEnttsIDs (void)                                                                      const override;
    virtual EntityID                 GetEnttIdByName(const char* name)                                                          const override;
    virtual const char*              GetEnttNameById(const EntityID id)                                                         const override;

    // transformations
    virtual bool        GetEnttTransformData        (const EntityID id, Vec3& pos, Vec3& dir, Vec4& rotQuat, float& uniScale)   const override;
    virtual bool        GetEnttWorldMatrix          (const EntityID id, DirectX::XMMATRIX& outMat)                              const override;

    virtual Vec3        GetEnttPosition             (const EntityID id)                                                         const override;
    virtual Vec3        GetEnttDirection            (const EntityID id)                                                         const override;
    virtual Vec4        GetEnttRotQuat              (const EntityID id)                                                         const override;
    virtual float       GetEnttScale                (const EntityID id)                                                         const override;

    virtual bool        SetEnttPosition             (const EntityID entityId, const Vec3& pos)                                        override;
    virtual bool        SetEnttDirection            (const EntityID entityId, const Vec3& dir)                                        override;
    virtual bool        SetEnttUniScale             (const EntityID entityId, const float scale)                                      override;

    virtual bool        RotateEnttByQuat            (const EntityID id, const Vec4& rotQuat)                                          override;


    // animations
    virtual bool                            SetEnttSkeletonAnimation    (const EntityID enttId, const AnimationID animId)             override;

    virtual SkeletonID                      GetEnttSkeletonId           (const EntityID id)                                     const override;
    virtual const char*                     GetSkeletonName             (const SkeletonID id)                                   const override;
    virtual size                            GetSkeletonNumAnimations    (const SkeletonID id)                                   const override;

    virtual const cvector<AnimationName>*   GetSkeletonAnimNames        (const SkeletonID id)                                   const override;
    virtual const char*                     GetSkeletonAnimName         (const SkeletonID skeletonId, const AnimationID animId) const override;

    virtual AnimationID                     GetEnttAnimationId          (const EntityID id)                                     const override;
    virtual float                           GetEnttCurrAnimTime         (const EntityID id)                                     const override;
    virtual float                           GetEnttEndAnimTime          (const EntityID id)                                     const override;


    // =============================================================================
    // get all the data of light entity by ID
    // =============================================================================
    virtual bool GetEnttLightType(const EntityID id, int& lightType) const override;

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

    virtual bool        SetLightActive  (const EntityID id, const bool state)       override;
    virtual bool        GetLightIsActive(const EntityID id)                   const override;


    // =============================================================================
    // set/get directed light props
    // =============================================================================
    virtual bool        SetDirectedLightAmbient  (const EntityID id, const ColorRGBA& rgba)       override;
    virtual bool        SetDirectedLightDiffuse  (const EntityID id, const ColorRGBA& rgba)       override;
    virtual bool        SetDirectedLightSpecular (const EntityID id, const ColorRGBA& rgba)       override;
    virtual bool        SetDirectedLightDirection(const EntityID id, const Vec3& dir)             override;

    virtual ColorRGBA   GetDirectedLightAmbient  (const EntityID id)                        const override;
    virtual ColorRGBA   GetDirectedLightDiffuse  (const EntityID id)                        const override;
    virtual ColorRGBA   GetDirectedLightSpecular (const EntityID id)                        const override;
    virtual Vec3        GetDirectedLightDirection(const EntityID id)                        const override;

    // =============================================================================
    // set/get point light props
    // =============================================================================
    virtual bool        SetPointLightAmbient    (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetPointLightDiffuse    (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetPointLightSpecular   (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetPointLightPos        (const EntityID id, const Vec3& position)         override;
    virtual bool        SetPointLightRange      (const EntityID id, const float range)            override;
    virtual bool        SetPointLightAttenuation(const EntityID id, const Vec3& attenuation)      override;

    virtual ColorRGBA   GetPointLightAmbient    (const EntityID id)                         const override;
    virtual ColorRGBA   GetPointLightDiffuse    (const EntityID id)                         const override;
    virtual ColorRGBA   GetPointLightSpecular   (const EntityID id)                         const override;
    virtual Vec3        GetPointLightPos        (const EntityID id)                         const override;
    virtual float       GetPointLightRange      (const EntityID id)                         const override;
    virtual Vec3        GetPointLightAttenuation(const EntityID id)                         const override;

    // =============================================================================
    // set/get spotlight props
    // =============================================================================
    virtual bool        SetSpotLightAmbient     (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetSpotLightDiffuse     (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetSpotLightSpecular    (const EntityID id, const ColorRGBA& color)       override;
    virtual bool        SetSpotLightPos         (const EntityID id, const Vec3& pos)              override;
    virtual bool        SetSpotLightDirection   (const EntityID id, const Vec3& direction)        override;
    virtual bool        SetSpotLightAttenuation (const EntityID id, const Vec3& att)              override;
    virtual bool        SetSpotLightRange       (const EntityID id, const float range)            override;
    virtual bool        SetSpotLightSpotExponent(const EntityID id, const float spotExp)          override;

    virtual ColorRGBA   GetSpotLightAmbient     (const EntityID id)                         const override;
    virtual ColorRGBA   GetSpotLightDiffuse     (const EntityID id)                         const override;
    virtual ColorRGBA   GetSpotLightSpecular    (const EntityID id)                         const override;
    virtual Vec3        GetSpotLightPos         (const EntityID id)                         const override;
    virtual Vec3        GetSpotLightDirection   (const EntityID id)                         const override;
    virtual Vec3        GetSpotLightAttenuation (const EntityID id)                         const override;
    virtual float       GetSpotLightRange       (const EntityID id)                         const override;
    virtual float       GetSpotLightSpotExponent(const EntityID id)                         const override;


    // =============================================================================
    // sky editor
    // =============================================================================
    virtual bool GetSkyData(
        const uint32_t skyEnttID,
        ColorRGB& center,
        ColorRGB& apex,
        Vec3& offset) override;

    virtual bool        SetSkyColorCenter       (const ColorRGB& color)                               override;
    virtual bool        SetSkyColorApex         (const ColorRGB& color)                               override;
    virtual bool        SetSkyOffset            (const Vec3& offset)                                  override;
    virtual bool        SetSkyTexture           (const int idx, const TexID textureId)                override;


    // =============================================================================
    // weather control
    // =============================================================================
    virtual bool GetFogData(
        ColorRGB& fogColor,
        float& fogStart,
        float& fogRange,
        bool& fogEnabled) override;

    virtual bool        SetFogStart             (const float start)                                   override;
    virtual bool        SetFogRange             (const float range)                                   override;
    virtual bool        SetFogEnabled           (const bool enabled)                                  override;
    virtual bool        SetFogColor             (const ColorRGB& color)                               override;
    
    virtual void        SetWeatherParam         (const eWeatherParam param, const float val)          override;
    virtual float       GetWeatherParam         (const eWeatherParam param)                     const override;


    // =============================================================================
    // for debugging
    // =============================================================================
    virtual bool        SwitchDebugState        (const int debugType)                                 override;
    virtual void        SwitchRenderDebugShape  (const eRenderDbgShape type, const bool state)        override;

    virtual void        EnableDepthPrepass      (const bool state)                                    override;
    virtual bool        IsEnabledDepthPrepass   (void)                                          const override;

    virtual void        EnableDepthVisualize    (const bool state)                                    override;
    virtual bool        IsEnabledDepthVisualize (void)                                          const override;

    // =============================================================================
    // post effects
    // =============================================================================
    virtual void        EnablePostFxs           (const bool state)                                    override;
    virtual bool        IsPostFxsEnabled        (void)                                          const override;

    virtual void        GetPostFxsQueue         (const void** queue)                                  override;
    virtual uint8       GetNumUsedPostFxs       (void)                                          const override;

    virtual void        PushPostFx              (const uint16 type)                                   override;
    virtual void        RemovePostFx            (const uint8 orderNum)                                override;
    virtual void        SetPostFxParam          (const uint16 param, const float value)               override;
    virtual float       GetPostFxParam          (const uint16 param)                            const override;

    // =============================================================================
    // ASSSETS BROWSERS: models/materials/textures/sounds/etc.
    // =============================================================================
    virtual bool        ImportModelFromFile     (const char* path, const char* modelName)       const override;
     
    virtual TexID       LoadTextureFromFile     (const char* path)                              const override;
    virtual bool        ReloadTextureFromFile   (const TexID id, const char* path)              const override;

    virtual bool        GetModelsNamesList      (cvector<std::string>& names)                   const override;
    virtual TexID       GetTextureIdByIdx       (const index idx)                               const override;
    virtual MaterialID  GetMaterialIdByIdx      (const index idx)                               const override;
    virtual const char* GetMaterialNameById     (const MaterialID id)                           const override;
    virtual bool        GetMaterialTexIds       (const MaterialID id, TexID* outTexIds)         const override;
    virtual const char* GetTextureNameById      (const TexID id)                                const override;
    virtual size        GetNumMaterials         (void)                                          const override;

    virtual bool        GetMaterialDataById     (const MaterialID id, MaterialData& outData)    const override;

    virtual uint         GetNumTexTypesNames    (void)                                          const override;
    virtual const char** GetTexTypesNames       (void)                                          const override;
    virtual const char*  GetTexTypeName         (const uint texType)                            const override;

    // get render states info (about rasterizer states, blending states, or depth-stencil states)
    virtual size                            GetNumRenderStates (const eRenderStatesGroup type)  const override;
    virtual const cvector<RenderStateName>* GetRenderStateNames(const eRenderStatesGroup type)  const override;

    // get info specific to blend states
    virtual int          GetNumBlendStateParams  (const eBlendStatePropType type)                const override;
    virtual const char** GetBlendStateParamsNames(const eBlendStatePropType type)                const override;
    virtual const char*  GetBsParamStr           (const BsID id, const eBlendStatePropType type)       override;
    virtual bool         GetBsParamBool          (const BsID id, const eBlendStatePropType type)       override;

    virtual void UpdateCustomBlendState(
        const bool alphaToCoverage,
        const bool independentBlend,
        const bool blendEnabled,
        const char* srcBlend,
        const char* dstBlend,
        const char* blendOp,
        const char* srcBlendAlpha,
        const char* dstBlendAlpha,
        const char* blendOpAlpha,
        const char* writeMask) override;

    // setup render states of material
    virtual uint        GetMaterialRndStateId     (const MaterialID id, const eRenderStatesGroup type) const override;
    virtual bool        SetMaterialRenderState    (const RenderStateSetup& params)                           override;
    virtual bool        SetMaterialRenderStateProp(const RenderStateSetup& params)                           override;


    virtual bool SetMaterialTexture(
        const MaterialID matId,
        const TexID texId,
        const uint texType) const override;

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
    virtual bool GetArrTexturesSRVs         (SRV**& outTexViews, size& outNumViews)                                 const override;
    virtual bool GetTexViewsByIds           (TexID* texIds, SRV** outTexViews, size numTexTypes)                    const override;

    virtual bool SetEnttMaterial            (const EntityID enttId, const SubsetID meshId, const MaterialID matId)        override;

    virtual bool InitMaterialBigIcon        (const uint width, const uint height)                                   const override;
    virtual bool InitMaterialsIcons         (const size numIcons, const int width, const int height)                      override;

    virtual bool RenderMaterialsIcons       (void)                                                                  const override;
    virtual bool RenderMaterialBigIconById  (const MaterialID matID, const float yAxisRotation)                           override;

    virtual void GetShadersIdsAndNames      (cvector<ShaderID>& outIds, cvector<ShaderName>& outNames)              const override;
    virtual bool SetMaterialShaderId        (const MaterialID matId, const ShaderID shaderId)                             override;


    //-------------------------------------------
    // model preview configuration (for model editor, or model screenshot tool)
    //-------------------------------------------
    virtual bool    InitModelFrameBuf       (const uint width, const uint height)                                   const override;
    virtual bool    RenderModelFrameBuf     (void)                                                                        override;

    virtual void    SetModelPreviewParam    (const eModelPreviewParams param, const float val)                            override;
    virtual float   GetModelPreviewParam    (const eModelPreviewParams param)                                       const override;

    virtual SRV*    GetModelFrameBufView    (void)                                                                  const override;

    // make screenshot of model in the tool
    virtual void    CreateEmptyTexAtlas     (const uint elemWidth, const uint elemHeight, const uint numElems)            override;
    virtual void    PushTexIntoAtlas        (SRV* pSRV)                                                                   override;
    virtual void    SaveTexAtlasToFile      (const char* filename)                                                        override;
    virtual void    ClearTexAtlasMemory     (void)                                                                        override;
    virtual void    SaveTexToFile           (const char* filename, SRV* pSRV, const DXGI_FORMAT targetFormat)             override;


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
    virtual int     GetTerrainNumMaxLOD     (void)                              const override;
    virtual int     GetTerrainDistanceToLOD (const int lod)                     const override;
    virtual bool    SetTerrainDistanceToLOD (const int lod, const int dist)           override;


    // =============================================================================
    // GRASS
    // =============================================================================
    virtual float   GetGrassDistFullSize    (void)                              const override;
    virtual float   GetGrassDistVisible     (void)                              const override;

    virtual bool    SetGrassDistFullSize    (const float dist)                        override;
    virtual bool    SetGrassDistVisible     (const float dist)                        override;

};

} // namespace UI
