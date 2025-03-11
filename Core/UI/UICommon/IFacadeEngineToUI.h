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


namespace UI
{

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
	virtual void GetCameraViewAndProj(const EntityID camEnttID,	float* view, float* proj) { assert(0 && "TODO: implement this virtual method in children"); }
	virtual void FocusCameraOnEntity(const EntityID id)                                   { assert(0 && "TODO: implement this virtual method in children"); }

	// -----------------------------------
	// for the entity editor
	// -----------------------------------
	virtual bool GetAllEnttsIDs(const uint32_t*& pEnttsIDsArr, int& numEntts)      { return false; }
	virtual uint32_t GetEnttIDByName(const char* name)                             { return 0;}
	virtual bool GetEnttNameByID(const EntityID id, std::string& name)             { return false; }

    virtual bool GetEnttsIDsOfTypeModel (const EntityID*& enttsIDs, int& numEntts) { return false; }
    virtual bool GetEnttsIDsOfTypeCamera(const EntityID*& enttsIDs, int& numEntts) { return false; }
    virtual bool GetEnttsIDsOfTypeLight (const EntityID*& enttsIDs, int& numEntts) { return false; }

	virtual bool GetEnttData(const EntityID id, Vec3& pos, Vec4& rotQuat, float& uniScale)                                               { return false; }
	virtual void SetEnttTransformation(const EntityID id, const DirectX::XMVECTOR& pos, const DirectX::XMVECTOR& rot, const float uniScale) { assert(0 && "TODO: implement this virtual method in children"); }
	virtual void GetEnttWorldMatrix(const EntityID id, DirectX::XMMATRIX& outMat)                                                           { assert(0 && "TODO: implement this virtual method in children"); }

	// get/set entity position/rotation_quat/uniform_scale
	virtual Vec3 GetEnttPosition    (const EntityID id)                           const { return GetInvalidVec3(); }
	virtual Vec4 GetEnttRotationQuat(const EntityID id)                           const { return GetInvalidVec4(); }
	virtual float GetEnttScale      (const EntityID id)                           const { return GetInvalidFloat(); }

	virtual bool SetEnttPosition    (const EntityID id, const Vec3& pos)                { return false; }
	virtual bool SetEnttRotationQuat(const EntityID id, const Vec4& rotQuat)            { return false; }
	virtual bool SetEnttUniScale    (const EntityID id, const float scale)              { return false; }

	//
	// functional for entities light sources
	//
	virtual bool IsEnttLightSource(const EntityID id, int& lightType)             const { return false; }

	// set/get point light props
	virtual bool SetPointLightAmbient      (const EntityID id, const ColorRGBA& color)  { return false; }
	virtual bool SetPointLightDiffuse      (const EntityID id, const ColorRGBA& color)  { return false; }
	virtual bool SetPointLightSpecular     (const EntityID id, const ColorRGBA& color)  { return false; }
	virtual bool SetPointLightPos          (const EntityID id, const Vec3& pos)         { return false; }
	virtual bool SetPointLightRange        (const EntityID id, const float range)       { return false; }
	virtual bool SetPointLightAttenuation  (const EntityID id, const Vec3& attenuation) { return false; }

	virtual ColorRGBA GetPointLightAmbient (const EntityID id)                    const { return GetInvalidRGBA(); }
	virtual ColorRGBA GetPointLightDiffuse (const EntityID id)                    const { return GetInvalidRGBA(); }
	virtual ColorRGBA GetPointLightSpecular(const EntityID id)                    const { return GetInvalidRGBA(); }
	virtual Vec3 GetPointLightPos          (const EntityID id)                    const { return GetInvalidVec3(); }
	virtual Vec3 GetPointLightAttenuation  (const EntityID id)                    const { return GetInvalidVec3(); }
	virtual float GetPointLightRange       (const EntityID id)                    const { return GetInvalidFloat(); }

	virtual bool GetEnttPointLightData(
		const EntityID id,
		ColorRGBA& ambient,
		ColorRGBA& diffuse,
		ColorRGBA& specular,
		Vec3& position,
		float& range,
		Vec3& attenuation)
	{
		return false;
	};

	virtual bool GetEnttSpotLightData(
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
		return false;
	}
		
	// set/get spotlight props
	virtual bool SetSpotLightAmbient      (const EntityID id, const ColorRGBA& color) { return false; };
	virtual bool SetSpotLightDiffuse      (const EntityID id, const ColorRGBA& color) { return false; };
	virtual bool SetSpotLightSpecular     (const EntityID id, const ColorRGBA& color) { return false; };
	virtual bool SetSpotLightPos          (const EntityID id, const Vec3& pos)        { return false; };
	virtual bool SetSpotLightDirectionVec (const EntityID id, const Vec3& direction)  { return false; };
	virtual bool SetSpotLightAttenuation  (const EntityID id, const Vec3& att)        { return false; };
	virtual bool SetSpotLightRange        (const EntityID id, const float range)      { return false; };
	virtual bool SetSpotLightSpotExponent (const EntityID id, const float spotExp)    { return false; };

	virtual ColorRGBA GetSpotLightAmbient (const EntityID id)                   const { return GetInvalidRGBA(); }
	virtual ColorRGBA GetSpotLightDiffuse (const EntityID id)                   const { return GetInvalidRGBA(); }
	virtual ColorRGBA GetSpotLightSpecular(const EntityID id)                   const { return GetInvalidRGBA(); }
	virtual Vec3 GetSpotLightPos          (const EntityID id)                   const { return GetInvalidVec3(); }
	virtual Vec3 GetSpotLightDirectionVec (const EntityID id)                   const { return GetInvalidVec3(); }
	virtual Vec3 GetSpotLightAttenuation  (const EntityID id)                   const { return GetInvalidVec3(); }
	virtual float GetSpotLightRange       (const EntityID id)                   const { return GetInvalidFloat(); }
	virtual float GetSpotLightSpotExponent(const EntityID id)                   const { return GetInvalidFloat(); }


	//
	// for the sky editor
	//
	virtual bool GetSkyData(const uint32_t skyEnttID, ColorRGB& center, ColorRGB& apex, Vec3& offset) { return false;};

	virtual bool SetSkyColorCenter(const ColorRGB& color)                                                   { return false; }
	virtual bool SetSkyColorApex(const ColorRGB& color)                                                     { return false; }
	virtual bool SetSkyOffset(const Vec3& offset)                                                           { return false; }
	virtual bool SetSkyTexture(const int idx, const uint32_t textureID)                                     { return false; }

	//
	// for the fog editor
	//
	virtual bool GetFogData(ColorRGB& fogColor, float& fogStart, float& fogRange)                   { return false; }
	virtual bool SetFogParams(const ColorRGB& color, const float start, const float range)          { return false; }

	//
	// for the debug
	//
	virtual bool SwitchDebugState(const int debugType)                                                  { return false; }


	//
	// for assets manager
	//
	virtual int  GetNumAssets()                                                { return 0; }
	virtual void GetAssetsNamesList(std::string* namesArr, const int numNames) { assert(0 && "TODO: implement this virtual method in children"); }
	
private:
	inline float GetInvalidFloat()    const { return NAN; }
	inline Vec3 GetInvalidVec3()      const { return { NAN, NAN, NAN }; }
	inline Vec4 GetInvalidVec4()      const { return { NAN, NAN, NAN, NAN }; }
	inline ColorRGBA GetInvalidRGB()  const { return { NAN, NAN, NAN, NAN }; }
	inline ColorRGBA GetInvalidRGBA() const { return { NAN, NAN, NAN, NAN }; }
};

} // namespace UI
