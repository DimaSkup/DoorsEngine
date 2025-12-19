#pragma once

typedef unsigned char BYTE;

class Color
{
public:
	Color();
	Color(unsigned int val);
	Color(BYTE r, BYTE g, BYTE b);
	Color(BYTE r, BYTE g, BYTE b, BYTE a);
	Color(const Color & src);

	Color & operator=(const Color & src);
	bool operator==(const Color & rhs) const;
	bool operator!=(const Color & rhs) const;

	inline const BYTE GetR() const { return rgba[0]; }
	inline const BYTE GetG() const { return rgba[1]; }
	inline const BYTE GetB() const { return rgba[2]; }
	inline const BYTE GetA() const { return rgba[3]; }

	inline void SetR(BYTE r) { rgba[0] = r; }
	inline void SetG(BYTE g) { rgba[1] = g; }
	inline void SetB(BYTE b) { rgba[2] = b; }
	inline void SetA(BYTE a) { rgba[3] = a; }
	
	inline const BYTE* GetRGBA() { return rgba;	}

private:
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
};

////////////////////////////////////////////////////////////////////////////////////

namespace Colors
{
	const Color UnloadedTextureColor(255, 255, 255);
	const Color UnhandledTextureColor(250, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////

inline Color::Color()
    : color(0)
{
}

inline Color::Color(unsigned int val)
    : color(val)
{
}

inline Color::Color(BYTE r, BYTE g, BYTE b)
    : Color(r, g, b, 255)
{
}

inline Color::Color(BYTE r, BYTE g, BYTE b, BYTE a)
{
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}

inline Color::Color(const Color& src)
    : color(src.color)
{
}


////////////////////////////////////////////////////////////////////////////////////
//                               PUBLIC OPERATORS
////////////////////////////////////////////////////////////////////////////////////

inline Color& Color::operator=(const Color& src)
{
    if (*this == src)
        return *this;

    this->color = src.color;
    return *this;
}

inline bool Color::operator==(const Color& rhs) const
{
    return (this->color == rhs.color);
}

inline bool Color::operator!=(const Color& rhs) const
{
    return !(*this == rhs.color);
}
