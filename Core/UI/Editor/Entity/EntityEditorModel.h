// =================================================================================
// Filename:    EntityEditorModel.h
// Description: holds entity data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     01.01.25  by DimaSkup
// =================================================================================
#pragma once

#include "../../UICommon/Vectors.h"


namespace Model
{

class Entity
{
private:
	Vec3     position_ { 0,0,0 };      // its position
	Vec4     dirQuat_ { 0,0,0,0 };     // direction quaternion
	float    uniformScale_ = 0.0;      // and uniform scale

public:
	//
	// setters
	//
	inline void SetData(
		const Vec3& position,
		const Vec4& dirQuat,
		const float uniformScale)
	{
		position_       = position;
		dirQuat_        = dirQuat;
		uniformScale_   = uniformScale;
	}

	inline void SetPosition(const Vec3& pos)          { position_ = pos; }
	inline void SetRotation(const Vec4& dirQuat)      { dirQuat_ = dirQuat; }
	inline void SetUniformScale(const float scale)    { uniformScale_ = scale;}

	//
	// getters
	//
	inline void  GetPosition(Vec3& pos)         const { pos = position_; }
	inline void  GetRotation(Vec4& dirQuat)     const { dirQuat = dirQuat_; }
	inline float GetScale()                     const { return uniformScale_; }

	inline void GetData(Vec3& pos, Vec4& dirQuat, float& scale) const
	{
		pos     = position_;
		dirQuat = dirQuat_;
		scale   = uniformScale_;
	}
};

}
