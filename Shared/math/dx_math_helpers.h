/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: dx_math_helpers.h
    Desc:     different helpers to make works with DirectXMath a bit easier
\**********************************************************************************/
#pragma once
#include <DirectXMath.h>
#include "random.h"


namespace DirectX
{
    //---------------------------------------------------------
    // Desc:  helpers for work with DX quaternions
    //---------------------------------------------------------
    inline XMVECTOR QuatRotAxis(const XMVECTOR& axis, const float angle)
    {
        return DirectX::XMQuaternionRotationAxis(axis, angle);
    }

    inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2)
    {
        return DirectX::XMQuaternionMultiply(q1, q2);
    }

    inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2, const XMVECTOR& q3)
    {
        return DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q1, q2), q3);
    }


    //-----------------------------------------------------
    // operators reload
    //-----------------------------------------------------
    static bool operator==(const XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }

    static bool operator==(const XMFLOAT4& lhs, const XMFLOAT4& rhs)
    {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) && (lhs.w == rhs.w);
    }

    static XMFLOAT4 operator*= (XMFLOAT4& lhs, const float scalar)
    {
        return { lhs.x * scalar, lhs.y * scalar, lhs.z * scalar, lhs.w * scalar };
    }

    static XMFLOAT4 operator* (XMFLOAT4 lhs, const float scalar)
    {
        return { lhs.x * scalar, lhs.y * scalar, lhs.z * scalar, lhs.w * scalar };
    }

    static bool operator==(const XMVECTOR& lhs, const XMVECTOR& rhs)
    {
        const float* vec = XMVectorEqual(lhs, rhs).m128_f32;
        return (vec[0] && vec[1] && vec[2] && vec[3]);
    }

    static bool operator != (const XMVECTOR& v1, const XMVECTOR& v2)
    {
        return !(v1 == v2);
    }

    static bool operator==(const XMMATRIX& lhs, const XMMATRIX& rhs)
    {
        // define if two input 4x4 matrices are equal
        return (lhs.r[0] == rhs.r[0]) &&
               (lhs.r[1] == rhs.r[1]) &&
               (lhs.r[2] == rhs.r[2]) &&
               (lhs.r[3] == rhs.r[3]);
    }

    static bool operator !=(const XMMATRIX& lhs, const XMMATRIX& rhs)
    {
        return !(lhs == rhs);
    }

    static XMFLOAT3 operator + (const XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        return XMFLOAT3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
    }

    static XMFLOAT3 operator - (const XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        return XMFLOAT3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
    }

    static XMFLOAT3& operator+=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        lhs.z += rhs.z;

        return lhs;
    }

    static void operator+=(XMFLOAT3& lhs, const XMVECTOR& rhs)
    {
        lhs.x += rhs.m128_f32[0];
        lhs.y += rhs.m128_f32[1];
        lhs.z += rhs.m128_f32[2];
    }

    static XMFLOAT3& operator*=(XMFLOAT3& lhs, const XMFLOAT3& rhs)
    {
        lhs.x *= rhs.x;
        lhs.y *= rhs.y;
        lhs.z *= rhs.z;

        return lhs;
    }

    static XMFLOAT3& operator*=(XMFLOAT3& lhs, const float value)
    {
        lhs.x *= value;
        lhs.y *= value;
        lhs.z *= value;

        return lhs;
    }

    static XMFLOAT3 operator*(const XMFLOAT3& lhs, const float value)
    {
        return { lhs.x*value, lhs.y*value, lhs.z*value };
    }

    static XMFLOAT3& operator*=(XMFLOAT3& lhs, const int value)
    {
        lhs.x *= value;
        lhs.y *= value;
        lhs.z *= value;

        return lhs;
    }

    static void XMFloat3Normalize(XMFLOAT3& n)
    {
        const float invLen = 1.0f / sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);

        n.x *= invLen;
        n.y *= invLen;
        n.z *= invLen;
    }

    static XMFLOAT3 XMFloat3Normalize(const XMFLOAT3& n)
    {
        const float invLen = 1.0f / sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
        return XMFLOAT3(n.x * invLen, n.y * invLen, n.z * invLen);
    }

#if 0
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
#endif
};


class MathHelper
{
public:
    static float AngleFromXY(const float x, const float y);

    static DirectX::XMFLOAT3 QuatToRollPitchYaw(const DirectX::XMVECTOR quaternion);

    static DirectX::XMMATRIX InverseTranspose(const DirectX::CXMMATRIX& M);
};
