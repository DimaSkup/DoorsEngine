// =================================================================================
// Filename:      IFacadeEngineToUI.h
// 
// Description:   the Facade class provides a simple interface to the complex
//                logic of the sereral engine subsystem (render, textures, ECS, etc.)
//                The Facade delegates the client requests to the appropritate
//                objects withing the subsystem. All of this shields the client 
//                from the undesired complexity of the subsystem.
// 
// Created:       31.12.24
// =================================================================================
#pragma once

#include "Color.h"
#include "Vectors.h"

#include <d3d11.h>
#include <cstdint>
#include <string>

class IFacadeEngineToUI
{
public:
	virtual ~IFacadeEngineToUI() {};


	// 
	// for using the textures manager
	//
	virtual bool GetShaderResourceViewByTexID(const uint32_t textureID, ID3D11ShaderResourceView*& pSRV) { return false; }


	//
	// get camera info
	//
	virtual bool GetCameraViewAndProj(
		const uint32_t camEnttID,
		float* camViewMatrix, 
		float* camProjMatrix) 
	{
		return false; 
	}

	//
	// for the entity editor
	//
	virtual bool GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts)   { return false; }
	virtual bool GetEnttIDByName(const char* name, uint32_t& id)                { return false;}
	virtual bool GetEnttNameByID(const uint32_t enttID, std::string& name)      { return false; }


	virtual bool GatherEnttData (
		const uint32_t entityID,
		Vec3& position, 
		Vec4& dirQuat,
		float& uniformScale)     
	{
		return false; 
	}

	virtual bool SetEnttPosition(const uint32_t entityID, const Vec3& pos)     { return false; }
	virtual bool SetEnttRotation(const uint32_t entityID, const Vec4& dir)     { return false; }
	virtual bool SetEnttUniScale(const uint32_t entityID, const float scale)   { return false; }

	//
	// for the sky editor
	//
	virtual bool GatherSkyData(ColorRGB& center, ColorRGB& apex, Vec3& offset) { return false;};

	virtual bool SetSkyColorCenter(const ColorRGB& color)                      { return false; }
	virtual bool SetSkyColorApex(const ColorRGB& color)                        { return false; }
	virtual bool SetSkyOffset(const Vec3& offset)                              { return false; }
	virtual bool SetSkyTexture(const int idx, const uint32_t textureID)        { return false; }

	//
	// for the fog editor
	//
	virtual bool GatherFogData(ColorRGB& fogColor, float& fogStart, float& fogRange)       { return false; }
	virtual bool SetFogParams(const ColorRGB& color, const float start, const float range) { return false; }

	//
	// for the debug
	//
	virtual bool SwitchDebugState(const int debugType)                         { return false; }
};