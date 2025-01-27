// =================================================================================
// Filename:     FacadeEngineToUI.h
// 
// Description:  concrete implementation of the IFacadeEngineToUI interface
// 
// Created:      31.12.24  by DimaSkup
// =================================================================================
#pragma once
#include "IFacadeEngineToUI.h"

#include "../../Texture/TextureMgr.h"   // texture mgr is used to get textures by its IDs
#include "Entity/EntityMgr.h"           // from the ECS module
#include "Render.h"                     // from the Render module


#include <d3d11.h>


class FacadeEngineToUI : public IFacadeEngineToUI
{
private:
	ID3D11DeviceContext* pContext_ = nullptr;
	Render::Render* pRender_       = nullptr;
	ECS::EntityMgr* pEntityMgr_    = nullptr;
	TextureMgr*     pTextureMgr_   = nullptr;

public:
	FacadeEngineToUI(
		ID3D11DeviceContext* pContext,
		Render::Render* pRender,
		ECS::EntityMgr* pEntityMgr,
		TextureMgr*     pTextureMgr);


	// 
	// for using the textures manager
	//
	virtual bool GetShaderResourceViewByTexID(const uint32_t textureID, ID3D11ShaderResourceView*& pSRV) override;


	//
	// get camera info
	//
	virtual bool GetCameraViewAndProj(
		const uint32_t camEnttID,
		float* camViewMatrix, 
		float* camProjMatrix) override;

	//
	// for the entity editor
	//
	virtual bool GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts) override;
	virtual uint32_t GetEnttIDByName(const char* name)                        override;
	virtual bool GetEnttNameByID(const uint32_t enttID, std::string& name)    override;

	virtual bool GatherEnttData(
		const uint32_t entityID,
		Vec3& position,
		Vec4& dirQuat,
		float& uniformScale) override;

	virtual bool SetEnttPosition(const uint32_t entityID, const Vec3& pos)   override;
	virtual bool SetEnttRotation(const uint32_t entityID, const Vec4& dir)   override;
	virtual bool SetEnttUniScale(const uint32_t entityID, const float scale) override;


	//
	// for the sky editor
	//
	virtual bool GatherSkyData(
		const uint32_t skyEnttID, 
		ColorRGB& center, 
		ColorRGB& apex, 
		Vec3& offset) override;

	virtual bool SetSkyColorCenter(const ColorRGB& color) override;
	virtual bool SetSkyColorApex(const ColorRGB& color) override;
	virtual bool SetSkyOffset(const Vec3& offset) override;
	virtual bool SetSkyTexture(const int idx, const uint32_t textureID) override;

	//
	// for the fog editor
	//
	virtual bool GatherFogData(ColorRGB& fogColor, float& fogStart, float& fogRange) override;  
	virtual bool SetFogParams(const ColorRGB& color, const float start, const float range) override;

	//
	// for debugging
	//
	virtual bool SwitchDebugState(const int debugType) override;
};