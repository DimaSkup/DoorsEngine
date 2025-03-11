#pragma once

#include "Color.h"
#include "Vectors.h"


namespace UI
{

class ICommand
{
public:
	ICommand() {}
	ICommand(const int type, const ColorRGB& rgb)   : type_(type), rgb_(rgb) {}
	ICommand(const int type, const ColorRGBA& rgba) : type_(type), rgba_(rgba) {}

	ICommand(const int type, const float value)     : type_(type), fvalue_(value) {}
	ICommand(const int type, const Vec3& v)         : type_(type), vec3_(v) {}
	ICommand(const int type, const Vec4& v)         : type_(type), vec4_(v) {}

	ColorRGB  GetColorRGB()  const { return rgb_; }
	ColorRGBA GetColorRGBA() const { return rgba_; }

	float GetFloat()         const { return fvalue_; }
	Vec3 GetVec3()           const { return vec3_; }
	Vec4 GetVec4()           const { return vec4_; }


	// command type
	int type_ = 0;    

	float fvalue_ = 0.0f;
	Vec3 vec3_;
	Vec4 vec4_;
	
	ColorRGB  rgb_;
	ColorRGBA rgba_;
};


} // namespace UI