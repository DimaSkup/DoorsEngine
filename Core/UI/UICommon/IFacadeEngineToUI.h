// =================================================================================
// Filename:      IFacadeEngineToUI.h
// 
// Description:   the Facade class provides an interface to the complex
//                logic of the sereral engine subsystem (render, textures, ECS, etc.)
//                The Facade delegates the client requests to the appropritate
//                objects withing the subsystem. All of this shields the client 
//                from the undesired complexity of the subsystem.
// 
// Created:       31.12.24
// =================================================================================
#pragma once

#include "ui_common_types.h"
#include <enum_weather_params.h>
#include <types.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include "mat_data_container.h"

#include <log.h>
#include "Color.h"

#include <cvector.h>
#include <string>

#include <d3d11.h>
#include <DirectXCollision.h>
#include <dxgiformat.h>


namespace UI
{

//---------------------------------------------------------
// is used when we want to get some string by invalid data
//---------------------------------------------------------
static const char* s_DummyStr = "LMFAO";

//---------------------------------------------------------

class IFacadeEngineToUI
{
private:
    #define NOTIFY LogErr(LOG, "LOL, you forgot to override this virtual method in its child");

public:
    using SRV = ID3D11ShaderResourceView;

    SRV*          pMaterialBigIcon_ = nullptr;      // big material icon for browsing/editing particular chosen material
    cvector<SRV*> materialIcons_;                   // list of icons in the editor material browser

    float         deltaTime = 0.0f;


public:
    virtual ~IFacadeEngineToUI() {};

    virtual ModelID GetModelIdByName(const char* name)                                                                                    { NOTIFY; return 0; }
    virtual const cvector<ModelName>* GetModelsNamesArrPtr()                                                                        const { NOTIFY; return nullptr; }

    // =============================================================================
    // Graphics control
    // =============================================================================
    virtual void SetAntiAliasingType(const uint8 type)                                                                                    { NOTIFY; return; }
    virtual uint8 GetAntiAliasingType()                                                                                             const { NOTIFY; return -1; };

    // =============================================================================
    // get/set camera properties
    // =============================================================================
    virtual void GetCameraViewAndProj(const EntityID camEnttId, float* view, float* proj)                                                 { NOTIFY; return; }
    virtual void FocusCameraOnEntity(const EntityID id)                                                                                   { NOTIFY; return; }
    

    // =============================================================================
    // common methods for entity editor
    // =============================================================================
    virtual EntityID CreateEntity(void) { NOTIFY; return 0; }

    virtual bool GetEnttAddedComponents(
        const EntityID id,
        eEnttComponentType* outAddedComponents,
        size& numAddedComponents) const
    { NOTIFY;  return false; }

    virtual bool        AddNameComponent            (const EntityID enttId, const char* name)                                             { NOTIFY;  return false; }
    virtual bool        AddTransformComponent       (const EntityID enttId, const Vec3& pos, const Vec3& dir, const float scale)          { NOTIFY;  return false; }
    virtual bool        AddModelComponent           (const EntityID enttId, const ModelID modelId)                                        { NOTIFY;  return false; }
    virtual bool        AddRenderedComponent        (const EntityID enttId)                                                               { NOTIFY;  return false; }
    virtual bool        AddBoundingComponent        (const EntityID enttId, const int boundType, const DirectX::BoundingBox& aabb)        { NOTIFY;  return false; }


    virtual const cvector<EntityID>* GetAllEnttsIDs (void)                                                                          const { NOTIFY;  return nullptr; }
    virtual EntityID                 GetEnttIdByName(const char* name)                                                              const { NOTIFY;  return INVALID_ENTT_ID; }
    virtual const char*              GetEnttNameById(const EntityID id)                                                             const { NOTIFY;  return s_DummyStr; }


    // extract entities with particular component
    virtual bool        GetEnttTransformData        (const EntityID id, Vec3& pos, Vec3& dir, Vec4& rotQuat, float& uniScale)       const { NOTIFY;  return false; }
    virtual bool        GetEnttWorldMatrix          (const EntityID id, DirectX::XMMATRIX& outMat)                                  const { NOTIFY;  return false; }


    // get/set entity position/direction/uniform_scale
    virtual Vec3        GetEnttPosition             (const EntityID id)                                                             const { NOTIFY;  return GetInvalidVec3(); }
    virtual Vec3        GetEnttDirection            (const EntityID id)                                                             const { NOTIFY;  return GetInvalidVec3(); }
    virtual Vec4        GetEnttRotQuat              (const EntityID id)                                                             const { NOTIFY;  return GetInvalidVec4(); }
    virtual float       GetEnttScale                (const EntityID id)                                                             const { NOTIFY;  return GetInvalidFloat(); }

    virtual bool        SetEnttPosition             (const EntityID id, const Vec3& pos)                                                  { NOTIFY;  return false; }
    virtual bool        SetEnttDirection            (const EntityID id, const Vec3& dir)                                                  { NOTIFY;  return false; }
    virtual bool        SetEnttScale                (const EntityID id, const float scale)                                                { NOTIFY;  return false; }

    virtual bool        RotateEnttByQuat            (const EntityID id, const Vec4& rotQuat)                                              { NOTIFY;  return false; }
    

    // animations
    virtual bool                            SetEnttSkeletonAnimation  (const EntityID enttId, const AnimationID animId)                   { NOTIFY; return false; }

    virtual SkeletonID                      GetEnttSkeletonId         (const EntityID id)                                           const { NOTIFY; return 0; }
    virtual const char*                     GetSkeletonName           (const SkeletonID id)                                         const { NOTIFY; return s_DummyStr; }
    virtual size                            GetSkeletonNumAnimations  (const SkeletonID id)                                         const { NOTIFY; return 0; }
    virtual const cvector<AnimationName>*   GetSkeletonAnimNames      (const SkeletonID id)                                         const { NOTIFY; return nullptr; }
    virtual const char*                     GetSkeletonAnimName       (const SkeletonID skeletonId, const AnimationID animId)       const { NOTIFY; return s_DummyStr; }

    virtual AnimationID                     GetEnttAnimationId        (const EntityID id)                                           const { NOTIFY; return 0; }
    virtual float                           GetEnttCurrAnimTime       (const EntityID id)                                           const { NOTIFY; return 0; };
    virtual float                           GetEnttEndAnimTime        (const EntityID id)                                           const { NOTIFY; return 0; };
    

    // =============================================================================
    // get all the data of light entity by ID
    // =============================================================================
    virtual bool GetEnttLightType(const EntityID id, int& lightType) const { NOTIFY;  return false; }

    virtual bool GetEnttDirectedLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular)
    {
        NOTIFY;
        return false;
    }

    virtual bool GetEnttPointLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range,
        bool& isActive)
    {
        NOTIFY;
        return false;
    }

    virtual bool GetEnttSpotLightData(
        const EntityID id,
        ColorRGBA& ambient,
        ColorRGBA& diffuse,
        ColorRGBA& specular,
        Vec3& attenuation,
        float& range,
        float& spotExponent)
    {
        NOTIFY;
        return false;
    }

    virtual bool SetLightActive  (const EntityID id, const bool state)       { NOTIFY; return false; }
    virtual bool GetLightIsActive(const EntityID id)                   const { NOTIFY; return false; }


    // =============================================================================
    // set/get directed light props
    // =============================================================================
    virtual bool        SetDirectedLightAmbient  (const EntityID id, const ColorRGBA& rgba)       { NOTIFY;  return false; }
    virtual bool        SetDirectedLightDiffuse  (const EntityID id, const ColorRGBA& rgba)       { NOTIFY;  return false; }
    virtual bool        SetDirectedLightSpecular (const EntityID id, const ColorRGBA& rgba)       { NOTIFY;  return false; }
    virtual bool        SetDirectedLightDirection(const EntityID id, const Vec3& dir)             { NOTIFY;  return false; }

    virtual ColorRGBA   GetDirectedLightAmbient  (const EntityID id)                        const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetDirectedLightDiffuse  (const EntityID id)                        const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetDirectedLightSpecular (const EntityID id)                        const { NOTIFY;  return GetInvalidRGBA(); }
    virtual Vec3        GetDirectedLightDirection(const EntityID id)                        const { NOTIFY;  return GetInvalidVec3(); }


    // =============================================================================
    // set/get point light props
    // =============================================================================
    virtual bool        SetPointLightAmbient    (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; }
    virtual bool        SetPointLightDiffuse    (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; }
    virtual bool        SetPointLightSpecular   (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; }
    virtual bool        SetPointLightPos        (const EntityID id, const Vec3& pos)              { NOTIFY;  return false; }
    virtual bool        SetPointLightRange      (const EntityID id, const float range)            { NOTIFY;  return false; }
    virtual bool        SetPointLightAttenuation(const EntityID id, const Vec3& attenuation)      { NOTIFY;  return false; }

    virtual ColorRGBA   GetPointLightAmbient    (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetPointLightDiffuse    (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetPointLightSpecular   (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual Vec3        GetPointLightPos        (const EntityID id)                         const { NOTIFY;  return GetInvalidVec3(); }
    virtual Vec3        GetPointLightAttenuation(const EntityID id)                         const { NOTIFY;  return GetInvalidVec3(); }
    virtual float       GetPointLightRange      (const EntityID id)                         const { NOTIFY;  return GetInvalidFloat(); }


    // =============================================================================
    // set/get spotlight props
    // =============================================================================
    virtual bool        SetSpotLightAmbient     (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; };
    virtual bool        SetSpotLightDiffuse     (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; };
    virtual bool        SetSpotLightSpecular    (const EntityID id, const ColorRGBA& color)       { NOTIFY;  return false; };
    virtual bool        SetSpotLightPos         (const EntityID id, const Vec3& pos)              { NOTIFY;  return false; };
    virtual bool        SetSpotLightDirection   (const EntityID id, const Vec3& direction)        { NOTIFY;  return false; };
    virtual bool        SetSpotLightAttenuation (const EntityID id, const Vec3& att)              { NOTIFY;  return false; };
    virtual bool        SetSpotLightRange       (const EntityID id, const float range)            { NOTIFY;  return false; };
    virtual bool        SetSpotLightSpotExponent(const EntityID id, const float spotExp)          { NOTIFY;  return false; };

    virtual ColorRGBA   GetSpotLightAmbient     (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetSpotLightDiffuse     (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual ColorRGBA   GetSpotLightSpecular    (const EntityID id)                         const { NOTIFY;  return GetInvalidRGBA(); }
    virtual Vec3        GetSpotLightPos         (const EntityID id)                         const { NOTIFY;  return GetInvalidVec3(); }
    virtual Vec3        GetSpotLightDirection   (const EntityID id)                         const { NOTIFY;  return GetInvalidVec3(); }
    virtual Vec3        GetSpotLightAttenuation (const EntityID id)                         const { NOTIFY;  return GetInvalidVec3(); }
    virtual float       GetSpotLightRange       (const EntityID id)                         const { NOTIFY;  return GetInvalidFloat(); }
    virtual float       GetSpotLightSpotExponent(const EntityID id)                         const { NOTIFY;  return GetInvalidFloat(); }


    // =============================================================================
    // sky editor
    // =============================================================================
    virtual bool GetSkyData(
        const EntityID skyEnttId,
        ColorRGB& center,
        ColorRGB& apex,
        Vec3& offset)
    { NOTIFY;  return false; }

    virtual bool        SetSkyColorCenter       (const ColorRGB& color)                           { NOTIFY;  return false; }
    virtual bool        SetSkyColorApex         (const ColorRGB& color)                           { NOTIFY;  return false; }
    virtual bool        SetSkyOffset            (const Vec3& offset)                              { NOTIFY;  return false; }
    virtual bool        SetSkyTexture           (const int idx, const TexID textureID)            { NOTIFY;  return false; }


    // =============================================================================
    // weather control
    // =============================================================================
    virtual bool GetFogData(
        ColorRGB& fogColor,
        float& fogStart,
        float& fogRange,
        bool& fogEnabled)
    { NOTIFY; return false; }

    virtual bool        SetFogStart             (const float startDist)                           { NOTIFY;  return false; }
    virtual bool        SetFogRange             (const float range)                               { NOTIFY;  return false; }
    virtual bool        SetFogEnabled           (const bool onOff)                                { NOTIFY;  return false; }
    virtual bool        SetFogColor             (const ColorRGB& color)                           { NOTIFY;  return false; }

    virtual void        SetWeatherParam         (const eWeatherParam prop, const float val)       { NOTIFY; return; }
    virtual float       GetWeatherParam         (const eWeatherParam prop)                  const { NOTIFY; return FLT_MIN; }
    

    // =============================================================================
    // for debug
    // =============================================================================
    virtual bool        SwitchDebugState        (const int debugType)                             { NOTIFY;  return false; }
    virtual void        SwitchRenderDebugShape  (const eRenderDbgShape type, const bool state)    { NOTIFY;  return; }

    virtual void        EnableDepthPrepass      (const bool state)                                { NOTIFY; return; }
    virtual bool        IsEnabledDepthPrepass   (void)                                      const { NOTIFY; return false; }

    virtual void        SetFullscreenInGameMode (const bool state)                                { NOTIFY; return; }
    virtual bool        IsFullscreenInGameMode  (void)                                      const { NOTIFY; return false; }

    virtual void        LockFrustumCulling      (const bool onOff)                                { NOTIFY; return; }
    virtual bool        IsLockedFrustumCulling  (void)                                      const { NOTIFY; return false; }

    virtual void        EnableDepthVisualize    (const bool state)                                { NOTIFY; return; }
    virtual bool        IsEnabledDepthVisualize (void)                                      const { NOTIFY; return false;}


    // =============================================================================
    // post effects
    // =============================================================================
    virtual void        EnablePostFxs           (const bool state)                                { NOTIFY; return; }
    virtual bool        IsPostFxsEnabled        (void)                                      const { NOTIFY; return false; }

    virtual void        GetPostFxsQueue         (const void** queue)                              { NOTIFY; return; }
    virtual uint8       GetNumUsedPostFxs       (void)                                      const { NOTIFY; return 0; }

    virtual void        PushPostFx              (const uint16 type)                               { NOTIFY; return; }
    virtual void        RemovePostFx            (const uint8 orderNum)                            { NOTIFY; return; }
    virtual void        SetPostFxParam          (const uint16 param, const float value)           { NOTIFY; return; }
    virtual float       GetPostFxParam          (const uint16 param)                        const { NOTIFY; return 0; }


    // =============================================================================
    // for assets managers: models/textures/materials/sounds/etc.
    // =============================================================================
    virtual bool        ImportModelFromFile     (const char* path, const char* modelName)   const { NOTIFY;  return false; }

    virtual TexID       LoadTextureFromFile     (const char* path)                          const { NOTIFY;  return 0; }
    virtual bool        ReloadTextureFromFile   (const TexID id, const char* path)          const { NOTIFY;  return false; }

    virtual bool        GetModelsNamesList      (cvector<std::string>& names)               const { NOTIFY;  return false; }
    virtual TexID       GetTextureIdByIdx       (const index idx)                           const { NOTIFY;  return INVALID_TEX_ID; }
    virtual MaterialID  GetMaterialIdByIdx      (const index idx)                           const { NOTIFY;  return INVALID_MAT_ID; }
    virtual const char* GetMaterialNameById     (const MaterialID id)                       const { NOTIFY;  return s_DummyStr; }
    virtual bool        GetMaterialTexIds       (const MaterialID id, TexID* outTexIds)     const { NOTIFY;  return false; }
    virtual const char* GetTextureNameById      (const TexID id)                            const { NOTIFY;  return s_DummyStr; }
    virtual size        GetNumMaterials         (void)                                      const { NOTIFY;  return 0; }

    virtual bool        GetMaterialDataById     (const MaterialID id, MaterialData& data)   const { NOTIFY;  return false; }

    virtual uint         GetNumTexTypesNames    (void)                                      const { NOTIFY; return 0; }
    virtual const char** GetTexTypesNames       (void)                                      const { NOTIFY; return &s_DummyStr; }
    virtual const char*  GetTexTypeName         (const uint texType)                        const { NOTIFY; return s_DummyStr; }


    // get render states info (about rasterizer states, blending states, or depth-stencil states)
    virtual size                            GetNumRenderStates (const eRndStatesGroup type)             const { NOTIFY; return 0; }
    virtual const cvector<RenderStateName>* GetRenderStateNames(const eRndStatesGroup type)             const { NOTIFY; return nullptr; }

    // get info specific to rasterizer states
    virtual int          GetNumRsParams             (const eRasterProp type)                            const { NOTIFY; return 0; }
    virtual const char** GetRsParamsNames           (const eRasterProp type)                            const { NOTIFY; return &s_DummyStr; }
    virtual const char*  GetRsParamStr              (const RsID id, const eRasterProp type)             const { NOTIFY; return s_DummyStr; }
    virtual int          GetRsParamInt              (const RsID id, const eRasterProp type)             const { NOTIFY; return 0; }
    virtual float        GetRsParamFloat            (const RsID id, const eRasterProp type)             const { NOTIFY; return 0.0f; }
    virtual bool         GetRsParamBool             (const RsID id, const eRasterProp type)             const { NOTIFY; return false; }

    // get info specific to blend states
    virtual int          GetNumBsParams             (const eBlendProp type)                             const { NOTIFY; return 0; }
    virtual const char** GetBsParamsNames           (const eBlendProp type)                             const { NOTIFY; return &s_DummyStr; }
    virtual const char*  GetBsParamStr              (const BsID id, const eBlendProp type)              const { NOTIFY; return s_DummyStr; }
    virtual bool         GetBsParamBool             (const BsID id, const eBlendProp type)              const { NOTIFY; return false; }

    // get info specific to depth-stencil states
    virtual int          GetNumDssParams            (const eDepthStencilProp type)                      const { NOTIFY; return 0; }
    virtual const char** GetDssParamsNames          (const eDepthStencilProp type)                      const { NOTIFY; return &s_DummyStr; }
    virtual const char*  GetDssParamStr             (const DssID id, const eDepthStencilProp type)      const { NOTIFY; return s_DummyStr; }
    virtual bool         GetDssParamBool            (const DssID id, const eDepthStencilProp type)      const { NOTIFY; return false; }

    // update render param
    virtual void         UpdateCustomRsParam        (const eRasterProp type, const bool value)          const { NOTIFY; return; }
    virtual void         UpdateCustomRsParam        (const eRasterProp type, const int value)           const { NOTIFY; return; }
    virtual void         UpdateCustomRsParam        (const eRasterProp type, const float value)         const { NOTIFY; return; }
    virtual void         UpdateCustomRsParam        (const eRasterProp type, const char* value)         const { NOTIFY; return; }

    virtual void         UpdateCustomBsParam        (const eBlendProp type, const bool value)           const { NOTIFY; return; }
    virtual void         UpdateCustomBsParam        (const eBlendProp type, const char* value)          const { NOTIFY; return; }

    virtual void         UpdateCustomDssParam       (const eDepthStencilProp type, const bool value)    const { NOTIFY; return; }
    virtual void         UpdateCustomDssParam       (const eDepthStencilProp type, const char* value)   const { NOTIFY; return; }

    // setup render states of material
    virtual uint         GetMaterialRenderStateId   (const MaterialID id, const eRndStatesGroup type)   const { NOTIFY; return 0; }
    virtual bool         SetMaterialRenderState     (const RenderStateSetup& params)                          { NOTIFY; return false; }


    virtual bool SetMaterialTexture(
        const MaterialID matId,
        const TexID texId,
        const uint texType) const
    { NOTIFY; return false; }


    virtual bool SetMaterialColorData(
        const MaterialID id,
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,                      // vec3(specular_color) + float(glossiness)
        const Vec4& reflect)
    { NOTIFY; return false; }

    virtual bool SetTerrainMaterialColors(
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,                      // vec3(specular_color) + float(glossiness)
        const Vec4& reflect)
    { NOTIFY; return false; }

    // get SRV (shader resource view)
    virtual bool GetArrTexturesSRVs         (SRV**& outTexViews, size& outNumViews)                                     const { NOTIFY; return false; }
    virtual bool GetTexViewsByIds           (TexID* texIds, SRV** outTexViews, size numTexTypes)                        const { NOTIFY; return false; }

    virtual bool SetEnttMaterial            (const EntityID enttId, const SubsetID meshId, const MaterialID matId)            { NOTIFY; return false; }

    virtual bool InitMaterialBigIcon        (const uint width, const uint height)                                       const { NOTIFY; return false; }
    virtual bool InitMaterialsIcons         (const size numIcons, const int width, const int weight)                          { NOTIFY; return false; }

    virtual bool RenderMaterialsIcons       (void)                                                                      const { NOTIFY; return false; }
    virtual bool RenderMaterialBigIconById  (const MaterialID matID, const float yAxisRotation)                               { NOTIFY; return false; }

    virtual void GetShadersIdsAndNames      (cvector<ShaderID>& outIds, cvector<ShaderName>& outNames)                  const { NOTIFY; return; }
    virtual bool SetMaterialShaderId        (const MaterialID matId, const ShaderID shaderId)                                 { NOTIFY; return false; }


    //-------------------------------------------
    // model preview configuration (for model editor, or model screenshot tool)
    //-------------------------------------------
    virtual bool    InitModelFrameBuf       (const uint width, const uint height)                                       const { NOTIFY; return false; }
    virtual bool    RenderModelFrameBuf     (void)                                                                            { NOTIFY; return false; }

    virtual void    SetModelPreviewParam    (const eModelPreviewParams param, const float val)                                { NOTIFY; return; }
    virtual float   GetModelPreviewParam    (const eModelPreviewParams param)                                           const { NOTIFY; return 0.0; }

    virtual SRV*    GetModelFrameBufView    (void)                                                                      const { NOTIFY; return nullptr; }

    // make screenshot of model in the tool
    virtual void    CreateEmptyTexAtlas     (const uint elemWidth, const uint elemHeight, const uint numElems)                { NOTIFY; return; }
    virtual void    PushTexIntoAtlas        (SRV* pSRV)                                                                       { NOTIFY; return; }
    virtual void    SaveTexAtlasToFile      (const char* filename)                                                            { NOTIFY; return; }
    virtual void    ClearTexAtlasMemory     (void)                                                                            { NOTIFY; return; }
    virtual void    SaveTexToFile           (const char* filename, SRV* pSRV, const DXGI_FORMAT targetFormat)                 { NOTIFY; return; }

    // =============================================================================
    // PARTICLES
    // =============================================================================
    virtual bool SetParticlesColor      (const EntityID id, const ColorRGB& c)         { NOTIFY;  return false; }
    virtual bool SetParticlesExternForce(const EntityID id, const Vec3& force)         { NOTIFY;  return false; }
    virtual bool SetParticlesSpawnRate  (const EntityID id, const int num)             { NOTIFY;  return false; }
    virtual bool SetParticlesLifespanMs (const EntityID id, const int milliseconds)    { NOTIFY;  return false; }
    virtual bool SetParticlesMass       (const EntityID id, const float mass)          { NOTIFY;  return false; }
    virtual bool SetParticlesSize       (const EntityID id, const float size)          { NOTIFY;  return false; }
    virtual bool SetParticlesFriction   (const EntityID id, const float airResistance) { NOTIFY;  return false; }

    virtual bool GetEnttParticleEmitterData(
        const EntityID id,
        ColorRGB& color,
        Vec3& externForce,
        int& spawnRate,
        int& lifespanMs,
        float& mass,
        float& size,
        float& friction)
    { NOTIFY; return false; }


    // =============================================================================
    // TERRAIN
    // =============================================================================
    virtual int      GetTerrainNumMaxLOD     (void)                           const { NOTIFY;  return -1; }
    virtual int      GetTerrainDistanceToLOD (const int lod)                  const { NOTIFY;  return -1; }
    virtual bool     SetTerrainDistanceToLOD (const int lod, const int dist)        { NOTIFY;  return false; }


    // =============================================================================
    // GRASS
    // =============================================================================
    virtual float    GetGrassDistFullSize    (void)                          const { NOTIFY;  return -1.0f; }
    virtual float    GetGrassDistVisible     (void)                          const { NOTIFY;  return -1.0f; }

    virtual bool     SetGrassDistFullSize    (const float dist)                    { NOTIFY;  return false; };
    virtual bool     SetGrassDistVisible     (const float dist)                    { NOTIFY;  return false; };

private:
    inline float     GetInvalidFloat()                                       const { return NAN; }
    inline Vec3      GetInvalidVec3()                                        const { return { NAN, NAN, NAN }; }
    inline Vec4      GetInvalidVec4()                                        const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGB()                                         const { return { NAN, NAN, NAN, NAN }; }
    inline ColorRGBA GetInvalidRGBA()                                        const { return { NAN, NAN, NAN, NAN }; }
};

} // namespace UI
