// =================================================================================
// Filename:    EnttTransformData.h
// Description: holds entity transformation data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     01.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <math/vec3.h>
#include <math/vec4.h>

namespace UI
{

class EnttTransformData
{
public:
    Vec3  position_     = { 0,0,0 };
    Vec3  direction_    = { 0,0,1 };
    Vec4  rotQuat_      = { 0,0,0,1 };
    float uniformScale_ = 0.0;

public:
    //
    // setters
    //
    inline void SetData(
        const Vec3& position,
        const Vec3& direction,
        const Vec4& rotQuat,
        const float uniformScale)
    {
        position_     = position;
        direction_    = direction;
        rotQuat_      = rotQuat;
        uniformScale_ = uniformScale;
    }

    inline void GetData(Vec3& pos, Vec3& direction, Vec4& rotQuat, float& scale) const
    {
        pos       = position_;
        direction = direction_;
        rotQuat   = rotQuat_;
        scale     = uniformScale_;
    }
};

} // namespace UI
