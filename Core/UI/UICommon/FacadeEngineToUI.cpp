// =================================================================================
// Filename:  FacadeEngineToUI.cpp
// 
// Created:   31.12.24
// =================================================================================
#include "FacadeEngineToUI.h"
#include "../Common/Assert.h"
#include "../Common/log.h"


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
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

	pEntityMgr_->cameraSystem_.GetViewAndProjByID(camEnttID, view, proj);

	// copy view and proj matrices into raw array of 16 floats
	memcpy(outView, view.r->m128_f32, sizeof(float) * 16);
	memcpy(outProj, proj.r->m128_f32, sizeof(float) * 16);
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::PlaceCameraNearEntt(const EntityID enttID)
{
	// set that the editor camera look at some selected entity and
	// place the camera away a bit from this entity so we can see it properly

	if (enttID == 0)
		return;

	using namespace DirectX;

	Camera& cam = *pEditorCamera_;
	ECS::EntityMgr& mgr = *pEntityMgr_;

	// set where camera look at
	//XMFLOAT3 enttPos = mgr.transformSystem_.GetPositionByID(enttID);
	//cam.SetFixedLookAt(enttPos);
	//cam.SetPosition(XMVECTOR{ enttPos.x, enttPos.y, enttPos.z - 3 });
	//cam.UpdateViewMatrix();

	/*
	// compute the distance from the entity center where we place the camera
	BoundingBox aabb;
	mgr.boundingSystem_.GetEnttAABB(enttID, aabb);
	XMVECTOR vecExtents = XMLoadFloat3(&aabb.Extents);
	XMMATRIX world = mgr.transformSystem_.GetWorldMatrixOfEntt(enttID);
	vecExtents = XMVector3Transform(vecExtents, world);
	float enttRadius    = XMVectorGetX(XMVector3Length(vecExtents));
	enttRadius = 10;

	XMVECTOR pos       = cam.GetPosition();   // current camera position
	XMVECTOR cameraDir = cam.GetDirection();  // normalized camera direction
	XMVECTOR newPos    = pos - cameraDir * enttRadius;   // p = p0 + v*t (where pos=p0, v=cameraDir)

	cam.SetPosition(newPos);
	cam.UpdateViewMatrix();
	
	*/

#if 0
	cam.DetachFixedLookAt();

	EntityID cameraID = mgr.nameSystem_.GetIdByName("editor_camera");
	if (cameraID != 0)
	{
		pEntityMgr_->cameraSystem_.Update(
			cameraID,
			cam.GetViewMatrix(),
			cam.GetProjectionMatrix());
	}
#endif
}


// =================================================================================
// For the entity editor
// =================================================================================
bool FacadeEngineToUI::GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts)
{
	pEnttsIDsArr = pEntityMgr_->GetAllEnttsIDs().data();
	numEntts     = static_cast<int>(pEntityMgr_->GetAllEnttsIDs().size());
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

bool FacadeEngineToUI::GatherEnttData(
	const EntityID entityID,
	Vec3& position,
	Vec4& dirQuat,
	float& uniformScale)
{
	XMFLOAT3 pos;
	XMVECTOR quat;
	float    scale;

	pEntityMgr_->transformSystem_.GetTransformByID(entityID, pos, quat, scale);

	position     = pos;
	dirQuat      = quat;
	uniformScale = scale;

	return true;
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::SetEnttTransformation(
	const EntityID id,
	const XMVECTOR& pos,          // position
	const XMVECTOR& rot,          // rotation
	const float uniScale)         // uniform scale
{
	pEntityMgr_->transformSystem_.SetTransformByID(id, pos, rot, uniScale);
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::GetEnttWorldMatrix(const EntityID id, XMMATRIX& outMat)
{
	outMat = pEntityMgr_->transformSystem_.GetWorldMatrixOfEntt(id);
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetEnttPosition(const uint32_t entityID, const Vec3& pos)
{
	pEntityMgr_->transformSystem_.SetPositionByID(entityID, pos.ToFloat3());
	return true;
}

bool FacadeEngineToUI::SetEnttRotation(const uint32_t entityID, const Vec4& dir)
{
	pEntityMgr_->transformSystem_.SetDirectionByID(entityID, dir.ToXMVector());
	return true;
}

bool FacadeEngineToUI::SetEnttUniScale(const uint32_t entityID, const float scale)
{
	pEntityMgr_->transformSystem_.SetUniScaleByID(entityID, scale);
	return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::IsEnttLightSource(const EntityID id, int& lightType)
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

///////////////////////////////////////////////////////////

void FacadeEngineToUI::SetPointLightAmbient(const EntityID id, const ColorRGBA& color)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::AMBIENT, color.ToFloat4());
}

void FacadeEngineToUI::SetPointLightDiffuse(const EntityID id, const ColorRGBA& color)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::DIFFUSE, color.ToFloat4());
}

void FacadeEngineToUI::SetPointLightSpecular(const EntityID id, const ColorRGBA& color)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::SPECULAR, color.ToFloat4());
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::SetPointLightPos(const EntityID id, const Vec3& pos)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::POSITION, pos.ToFloat4());
}

void FacadeEngineToUI::SetPointLightRange(const EntityID id, const float range)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::RANGE, range);
}

void FacadeEngineToUI::SetPointLightAttenuation(const EntityID id, const Vec3& att)
{
	pEntityMgr_->lightSystem_.SetPointLightProp(id, ECS::LightProps::ATTENUATION, att.ToFloat4());
}

///////////////////////////////////////////////////////////

void FacadeEngineToUI::GetEnttPointLightData(
	const EntityID id,
	ColorRGBA& ambient,
	ColorRGBA& diffuse,
	ColorRGBA& specular,
	Vec3& position,
	float& range,
	Vec3& attenuation)
{
	// get data of a point light entity by input ID

	XMFLOAT4 amb, diff, spec;
	XMFLOAT3 pos, att;
	float rang;

	if (pEntityMgr_->lightSystem_.GetPointLightData(id, amb, diff, spec, pos, rang, att))
	{
		ambient = amb;
		diffuse = diff;
		specular = spec;
		position = pos;
		range = rang;
		attenuation = att;
	}
	else
	{
		Log::Error("can't get data of the point light entity by ID: " + std::to_string(id));
	}
}




// =================================================================================
// For the sky editor
// =================================================================================

bool FacadeEngineToUI::GatherSkyData(
	const uint32_t skyEnttID,
	ColorRGB& center,
	ColorRGB& apex,
	Vec3& offset)
{
	// the sky editor model must be initialized with some reasonable data
	// so we gather this data here

	const Render::SkyDomeShader& skyDomeShader = pRender_->GetShadersContainer().skyDomeShader_;

	center = skyDomeShader.GetColorCenter();
	apex   = skyDomeShader.GetColorApex();
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

bool FacadeEngineToUI::GatherFogData(
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
int FacadeEngineToUI::GetNumAssets()
{
	// return the number of all the currently loaded assets
	return pModelStorage_->GetNumAssets();
}

void FacadeEngineToUI::GetAssetsNamesList(std::string* namesArr, const int numNames)
{
	// get a name of each loaded asset
	pModelStorage_->GetAssetsNamesList(namesArr, numNames);
}