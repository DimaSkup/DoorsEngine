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

	ColorRGB(const ColorRGB& color) :
		r(color.r), g(color.g), b(color.b) {}

	ColorRGB(const DirectX::XMFLOAT3& color) :
		r(color.x), g(color.y), b(color.z) {}


	// ------------------------------------------

	inline DirectX::XMFLOAT3 ToFloat3() const { return { r, g, b };	}

	inline ColorRGB& operator=(const ColorRGB& color)
	{
		if (this == &color) return *this;

		r = color.r;
		g = color.g;
		b = color.b;

		return *this;
	}

	inline ColorRGB& operator=(ColorRGB&& color) noexcept
	{
		r = color.r;
		g = color.g;
		b = color.b;

		return *this;
	}

public:
	// data: can get as an array (rgb), or can get values separately (r, g, b)
	union
	{
		float rgb[3]{ 0.0f };

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

	ColorRGBA(const ColorRGBA& color) :
		r(color.r), g(color.g), b(color.b), a(color.a) {}

	ColorRGBA(const DirectX::XMFLOAT4& color) :
		r(color.x), g(color.y), b(color.z), a(color.w) {}

	// ------------------------------------------

	inline DirectX::XMFLOAT4 ToFloat4() const { return { r, g, b, a }; }
	inline ColorRGB          ToRGB()    const { return { r, g, b }; }

	inline ColorRGBA& operator=(const ColorRGBA& color)
	{
		if (this == &color) return *this;

		r = color.r;
		g = color.g;
		b = color.b;
		a = color.a;

		return *this;
	}

	inline ColorRGBA& operator=(ColorRGBA&& color) noexcept
	{
		r = color.r;
		g = color.g;
		b = color.b;
		a = color.a;

		return *this;
	}

public:
	// data: can get as an array (rgba), or can get values separately (r, g, b, a)
	union
	{
		float rgba[4]{ 0, 0, 0, 1 };

		struct {
			float r, g, b, a;
		};
	};
};

} // namespace UI