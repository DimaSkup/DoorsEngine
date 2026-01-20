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
// flags for turning on/off rendering of particular debug shapes
//---------------------------------------------------------
enum eRenderDbgShape
{
    RENDER_DBG_SHAPES,           // turn on/off rendering of debug shapes at all

    RENDER_DBG_SHAPES_LINE,
    RENDER_DBG_SHAPES_CROSS,
    RENDER_DBG_SHAPES_SPHERE,

    RENDER_DBG_SHAPES_CIRCLE,
    RENDER_DBG_SHAPES_AXES,
    RENDER_DBG_SHAPES_TRIANGLE,

    RENDER_DBG_SHAPES_AABB,
    RENDER_DBG_SHAPES_OBB,
    RENDER_DBG_SHAPES_FRUSTUM,
    RENDER_DBG_SHAPES_TERRAIN_AABB
};

//---------------------------------------------------------
// list of ECS components
//---------------------------------------------------------
enum eEnttComponentType
{
    NameComponent,                 // REQUIRED: attach some name for the entity
    TransformComponent,            // REQUIRED: set that entity has properties: position (x,y,z), direction (quaternion), and scale (uniform)
    MoveComponent,                 // set that this entity must be transformed over the time using some transformation matrix (for instance rotate around itself and go into some particular direction)
    RenderedComponent,             // set that this entity is renderable (preferably it is a model), set that this entity must be rendered with particular kind of shader, maybe with specific primitive topology
    ModelComponent,                // attach to entity a 2D/3D model by ID

    CameraComponent,               // attach to entity a camera
    MaterialComponent,
    TextureTransformComponent,     // set that texture has some kind of transformation (maybe it is a translation over some atlas texture so we create an animation, or rotation around texture center -- creates a rotating fireball)
    LightComponent,                // attach to entity some type of light source (directed, point, spotlight, etc.)
    RenderStatesComponent,         // for using different render states: blending, alpha clipping, fill mode, cull mode, etc.
    BoundingComponent,             // for using AABB, OBB, bounding spheres

    PlayerComponent,               // to hold First-Person-Shooter (FPS) player's data
    ParticlesComponent,
    InventoryComponent,            // inventory of entity
    AnimationComponent,            // for model skinning

    // NOT IMPLEMENTED YET
    AIComponent,
    HealthComponent,
    DamageComponent,
    EnemyComponent,
    ColliderComponent,

    PhysicsTypeComponent,
    VelocityComponent,
    GroundedComponent,
    CollisionComponent,

    NUM_COMPONENTS
};

///////////////////////////////////////////////////////////

enum eRndStatesGroup
{
    RND_STATES_RASTER,
    RND_STATES_BLEND,
    RND_STATES_DEPTH_STENCIL,

    NUM_GROUPS_RND_STATES
};

enum eRasterProp
{
    UI_RS_FILL,
    UI_RS_CULL,
    UI_RS_FRONT_CCW,
    UI_RS_DEPTH_BIAS,
    UI_RS_DEPTH_BIAS_CLAMP,
    UI_RS_SLOPE_SCALED_DEPTH_BIAS,
    UI_RS_DEPTH_CLIP_ENABLE,
    UI_RS_SCISSOR_ENABLE,
    UI_RS_MULTISAMPLE_ENABLE,
    UI_RS_ANTIALIASED_LINE_ENABLE,
};

enum eBlendProp
{
    UI_BLEND_OPERATION,
    UI_BLEND_FACTOR,

    //-----------------------

    UI_BLEND_RND_TARGET_WRITE_MASK,

    UI_BLEND_SRC_BLEND,
    UI_BLEND_DST_BLEND,
    UI_BLEND_OP,

    UI_BLEND_SRC_BLEND_ALPHA,
    UI_BLEND_DST_BLEND_ALPHA,
    UI_BLEND_OP_ALPHA,

    UI_BLEND_IS_ALPHA_TO_COVERAGE,
    UI_BLEND_IS_INDEPENDENT,
    UI_BLEND_IS_ENABLED,
};

enum eDepthStencilProp
{
    UI_DSS_STENCIL_OP,
    UI_DSS_COMPARISON_FUNC,

    //-----------------------

    UI_DSS_DEPTH_ENABLED,
    UI_DSS_DEPTH_WRITE_MASK,
    UI_DSS_DEPTH_FUNC,

    UI_DSS_STENCIL_ENABLED,
    UI_DSS_STENCIL_READ_MASK,
    UI_DSS_STENCIL_WRITE_MASK,

    UI_DSS_FRONT_FACE_STENCIL_FAIL_OP,
    UI_DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP,
    UI_DSS_FRONT_FACE_STENCIL_PASS_OP,
    UI_DSS_FRONT_FACE_STENCIL_FUNC,

    UI_DSS_BACK_FACE_STENCIL_FAIL_OP,
    UI_DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP,
    UI_DSS_BACK_FACE_STENCIL_PASS_OP,
    UI_DSS_BACK_FACE_STENCIL_FUNC,
};

//---------------------------------------------------------
// model preview parameters (for model editor, or model screenshot tool)
//---------------------------------------------------------
enum eModelPreviewParams
{
    MODEL_ID,                    // which model to render
    MODEL_POS_X,
    MODEL_POS_Y,
    MODEL_POS_Z,
    MODEL_ROT_X,
    MODEL_ROT_Y,
    MODEL_ROT_Z,
    MODEL_SCALE,                 // uniform scale

    CAMERA_POS_X,
    CAMERA_POS_Y,
    CAMERA_POS_Z,
    CAMERA_ROT_X,
    CAMERA_ROT_Y,
    CAMERA_ROT_Z,

    FRAME_BUF_WIDTH,
    FRAME_BUF_HEIGHT,

    ORTHO_MATRIX_VIEW_HEIGHT,
    USE_ORTHO_MATRIX,            // flag: use ortho matrix or not

    BG_COLOR_R,
    BG_COLOR_G,
    BG_COLOR_B,

    NUM_MODEL_PREVIEW_PARAMS
};

//---------------------------------------------------------
// data container for setup a custom render state or switch to some predefined state 
//---------------------------------------------------------
struct RenderStateSetup
{
    MaterialID matId;
    eRndStatesGroup rndState;
    int rndStateId = 0;
    char propName[32];
    char propValue[32];
};

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
    virtual EntityID    CreateEntity(void)                                                                                                { NOTIFY; return 0; }

    virtual bool        GetEnttAddedComponentsNames (const EntityID id, cvector<std::string>& componentsNames)                      const { NOTIFY;  return false; }
    virtual bool        GetEnttAddedComponentsTypes (const EntityID id, cvector<eEnttComponentType>& componentTypes)                const { NOTIFY;  return false; }

    virtual bool        AddNameComponent            (const EntityID enttId, const char* name)                                             { NOTIFY;  return false; }
    virtual bool        AddTransformComponent       (const EntityID enttId, const Vec3& pos, const Vec3& dir, const float scale)          { NOTIFY;  return false; }
    virtual bool        AddModelComponent           (const EntityID enttId, const ModelID modelId)                                        { NOTIFY;  return false; }
    virtual bool        AddRenderedComponent        (const EntityID enttId)                                                               { NOTIFY;  return false; }
    virtual bool        AddBoundingComponent        (const EntityID enttId, const int boundType, const DirectX::BoundingBox& aabb)        { NOTIFY;  return false; }


    virtual const cvector<EntityID>* GetAllEnttsIDs (void)                                                                          const { NOTIFY;  return nullptr; }
    virtual EntityID                 GetEnttIdByName(const char* name)                                                              const { NOTIFY;  return INVALID_ENTITY_ID; }
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
    virtual bool        SetEnttUniScale             (const EntityID id, const float scale)                                                { NOTIFY;  return false; }

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
    virtual MaterialID  GetMaterialIdByIdx      (const index idx)                           const { NOTIFY;  return INVALID_MATERIAL_ID; }
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
