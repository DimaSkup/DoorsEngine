#pragma once

#include "Color.h"
#include "Vectors.h"

class ICommand
{
public:
	ICommand(const int type) : type_(type) {}

	virtual ColorRGB  GetColorRGB()  const { return ColorRGB(); }
	virtual ColorRGBA GetColorRGBA() const { return ColorRGBA(); }

	virtual float GetFloat()         const { return 0.0f; }
	// virtual float GetVec2()       const { return Vec2(); }
	virtual Vec3 GetVec3()           const { return Vec3(); }
	virtual Vec4 GetVec4()           const { return Vec4(); }

	// command type
	int type_ = 0;                     
};