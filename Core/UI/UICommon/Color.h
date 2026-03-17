// =================================================================================
// Filename:     Color.h
// Description:  a simple and little color classes for the DoorsEngine editor
// 
// Created:      29.12.24
// =================================================================================
#pragma once

#include <DirectXMath.h>


namespace UI
{

class ColorRGB
{
public:
	ColorRGB() :
		r(0), g(0), b(0) {}

	ColorRGB(const float r_, const float g_, const float b_) :
		r(r_), g(g_), b(b_) {}

	ColorRGB(const ColorRGB& c) :
		r(c.r), g(c.g), b(c.b) {}

	ColorRGB(const DirectX::XMFLOAT3& c) :
		r(c.x), g(c.y), b(c.z) {}


	// ------------------------------------------

	inline DirectX::XMFLOAT3 ToFloat3() const { return { r, g, b };	}

	inline ColorRGB& operator=(const ColorRGB& c)
	{
		if (this == &c) return *this;

		r = c.r;
		g = c.g;
		b = c.b;

		return *this;
	}

	inline ColorRGB& operator=(ColorRGB&& c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;

		return *this;
	}

public:
	// data: can get as an array (rgb), or can get values separately (r, g, b)
	union
	{
		float rgb[3];

		struct {
			float r, g, b;
		};
	};
};

// =================================================================================

class ColorRGBA
{
public:
	ColorRGBA() :
		r(0), g(0), b(0), a(1) {}

	ColorRGBA(const float r_, const float g_, const float b_, const float a_) :
		r(r_), g(g_), b(b_), a(a_) {}

	ColorRGBA(const ColorRGBA& c) :
		r(c.r), g(c.g), b(c.b), a(c.a) {}

	ColorRGBA(const DirectX::XMFLOAT4& c) :
		r(c.x), g(c.y), b(c.z), a(c.w) {}

	// ------------------------------------------

	inline DirectX::XMFLOAT4 ToFloat4() const { return { r, g, b, a }; }
	inline ColorRGB          ToRGB()    const { return { r, g, b }; }

	inline ColorRGBA& operator=(const ColorRGBA& c)
	{
		if (this == &c) return *this;

		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;

		return *this;
	}

	inline ColorRGBA& operator=(ColorRGBA&& c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;

		return *this;
	}

public:
	// data: can get as an array (rgba), or can get values separately (r, g, b, a)
	union
	{
		float rgba[4];

		struct {
			float r, g, b, a;
		};
	};
};

} // namespace UI
