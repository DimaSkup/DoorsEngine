#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

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

	DirectX::XMFLOAT4 GetFloat4() const;

private:
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
};

namespace Colors
{
	const Color UnloadedTextureColor(255, 255, 255);
	const Color UnhandledTextureColor(250, 0, 0);

	// color XM-vectors
	XMGLOBALCONST DirectX::XMVECTORF32 vWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vRed = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vGreen = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vBlue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vYellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vCyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vMagenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vSilver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 vLightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };


	XMGLOBALCONST DirectX::PackedVector::XMCOLOR White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Red= { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST DirectX::PackedVector::XMCOLOR LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}