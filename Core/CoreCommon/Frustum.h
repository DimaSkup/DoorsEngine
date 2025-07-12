//==================================================================================
// Filename:  Frustum.h
// Desc:      just frustum
//
// Created:   
//==================================================================================
#pragma once
#include <DMath.h>


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

    Vec3  n;     // normalized normal vector
    float d;     // d = -dot(n*p0)
};

///////////////////////////////////////////////////////////

class Frustum
{
public:
    Frustum() {}

    void Initialize(
        const float fovX,
        const float fovY,
        const float nearZ,
        const float farZ);

    void Initialize(
        const float* viewMatrix,
        const float* projMatrix);

    void Initialize(
        const float* nearPlane,
        const float* farPlane,
        const float* rightPlane,
        const float* leftPlane,
        const float* topPlane,
        const float* bottomPlane);

    bool VertexTest(const float x, const float y, const float z);
    bool SphereTest(const float x, const float y, const float z, const float radius);
    bool CubeTest  (const float x, const float y, const float z, const float size);


public:
    FrustumPlane planes_[6];

    float fovX_  = 0;
    float fovY_  = 0;
    float nearZ_ = 0;
    float farZ_  = 0;
};
