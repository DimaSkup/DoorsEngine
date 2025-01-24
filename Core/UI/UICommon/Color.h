// =================================================================================
// Filename:     Color.h
// Description:  a simple and little color classes for the DoorsEngine editor
// 
// Created:      29.12.24
// =================================================================================
#pragma once

#include <DirectXMath.h>

class ColorRGB
{
public:
	ColorRGB() {}

	ColorRGB(const float r, const float g, const float b) :
		r_(r), g_(g), b_(b) {}

	ColorRGB(const ColorRGB& color) :
		r_(color.r_), g_(color.g_), b_(color.b_) {}

	ColorRGB(const DirectX::XMFLOAT3& color) :
		r_(color.x), g_(color.y), b_(color.z) {}


	inline DirectX::XMFLOAT3 GetFloat3() const { return { r_, g_, b_ };	}

	inline ColorRGB& operator=(const ColorRGB& color)
	{
		if (this == &color) return *this;

		r_ = color.r_;
		g_ = color.g_;
		b_ = color.b_;

		return *this;
	}

	inline ColorRGB& operator=(ColorRGB&& color) noexcept
	{
		r_ = color.r_;
		g_ = color.g_;
		b_ = color.b_;

		return *this;
	}

public:
	// data: can get as an array (rgb), or can get values separately (r, g, b)
	union
	{
		float rgb_[3]{ 0.0f };

		struct {
			float r_, g_, b_;
		};
	};
};