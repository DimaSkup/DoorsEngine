// =================================================================================
// Filename:    EntityEditorModel.h
// Description: holds entity data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     01.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <CoreCommon/MathHelper.h>
#include <UICommon/Vectors.h>


namespace UI
{

class ModelEntity
{
private:
	Vec3  position_     = { 0,0,0 };
	float pitchInDeg_   = 0.0f;        // in degrees
	float yawInDeg_     = 0.0f;
	float rollInDeg_    = 0.0f;
	float uniformScale_ = 0.0;

public:
	//
	// setters
	//
	inline void SetData(
		const Vec3& position,
		const Vec4& rotationQuat,
		const float uniformScale)
	{
		position_       = position;
		uniformScale_   = uniformScale;
		SetRotation(rotationQuat);
	}

	inline void SetPosition(const Vec3& pos)       { position_ = pos; }
	inline void SetUniformScale(const float scale) { uniformScale_ = scale; }

	void SetRotation(const Vec4& rotationQuat)
	{ 
		DirectX::XMFLOAT3 rollPitchYaw = MathHelper::QuatToRollPitchYaw(rotationQuat.ToXMVector());
		rollInDeg_  = DirectX::XMConvertToDegrees(rollPitchYaw.x);
		pitchInDeg_ = DirectX::XMConvertToDegrees(rollPitchYaw.y);
		yawInDeg_   = DirectX::XMConvertToDegrees(rollPitchYaw.z);
	}	

	//
	// getters
	//
	inline Vec3  GetPosition()                  const { return position_; }
	inline float GetPitchInDeg()                const { return pitchInDeg_; }
	inline float GetYawInDeg()                  const { return yawInDeg_; }
	inline float GetRollInDeg()                 const { return rollInDeg_; }

	inline Vec3  GetRotationPitchYawRollInDeg() const { return Vec3(pitchInDeg_, yawInDeg_, rollInDeg_); }
	inline float GetScale()                     const { return uniformScale_; }

	inline void GetData(Vec3& pos, Vec3& pitchYawRollInDeg, float& scale) const
	{
		pos               = position_;
		pitchYawRollInDeg = Vec3(pitchInDeg_, yawInDeg_, rollInDeg_);
		scale             = uniformScale_;
	}
};

} // namespace UI