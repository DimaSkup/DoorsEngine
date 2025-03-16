#pragma once

#include "Color.h"
#include "Vectors.h"

namespace UI
{

class ICommand
{
public:
    ICommand() {}

    ICommand(const int type, const float value)
        : type_(type), x(value) {}


    // colors (RGB and RGBA)
    ICommand(const int type, const ColorRGB& rgb) :
        type_(type), x(rgb.r), y(rgb.g), z(rgb.b) {}

    ICommand(const int type, const ColorRGBA& rgba) :
        type_(type), x(rgba.r), y(rgba.g), z(rgba.b), w(rgba.a) {}


    // vectors (Vec3 and Vec4)
    ICommand(const int type, const Vec3& v)
        : type_(type), x(v.x), y(v.y), z(v.z) {}

    ICommand(const int type, const Vec4& v)
        : type_(type), x(v.x), y(v.y), z(v.z), w(v.w) {}

    //
    // Getters
    //
    ColorRGB  GetColorRGB()  const { return ColorRGB(x,y,z); }
    ColorRGBA GetColorRGBA() const { return ColorRGBA(x,y,z,w); }

    float GetFloat()         const { return x; }
    Vec3 GetVec3()           const { return Vec3(x,y,z); }
    Vec4 GetVec4()           const { return Vec4(x,y,z,w); }

    // command type
    int type_ = 0;

private:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};


} // namespace UI
