// =================================================================================
// Filename:     Vectors.h
// Description:  a simple and little vectors (array of floats)
//               classes for the DoorsEngine editor
// 
// Created:      29.12.24
// =================================================================================
#pragma once

#include <DirectXMath.h>

namespace UI
{

// =================================================================================
// Vec3: sequence of 3 floats
// =================================================================================
class Vec3
{
public:
	Vec3() :
		x(0), y(0), z(0) {}

	Vec3(const float x_, const float y_, const float z_) :
		x(x_), y(y_), z(z_) {}

	Vec3(const DirectX::XMFLOAT3& rhs) :
		x(rhs.x), y(rhs.y), z(rhs.z) {}

	Vec3(const DirectX::XMVECTOR& rhs)
	{
		*this = rhs;
	}

	// ---------------------------------------------

	inline DirectX::XMFLOAT3 ToFloat3()   const { return { x, y, z }; }
	inline DirectX::XMFLOAT4 ToFloat4()   const { return { x, y, z, 1.0f }; }
	inline DirectX::XMVECTOR ToXMVector() const { return { x, y, z, 1.0f }; }

	inline Vec3& operator=(const Vec3& vec)
	{
		if (this == &vec) return *this;

		x = vec.x;
		y = vec.y;
		z = vec.z;

		return *this;
	}

	inline Vec3& operator=(const DirectX::XMFLOAT3& float3)
	{
		x = float3.x; 
		y = float3.y;
		z = float3.z;

		return *this;
	}

	inline Vec3& operator=(const DirectX::XMVECTOR& vec)
	{
		x = DirectX::XMVectorGetX(vec);
		y = DirectX::XMVectorGetY(vec);
		z = DirectX::XMVectorGetZ(vec);

		return *this;
	}

public:
	union
	{
		float xyz[3]{ 0.0f };

		struct {
			float x, y, z;
		};
	};
};

// =================================================================================
// Vec4: sequence of 4 floats
// =================================================================================
class Vec4
{
public:
	Vec4() :
		x(0), y(0), z(0), w(0) {}

	Vec4(const float x_, const float y_, const float z_, const float w_) :
		x(x_), y(y_), z(z_), w(w_) {}

	Vec4(const DirectX::XMFLOAT4& rhs) :
		x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}

	Vec4(const DirectX::XMVECTOR& rhs)
	{
		*this = rhs;
	}

    Vec4(const float* arr)
    {
        // init vec4 with array of 4 floats
        if (arr)
        {
            *this = Vec4(arr[0], arr[1], arr[2], arr[3]);
        }
    }

	// ----------------------------------------------------

	inline Vec3              ToVec3()     const { return { x, y, z }; }
	inline DirectX::XMFLOAT4 ToFloat4()   const { return { x, y, z, w }; }
	inline DirectX::XMVECTOR ToXMVector() const { return { x, y, z, w }; }


	inline Vec4& operator=(const Vec4& vec)
	{
		if (this == &vec) return *this;

		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = vec.w;

		return *this;
	}

	inline Vec4& operator=(const DirectX::XMFLOAT4& float4)
	{
		x = float4.x;
		y = float4.y;
		z = float4.z;
		w = float4.w;

		return *this;
	}

	inline Vec4& operator=(const DirectX::XMVECTOR& vec)
	{
		x = DirectX::XMVectorGetX(vec);
		y = DirectX::XMVectorGetY(vec);
		z = DirectX::XMVectorGetZ(vec);
		w = DirectX::XMVectorGetW(vec);

		return *this;
	}

public:
	union
	{
		float xyzw[4]{ 0.0f };

		struct {
			float x, y, z, w;
		};
	};
};

} // namespace UI
