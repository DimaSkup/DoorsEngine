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
	TextureMgr* pTextureMgr)
	:
	pContext_(pContext),
	pRender_(pRender),
	pEntityMgr_(pEntityMgr),
	pTextureMgr_(pTextureMgr)
{
	// set pointers to the subsystems

	Assert::NotNullptr(pRender, "ptr to render == nullptr");
	Assert::NotNullptr(pContext, "ptr to device context == nullptr");
	Assert::NotNullptr(pEntityMgr, "ptr to the entt mgr == nullptr");
	Assert::NotNullptr(pTextureMgr, "ptr to the texture mgr == nullptr");
}



// =================================================================================
//                       for using the textures manager
// =================================================================================

bool FacadeEngineToUI::GetShaderResourceViewByTexID(
	const uint32_t textureID,
	ID3D11ShaderResourceView*& pSRV)
{
	pTextureMgr_->GetSRVByTexID(textureID, pSRV);
	return true;
}

// =================================================================================
//                            get camera info
// =================================================================================
bool FacadeEngineToUI::GetCameraViewAndProj(
	const uint32_t camEnttID,
	float* camViewMatrix, 
	float* camProjMatrix)
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

	pEntityMgr_->cameraSystem_.GetViewAndProjByID(camEnttID, view, proj);

	// copy view and proj matrices into raw array of 16 floats
	memcpy(camViewMatrix, view.r->m128_f32, sizeof(float) * 16);
	memcpy(camProjMatrix, proj.r->m128_f32, sizeof(float) * 16);

	return true;
}



// =================================================================================
//                          for the entity editor
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
	const uint32_t entityID,
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

bool FacadeEngineToUI::SetEnttPosition(const uint32_t entityID, const Vec3& pos)
{
	pEntityMgr_->transformSystem_.SetPositionByID(entityID, pos.GetFloat3());
	return true;
}

bool FacadeEngineToUI::SetEnttRotation(const uint32_t entityID, const Vec4& dir)
{
	pEntityMgr_->transformSystem_.SetDirectionByID(entityID, dir.GetXMVector());
	return true;
}

bool FacadeEngineToUI::SetEnttUniScale(const uint32_t entityID, const float scale)
{
	pEntityMgr_->transformSystem_.SetUniScaleByID(entityID, scale);
	return true;
}




// =================================================================================
//                             for the sky editor
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
	pRender_->SetSkyColorCenter(pContext_, color.GetFloat3());
	return true;
}

bool FacadeEngineToUI::SetSkyColorApex(const ColorRGB& color)
{
	pRender_->SetSkyColorApex(pContext_, color.GetFloat3());
	return true;
}

///////////////////////////////////////////////////////////

bool FacadeEngineToUI::SetSkyOffset(const Vec3& offset)
{
	const EntityID enttID = pEntityMgr_->nameSystem_.GetIdByName("sky");

	// if we found the sky entity we change its offset
	if (enttID != INVALID_ENTITY_ID) 
	{
		pEntityMgr_->transformSystem_.SetPositionByID(enttID, offset.GetFloat3());
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
//                               for the fog editor
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
	pRender_->SetFogParams(pContext_, color.GetFloat3(), start, range);
	return true;
}


// =================================================================================
//                          for the debug editor
// =================================================================================

bool FacadeEngineToUI::SwitchDebugState(const int debugType)
{
	pRender_->SwitchDebugState(pContext_, Render::DebugState(debugType));
	return true;
}