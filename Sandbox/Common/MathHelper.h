#pragma once

#include <cstdlib>
#include <DirectXMath.h>

namespace Game
{

class MathHelper
{
	using UINT = unsigned int;

public:
	static const float Infinity;
	static const float Pi;

	inline static UINT RandUINT(const UINT min, const UINT max)
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

	// return pitch/yaw/roll packed into FLOAT3
	static DirectX::XMFLOAT3 QuatToRollPitchYaw(const DirectX::XMVECTOR quaternion);

	inline static void ClampPitch(float& pitch)
	{
		// limit the pitch value in range (-(PI/2)+0.1f < pitch < (PI/2)-0.1f)
		if (pitch > DirectX::XM_PIDIV2 - 0.1f)
		{
			pitch = DirectX::XM_PIDIV2 - 0.1f;
		}
		else if (pitch < -DirectX::XM_PIDIV2 + 0.1f)
		{
			pitch = -DirectX::XM_PIDIV2 + 0.1f;
		}
	}

	inline static void ClampYaw(float& yaw)
	{
		// limit the yaw value in range (-2PI < yaw < 2PI)
		if (yaw > DirectX::XM_2PI)
		{
			yaw = -DirectX::XM_2PI;
		}
		else if (yaw < -DirectX::XM_2PI)
		{
			yaw = DirectX::XM_2PI;
		}
	}

	// ----------------------------------------------------
	static const DirectX::XMMATRIX InverseTranspose(const DirectX::CXMMATRIX& M);
};

} // namespace Game
