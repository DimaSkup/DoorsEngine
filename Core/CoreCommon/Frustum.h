//==================================================================================
// Filename:  Frustum.h
// Desc:      just frustum
//
// Created:   
//==================================================================================
#pragma once
#include <math/vec3.h>
#include <CoreCommon/Matrix.h>


// =================================================================================
// Enums / strustures
// =================================================================================

enum eFrustumPlane
{
    FRUSTUM_NEAR,
    FRUSTUM_FAR,
    FRUSTUM_LEFT,
    FRUSTUM_RIGHT,
    FRUSTUM_TOP,
    FRUSTUM_BOTTOM
};

///////////////////////////////////////////////////////////

enum eContainmentType
{
    DISJOINT   = 0,
    INTERSECTS = 1,
    CONTAINS   = 2,
};

///////////////////////////////////////////////////////////

// normalized plane
struct FrustumPlane
{
    FrustumPlane() :
        n{ 0,0,0 }, d(0) {}

    FrustumPlane(const float nx, const float ny, const float nz, const float inD) :
        n{ nx, ny, nz }, d(inD) {}

    FrustumPlane(const Vec4& v) :
        n{v.x, v.y, v.z}, d(v.w) {}

    Vec3  n;     // normalized normal vector
    float d;     // d = -dot(n*p0)
};

///////////////////////////////////////////////////////////

class Frustum
{
public:
    Frustum(
        const float fov,
        const float aspectRatio,
        const float nearZ,
        const float farZ);

    Frustum(const Matrix& viewProj);

    Frustum(
        const float* nearPlane,
        const float* farPlane,
        const float* rightPlane,
        const float* leftPlane,
        const float* topPlane,
        const float* bottomPlane);


    bool PointTest (const float x, const float y, const float z) const;
    bool SphereTest(const float x, const float y, const float z, const float radius) const;
    bool CubeTest  (const float x, const float y, const float z, const float size) const;

private:
    bool TestFrustum();

public:
    FrustumPlane nearClipPlane_;
    FrustumPlane farClipPlane_;
    FrustumPlane rightClipPlane_;

    FrustumPlane leftClipPlane_;
    FrustumPlane topClipPlane_;
    FrustumPlane bottomClipPlane_;
};
