// =================================================================================
// Filename:  FacadeEngineToUI.cpp
// 
// Created:   31.12.24
// =================================================================================
#include <CoreCommon/pch.h>
#include <post_fx_enum.h>
#include "FacadeEngineToUI.h"

#include <Mesh/material_mgr.h>
#include <Texture/texture_mgr.h>   // texture mgr is used to get textures by its IDs
#include <Shaders/Shader.h>
#include <Render/debug_draw_manager.h>
#include <Model/model_mgr.h>
#include <Model/model_creator.h>
#include <Model/model_exporter.h>
#include <Model/animation_mgr.h>
#include <ImgConverter.h>

#pragma warning (disable : 4996)

using namespace DirectX;
using namespace Core;


namespace UI
{

FacadeEngineToUI::FacadeEngineToUI(
    Core::Engine* pEngine,
    ID3D11DeviceContext* pContext,
    Render::CRender* pRender,
    ECS::EntityMgr* pEntityMgr,
    Core::CGraphics* pGraphics,
    Core::TerrainGeomip* pTerrain)
    :
    pEngine_(pEngine),
    pContext_(pContext),
    pRender_(pRender),
    pEnttMgr_(pEntityMgr),
    pGraphics_(pGraphics),
    pTerrain_(pTerrain)
{
    // set pointers to the subsystems
    CAssert::NotNullptr(pEngine,    "a ptr to engine == nullptr");
    CAssert::NotNullptr(pRender,    "a ptr to render == nullptr");
    CAssert::NotNullptr(pContext,   "a ptr to device context == nullptr");
    CAssert::NotNullptr(pEntityMgr, "a ptr to the entt mgr == nullptr");
    CAssert::NotNullptr(pGraphics,  "a ptr to the graphics class == nullptr");
    CAssert::NotNullptr(pTerrain,   "a ptr to the terrain == nullptr");
}

///////////////////////////////////////////////////////////

ModelID FacadeEngineToUI::GetModelIdByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return INVALID_MODEL_ID;
    }

    return g_ModelMgr.GetModelIdByName(name);
}

//---------------------------------------------------------
// Desc:  get a ptr to array of all the models names
//---------------------------------------------------------
const cvector<ModelName>* FacadeEngineToUI::GetModelsNamesArrPtr() const
{
    return g_ModelMgr.GetModelsNamesArrPtr();
}


// =============================================================================
// Graphics control
// =============================================================================
void FacadeEngineToUI::SetAntiAliasingType(const uint8 type)
{
    pGraphics_->SetAntiAliasingType(type);
}

uint8 FacadeEngineToUI::GetAntiAliasingType() const
{
    return pGraphics_->GetAntiAliasingType();
}

// =================================================================================
// Get camera info
// =================================================================================
void FacadeEngineToUI::GetCameraViewAndProj(
    const EntityID camEnttID,
    float* outView,
    float* outProj)
{
    if (!outView || !outProj)
    {
        LogErr(LOG, "some of input matrices ptrs == NULL");
        return;
    }

    const DirectX::XMMATRIX& view = pEnttMgr_->cameraSystem_.GetView(camEnttID);
    const DirectX::XMMATRIX& proj = pEnttMgr_->cameraSystem_.GetProj(camEnttID);

    // copy view and proj matrices into raw array of 16 floats
    memcpy(outView, (void*)view.r->m128_f32, sizeof(float) * 16);
    memcpy(outProj, (void*)proj.r->m128_f32, sizeof(float) * 16);
}

//---------------------------------------------------------
// Desc:  focus the editor camera on the selected entity
//---------------------------------------------------------
void FacadeEngineToUI::FocusCameraOnEntity(const EntityID enttID)
{
    //assert(0 && "FIXME");
#if 0
    if (enttID == 0)
        return;

    ECS::EntityMgr& mgr = *pEnttMgr_;
    Camera& cam = *pEditorCamera_;

    DirectX::XMFLOAT3 enttPos = mgr.transformSystem_.GetPositionByID(enttID);
    cam.LookAt(cam.GetPosition(), enttPos, { 0, 1, 0 });
#endif
}

// =================================================================================
// For the entity editor
// =================================================================================
EntityID FacadeEngineToUI::CreateEntity()
{
    return pEnttMgr_->CreateEntity();
}

//---------------------------------------------------------
// out:    names array of components which are added to entity by ID;
// return: false if there is no entity by ID in the Entity Manager
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttAddedComponentsNames(
    const EntityID id,
    cvector<std::string>& componentsNames) const
{
    if (pEnttMgr_->GetComponentNamesByEntt(id, componentsNames))
        return true;

    // we didn't manage to get components names
    LogErr(LOG, "can't get components names by entity");
    return false;
}

//---------------------------------------------------------
// out:    array of components types which are added to entity by ID;
// return: false if there is no entity by ID in the Entity Manager
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttAddedComponentsTypes(
    const EntityID id,
    cvector<eEnttComponentType>& componentTypes) const
{
    cvector<uint8> types;

    if (pEnttMgr_->GetComponentTypesByEntt(id, types))
    {
        // store received component types into the output array
        componentTypes.resize(types.size());

        for (int i = 0; const uint8 type : types)
            componentTypes[i++] = (eEnttComponentType)type;

        return true;
    }

    // we didn't manage to get components types
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::AddNameComponent(const EntityID id, const char* name)
{
    pEnttMgr_->AddNameComponent(id, name);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::AddTransformComponent(
    const EntityID id,
    const Vec3& pos,
    const Vec3& dir,
    const float scale)
{
    pEnttMgr_->AddTransformComponent(id, XMFLOAT3(pos.x, pos.y, pos.z), XMVECTOR{ dir.x, dir.y, dir.z }, scale);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::AddModelComponent(const EntityID enttID, const uint32_t modelID)
{
    pEnttMgr_->AddModelComponent(enttID, modelID);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::AddRenderedComponent(const EntityID enttID)
{
    pEnttMgr_->AddRenderingComponent(enttID);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::AddBoundingComponent(const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)
{
    pEnttMgr_->AddBoundingComponent(id, aabb);
    return true;
}

///////////////////////////////////////////////////////////

const cvector<EntityID>* FacadeEngineToUI::GetAllEnttsIDs(void) const
{
    return &pEnttMgr_->GetAllEnttsIDs();
}

//---------------------------------------------------------
// Desc:  return an ID of entity by its name
//---------------------------------------------------------
EntityID FacadeEngineToUI::GetEnttIdByName(const char* name) const
{
    return pEnttMgr_->nameSystem_.GetIdByName(name);
}

//---------------------------------------------------------
// Desc:  return a name of entity by its id
//---------------------------------------------------------
const char* FacadeEngineToUI::GetEnttNameById(const EntityID id) const
{
    return pEnttMgr_->nameSystem_.GetNameById(id);
}

//---------------------------------------------------------
// Desc:   get position, direction, and uniform scale of entt by ID
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttTransformData(
    const EntityID id,
    Vec3& outPos,
    Vec3& outDir,
    Vec4& outRotQuat,
    float& outScale) const
{
    outPos     = GetEnttPosition(id);
    outDir     = GetEnttDirection(id);
    outRotQuat = GetEnttRotQuat(id);
    outScale   = GetEnttScale(id);

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttWorldMatrix(const EntityID id, DirectX::XMMATRIX& outMat) const
{
    outMat = pEnttMgr_->transformSystem_.GetWorld(id);
    return true;
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetEnttPosition(const EntityID id) const
{
    const XMFLOAT3 pos = pEnttMgr_->transformSystem_.GetPosition(id);
    return Vec3(pos.x, pos.y, pos.z);
}

Vec3 FacadeEngineToUI::GetEnttDirection(const EntityID id) const
{
    const XMFLOAT3 dir = pEnttMgr_->transformSystem_.GetDirection(id);
    return Vec3(dir.x, dir.y, dir.z);
}

Vec4 FacadeEngineToUI::GetEnttRotQuat(const EntityID id) const
{
    const XMVECTOR rotQuat = pEnttMgr_->transformSystem_.GetRotationQuat(id);
    return Vec4(
        XMVectorGetX(rotQuat),
        XMVectorGetY(rotQuat),
        XMVectorGetZ(rotQuat),
        XMVectorGetW(rotQuat));
}

float FacadeEngineToUI::GetEnttScale(const EntityID id) const
{
    return pEnttMgr_->transformSystem_.GetScale(id);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetEnttPosition(const EntityID id, const Vec3& pos)
{
    pEnttMgr_->AddEvent(ECS::EventTranslate(id, pos.x, pos.y, pos.z));
    return true;
}

bool FacadeEngineToUI::SetEnttDirection(const EntityID id, const Vec3& dir)
{
    return pEnttMgr_->transformSystem_.SetDirection(id, XMVECTOR{ dir.x, dir.y, dir.z });
}

bool FacadeEngineToUI::SetEnttUniScale(const EntityID id, const float scale)
{
    return pEnttMgr_->transformSystem_.SetUniScale(id, scale);
}

bool FacadeEngineToUI::RotateEnttByQuat(const EntityID id, const Vec4& q)
{
    return pEnttMgr_->transformSystem_.RotateLocalSpaceByQuat(id, XMVECTOR{ q.x, q.y, q.z, q.w });
}

//---------------------------------------------------------
// output:  a code of light type which is added to entity by ID
// return:  false - if there is no light which is added to the entity;
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttLightType(const EntityID id, int& lightType) const
{
    if (!pEnttMgr_->lightSystem_.IsLightSource(id))
    {
        lightType = -1;
        return false;
    }

    lightType = pEnttMgr_->lightSystem_.GetLightType(id);
    return true;
}

//---------------------------------------------------------
// Desc:  change skeleton animation for entity by enttId
//---------------------------------------------------------
bool FacadeEngineToUI::SetEnttSkeletonAnimation(const EntityID enttId, const AnimationID animId)
{
    ECS::AnimationSystem& animSys        = pEnttMgr_->animationSystem_;
    const SkeletonID      skeletonId     = animSys.GetSkeletonId(enttId);
    const AnimSkeleton&   skeleton       = g_AnimationMgr.GetSkeleton(skeletonId);
    const AnimationClip&  anim           = skeleton.GetAnimation(animId);
    const bool            isAnimRepeated = true;

    return animSys.SetAnimation(enttId, animId, anim.GetEndTime(), isAnimRepeated);
}

//---------------------------------------------------------
// Desc:  get animation data of entity
//---------------------------------------------------------
SkeletonID FacadeEngineToUI::GetEnttSkeletonId(const EntityID enttId) const
{
    return pEnttMgr_->animationSystem_.GetSkeletonId(enttId);
}

//---------------------------------------------------------

const char* FacadeEngineToUI::GetSkeletonName(const SkeletonID id) const
{
    return g_AnimationMgr.GetSkeleton(id).GetName();
}

//---------------------------------------------------------

size FacadeEngineToUI::GetSkeletonNumAnimations(const SkeletonID id) const
{
    return g_AnimationMgr.GetSkeleton(id).GetNumAnimations();
}

//---------------------------------------------------------

const cvector<AnimationName>* FacadeEngineToUI::GetSkeletonAnimNames(const SkeletonID id) const
{
    Core::AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton(id);
    return &skeleton.GetAnimNames();
}

//---------------------------------------------------------

const char* FacadeEngineToUI::GetSkeletonAnimName(
    const SkeletonID skeletonId,
    const AnimationID animId) const
{
    return g_AnimationMgr.GetSkeleton(skeletonId).GetAnimationName(animId);
}

//---------------------------------------------------------

AnimationID FacadeEngineToUI::GetEnttAnimationId(const EntityID id) const
{
    return pEnttMgr_->animationSystem_.GetAnimationId(id);
}

//---------------------------------------------------------

float FacadeEngineToUI::GetEnttCurrAnimTime(const EntityID id) const
{
    return pEnttMgr_->animationSystem_.GetCurrAnimTime(id);
}

//---------------------------------------------------------

float FacadeEngineToUI::GetEnttEndAnimTime(const EntityID id) const
{
    return pEnttMgr_->animationSystem_.GetEndAnimTime(id);
}


//---------------------------------------------------------
// Desc:   get directed light data of entt by id
// Ret:    false if entt hasn't any directed light component
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttDirectedLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular)
{
    ECS::DirLight data;

    bool result = pEnttMgr_->lightSystem_.GetDirectedLightData(id, data);
    if (!result)
    {
        LogErr(LOG, "can't get directed light data of the entity by ID: %" PRIu32, id);
        return false;
    }

    ambient  = data.ambient;
    diffuse  = data.diffuse;
    specular = data.specular;

    return true;
}

//---------------------------------------------------------
// Desc:   get point light data of entt by id
// Ret:    false if entt hasn't any point light component
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttPointLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular,
    Vec3& attenuation,
    float& range,
    bool& isActive)
{
    ECS::PointLight data;

    bool result = pEnttMgr_->lightSystem_.GetPointLightData(id, data);
    if (!result)
    {
        LogErr(LOG, "can't get point light data of the entity by ID: %" PRIu32, id);
        return false;
    }

    ambient     = data.ambient;
    diffuse     = data.diffuse;
    specular    = data.specular;
    range       = data.range;
    attenuation = Vec3(data.att.x, data.att.y, data.att.z);
    isActive    = GetLightIsActive(id);

    return true;
}

//---------------------------------------------------------
// Desc:   get spotlight data of entt by id
// Ret:    false if entt hasn't any spotlight component
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttSpotLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular,
    Vec3& attenuation,
    float& range,
    float& spotExponent)
{
    ECS::SpotLight data;

    bool result = pEnttMgr_->lightSystem_.GetSpotLightData(id, data);
    if (!result)
    {
        LogErr(LOG, "can't get spotlight data of the entity by ID: %" PRIu32, id);
        return false;
    }

    ambient      = data.ambient;
    diffuse      = data.diffuse;
    specular     = data.specular;
    range        = data.range;
    spotExponent = data.spot;
    attenuation  = Vec3(data.att.x, data.att.y, data.att.z);

    return true;
}

//---------------------------------------------------------
// Desc:  turn on/off light source by ID
//---------------------------------------------------------
bool FacadeEngineToUI::SetLightActive(const EntityID id, const bool state)
{
    return pEnttMgr_->lightSystem_.SetLightIsActive(id, state);
}

//---------------------------------------------------------
// Desc:  is light source by ID currently active?
//---------------------------------------------------------
bool FacadeEngineToUI::GetLightIsActive(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.IsLightActive(id);
}


// =============================================================================
// SET directed light props
// =============================================================================
bool FacadeEngineToUI::SetDirectedLightAmbient(const EntityID id, const ColorRGBA& rgba)
{
    return pEnttMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::AMBIENT, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightDiffuse(const EntityID id, const ColorRGBA& rgba)
{
    return pEnttMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::DIFFUSE, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightSpecular(const EntityID id, const ColorRGBA& rgba)
{
    return pEnttMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::SPECULAR, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightDirection(const EntityID id, const Vec3& dir)
{
    return pEnttMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::DIRECTION, XMFLOAT3(dir.x, dir.y, dir.z));
}


// =============================================================================
// GET directed light props
// =============================================================================
ColorRGBA FacadeEngineToUI::GetDirectedLightAmbient(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetDirectedLightDiffuse(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetDirectedLightSpecular(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::SPECULAR);
}

Vec3 FacadeEngineToUI::GetDirectedLightDirection(const EntityID id) const
{
    DirectX::XMFLOAT4 dir = pEnttMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::DIRECTION);
    return Vec3(dir.x, dir.y, dir.z);
}


// =================================================================================
// SET point light properties
// =================================================================================

bool FacadeEngineToUI::SetPointLightAmbient(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::AMBIENT, color.ToFloat4());
}

bool FacadeEngineToUI::SetPointLightDiffuse(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::DIFFUSE, color.ToFloat4());
}

bool FacadeEngineToUI::SetPointLightSpecular(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::SPECULAR, color.ToFloat4());
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightPos(const EntityID id, const Vec3& pos)
{
    const ECS::LightProp prop     = ECS::LightProp::POSITION;
    const XMFLOAT4       position = { pos.x, pos.y, pos.z, 1.0f };

    if (pEnttMgr_->lightSystem_.SetPointLightProp(id, prop, position))
    {
        return SetEnttPosition(id, pos);
    }

    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightRange(const EntityID id, const float range)
{
    return pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::RANGE, range);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightAttenuation(const EntityID id, const Vec3& att)
{
    const ECS::LightProp prop        = ECS::LightProp::ATTENUATION;
    const XMFLOAT4       attenuation = { att.x, att.y, att.z, 1.0f };

    return pEnttMgr_->lightSystem_.SetPointLightProp(id, prop, attenuation);
}


// =================================================================================
// GET point light properties
// =================================================================================

ColorRGBA FacadeEngineToUI::GetPointLightAmbient(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetPointLightDiffuse(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetPointLightSpecular(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::SPECULAR);
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetPointLightPos(const EntityID id) const
{
    // position values are stored in x,y,z
    XMFLOAT4 pos = pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::POSITION);
    return Vec3(pos.x, pos.y, pos.z);
}

Vec3 FacadeEngineToUI::GetPointLightAttenuation(const EntityID id) const
{
    // attenuation values are stored in x,y,z
    XMFLOAT4 att = pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::ATTENUATION);
    return Vec3(att.x, att.y, att.z);
}

///////////////////////////////////////////////////////////

float FacadeEngineToUI::GetPointLightRange(const EntityID id) const
{
    // the same value of range is stored in each component of Vec4
    XMFLOAT4 range = pEnttMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::RANGE);
    return range.x;
}


// =================================================================================
// SET spotlight properties
// =================================================================================

bool FacadeEngineToUI::SetSpotLightAmbient(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::AMBIENT, color.ToFloat4());
}

bool FacadeEngineToUI::SetSpotLightDiffuse(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::DIFFUSE, color.ToFloat4());
}

bool FacadeEngineToUI::SetSpotLightSpecular(const EntityID id, const ColorRGBA& color)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::SPECULAR, color.ToFloat4());
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSpotLightPos(const EntityID id, const Vec3& pos)
{
    if (pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::POSITION, XMFLOAT4(pos.x, pos.y, pos.z, 1.0f)))
    {
        return SetEnttPosition(id, pos);
    }

    return false;
}

bool FacadeEngineToUI::SetSpotLightDirection(const EntityID id, const Vec3& dir)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::DIRECTION, { dir.x, dir.y, dir.z, 0.0f });
}

bool FacadeEngineToUI::SetSpotLightAttenuation(const EntityID id, const Vec3& att)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::ATTENUATION, XMFLOAT4(att.x, att.y, att.z, 1.0f));
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSpotLightRange(const EntityID id, const float range)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::RANGE, range);
}

bool FacadeEngineToUI::SetSpotLightSpotExponent(const EntityID id, const float spotExp)
{
    return pEnttMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::SPOT_EXP, spotExp);
}


// =================================================================================
// GET spotlight properties
// =================================================================================

ColorRGBA FacadeEngineToUI::GetSpotLightAmbient(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetSpotLightDiffuse(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetSpotLightSpecular(const EntityID id) const
{
    return pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::SPECULAR);
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetSpotLightPos(const EntityID id) const
{
    // spotlight position is stored in the Transform component
    const XMFLOAT3 pos = pEnttMgr_->transformSystem_.GetPosition(id);
    return Vec3(pos.x, pos.y, pos.z);
}

Vec3 FacadeEngineToUI::GetSpotLightDirection(const EntityID id) const
{
    // spotlight direction is stored in the Transform component
    const XMFLOAT3 dir = pEnttMgr_->transformSystem_.GetDirection(id);
    return Vec3(dir.x, dir.y, dir.z);
}

Vec3 FacadeEngineToUI::GetSpotLightAttenuation(const EntityID id) const
{
    // attenuation values are stored in x,y,z
    const XMFLOAT4 att = pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::ATTENUATION);
    return Vec3(att.x, att.y, att.z);
}

///////////////////////////////////////////////////////////

float FacadeEngineToUI::GetSpotLightRange(const EntityID id) const
{
    // the same value of range is stored in each component of Vec4 (so just return x)
    return pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::RANGE).x;
}

float FacadeEngineToUI::GetSpotLightSpotExponent(const EntityID id) const
{
    // the same value of exponent is stored in each component of Vec4 (so just return x)
    return pEnttMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::SPOT_EXP).x;
}


// =================================================================================
// For the sky editor
// =================================================================================
bool FacadeEngineToUI::GetSkyData(
    const uint32_t skyEnttID,
    ColorRGB& center,
    ColorRGB& apex,
    Vec3& offset)
{
    // the sky editor model must be initialized with some reasonable data
    // so we gather this data here

    center = pRender_->GetSkyCenterColor();
    apex   = pRender_->GetSkyApexColor();

    const XMFLOAT3 pos = pEnttMgr_->transformSystem_.GetPosition(skyEnttID);
    offset = Vec3(pos.x, pos.y, pos.z);

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyColorCenter(const ColorRGB& color)
{
    pRender_->SetSkyColorCenter(color.ToFloat3());
    return true;
}

bool FacadeEngineToUI::SetSkyColorApex(const ColorRGB& color)
{
    pRender_->SetSkyColorApex(color.ToFloat3());
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyOffset(const Vec3& offset)
{
    const EntityID id = pEnttMgr_->nameSystem_.GetIdByName("sky");

    // if we found the sky entity we change its offset
    if (id != 0)
    {
        return pEnttMgr_->transformSystem_.SetPosition(id, { offset.x, offset.y, offset.z });
    }

    LogErr(LOG, "there is no entity by such a name: sky");
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyTexture(const int idx, const TexID textureID)
{
    assert(0 && "IMPLEMENT ME GOD DAMN!");
    return true;
}


// =================================================================================
// weather control
// =================================================================================

bool FacadeEngineToUI::GetFogData(
    ColorRGB& fogColor,
    float& fogStart,     // distance where the fog starts
    float& fogRange,     // distance after which the objects are fully fogged
    bool& fogEnabled)
{
    DirectX::XMFLOAT3 color;

    pRender_->GetFogData(color, fogStart, fogRange, fogEnabled);
    fogColor = color;

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetFogStart  (const float start)     { pRender_->SetFogStart(start);             return true; }
bool FacadeEngineToUI::SetFogRange  (const float range)     { pRender_->SetFogRange(range);             return true; }
bool FacadeEngineToUI::SetFogEnabled(const bool enabled)    { pRender_->SetFogEnabled(enabled);         return true; }
bool FacadeEngineToUI::SetFogColor  (const ColorRGB& color) { pRender_->SetFogColor(color.ToFloat3());  return true; }

void FacadeEngineToUI::SetWeatherParam(const eWeatherParam param, const float val)
{
    pRender_->UpdateCbWeatherParam(param, val);
}

float FacadeEngineToUI::GetWeatherParam(const eWeatherParam param) const
{
    return pRender_->GetWeatherParam(param);
}

// =================================================================================
// For the debug editor
// =================================================================================
bool FacadeEngineToUI::SwitchDebugState(const int debugType)
{
    pRender_->UpdateCbDebugType((eRndDbgType)debugType);
    return true;
}


// =================================================================================
// For assets manager
// =================================================================================

//---------------------------------------------------------
// Desc:   import a model from external format (.fbx, .obj, etc.)
//         and convert it into model's internal format (.de3d)
//---------------------------------------------------------
bool FacadeEngineToUI::ImportModelFromFile(const char* filepath, const char* modelName) const
{
    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "can't import a model from file: input path is empty!");
        return false;
    }

    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "can't import a model from file: input model name is empty!");
        return false;
    }


    ModelsCreator creator;
    ModelExporter exporter;
    const ModelID modelId = creator.ImportFromFile(filepath);

    if (modelId == INVALID_MODEL_ID)
    {
        LogErr(LOG, "can't import a model from file: %s", filepath);
        return false;
    }

    const BasicModel& model = g_ModelMgr.GetModelById(modelId);
    char dirPath[256]{'\0'};
    snprintf(dirPath, 256, "%s/", modelName);

    return exporter.ExportIntoDE3D(&model, dirPath, modelName);
}

//---------------------------------------------------------
// Desc:   load a new texture from file and add it into textures manager
//---------------------------------------------------------
TexID FacadeEngineToUI::LoadTextureFromFile(const char* path) const
{
    return g_TextureMgr.LoadFromFile(path);
}

//---------------------------------------------------------
// Desc:   reinit a texture by id with new data loaded from a file by path
//---------------------------------------------------------
bool FacadeEngineToUI::ReloadTextureFromFile(const TexID id, const char* path) const
{
    return g_TextureMgr.ReloadFromFile(id, path);
}

//---------------------------------------------------------
// Desc:   get a name of each loaded model (geometry)
//---------------------------------------------------------
bool FacadeEngineToUI::GetModelsNamesList(cvector<std::string>& outNames) const
{
    cvector<ModelName> modelsNames;
    g_ModelMgr.GetModelsNamesList(modelsNames);

    outNames.resize(modelsNames.size());

    for (int i = 0; const ModelName& name : modelsNames)
        outNames[i++] = name.name;

    return true;
}

//---------------------------------------------------------
// Desc:  get texture ID by its index in the texture manager
//---------------------------------------------------------
TexID FacadeEngineToUI::GetTextureIdByIdx(const index idx) const
{
    return g_TextureMgr.GetTexIdByIdx(idx);
}

//---------------------------------------------------------
// Desc:  get material ID by its index in the material manager
//---------------------------------------------------------
MaterialID FacadeEngineToUI::GetMaterialIdByIdx(const index idx) const
{
    return g_MaterialMgr.GetMatIdByIdx(idx);
}

//---------------------------------------------------------
// Desc:   get a name of material by its id
// Args:   - id:          material identifier
//---------------------------------------------------------
const char* FacadeEngineToUI::GetMaterialNameById(const MaterialID id) const
{
    return g_MaterialMgr.GetMatById(id).name;
}

//---------------------------------------------------------
// Desc:   get an arr of texture ids which are bounded to this material by id
// Args:   - id:         material identifier
//         - outTexIds:  output arr of textures ids
//---------------------------------------------------------
bool FacadeEngineToUI::GetMaterialTexIds(const MaterialID id, TexID* outTexIds) const
{
    if (!outTexIds)
    {
        LogErr(LOG, "can't get texture ids for material: in-out arr of texture ids == nullptr");
        return false;
    }

    Material& mat = g_MaterialMgr.GetMatById(id);

    if (mat.id == INVALID_TEXTURE_ID)
    {
        LogErr(LOG, "can't get texture ids for material: there is no material by id: %" PRIu32, id);
        return false;
    }

    memcpy(outTexIds, mat.texIds, sizeof(TexID) * NUM_TEXTURE_TYPES);
    return true;
}

//---------------------------------------------------------
// Desc:   get a name of texture by input ID
//---------------------------------------------------------
const char* FacadeEngineToUI::GetTextureNameById(const TexID id) const
{
    return g_TextureMgr.GetTexByID(id).GetName().c_str();
}

//---------------------------------------------------------
// get the number of all the currenly loaded materials
//---------------------------------------------------------
size FacadeEngineToUI::GetNumMaterials(void) const
{
    return g_MaterialMgr.GetNumAllMaterials();
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetMaterialDataById(const MaterialID id, MaterialData& outData) const
{
    const Core::Material& mat = g_MaterialMgr.GetMatById(id);

    if (mat.id != id)
    {
        LogErr(LOG, "you've asked for material with id '%d', but got material with id '%d', so something went wrong", (int)id, (int)mat.id);
        return false;
    }

    outData.materialId  = id;
    outData.shaderId    = mat.shaderId;

    outData.ambient     = mat.ambient;
    outData.diffuse     = mat.diffuse;
    outData.specular    = mat.specular;
    outData.reflect     = mat.reflect;

    strncpy(outData.name, mat.name, MAX_LEN_MAT_NAME);
    memcpy(outData.textureIDs, mat.texIds, sizeof(TexID) * NUM_TEXTURE_TYPES);

    return true;
}

//---------------------------------------------------------
// Desc:  get number of render state props by input group type
//---------------------------------------------------------
uint FacadeEngineToUI::GetNumRenderStates(const eMaterialPropGroup type) const
{
    using enum eMaterialPropGroup;

    switch (type)
    {
        case FILL:
            return pRender_->GetRenderStates().GetNumFillModes();

        case CULL:
            return pRender_->GetRenderStates().GetNumCullModes();

        case WINDING_ORDER:
            return pRender_->GetRenderStates().GetNumWindingOrders();

        case BLENDING:
            return pRender_->GetRenderStates().GetNumBlendStates();

        case DEPTH_STENCIL:
            return pRender_->GetRenderStates().GetNumDepthStencilStates();

        default:
            LogErr(LOG, "unknown type of render states group: %d", (int)type);
            return 0;
    }
}

//---------------------------------------------------------
// Desc:   get names array of particular render states group
//         (groups: fill, cull, blending, etc.)
// 
// Args:   - type: what kind of names we want to get
// Ret:    - arr of names
//---------------------------------------------------------
const char** FacadeEngineToUI::GetRenderStateNames(const eMaterialPropGroup type) const
{
    using enum eMaterialPropGroup;

    switch (type)
    {
        case FILL:
            return pRender_->GetRenderStates().GetFillModesNames();

        case CULL:
            return pRender_->GetRenderStates().GetCullModesNames();

        case WINDING_ORDER:
            return pRender_->GetRenderStates().GetWindingOrdersNames();

        case BLENDING:
            return pRender_->GetRenderStates().GetBlendStatesNames();

        case DEPTH_STENCIL:
            return pRender_->GetRenderStates().GetDepthStencilStatesNames();

        default:
            LogErr(LOG, "unknown type of render states group: %d", (int)type);
            return nullptr;
    }
}

//---------------------------------------------------------
// Desc:  return a number of texture types names,
//        arr of texture types names,
//        or a name of texture type by its code (idx)
//---------------------------------------------------------
uint FacadeEngineToUI::GetNumTexTypesNames() const
{
    return g_TextureMgr.GetNumTexTypesNames();
}

const char** FacadeEngineToUI::GetTexTypesNames() const
{
    return g_TextureMgr.GetTexTypesNames();
}

const char* FacadeEngineToUI::GetTexTypeName(const uint texType) const
{
    return g_TextureMgr.GetTexTypeName(texType);
}

//---------------------------------------------------------
// Desc:  bind a texture to material at a particular texture slot by texType
//---------------------------------------------------------
bool FacadeEngineToUI::SetMaterialTexture(
    const MaterialID matId,
    const TexID texId,
    const uint texType) const
{
    return g_MaterialMgr.SetMatTexture(matId, texId, texType);
}

//---------------------------------------------------------
// Desc:   setup render state of particular type (group) for material by id
//         (groups: fill, cull, blending, etc.)
// Args:   - id:        material identifier
//         - stateIdx:  an index of render state inside its group
//         - type:      what kind of render states group we want to change
//---------------------------------------------------------
bool FacadeEngineToUI::SetMaterialRenderState(
    const MaterialID id,
    const uint32 stateIdx,
    const eMaterialPropGroup type) const
{
    using enum eMaterialPropGroup;
    Material& mat = g_MaterialMgr.GetMatById(id);

    if (mat.id == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "can't change render state "
                    "(idx: %" PRIu32 "; type: %d\n"
                    "because there is no material by id : %" PRIu32, id);
        return false;
    }


    switch (type)
    {
        case ALPHA_CLIP:
            // for alpha clipping we switch the state to the opposite
            mat.SetAlphaClip(!mat.HasAlphaClip());
            break;

        case FILL:
            mat.SetFillByIdx(stateIdx);
            break;

        case CULL:
            mat.SetCullByIdx(stateIdx);
            break;

        case WINDING_ORDER:
            break;

        case BLENDING:
            mat.SetBlendingByIdx(stateIdx);
            break;

        case DEPTH_STENCIL:
            mat.SetDepthStencilByIdx(stateIdx);
            break;

        default:
            LogErr(LOG, "unknown type of render states group: %u", type);
            return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   change color properties of material by id
//---------------------------------------------------------
bool FacadeEngineToUI::SetMaterialColorData(
    const MaterialID id,
    const Vec4& amb,           // ambient
    const Vec4& diff,          // diffuse
    const Vec4& spec,          // specular = vec3(specular_color) + float(specular_power)
    const Vec4& refl)          // reflect
{
    return g_MaterialMgr.SetMatColorData(id, amb, diff, spec, refl);
}

//---------------------------------------------------------
// Desc:  change color properties of terrain's material
//---------------------------------------------------------
bool FacadeEngineToUI::SetTerrainMaterialColors(
    const Vec4& ambient,
    const Vec4& diffuse,
    const Vec4& specular,                      // vec3(specular_color) + float(glossiness)
    const Vec4& reflect)
{
    const char*    matName = "terrain_mat";
    const MaterialID matId = g_MaterialMgr.GetMatIdByName(matName);

    if (matId == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "there is no material by name: %s", matName);
        return false;
    }

    g_MaterialMgr.SetMatColorData(matId, ambient, diffuse, specular, reflect);
    pRender_->UpdateCbTerrainMaterial(ambient, diffuse, specular, reflect);

    return true;
}


//---------------------------------------------------------
// output: 1. a ptr to arr of pointers to shader resource views (of all the loaded textures)
//         2. size of this array (number of currently loaded textures)
//---------------------------------------------------------
bool FacadeEngineToUI::GetArrTexturesSRVs(
    ID3D11ShaderResourceView**& outTexViews,
    size& outNumViews) const
{
    outTexViews = (ID3D11ShaderResourceView**)g_TextureMgr.GetAllShaderResourceViews();
    outNumViews                = g_TextureMgr.GetNumShaderResourceViews();
    return true;
}

//---------------------------------------------------------
// Desc:   get arr of shader resource views by ids of its texture objects
// Args:   - texIds:       texture objects identifiers (look at TextureMgr)
//         - outTexViews:  output array of ptrs to shader resource views
//         - numTexTypes:  size of texIds and outTextures arr
//---------------------------------------------------------
bool FacadeEngineToUI::GetTexViewsByIds(
    TexID* texIds,
    ID3D11ShaderResourceView** outTexViews,
    size numTexTypes) const
{
    if (!texIds || !outTexViews)
    {
        LogErr(LOG, "can't get textures by ids: some input arr == nullptr");
        return false;
    }

    if (numTexTypes != NUM_TEXTURE_TYPES)
    {
        LogErr(LOG, "input number of texture types must == %d", (int)numTexTypes);
        return false;
    }

    g_TextureMgr.GetTexViewsByIds(texIds, NUM_TEXTURE_TYPES, outTexViews);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetEnttMaterial(
    const EntityID enttID,
    const SubsetID subsetID,
    const MaterialID matID)
{
    // set a material (matID) for subset/mesh (subsetID) of entity (enttID)

    pEnttMgr_->materialSystem_.SetMaterial(enttID, subsetID, matID);
    return true;
}

//---------------------------------------------------------
// Desc:   create and initialize a frame buffer for models rendering
// Args:   - width, height:  dimensions for the frame buffer
//
// NOTE:  we also use this function for "resizing" of the buffer
//        (we drop it and recreate anew)
//---------------------------------------------------------
bool FacadeEngineToUI::InitModelFrameBuf(const uint width, const uint height) const
{
    return pGraphics_->InitModelFrameBuf(width, height);
}

//---------------------------------------------------------
// Desc:   init a frame buffer for rendering of material big icon (from material editor)
// Args:   - width, height:  frame buffer's dimensions (icon size)
//---------------------------------------------------------
bool FacadeEngineToUI::InitMaterialBigIcon(const uint width, const uint height) const
{
    return pGraphics_->InitMatBigIconFrameBuf(width, height);
}

//---------------------------------------------------------
// Desc:   init frame buffers for rendering of materials icons,
//         so each frame buffer will be responsible for visualization of one material
//---------------------------------------------------------
bool FacadeEngineToUI::InitMaterialsIcons(
    const size numIcons,
    const int iconWidth,
    const int iconHeight)
{
    return pGraphics_->InitMatIconFrameBuffers(
        numIcons,
        iconWidth,
        iconHeight,
        materialIcons_);
}

//---------------------------------------------------------
// Desc:  render a model into responsible frame buffer;
//        later we can use a texture with rendered model for any our purposes
//---------------------------------------------------------
bool FacadeEngineToUI::RenderModelFrameBuf()
{
    return pGraphics_->RenderModelIntoFrameBuf();
}

//==================================================================================
// model preview configuration (for model editor, or model screenshot tool)
//==================================================================================

//---------------------------------------------------------
// Desc:  SET a model's preview parameter value by input type
// NOTE:  different primitive types are casted to float (uint, booleans, etc.)
//        so we are able to use only one method (this)
//---------------------------------------------------------
void FacadeEngineToUI::SetModelPreviewParam(
    const eModelPreviewParams param,
    const float val)
{
    pGraphics_->SetModelPreviewParam(param, val);
}

//---------------------------------------------------------
// Desc:  GET a model's preview parameter value by input type
// NOTE:  different primitive types are casted to float (uint, booleans, etc.)
//        so we are able to use only one method (this)
//---------------------------------------------------------
float FacadeEngineToUI::GetModelPreviewParam(const eModelPreviewParams param) const
{
    return pGraphics_->GetModelPreviewParam(param);
}

//---------------------------------------------------------
// Desc:  get a shader resource view of rendered model
//---------------------------------------------------------
ID3D11ShaderResourceView* FacadeEngineToUI::GetModelFrameBufView() const
{
    ID3D11ShaderResourceView* pSRV = pGraphics_->GetModelFrameBufView();
    if (!pSRV)
    {
        LogErr(LOG, "can't receive a shader resource view of rendered model");
    }

    return pSRV;
}


//---------------------------------------------------------

static DirectX::Image texAtlasImg;
static uint atlasElemIdx = 0;

//---------------------------------------------------------
// Desc:  create an empty atlas texture and fill it with zeros;
//        this atlas will be built from the frames of the same size
//
// Args:  - elemWidth:   width of a single frame
//        - elemHeight:  height of a single frame (and atlas as well)
//        - numElems:    the number of frames
//---------------------------------------------------------
void FacadeEngineToUI::CreateEmptyTexAtlas(
    const uint elemWidth,
    const uint elemHeight,
    const uint numElems)
{
    const uint bytesPerPixel     = 4;
    const uint atlasWidth        = elemWidth * numElems;          // in pixels
    const uint atlasWidthInBytes = atlasWidth * bytesPerPixel;
    const uint atlasSizeInBytes  = atlasWidthInBytes * elemHeight;

    texAtlasImg.width      = atlasWidth;
    texAtlasImg.height     = elemHeight;
    texAtlasImg.format     = DXGI_FORMAT_R8G8B8A8_UNORM;
    texAtlasImg.rowPitch   = atlasWidthInBytes;
    texAtlasImg.slicePitch = atlasSizeInBytes;


    // prevent memory leak
    if (texAtlasImg.pixels != nullptr)
        SafeDeleteArr(texAtlasImg.pixels);

    // alloc memory
    texAtlasImg.pixels = NEW uint8_t[atlasSizeInBytes];
    if (!texAtlasImg.pixels)
    {
        LogErr(LOG, "can't alloc memory for texture atlas");
        return;
    }
    memset(texAtlasImg.pixels, 0, atlasSizeInBytes);
}

//---------------------------------------------------------
// Desc:  append pixels of input texture line by line at the right side of
//        the previous frame of the atlas, so it goes one by one like that:
//
//        *-------*-------*-------*-------*-------*
//        |       |       |       |       |       |
//        |   1   |   2   |   3   |   4   |   5   |
//        |       |       |       |       |       |
//        *-------*-------*-------*-------*-------*
//---------------------------------------------------------
void FacadeEngineToUI::PushTexIntoAtlas(ID3D11ShaderResourceView* pSRV)
{
    DirectX::ScratchImage srcImage;
    Img::ImgConverter converter;

    ID3D11Resource* pTexResource = nullptr;
    pSRV->GetResource(&pTexResource);

    converter.LoadFromMemory(pRender_->GetDevice(), pRender_->GetContext(), pTexResource, srcImage);

    const uint8_t* pixels = srcImage.GetPixels();
    uint srcOffset = 0;
    uint dstOffset = 0;

    uint srcRowPitch = (uint)srcImage.GetImages()->rowPitch;
    uint dstRowPitch = (uint)texAtlasImg.rowPitch;

    for (uint y = 0; y < texAtlasImg.height; ++y)
    {
        srcOffset = y * srcRowPitch;
        dstOffset = y * dstRowPitch + (atlasElemIdx * srcRowPitch);

        memcpy(texAtlasImg.pixels + dstOffset, pixels + srcOffset, srcRowPitch);
    }

    atlasElemIdx++;
}

//---------------------------------------------------------
// Desc:  save the atlas texture into a .dds file
//
// NOTE:  don't forget to clear memory from the atlas if you don't need it anymore
//---------------------------------------------------------
void FacadeEngineToUI::SaveTexAtlasToFile(const char* filename)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "can't save a texture atlas into file: input name is empty!");
        return;
    }

    // check if we have any data to save
    if (texAtlasImg.pixels == nullptr)
    {
        LogErr(LOG, "can't save a texture atlas into file: %s\n there is no data!!!", filename);
        return;
    }

    DirectX::ScratchImage scratchImg;
    HRESULT hr = scratchImg.InitializeFromImage(texAtlasImg);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create scratch image from src image");
        SafeDeleteArr(texAtlasImg.pixels);
        return;
    }

    // save into .dds
    Img::ImgConverter converter;
    converter.SaveToFile(scratchImg, DirectX::DDS_FLAGS_NONE, filename);
}

//---------------------------------------------------------
// Desc:  clear memory from the atlas texture, and reset some counters
//---------------------------------------------------------
void FacadeEngineToUI::ClearTexAtlasMemory()
{
    SafeDeleteArr(texAtlasImg.pixels);
    atlasElemIdx = 0;
}

//---------------------------------------------------------
// Desc:  save input texture into .dds file (into screenshot directory)
// 
// Args:  - filename:      a path for saving the texture (relatively to working directory)
//        - pSRV:
//        - targetFormat:  output image compression type or save without compression
//                         (by default no compression)
//---------------------------------------------------------
void FacadeEngineToUI::SaveTexToFile(
    const char* filename,
    SRV* pSRV,
    const DXGI_FORMAT targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "can't save texture into file: input filename is empty!");
        return;
    }

    if (!pSRV)
    {
        LogErr(LOG, "input ptr to shader resource view == nullptr for texture: %s", filename);
        return;
    }

    // target image params
    //DXGI_FORMAT targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    //const DXGI_FORMAT targetFormat = DXGI_FORMAT_BC3_UNORM;
    const DirectX::TEX_FILTER_FLAGS filter = DirectX::TEX_FILTER_DEFAULT;

    // ---------------------------------------------

    // load a texture data from memory and store it as .dds file
    DirectX::ScratchImage srcImage;
    DirectX::ScratchImage dstImage;
    Img::ImgConverter converter;

    ID3D11Resource* pTexResource = nullptr;
    pSRV->GetResource(&pTexResource);


    converter.LoadFromMemory(pRender_->GetDevice(), pRender_->GetContext(), pTexResource, srcImage);

    // if we need to execute any processing
    if (srcImage.GetMetadata().format != targetFormat)
    {
        if (srcImage.GetMetadata().width == 1 || srcImage.GetMetadata().height == 1)
        {
            LogErr(LOG, "it isn't possible to create a mipmap for 1x1 texture so skip storing texture into .dds: %s", filename);
            return;
        }

        DirectX::ScratchImage mipChain(converter.GenMipMaps(srcImage, filter));

        // do compression if necessary (according to targetFormat)
        converter.ProcessImage(mipChain, targetFormat, dstImage);
        
        // save into .dds
        converter.SaveToFile(dstImage, DirectX::DDS_FLAGS_NONE, filename);

        LogMsg(LOG, "texture is saved into: %s", filename);
    }
    // src texture is already has the proper format
    else
    {
        dstImage = std::move(converter.GenMipMaps(srcImage, filter));
        converter.SaveToFile(dstImage, DirectX::DDS_FLAGS_NONE, filename);
    }
}

//---------------------------------------------------------
// Desc:   in the material browser: we see a bunch of material icons
//         (so here we call its rendering)
//---------------------------------------------------------
bool FacadeEngineToUI::RenderMaterialsIcons() const
{
    return pGraphics_->RenderMaterialsIcons();
}

//---------------------------------------------------------
// Desc:   in the material browser we can select editing of the chosen material;
//         in the editor window we need to visualize the current state of the
//         material so we render this big material icon
//---------------------------------------------------------
bool FacadeEngineToUI::RenderMaterialBigIconById(
    const MaterialID matID,
    const float yAxisRotation)
{
    return pGraphics_->RenderBigMaterialIcon(matID, yAxisRotation, &pMaterialBigIcon_);
}

//---------------------------------------------------------
// Desc:   get an arr of elems [shaderId, shaderName, etc.] about
//         all the currently loaded shaders
//---------------------------------------------------------
void FacadeEngineToUI::GetShadersIdsAndNames(
    cvector<ShaderID>& outIds,
    cvector<ShaderName>& outNames) const
{
    pRender_->GetArrShadersIds(outIds);
    pRender_->GetArrShadersNames(outNames);
}

//---------------------------------------------------------
// Desc:   setup a rendering shader for material
// Args:   - matId:     a material by this identifier will be changed
//         - shaderId:  a shader by this identifier will be used to render
//---------------------------------------------------------
bool FacadeEngineToUI::SetMaterialShaderId(
    const MaterialID matId,
    const ShaderID shaderId)
{
    if (!pRender_->ShaderExist(shaderId))
    {
        LogErr(LOG, "can't setup shader (id: %" PRIu32 ") \n\t"
                    "for material (id: %d" PRIu32 ") because there is no such shader", shaderId, matId);
        return false;
    }

    // setup rendering shader for material
    Material& mat = g_MaterialMgr.GetMatById(matId);
    mat.shaderId = shaderId;

    return true;
}


// =============================================================================
// PARTICLES
// =============================================================================
bool FacadeEngineToUI::SetParticlesColor(const EntityID id, const ColorRGB& color)
{
    pEnttMgr_->particleSystem_.SetColor(id, color.r, color.g, color.b);
    return true;
}

bool FacadeEngineToUI::SetParticlesExternForce(const EntityID id, const Vec3& force)
{
    pEnttMgr_->particleSystem_.SetExternForces(id, force.x, force.y, force.z);
    return true;
}

bool FacadeEngineToUI::SetParticlesSpawnRate(const EntityID id, const int num)
{
    pEnttMgr_->particleSystem_.SetSpawnRate(id, num);
    return true;
}

bool FacadeEngineToUI::SetParticlesLifespanMs(const EntityID id, const int milliseconds)
{
    pEnttMgr_->particleSystem_.SetLife(id, (float)milliseconds);
    return true;
}

bool FacadeEngineToUI::SetParticlesMass(const EntityID id, const float mass)
{
    pEnttMgr_->particleSystem_.SetMass(id, mass);
    return true;
}

bool FacadeEngineToUI::SetParticlesSize(const EntityID id, const float size)
{
    pEnttMgr_->particleSystem_.SetSize(id, size);
    return true;
}

bool FacadeEngineToUI::SetParticlesFriction(const EntityID id, const float airResistance)
{
    pEnttMgr_->particleSystem_.SetFriction(id, airResistance);
    return true;
}

//---------------------------------------------------------
// Desc:   get params of particle emitter which is bound to entt by ID
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttParticleEmitterData(
    const EntityID id,
    ColorRGB& color,
    Vec3& externForce,
    int& spawnRate,
    int& lifespanMs,
    float& mass,
    float& size,
    float& friction)
{
    ECS::ParticleEmitter& emitter = pEnttMgr_->particleSystem_.GetEmitterByEnttId(id);

    color           = emitter.startColor;
    externForce.x   = DirectX::XMVectorGetX(emitter.forces);
    externForce.y   = DirectX::XMVectorGetY(emitter.forces);
    externForce.z   = DirectX::XMVectorGetZ(emitter.forces);
    spawnRate       = emitter.spawnRate;
    lifespanMs      = (int)(emitter.life * 1000.0f);
    mass            = emitter.mass;
    size            = emitter.size;
    friction        = emitter.friction;

    return true;
}


// =============================================================================
// TERRAIN
// =============================================================================

//---------------------------------------------------------
// Desc:   return a maximal number for terrain LOD
//---------------------------------------------------------
int FacadeEngineToUI::GetTerrainNumMaxLOD() const
{
    return pTerrain_->GetNumMaxLOD();
}

//---------------------------------------------------------
// Desc:   return a distance from the camera where LOD starts
//---------------------------------------------------------
int FacadeEngineToUI::GetTerrainDistanceToLOD(const int lod) const
{
    const int dist = pTerrain_->GetDistanceToLOD(lod);

    if (dist == -1)
        LogErr(LOG, "can't get a distance to lod: %d", lod);

    return dist;
}

//---------------------------------------------------------
// Desc:   set a distance from the camera where LOD starts
// Args:   - lod:   lod number to change
//         - dist:  new distance to this lod
//---------------------------------------------------------
bool FacadeEngineToUI::SetTerrainDistanceToLOD(const int lod, const int dist)
{
    return pTerrain_->SetDistanceToLOD(lod, dist);
}

// =============================================================================
// GRASS
// =============================================================================
float FacadeEngineToUI::GetGrassDistFullSize() const
{
    return pRender_->GetGrassDistFullSize();
}

float FacadeEngineToUI::GetGrassDistVisible() const
{
    return pRender_->GetGrassDistVisible();
}

bool FacadeEngineToUI::SetGrassDistFullSize(const float dist)
{
    return pRender_->SetGrassDistFullSize(dist);
}

bool FacadeEngineToUI::SetGrassDistVisible(const float dist)
{
    return pRender_->SetGrassDistVisible(dist);
}

// =============================================================================
// draw debug shapes (turn on/off rendering of particular debug shapes)
// =============================================================================
void FacadeEngineToUI::SwitchRenderDebugShape(
    const eRenderDbgShape shapeType,
    const bool state)
{
    switch (shapeType)
    {
        case RENDER_DBG_SHAPES:              g_DebugDrawMgr.doRendering_          = state; break;
        case RENDER_DBG_SHAPES_LINE:         g_DebugDrawMgr.renderDbgLines_       = state; break;
        case RENDER_DBG_SHAPES_CROSS:        g_DebugDrawMgr.renderDbgCross_       = state; break;
        case RENDER_DBG_SHAPES_SPHERE:       g_DebugDrawMgr.renderDbgSphere_      = state; break;
             
        case RENDER_DBG_SHAPES_CIRCLE:       g_DebugDrawMgr.renderDbgCircle_      = state; break;
        case RENDER_DBG_SHAPES_AXES:         g_DebugDrawMgr.renderDbgAxes_        = state; break;
        case RENDER_DBG_SHAPES_TRIANGLE:     g_DebugDrawMgr.renderDbgTriangle_    = state; break;

        case RENDER_DBG_SHAPES_AABB:         g_DebugDrawMgr.renderDbgAABB_        = state; break;
        case RENDER_DBG_SHAPES_OBB:          g_DebugDrawMgr.renderDbgOBB_         = state; break;
        case RENDER_DBG_SHAPES_FRUSTUM:      g_DebugDrawMgr.renderDbgFrustum_     = state; break;
        case RENDER_DBG_SHAPES_TERRAIN_AABB: g_DebugDrawMgr.renderDbgTerrainAABB_ = state; break;

        default:
        {
            LogErr(LOG, "can't switch rendering for unknown debug shape type: %d", (int)shapeType);
        }
    }
}

//---------------------------------------------------------
// Desc:  enable/disable visualization of the depth
// Desc:  check if visualization of the depth is enabled
//---------------------------------------------------------
void FacadeEngineToUI::EnableDepthVisualize(const bool state)
{
    pGraphics_->EnableDepthVisualize(state);
}

bool FacadeEngineToUI::IsEnabledDepthVisualize() const
{
    return pGraphics_->IsEnabledDepthVisualize();
}

//---------------------------------------------------------
// Desc:  enable/disable executing the depth prepass before
//        the main color/light rendering pass
// Desc:  check if depth prepass is enabled
//---------------------------------------------------------
void FacadeEngineToUI::EnableDepthPrepass(const bool state)
{
    pGraphics_->EnableDepthPrepass(state);
}

bool FacadeEngineToUI::IsEnabledDepthPrepass() const
{
    return pGraphics_->IsEnabledDepthPrepass();
}

//---------------------------------------------------------
// Desc:  turn on/off using of post effects
//---------------------------------------------------------
void FacadeEngineToUI::EnablePostFxs(const bool state)
{
    pGraphics_->EnablePostFxs(state);
}

//---------------------------------------------------------
// Desc:  is using of post effects enabled?
//---------------------------------------------------------
bool FacadeEngineToUI::IsPostFxsEnabled() const
{
    return pGraphics_->IsPostFxsEnabled();
}

//---------------------------------------------------------
// Desc:  get a ptr to queue of currently used post effects
//        (a ptr to arr of ePostFxType)
//---------------------------------------------------------
void FacadeEngineToUI::GetPostFxsQueue(const void** queue)
{
    *queue = pGraphics_->GetPostFxsQueue();
}

//---------------------------------------------------------
// Desc:  return a number of currently used post effects
//---------------------------------------------------------
uint8 FacadeEngineToUI::GetNumUsedPostFxs() const
{
    return pGraphics_->GetNumUsedPostFxs();
}

//---------------------------------------------------------
// Desc:  push back a post fx into the post effects rendering queue
// NOTE:  as input param we pass ePostFxParam casted to uint16
//---------------------------------------------------------
void FacadeEngineToUI::PushPostFx(const uint16 type)
{
    pGraphics_->PushPostFx((ePostFxType)type);
}

//---------------------------------------------------------
// Desc:  remove a post fx by input order number from the post effects rendering queue
//---------------------------------------------------------
void FacadeEngineToUI::RemovePostFx(const uint8 orderNum)
{
    pGraphics_->RemovePostFx(orderNum);
}

//---------------------------------------------------------
// Desc:  update a post effect parameter by input type (param)
// NOTE:  as input param we pass ePostFxParam casted to uint16
//---------------------------------------------------------
void FacadeEngineToUI::SetPostFxParam(const uint16 param, const float value)
{
    pRender_->SetPostFxParam(param, value);
}

//---------------------------------------------------------
// Desc:  get a post effect parameter value by input param type
// NOTE:  as input param we pass ePostFxParam casted to uint16
//---------------------------------------------------------
float FacadeEngineToUI::GetPostFxParam(const uint16 param) const
{
    return pRender_->GetPostFxParam(param);
}

} // namespace Core
