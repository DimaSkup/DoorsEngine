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

#include "Color.h"
#include "Vectors.h"

#include <d3d11.h>
#include <cstdint>
#include <string>

class IFacadeEngineToUI
{
public:
	using EntityID = uint32_t;

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
	virtual bool GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts)     { return false; }
	virtual uint32_t GetEnttIDByName(const char* name)                            { return 0;}
	virtual bool GetEnttNameByID(const EntityID id, std::string& name)            { return false; }

	virtual bool GatherEnttData(const EntityID id, Vec3& pos, Vec4& dirQuat, float& uniScale)                                               { return false; }
	virtual void SetEnttTransformation(const EntityID id, const DirectX::XMVECTOR& pos, const DirectX::XMVECTOR& rot, const float uniScale) { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void GetEnttWorldMatrix(const EntityID id, DirectX::XMMATRIX& outMat)                                                           { assert(0 && "TODO: impelemnt this virtual method in children"); }


	virtual bool SetEnttPosition(const uint32_t entityID, const Vec3& pos)        { return false; }
	virtual bool SetEnttRotation(const uint32_t entityID, const Vec4& dir)        { return false; }
	virtual bool SetEnttUniScale(const uint32_t entityID, const float scale)      { return false; }


	//
	// for the LIGHT entities editor
	//
	virtual bool IsEnttLightSource(const EntityID id, int& lightType) { return false; }

	virtual void SetPointLightAmbient    (const EntityID id, const ColorRGBA& color)  { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void SetPointLightDiffuse    (const EntityID id, const ColorRGBA& color)  { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void SetPointLightSpecular   (const EntityID id, const ColorRGBA& color)  { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void SetPointLightPos        (const EntityID id, const Vec3& pos)         { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void SetPointLightRange      (const EntityID id, const float range)       { assert(0 && "TODO: impelemnt this virtual method in children"); }
	virtual void SetPointLightAttenuation(const EntityID id, const Vec3& attenuation) { assert(0 && "TODO: impelemnt this virtual method in children"); }

	virtual void GetEnttPointLightData(
		const EntityID id,
		ColorRGBA& ambient,
		ColorRGBA& diffuse,
		ColorRGBA& specular,
		Vec3& position,
		float& range,
		Vec3& attenuation)
	{
		assert(0 && "TODO: impelemnt this virtual method in children");
	};
		



	//
	// for the sky editor
	//
	virtual bool GatherSkyData(const uint32_t skyEnttID, ColorRGB& center, ColorRGB& apex, Vec3& offset) { return false;};

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