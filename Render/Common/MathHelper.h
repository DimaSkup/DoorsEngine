#pragma once

//#include <xnamath.h>
#include <cstdlib>
#include <DirectXMath.h>
#include "Types.h"


namespace DirectX
{
	static bool operator==(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
	}

	static bool operator==(const XMFLOAT4& lhs, const XMFLOAT4& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) && (lhs.w == rhs.w);
	}

	static bool operator==(const XMVECTOR& lhs, const XMVECTOR& rhs)
	{
		// define if two input XMVECTORs are equal

		const float* vec = XMVectorEqual(lhs, rhs).m128_f32;
		return (vec[0] && vec[1] && vec[2] && vec[3]);
	}

	static bool operator==(const XMMATRIX& lhs, const XMMATRIX& rhs)
	{
		// define if two input 4x4 matrices are equal

		return (lhs.r[0] == rhs.r[0]) &&
			(lhs.r[1] == rhs.r[1]) &&
			(lhs.r[2] == rhs.r[2]) &&
			(lhs.r[3] == rhs.r[3]);
	}

	static XMFLOAT3& operator+=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		lhs.z += rhs.z;

		return lhs;
	}

	static XMFLOAT3 XMFloat3Normalize(const XMFLOAT3& n)
	{
		const float invLen = 1.0f / sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
		return XMFLOAT3(n.x * invLen, n.y * invLen, n.z * invLen);
	}

	class CompareXMVECTOR
	{
	public:
		CompareXMVECTOR() {}

		bool operator()(const XMVECTOR& lhs, const XMVECTOR& rhs) const
		{
			const float* vec = XMVectorEqual(lhs, rhs).m128_f32;
			return (vec[0] && vec[1] && vec[2] && vec[3]);
		}
	};
};


namespace Render
{

class MathHelper
{
public:
	static const float Infinity;
	static const float Pi;

	inline static u32 RandUINT(const u32 min, const u32 max)
	{
		// return random unsigned int in range [min, max)
		return min + (rand()) % (max - min);
	}
	// ----------------------------------------------------
	inline static float RandF()
	{
		// returns random float in [0, 1)
		return (float)(rand()) / (float)RAND_MAX;
	}
	// ----------------------------------------------------
	inline static float RandF(const float a, const float b)
	{
		// returns random float in [a, b)
		return a + RandF()*(b-a);
	}
	// ----------------------------------------------------
	inline static DirectX::XMFLOAT3 RandColorRGB()
	{
		// returns a random color as XMFLOAT3 (RGB)
		return { RandF(), RandF(), RandF() };
	}
	// ----------------------------------------------------
	inline static DirectX::XMFLOAT4 RandColorRGBA()
	{
		// returns a random color as XMFLOAT4 (RGBA);
		// note: alpha value == 1.0f by default
		return{ RandF(), RandF(), RandF(), 1.0f};
	}


	// ----------------------------------------------------
	// inline template methods to generate random XM-value

	template<typename T>
	inline static T RandXM();
	// ----------------------------------------------------
	template<>
	inline static DirectX::XMFLOAT2 RandXM<DirectX::XMFLOAT2>()
	{
		return{ RandF(), RandF() };
	}
	// ----------------------------------------------------
	template<>
	inline static DirectX::XMFLOAT3 RandXM<DirectX::XMFLOAT3>()
	{
		return{ RandF(), RandF(), RandF() };
	}
	// ----------------------------------------------------
	template<>
	inline static DirectX::XMFLOAT4 RandXM<DirectX::XMFLOAT4>()
	{
		return{ RandF(), RandF(), RandF(), RandF() };
	}
	// ----------------------------------------------------
	template<>
	inline static DirectX::XMVECTOR RandXM<DirectX::XMVECTOR>()
	{
		return{ RandF(), RandF(), RandF(), RandF() };
	}


	// ----------------------------------------------------
	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}
	// ----------------------------------------------------
	static float AngleFromXY(const float x, const float y);
	// ----------------------------------------------------
	static const DirectX::XMMATRIX InverseTranspose(const DirectX::CXMMATRIX& M);
};


}; // namespace Render