// =================================================================================
// Filename:  FacadeEngineToUI.cpp
// 
// Created:   31.12.24
// =================================================================================
#include <CoreCommon/pch.h>
#include "FacadeEngineToUI.h"

#include "../../Mesh/MaterialMgr.h"
#include "../../Model/ModelMgr.h"
#include "../../Texture/TextureMgr.h"   // texture mgr is used to get textures by its IDs
#include <Shaders/Shader.h>

#pragma warning (disable : 4996)

using namespace DirectX;
using namespace Core;

namespace UI
{

FacadeEngineToUI::FacadeEngineToUI(
    ID3D11DeviceContext* pContext,
    Render::CRender* pRender,
    ECS::EntityMgr* pEntityMgr,
    Core::CGraphics* pGraphics,
    Core::TerrainGeomip* pTerrain)
    :
    pContext_(pContext),
    pRender_(pRender),
    pEnttMgr_(pEntityMgr),
    pGraphics_(pGraphics),
    pTerrain_(pTerrain)
{
    // set pointers to the subsystems
    CAssert::NotNullptr(pRender,    "a ptr to render == nullptr");
    CAssert::NotNullptr(pContext,   "a ptr to device context == nullptr");
    CAssert::NotNullptr(pEntityMgr, "a ptr to the entt mgr == nullptr");
    CAssert::NotNullptr(pGraphics,  "a ptr to the graphics class == nullptr");
    CAssert::NotNullptr(pTerrain,   "a ptr to the terrain == nullptr");
}

///////////////////////////////////////////////////////////

ModelID FacadeEngineToUI::GetModelIdByName(const std::string& name)
{
    return g_ModelMgr.GetModelIdByName(name.c_str());
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
        LogErr("some of input ptrs == NULL");
        return;
    }

    DirectX::XMMATRIX view = pEnttMgr_->cameraSystem_.GetView(camEnttID);
    DirectX::XMMATRIX proj = pEnttMgr_->cameraSystem_.GetProj(camEnttID);

    // copy view and proj matrices into raw array of 16 floats
    memcpy(outView, (void*)view.r->m128_f32, sizeof(float) * 16);
    memcpy(outProj, (void*)proj.r->m128_f32, sizeof(float) * 16);
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::FocusCameraOnEntity(const EntityID enttID)
{
    // focus the editor camera on the selected entity

    assert(0 && "FIXME");
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

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEntityAddedComponentsNames(
    const EntityID id,
    cvector<std::string>& componentsNames) const
{
    // out:    names array of components which are added to entity by ID;
    // return: false if there is no entity by ID in the Entity Manager

    if (pEnttMgr_->GetComponentNamesByEntt(id, componentsNames))
        return true;

    // we didn't manage to get components names
    LogErr("can't get components names by entity");
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEntityAddedComponentsTypes(
    const EntityID id,
    cvector<eEnttComponentType>& componentTypes) const
{
    // out:    array of components types which are added to entity by ID;
    // return: false if there is no entity by ID in the Entity Manager

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

bool FacadeEngineToUI::AddTransformComponent(const EntityID id, const Vec3& pos, const Vec3& dir, const float uniformScale)
{
    pEnttMgr_->AddTransformComponent(id, XMFLOAT3(pos.x, pos.y, pos.z), XMVECTOR{ dir.x, dir.y, dir.z }, uniformScale);
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
    pEnttMgr_->AddBoundingComponent(id, ECS::BoundingType(1), aabb);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetAllEnttsIDs(
    const EntityID*& outPtrToEnttsIDsArr,
    int& outNumEntts) const
{
    outPtrToEnttsIDsArr = pEnttMgr_->GetAllEnttsIDs();    // +1 because entity by [0] is the default invalid entity
    outNumEntts         = static_cast<int>(pEnttMgr_->GetNumAllEntts());
    return true;
}

///////////////////////////////////////////////////////////

EntityID FacadeEngineToUI::GetEnttIdByName(const char* name) const
{
    // return 0 if there is no entity by such a name
    return pEnttMgr_->nameSystem_.GetIdByName(name);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttNameById(const EntityID enttID, std::string& name) const
{
    name = pEnttMgr_->nameSystem_.GetNameById(enttID);
    return true;
}

//---------------------------------------------------------
// Desc:   get position, direction, and uniform scale of entt by ID
//---------------------------------------------------------
bool FacadeEngineToUI::GetEnttTransformData(
    const EntityID id,
    Vec3& outPos,
    Vec3& outDir,
    float& outScale) const
{
    const ECS::TransformSystem& sys = pEnttMgr_->transformSystem_;
    const XMFLOAT3 pos = sys.GetPosition(id);
    const XMFLOAT3 dir = sys.GetDirection(id);
    const float scale  = sys.GetUniformScale(id);

    outPos = Vec3(pos.x, pos.y, pos.z);
    outDir = Vec3(dir.x, dir.y, dir.z);
    outScale = scale;

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetEnttWorldMatrix(const EntityID id, DirectX::XMMATRIX& outMat) const
{
    outMat = pEnttMgr_->transformSystem_.GetWorldMatrixOfEntt(id);

    // if we have NAN in the m00 component of matrix that means there is no world matrix in the Transform component (ECS) for entity by ID 
    return isnan(DirectX::XMVectorGetX(outMat.r[0]));
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

float FacadeEngineToUI::GetEnttScale(const EntityID id) const
{
    return pEnttMgr_->transformSystem_.GetUniformScale(id);
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
    if (pEnttMgr_->lightSystem_.IsLightSource(id))
    {
        lightType = pEnttMgr_->lightSystem_.GetLightType(id);
        return true;
    }
    else
    {
        lightType = -1;
        return false;
    }
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

    if (pEnttMgr_->lightSystem_.GetDirectedLightData(id, data))
    {
        ambient   = data.ambient;
        diffuse   = data.diffuse;
        specular  = data.specular;

        return true;
    }
    else
    {
        LogErr(LOG, "can't get directed light data of the entity by ID: %" PRIu32, id);
        return false;
    }
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
    float& range)
{
    ECS::PointLight data;

    if (pEnttMgr_->lightSystem_.GetPointLightData(id, data))
    {
        ambient     = data.ambient;
        diffuse     = data.diffuse;
        specular    = data.specular;
        range       = data.range;
        attenuation = Vec3(data.att.x, data.att.y, data.att.z);

        return true;
    }
    else
    {
        LogErr(LOG, "can't get point light data of the entity by ID: %" PRIu32, id);
        return false;
    }
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

    if (pEnttMgr_->lightSystem_.GetSpotLightData(id, data))
    {
        ambient      = data.ambient;
        diffuse      = data.diffuse;
        specular     = data.specular;
        range        = data.range;
        spotExponent = data.spot;
        attenuation  = Vec3(data.att.x, data.att.y, data.att.z);

        return true;
    }
    else
    {
        LogErr(LOG, "can't get spotlight data of the entity by ID: %" PRIu32, id);
        return false;
    }
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
    if (pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::POSITION, XMFLOAT4(pos.x, pos.y, pos.z, 1.0f)))
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
    return pEnttMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProp::ATTENUATION, XMFLOAT4(att.x, att.y, att.z, 1.0f));
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
    const EntityID id = pEnttMgr_->nameSystem_.GetIdByName("sky");

    // if we found the sky entity we change its offset
    if (id != 0)
    {
        return pEnttMgr_->transformSystem_.SetPosition(id, XMFLOAT3(offset.x, offset.y, offset.z));
        return true;
    }

    LogErr(LOG, "there is no entity by such a name: sky");
    return false;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyTexture(const int idx, const uint32_t textureID) { return true; }


// =================================================================================
// For the fog editor
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

bool FacadeEngineToUI::SetFogStart  (const float start)     { pRender_->SetFogStart(pContext_, start); return true; }
bool FacadeEngineToUI::SetFogRange  (const float range)     { pRender_->SetFogRange(pContext_, range); return true; }
bool FacadeEngineToUI::SetFogEnabled(const bool enabled)    { pRender_->SetFogEnabled(pContext_, enabled); return true; }
bool FacadeEngineToUI::SetFogColor  (const ColorRGB& color) { pRender_->SetFogColor(pContext_, color.ToFloat3()); return true; }


// =================================================================================
// For the debug editor
// =================================================================================
bool FacadeEngineToUI::SwitchDebugState(const int debugType)
{
    pRender_->SwitchDebugState(pContext_, Render::eDebugState(debugType));
    return true;
}


// =================================================================================
// For assets manager
// =================================================================================

//---------------------------------------------------------
// Desc:   load a new texture from file and add it into textures manager
//---------------------------------------------------------
bool FacadeEngineToUI::LoadTextureFromFile(const char* path) const
{
    return g_TextureMgr.LoadFromFile(path) != INVALID_TEXTURE_ID;
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

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetTextureIdByIdx(const index idx, TexID& outTextureID) const
{
    outTextureID = g_TextureMgr.GetTexIdByIdx(idx);

    // if we got an "invalid" texture ID
    if (outTextureID == 0)
    {
        LogErr(LOG, "there is no texture ID by idx: %lld", idx);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetMaterialIdByIdx(const index idx, MaterialID& outMatID) const
{
    outMatID = g_MaterialMgr.GetMatIdByIdx(idx);
    return true;
}

//---------------------------------------------------------
// Desc:   get a name of material by its id
// Args:   - id:          material identifier
//         - outName:     output char arr for name
//         - nameMaxLen:  output container size
//---------------------------------------------------------
bool FacadeEngineToUI::GetMaterialNameById(const MaterialID id, char* outName, const int nameMaxLen) const
{
    if (!outName)
    {
        LogErr(LOG, "input name ptr is invalid");
        return false;
    }

    if (nameMaxLen <= 0)
    {
        LogErr(LOG, "input max length for name is invalid (must be > 0): %d", nameMaxLen);
        return false;
    }

    // reset output char arr and fill it with data
    memset(outName, 0, nameMaxLen);
    const char* matName = g_MaterialMgr.GetMatById(id).name;
    return (bool)strncpy(outName, matName, nameMaxLen);
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
bool FacadeEngineToUI::GetTextureNameById(const TexID id, TexName& outName) const
{
    const char* texName = g_TextureMgr.GetTexByID(id).GetName().c_str();
    strncpy(outName.name, texName, MAX_LENGTH_TEXTURE_NAME);
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetNumMaterials(size& numMaterials) const
{
    // get the number of all the currenly loaded materials
    numMaterials = g_MaterialMgr.GetNumAllMaterials();
    return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::GetMaterialDataById(const MaterialID id, MaterialData& outData) const
{
    Core::Material& mat = g_MaterialMgr.GetMatById(id);

    outData.materialId  = id;
    outData.ambient     = mat.ambient;
    outData.diffuse     = mat.diffuse;
    outData.specular    = mat.specular;
    outData.reflect     = mat.reflect;

    strncpy(outData.name, mat.name, MAX_LENGTH_MATERIAL_NAME);
    memcpy(outData.textureIDs, mat.texIds, sizeof(TexID) * NUM_TEXTURE_TYPES);

    return true;
}

//---------------------------------------------------------
// Desc:   get names array of particular render states group
//         (groups: fill, cull, blending, etc.)
// Args:   - type:      what kind of names we want to get
//         - outNames:  output arr of names
//---------------------------------------------------------
void FacadeEngineToUI::GetMaterialRenderStateNames(
    const eMaterialPropGroup type,
    cvector<std::string>& outNames) const
{
    using enum eMaterialPropGroup;

    switch (type)
    {
        case FILL:
            g_MaterialMgr.GetFillModesNames(outNames);
            break;

        case CULL:
            g_MaterialMgr.GetCullModesNames(outNames);
            break;

        case WINDING_ORDER:
            break;

        case BLENDING:
            g_MaterialMgr.GetBlendingStatesNames(outNames);
            break;

        case DEPTH_STENCIL:
            g_MaterialMgr.GetDepthStencilStatesNames(outNames);
            break;

        default:
            LogErr(LOG, "unknown type of render states group: %d", (int)type);
            return;
    }
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
    return g_MaterialMgr.SetMatColorData(
        id,
        Vec4(amb.x, amb.y, amb.z, amb.w),
        Vec4(diff.x, diff.y, diff.z, diff.w),
        Vec4(spec.x, spec.y, spec.z, spec.w),
        Vec4(refl.x, refl.y, refl.z, refl.w));
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
// Desc:   init a frame buffer for rendering of material big icon (from material editor)
// Args:   - width, height:  frame buffer's dimensions (icon size)
//---------------------------------------------------------
bool FacadeEngineToUI::InitMaterialBigIcon(const int width, const int height) const
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
// Desc:   in the material browser: we see a bunch of material icons
//         (so here we call its rendering)
//---------------------------------------------------------
bool FacadeEngineToUI::RenderMaterialsIcons() const
{
    return pGraphics_->RenderMaterialsIcons(pRender_);
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
    return pGraphics_->RenderBigMaterialIcon(matID, yAxisRotation, pRender_, &pMaterialBigIcon_);
}

//---------------------------------------------------------
// Desc:   get an arr of elems [shaderId, shaderName, etc.] about
//         all the currently loaded shaders
//---------------------------------------------------------
bool FacadeEngineToUI::GetShadersIdAndName(cvector<ShaderData>& outData)
{
    const auto& map = pRender_->shaderMgr_.GetMapIdsToShaders();
    const size numElems = std::ssize(map);

    // prepare enough memory
    outData.resize(numElems);

    // fill the output arr with data 
    for (int i = 0; const auto& it : map)
    {
        outData[i].id = it.first;                             // shader id
        strncpy(outData[i].name, it.second->GetName(), 32);   // shader name

        ++i;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   setup a rendering shader for material
// Args:   - matId:     a material by this identifier will be changed
//         - shaderId:  a shader by this identifier will be used to render
//---------------------------------------------------------
bool FacadeEngineToUI::SetMaterialShaderId(const MaterialID matId, const ShaderID shaderId)
{
    // get material
    Material& mat = g_MaterialMgr.GetMatById(matId);
    if (mat.id == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "can't setup shader (id: %" PRIu32 ") \n\t"
                    "for material because there is no material by id : %" PRIu32, shaderId, matId);
        return false;
    }

    // get shader
    Render::Shader* pShader = pRender_->shaderMgr_.GetShaderById(shaderId);
    if (!pShader)
    {
        LogErr(LOG, "can't setup shader (id: %" PRIu32 ") \n\t"
                    "for material (id: %d" PRIu32 ") because there is no such shader", shaderId, matId);
        return false;
    }

    // setup rendering shader for material
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

    color           = emitter.color;
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

} // namespace Core
