// =================================================================================
// Filename:  FacadeEngineToUI.cpp
// 
// Created:   31.12.24
// =================================================================================
#include "FacadeEngineToUI.h"
#include <CoreCommon/Assert.h>
#include <CoreCommon/log.h>
#include <CoreCommon/MathHelper.h>

using namespace Core;

namespace UI
{

FacadeEngineToUI::FacadeEngineToUI(
    ID3D11DeviceContext* pContext,
    Render::Render* pRender,
    ECS::EntityMgr* pEntityMgr,
    TextureMgr* pTextureMgr,
    ModelStorage* pModelStorage,
    Camera* pEditorCamera)
    :
    pContext_(pContext),
    pRender_(pRender),
    pEntityMgr_(pEntityMgr),
    pTextureMgr_(pTextureMgr),
    pModelStorage_(pModelStorage),
    pEditorCamera_(pEditorCamera)
{
    // set pointers to the subsystems

    Assert::NotNullptr(pRender, "ptr to render == nullptr");
    Assert::NotNullptr(pContext, "ptr to device context == nullptr");
    Assert::NotNullptr(pEntityMgr, "ptr to the entt mgr == nullptr");
    Assert::NotNullptr(pTextureMgr, "ptr to the texture mgr == nullptr");
    Assert::NotNullptr(pModelStorage, "ptr to the model storage == nullptr");
    Assert::NotNullptr(pEditorCamera, "ptr to the editor camera == nullptr");
}

///////////////////////////////////////////////////////////

ModelID FacadeEngineToUI::GetModelIdByName(const std::string& name)
{
    return pModelStorage_->GetModelIdByName(name);
}


// =================================================================================
// For using the textures manager
// =================================================================================
bool FacadeEngineToUI::GetShaderResourceViewByTexID(
    const uint32_t textureID,
    ID3D11ShaderResourceView*& pSRV)
{
    pTextureMgr_->GetSRVByTexID(textureID, pSRV);
    return true;
}

// =================================================================================
// Get camera info
// =================================================================================
void FacadeEngineToUI::GetCameraViewAndProj(
    const EntityID camEnttID,
    float* outView,
    float* outProj)
{
    assert(outView && outProj && "some of input ptrs == NULL");

    DirectX::XMMATRIX view;
    DirectX::XMMATRIX proj;

    pEntityMgr_->cameraSystem_.GetViewAndProjByID(camEnttID, view, proj);

    // copy view and proj matrices into raw array of 16 floats
    memcpy(outView, (void*)view.r->m128_f32, sizeof(float) * 16);
    memcpy(outProj, (void*)proj.r->m128_f32, sizeof(float) * 16);
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::FocusCameraOnEntity(const EntityID enttID)
{
    // focus the editor camera on the selected entity

    if (enttID == 0)
        return;

    ECS::EntityMgr& mgr = *pEntityMgr_;
    Camera& cam = *pEditorCamera_;

    DirectX::XMFLOAT3 enttPos = mgr.transformSystem_.GetPositionByID(enttID);
    cam.LookAt(cam.GetPosition(), enttPos, { 0, 1, 0 });
}



// =================================================================================
// For the entity editor
// =================================================================================
EntityID FacadeEngineToUI::CreateEntity()
{
    return pEntityMgr_->CreateEntity();
}

bool FacadeEngineToUI::AddNameComponent(const EntityID id, const std::string& name)
{
    pEntityMgr_->AddNameComponent(id, name);
    return true;
}

bool FacadeEngineToUI::AddTransformComponent(const EntityID id, const Vec3& pos, const Vec3& direction, const float uniformScale)
{
    pEntityMgr_->AddTransformComponent(id, pos.ToFloat3(), direction.ToXMVector(), uniformScale);
    return true;
}

bool FacadeEngineToUI::AddModelComponent(const EntityID enttID, const uint32_t modelID)
{
    pEntityMgr_->AddModelComponent(enttID, modelID);
    return true;
}

bool FacadeEngineToUI::AddRenderedComponent(const EntityID enttID)
{
    ECS::RenderInitParams renderParams;
    renderParams.shaderType = ECS::LIGHT_SHADER;
    renderParams.topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    pEntityMgr_->AddRenderingComponent(enttID, renderParams);
    return true;
}

bool FacadeEngineToUI::AddBoundingComponent(const EntityID id, const int boundType, const DirectX::BoundingBox& aabb)
{
    pEntityMgr_->AddBoundingComponent(id, ECS::BoundingType(1), aabb);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts)
{
    pEnttsIDsArr = pEntityMgr_->GetAllEnttsIDs().data() + 1;    // +1 because entity by [0] is the default invalid entity
    numEntts = static_cast<int>(pEntityMgr_->GetAllEnttsIDs().size() - 1);
    return true;
}

///////////////////////////////////////////////////////////

uint32_t FacadeEngineToUI::GetEnttIDByName(const char* name)
{
    // return 0 if there is no entity by such a name
    return pEntityMgr_->nameSystem_.GetIdByName({ name });
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttNameByID(const uint32_t enttID, std::string& name)
{
    name = pEntityMgr_->nameSystem_.GetNameById(enttID);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttsIDsOfTypeModel(const EntityID*& enttsIDs, int& numEntts)
{
    pEntityMgr_->modelSystem_.GetAllEntts(enttsIDs, (size&)numEntts);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttsIDsOfTypeCamera(const EntityID*& enttsIDs, int& numEntts)
{
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttsIDsOfTypeLight(const EntityID*& enttsIDs, int& numEntts)
{
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttData(
    const EntityID enttID,
    Vec3& position,
    Vec4& rotQuat,
    float& uniformScale)
{
    position = pEntityMgr_->transformSystem_.GetPositionByID(enttID);
    rotQuat = pEntityMgr_->transformSystem_.GetRotationQuatByID(enttID);
    uniformScale = pEntityMgr_->transformSystem_.GetUniformScaleByID(enttID);

    return true;
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::GetEnttWorldMatrix(const EntityID id, XMMATRIX& outMat)
{
    outMat = pEntityMgr_->transformSystem_.GetWorldMatrixOfEntt(id);
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetEnttPosition(const EntityID id) const
{
    return pEntityMgr_->transformSystem_.GetPositionByID(id);
}

Vec4 FacadeEngineToUI::GetEnttRotationQuat(const EntityID id) const
{
    return pEntityMgr_->transformSystem_.GetRotationQuatByID(id);
}

float FacadeEngineToUI::GetEnttScale(const EntityID id) const
{
    return pEntityMgr_->transformSystem_.GetUniformScaleByID(id);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetEnttPosition(const uint32_t entityID, const Vec3& pos)
{
    pEntityMgr_->transformSystem_.SetPositionByID(entityID, pos.ToFloat3());
    return true;
}

bool FacadeEngineToUI::SetEnttRotationQuat(const uint32_t entityID, const Vec4& rotationQuat)
{
    pEntityMgr_->transformSystem_.SetRotationQuatByID(entityID, rotationQuat.ToXMVector());
    return true;
}

bool FacadeEngineToUI::SetEnttUniScale(const uint32_t entityID, const float scale)
{
    pEntityMgr_->transformSystem_.SetUniScaleByID(entityID, scale);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::IsEnttLightSource(const EntityID id, int& lightType) const
{
    // return flag to define if input entt is a light source;
    // if it's a light source we return its type code in lightType argument or -1 if not
    if (pEntityMgr_->lightSystem_.IsLightSource(id))
    {
        lightType = pEntityMgr_->lightSystem_.GetLightType(id);
        return true;
    }
    else
    {
        lightType = -1;
        return false;
    }
}


// =============================================================================
// get all the data of light entity by ID
// =============================================================================
bool FacadeEngineToUI::GetEnttDirectedLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular,
    Vec3& direction)
{
    // get data of a directed light entity by input ID

    ECS::DirLight data;

    if (pEntityMgr_->lightSystem_.GetDirectedLightData(id, data))
    {
        ambient   = data.ambient;
        diffuse   = data.diffuse;
        specular  = data.specular;
        direction = data.direction;

        return true;
    }
    else
    {
        Log::Error("can't get data of the directed light entity by ID: " + std::to_string(id));
        return false;
    }
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttPointLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular,
    Vec3& position,
    float& range,
    Vec3& attenuation)
{
    // get data of a point light entity by input ID

    ECS::PointLight data;

    if (pEntityMgr_->lightSystem_.GetPointLightData(id, data))
    {
        ambient     = data.ambient;
        diffuse     = data.diffuse;
        specular    = data.specular;
        position    = data.position;
        range       = data.range;
        attenuation = data.att;

        return true;
    }
    else
    {
        Log::Error("can't get data of the point light entity by ID: " + std::to_string(id));
        return false;
    }
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttSpotLightData(
    const EntityID id,
    ColorRGBA& ambient,
    ColorRGBA& diffuse,
    ColorRGBA& specular,
    Vec3& position,
    float& range,
    Vec3& direction,
    float& spotExponent,
    Vec3& attenuation)
{
    // get all the data of the spotlight entity by ID
    ECS::SpotLight spotlight;

    if (pEntityMgr_->lightSystem_.GetSpotLightData(id, spotlight))
    {
        ambient      = spotlight.ambient;
        diffuse      = spotlight.diffuse;
        specular     = spotlight.specular;
        position     = spotlight.position;
        range        = spotlight.range;
        direction    = spotlight.direction;
        spotExponent = spotlight.spot;
        attenuation  = spotlight.att;

        return true;
    }
    else
    {
        Log::Error("can't get data of the spotlight entity by ID: " + std::to_string(id));
        return false;
    }
}


// =============================================================================
// SET directed light props
// =============================================================================
bool FacadeEngineToUI::SetDirectedLightAmbient(const EntityID id, const ColorRGBA& rgba)
{
    return pEntityMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::AMBIENT, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightDiffuse(const EntityID id, const ColorRGBA& rgba)
{
    return pEntityMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::DIFFUSE, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightSpecular(const EntityID id, const ColorRGBA& rgba)
{
    return pEntityMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::SPECULAR, rgba.ToFloat4());
}

bool FacadeEngineToUI::SetDirectedLightDirection(const EntityID id, const Vec3& dir)
{
    if (pEntityMgr_->lightSystem_.SetDirLightProp(id, ECS::LightProp::DIRECTION, dir.ToFloat3()))
    {
        return true;
    }

    return false;
}


// =============================================================================
// GET directed light props
// =============================================================================
ColorRGBA FacadeEngineToUI::GetDirectedLightAmbient(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetDirectedLightDiffuse(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetDirectedLightSpecular(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::SPECULAR);
}

Vec3 FacadeEngineToUI::GetDirectedLightDirection(const EntityID id) const
{
    XMFLOAT4 dir = pEntityMgr_->lightSystem_.GetDirLightProp(id, ECS::LightProp::DIRECTION);
    return Vec3(dir.x, dir.y, dir.z);
}


// =================================================================================
// SET point light properties
// =================================================================================

bool FacadeEngineToUI::SetPointLightAmbient(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::AMBIENT, color.ToFloat4());
}

bool FacadeEngineToUI::SetPointLightDiffuse(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::DIFFUSE, color.ToFloat4());
}

bool FacadeEngineToUI::SetPointLightSpecular(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::SPECULAR, color.ToFloat4());
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightPos(const EntityID id, const Vec3& pos)
{
    if (pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::POSITION, pos.ToFloat4()))
    {
        return SetEnttPosition(id, pos);
    }

    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightRange(const EntityID id, const float range)
{
    return pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::RANGE, range);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetPointLightAttenuation(const EntityID id, const Vec3& att)
{
    return pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::ATTENUATION, att.ToFloat4());
}


// =================================================================================
// GET point light properties
// =================================================================================

ColorRGBA FacadeEngineToUI::GetPointLightAmbient(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetPointLightDiffuse(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetPointLightSpecular(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::SPECULAR);
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetPointLightPos(const EntityID id) const
{
    // position values are stored in x,y,z
    Vec4 pos = pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::POSITION);
    return Vec3(pos.x, pos.y, pos.z);
}

Vec3 FacadeEngineToUI::GetPointLightAttenuation(const EntityID id) const
{
    // attenuation values are stored in x,y,z
    Vec4 att = pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::ATTENUATION);
    return Vec3(att.x, att.y, att.z);
}

///////////////////////////////////////////////////////////

float FacadeEngineToUI::GetPointLightRange(const EntityID id) const
{
    // the same value of range is stored in each component of Vec4
    Vec4 range = pEntityMgr_->lightSystem_.GetPointLightProp(id, ECS::LightProp::RANGE);
    return range.x;
}


// =================================================================================
// SET spotlight properties
// =================================================================================

bool FacadeEngineToUI::SetSpotLightAmbient(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::AMBIENT, color.ToFloat4());
}

bool FacadeEngineToUI::SetSpotLightDiffuse(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::DIFFUSE, color.ToFloat4());
}

bool FacadeEngineToUI::SetSpotLightSpecular(const EntityID id, const ColorRGBA& color)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::SPECULAR, color.ToFloat4());
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSpotLightPos(const EntityID id, const Vec3& pos)
{
    if (pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::POSITION, pos.ToFloat4()))
    {
        return SetEnttPosition(id, pos);
    }

    return false;
}

bool FacadeEngineToUI::SetSpotLightDirection(const EntityID id, const Vec3& dir)
{
    if (pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::DIRECTION, { dir.x, dir.y, dir.z, 0.0f }))
    {
        return true;
        //return SetEnttRotationQuat(id, dirQuat);
    }
    return false;
}

bool FacadeEngineToUI::SetSpotLightAttenuation(const EntityID id, const Vec3& att)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::ATTENUATION, att.ToFloat4());
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSpotLightRange(const EntityID id, const float range)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::RANGE, range);
}

bool FacadeEngineToUI::SetSpotLightSpotExponent(const EntityID id, const float spotExp)
{
    return pEntityMgr_->lightSystem_.SetSpotLightProp(id, ECS::LightProp::SPOT_EXP, spotExp);
}


// =================================================================================
// GET spotlight properties
// =================================================================================

ColorRGBA FacadeEngineToUI::GetSpotLightAmbient(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::AMBIENT);
}

ColorRGBA FacadeEngineToUI::GetSpotLightDiffuse(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::DIFFUSE);
}

ColorRGBA FacadeEngineToUI::GetSpotLightSpecular(const EntityID id) const
{
    return pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::SPECULAR);
}

///////////////////////////////////////////////////////////

Vec3 FacadeEngineToUI::GetSpotLightPos(const EntityID id) const
{
    // position values are stored in x,y,z
    Vec4 pos = pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::POSITION);
    return pos.ToVec3();
}

Vec3 FacadeEngineToUI::GetSpotLightDirection(const EntityID id) const
{
    const XMFLOAT4 dir = pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::DIRECTION);
    //const XMVECTOR q   = DirectX::XMQuaternionRotationRollPitchYaw(dir.y, dir.x, dir.w);
    //const XMVECTOR q = pEntityMgr_->transformSystem_.GetRotationQuatByID(id);

    return { dir.x, dir.y, dir.z };
}

Vec3 FacadeEngineToUI::GetSpotLightAttenuation(const EntityID id) const
{
    // attenuation values are stored in x,y,z
    Vec4 v = pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::ATTENUATION);
    return v.ToVec3();
}

///////////////////////////////////////////////////////////

float FacadeEngineToUI::GetSpotLightRange(const EntityID id) const
{
    // the same value of range is stored in each component of Vec4
    Vec4 range = pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::RANGE);
    return range.x;
}

float FacadeEngineToUI::GetSpotLightSpotExponent(const EntityID id) const
{
    // the same value of exponent is stored in each component of Vec4
    Vec4 spotExp = pEntityMgr_->lightSystem_.GetSpotLightProp(id, ECS::LightProp::SPOT_EXP);
    return spotExp.x;
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

    const Render::SkyDomeShader& skyDomeShader = pRender_->GetShadersContainer().skyDomeShader_;

    center = skyDomeShader.GetColorCenter();
    apex = skyDomeShader.GetColorApex();
    offset = pEntityMgr_->transformSystem_.GetPositionByID(skyEnttID);

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyColorCenter(const ColorRGB& color)
{
    pRender_->SetSkyColorCenter(pContext_, color.ToFloat3());
    return true;
}

bool FacadeEngineToUI::SetSkyColorApex(const ColorRGB& color)
{
    pRender_->SetSkyColorApex(pContext_, color.ToFloat3());
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyOffset(const Vec3& offset)
{
    const EntityID enttID = pEntityMgr_->nameSystem_.GetIdByName("sky");

    // if we found the sky entity we change its offset
    if (enttID != INVALID_ENTITY_ID)
    {
        pEntityMgr_->transformSystem_.SetPositionByID(enttID, offset.ToFloat3());
        return true;
    }

    Log::Error("there is no entity by such a name: sky");
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyTexture(const int idx, const uint32_t textureID)
{
    return true;
}


// =================================================================================
// For the fog editor
// =================================================================================

bool FacadeEngineToUI::GetFogData(
    ColorRGB& fogColor,
    float& fogStart,     // distance where the fog starts
    float& fogRange)     // distance after which the objects are fully fogged
{
    DirectX::XMFLOAT3 color;

    pRender_->GetFogData(color, fogStart, fogRange);
    fogColor = { color.x, color.y, color.z };

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetFogParams(const ColorRGB& color, const float start, const float range)
{
    pRender_->SetFogParams(pContext_, color.ToFloat3(), start, range);
    return true;
}


// =================================================================================
// For the debug editor
// =================================================================================

bool FacadeEngineToUI::SwitchDebugState(const int debugType)
{
    pRender_->SwitchDebugState(pContext_, Render::DebugState(debugType));
    return true;
}


// =================================================================================
// For assets manager
// =================================================================================
bool FacadeEngineToUI::GetAssetsNamesList(Core::cvector<std::string>& names)
{
    // get a name of each loaded asset
    pModelStorage_->GetAssetsNamesList(names);
    return true;
}

} // namespace Core
