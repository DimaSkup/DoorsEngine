#pragma once

//#include <xnamath.h>
#include <cstdlib>
#include <DirectXMath.h>


namespace DirectX 
{
	static XMFLOAT3& operator+=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		lhs.z += rhs.z;

		return lhs;
	}

	static XMFLOAT3 XMFloat3Normalize(const XMFLOAT3& n)
	{
		const float invLen = 1.0f / sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
		return XMFLOAT3(n.x * invLen, n.y * invLen, n.z * invLen);
	}
}

namespace ECS
{

class MathHelper
{
public:
	
	inline static float RandF()
	{
		// returns random float in [0, 1)
		return (float)(rand()) / (float)RAND_MAX;
	}

	
	inline static float RandF(const float a, const float b)
	{
		// returns random float in [a, b)
		return a + RandF()*(b-a);
	}

	inline static DirectX::XMFLOAT3 RandColorXMFLOAT3()
	{
		// returns a random color as XMFLOAT3 (RGB)
		return{ RandF(), RandF(), RandF() };
	}

	inline static DirectX::XMFLOAT4 RandColorXMFLOAT4()
	{
		// returns a random color as XMFLOAT4 (RGBA)
		return{ RandF(), RandF(), RandF(), 1.0f};
	}



	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	static float AngleFromXY(const float x, const float y);

	static const DirectX::XMMATRIX InverseTranspose(const DirectX::CXMMATRIX& M);
};

}; // namespace ECS