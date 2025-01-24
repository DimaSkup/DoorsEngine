#include "Color.h"


Color::Color()
	: color(0)
{
}

Color::Color(unsigned int val)
	: color(val)
{
}

Color::Color(BYTE r, BYTE g, BYTE b)
	: Color(r, g, b, 255)
{
}

Color::Color(BYTE r, BYTE g, BYTE b, BYTE a)
{
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
}

Color::Color(const Color & src)
	: color(src.color)
{
}


////////////////////////////////////////////////////////////////////////////////////////////
//                               PUBLIC OPERATORS
////////////////////////////////////////////////////////////////////////////////////////////

Color & Color::operator=(const Color & src)
{
	if (*this == src)
		return *this;

	this->color = src.color;
	return *this;
}

bool Color::operator==(const Color & rhs) const
{
	return (this->color == rhs.color);
}

bool Color::operator!=(const Color & rhs) const
{
	return !(*this == rhs.color);
}



////////////////////////////////////////////////////////////////////////////////////////////
//                            PUBLIC GETTERS / SETTERS
////////////////////////////////////////////////////////////////////////////////////////////

DirectX::XMFLOAT4 Color::GetFloat4() const
{
	const float inv_255 = 1.0f / 255.0f;

	return DirectX::XMFLOAT4((float)rgba[0] * inv_255,
		                     (float)rgba[1] * inv_255,
		                     (float)rgba[2] * inv_255,
		                     (float)rgba[3] * inv_255);
}