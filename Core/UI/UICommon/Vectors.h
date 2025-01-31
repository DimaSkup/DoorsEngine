// =================================================================================
// Filename:     Vectors.h
// Description:  a simple and little vectors (array of floats)
//               classes for the DoorsEngine editor
// 
// Created:      29.12.24
// =================================================================================
#pragma once


// =================================================================================
// Vec3: sequence of 3 floats
// =================================================================================
class Vec3
{
public:
	Vec3() :
		x_(0), y_(0), z_(0) {}

	Vec3(const float x, const float y, const float z) :
		x_(x), y_(y), z_(z) {}

	Vec3(const DirectX::XMVECTOR& rhs)
	{
		*this = rhs;
	}

	// ---------------------------------------------

	inline DirectX::XMFLOAT3 ToFloat3()   const { return { x_, y_, z_ }; }
	inline DirectX::XMFLOAT4 ToFloat4()   const { return { x_, y_, z_, 1.0f }; }
	inline DirectX::XMVECTOR ToXMVector() const { return { x_, y_, z_, 1.0f }; }

	inline Vec3& operator=(const Vec3& vec)
	{
		if (this == &vec) return *this;

		x_ = vec.x_;
		y_ = vec.y_;
		z_ = vec.z_;

		return *this;
	}

	inline Vec3& operator=(const DirectX::XMFLOAT3& float3)
	{
		x_ = float3.x; 
		y_ = float3.y;
		z_ = float3.z;

		return *this;
	}

	inline Vec3& operator=(const DirectX::XMVECTOR& vec)
	{
		x_ = DirectX::XMVectorGetX(vec);
		y_ = DirectX::XMVectorGetY(vec);
		z_ = DirectX::XMVectorGetZ(vec);

		return *this;
	}

public:
	union
	{
		float xyz_[3]{ 0.0f };

		struct {
			float x_, y_, z_;
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
		x_(0), y_(0), z_(0), w_(0) {}

	Vec4(const float x, const float y, const float z, const float w) :
		x_(x), y_(y), z_(z), w_(w) {}

	Vec4(const DirectX::XMFLOAT4& rhs) :
		x_(rhs.x), y_(rhs.y), z_(rhs.z), w_(rhs.w) {}

	Vec4(const DirectX::XMVECTOR& rhs)
	{
		*this = rhs;
	}

	// ----------------------------------------------------

	inline Vec3              ToVec3()     const { return { x_, y_, z_ }; }
	inline DirectX::XMFLOAT4 ToFloat4()   const { return { x_, y_, z_, w_ }; }
	inline DirectX::XMVECTOR ToXMVector() const { return { x_, y_, z_, w_ }; }


	inline Vec4& operator=(const Vec4& vec)
	{
		if (this == &vec) return *this;

		x_ = vec.x_;
		y_ = vec.y_;
		z_ = vec.z_;
		w_ = vec.w_;

		return *this;
	}

	inline Vec4& operator=(const DirectX::XMFLOAT4& float4)
	{
		x_ = float4.x;
		y_ = float4.y;
		z_ = float4.z;
		w_ = float4.w;

		return *this;
	}

	inline Vec4& operator=(const DirectX::XMVECTOR& vec)
	{
		x_ = DirectX::XMVectorGetX(vec);
		y_ = DirectX::XMVectorGetY(vec);
		z_ = DirectX::XMVectorGetZ(vec);
		w_ = DirectX::XMVectorGetW(vec);

		return *this;
	}

public:
	union
	{
		float xyzw_[4]{ 0.0f };

		struct {
			float x_, y_, z_, w_;
		};
	};
};