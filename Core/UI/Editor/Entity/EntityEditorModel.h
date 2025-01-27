// =================================================================================
// Filename:    EntityEditorModel.h
// Description: holds everything that is entt data related 
//              (and used for the editor)
// Created:     01.01.25
// =================================================================================
#pragma once

#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"


namespace Model
{

class Entity
{
private:
	uint32_t selectedEnttID_ = 0;      // an ID of the currently chosen/picked entity
	Vec3     position_ { 0,0,0 };      // its position
	Vec4     dirQuat_ { 0,0,0,0 };     // direction quaternion
	float    uniformScale_ = 0.0;      // and uniform scale

public:

	inline void SetSelectedEnttData(
		const uint32_t entityID, 
		const Vec3& position,
		const Vec4& dirQuat,
		const float uniformScale)
	{
		selectedEnttID_ = entityID;
		position_       = position;
		dirQuat_        = dirQuat;
		uniformScale_   = uniformScale;
	}

	// setters
	inline void SetPosition(const Vec3& pos)          { position_ = pos; }
	inline void SetRotation(const Vec4& dirQuat)      { dirQuat_ = dirQuat; }
	inline void SetUniformScale(const float scale)    { uniformScale_ = scale;}

	// getters
	inline uint32_t GetSelectedEntityID()       const { return selectedEnttID_; }
	inline void     GetPosition(Vec3& pos)      const { pos = position_; }
	inline void     GetRotation(Vec4& dirQuat)  const { dirQuat = dirQuat_; }
	inline float    GetScale()                  const { return uniformScale_; }

	inline void GetTransformation(Vec3& pos, Vec4& dirQuat, float& scale) const
	{
		pos     = position_;
		dirQuat = dirQuat_;
		scale   = uniformScale_;
	}
};

}
