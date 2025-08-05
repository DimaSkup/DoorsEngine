// =================================================================================
// Filename:    EnttTransformData.h
// Description: holds entity transformation data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     01.01.25  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/Vectors.h>            // math vectors

namespace UI
{

class EnttTransformData
{
public:
    Vec3  position_     = { 0,0,0 };
    Vec3  direction_    = { 0,0,1 };
    float uniformScale_ = 0.0;

public:
    //
    // setters
    //
    inline void SetData(
        const Vec3& position,
        const Vec3& direction,
        const float uniformScale)
    {
        position_     = position;
        direction_    = direction;
        uniformScale_ = uniformScale;
    }

    //inline void SetPosition(const Vec3& pos)       { position_ = pos; }
    //inline void SetDirection(const Vec3& dir)      { direction_ = dir; }
    //inline void SetUniformScale(const float scale) { uniformScale_ = scale; }

    //
    // getters
    //
    //inline Vec3  GetPosition()  const { return position_; }
    //inline Vec3  GetDirection() const { return direction_; }
    //inline float GetScale()     const { return uniformScale_; }

    inline void GetData(Vec3& pos, Vec3& direction, float& scale) const
    {
        pos       = position_;
        direction = direction_;
        scale     = uniformScale_;
    }
};

} // namespace UI
