#include "frustum.h"
#include <geometry/plane3d_functions.h>
#include <geometry/rect3d_functions.h>
#include <math/matrix.h>


int numTests = 0;

int Frustum::GetNumTests() const
{
    return numTests;
}

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
Frustum::Frustum()
{
    numTests = 0;
}

//---------------------------------------------------------
// Desc:   create view frustum planes in camera space in terms of
//         the field of view (fov), aspect ratio, near and far plane distance
//---------------------------------------------------------
Frustum::Frustum(
    const float fov,
    const float aspectRatio, 
    const float zn, 
    const float zf)
{
    Init(fov, aspectRatio, zn, zf);
}

//---------------------------------------------------------
// Desc:  init frustum with 6 input planes
//---------------------------------------------------------
Frustum::Frustum(
    const Plane3d& left, 
    const Plane3d& right, 
    const Plane3d& top, 
    const Plane3d& bottom, 
    const Plane3d& nearPlane, 
    const Plane3d& farPlane) 
    :
    leftPlane_(left),
    rightPlane_(right),
    topPlane_(top),
    bottomPlane_(bottom),
    nearPlane_(nearPlane),
    farPlane_(farPlane)
{
}

//---------------------------------------------------------
// Desc:   create view frustum planes in camera space in
//         terms of the focal length, aspect ratio, near and far plane distance
// Args:   - fov:          field of view in radians
//         - aspectRatio:  screen width divided by height
//         - nearZ, farZ:  a distance to near and far planes by Z-axis
//---------------------------------------------------------
void Frustum::Init(
    const float fov,
    const float aspectRatio,
    const float nearZ,
    const float farZ)
{
    assert(fov > 0.0f);
    assert(aspectRatio > 0.0f);
    assert(nearZ > 0.0f && farZ > 0.0f);
    assert(nearZ < farZ);

    const float aspect = 1.0f / aspectRatio;

    // compute focal length
    const float e   = 1.0f / tanf(fov * 0.5f);

    float invDenom1 = 1.0f / sqrtf(SQR(e) + 1);
    float invDenom2 = 1.0f / sqrtf(SQR(e) + SQR(aspect));

    // normalized normal vectors
    float horizNx = e * invDenom1;
    float horizNz = invDenom1;
    float vertNy  = e * invDenom2;
    float vertNz  = aspect * invDenom2;

    nearZ_ = nearZ;
    farZ_  = farZ;

    nearPlane_   = Plane3d(0, 0, +1, -nearZ);
    farPlane_    = Plane3d(0, 0, -1, farZ);

    // since we have Z-forward the nz component for r/l/t/b planes is POSITIVE
    rightPlane_  = Plane3d(-horizNx, 0, horizNz, 0);
    leftPlane_   = Plane3d(+horizNx, 0, horizNz, 0);

    topPlane_    = Plane3d(0, -vertNy, vertNz, 0);
    bottomPlane_ = Plane3d(0, +vertNy, vertNz, 0);

    // sizes in view space
    nearPlaneWidth_  = 2 * nearZ / e;
    nearPlaneHeight_ = 2 * nearZ / (e * aspectRatio);
    farPlaneWidth_   = 2 * farZ / e;
    farPlaneHeight_  = 2 * farZ / (e * aspectRatio);
}

//---------------------------------------------------------
// Desc:   setup the fructum clipping planes using input proj matrix
//---------------------------------------------------------
Frustum::Frustum(const Matrix& m)
{
    Matrix mt;
    MatrixTranspose(m, mt);

    const Vec4 row1(mt[0][0], mt[0][1], mt[0][2], mt[0][3]);
    const Vec4 row2(mt[1][0], mt[1][1], mt[1][2], mt[1][3]);
    const Vec4 row3(mt[2][0], mt[2][1], mt[2][2], mt[2][3]);
    const Vec4 row4(mt[3][0], mt[3][1], mt[3][2], mt[3][3]);

    nearPlane_   = { row3 + row4 };
    farPlane_    = { row3 - row4 };
    rightPlane_  = { row1 - row4 };
    leftPlane_   = { row1 + row4 };
    topPlane_    = { row2 - row4 };
    bottomPlane_ = { row2 + row4 };

    nearPlane_.Normalize();
    farPlane_.Normalize();
    rightPlane_.Normalize();
    leftPlane_.Normalize();
    topPlane_.Normalize();
    bottomPlane_.Normalize();
}


//---------------------------------------------------------
// Desc:   extract frustum planes from input projection matrix
//---------------------------------------------------------
void Frustum::CreateFromProjMatrix(
    const Matrix& proj,
    const bool normalizePlanes)
{
    const Matrix& matrix = proj;

    // Left clipping plane
    leftPlane_.normal.x = matrix.m03 + matrix.m00;
    leftPlane_.normal.y = matrix.m13 + matrix.m10;
    leftPlane_.normal.z = matrix.m23 + matrix.m20;
    leftPlane_.distance = matrix.m33 + matrix.m30;

    // Right clipping plane
    rightPlane_.normal.x = matrix.m03 - matrix.m00;
    rightPlane_.normal.y = matrix.m13 - matrix.m10;
    rightPlane_.normal.z = matrix.m23 - matrix.m20;
    rightPlane_.distance = matrix.m33 - matrix.m30;

    // Top clipping plane
    topPlane_.normal.x = matrix.m03 - matrix.m01;
    topPlane_.normal.y = matrix.m13 - matrix.m11;
    topPlane_.normal.z = matrix.m23 - matrix.m21;
    topPlane_.distance = matrix.m33 - matrix.m31;

    // Bottom clipping plane
    bottomPlane_.normal.x = matrix.m03 + matrix.m01;
    bottomPlane_.normal.y = matrix.m13 + matrix.m11;
    bottomPlane_.normal.z = matrix.m23 + matrix.m21;
    bottomPlane_.distance = matrix.m33 + matrix.m31;

    // Near clipping plane 
    nearPlane_.normal.x = matrix.m02;
    nearPlane_.normal.y = matrix.m12;
    nearPlane_.normal.z = matrix.m22;
    nearPlane_.distance = matrix.m32;

    // Far clipping plane 
    farPlane_.normal.x = matrix.m03 - matrix.m02;
    farPlane_.normal.y = matrix.m13 - matrix.m12;
    farPlane_.normal.z = matrix.m23 - matrix.m22;
    farPlane_.distance = matrix.m33 - matrix.m32;


    // it is not always nessesary to normalize the planes of the frustum.
    // Non-normalized planes can still be used for basic intersection tests.
    if (normalizePlanes)
    {
        leftPlane_.Normalize();
        rightPlane_.Normalize();
        topPlane_.Normalize();
        bottomPlane_.Normalize();
        nearPlane_.Normalize();
        farPlane_.Normalize();
    }
}

//---------------------------------------------------------
// Desc:   transform the current frustum with input matrix and 
//         store the result into outFrustum
//---------------------------------------------------------
void Frustum::Transform(Frustum& outFrustum, const Matrix& mat) const
{
    // compute an inverse transpose matrix for proper
    // transformation of the plane's normal vector
    Matrix invMT;
    MatrixInverse(invMT, nullptr, mat);
    MatrixTranspose(invMT);

    outFrustum = *this;

    outFrustum.leftPlane_.Transform(&invMT);
    outFrustum.rightPlane_.Transform(&invMT);
    outFrustum.topPlane_.Transform(&invMT);
    outFrustum.bottomPlane_.Transform(&invMT);
    outFrustum.nearPlane_.Transform(&invMT);
    outFrustum.farPlane_.Transform(&invMT);
}


//==================================================================================
// TESTS
//==================================================================================
bool Frustum::TestPoint(const Vec3& p) const
{
    return  (leftPlane_.SignedDistance(p)        >= 0.0f) &&
            (rightPlane_.SignedDistance(p)       >= 0.0f) &&
            (topPlane_.SignedDistance(p)         >= 0.0f) &&
            (bottomPlane_.SignedDistance(p)      >= 0.0f) &&
            (nearPlane_.SignedDistance(p)        >= 0.0f) &&
            (farPlane_.SignedDistance(p)         >= 0.0f);
}

//---------------------------------------------------------
// Desc:  test if input 3d rectangle is contained or intersected by the frustum
//---------------------------------------------------------
bool Frustum::TestRect(const Rect3d& rect) const
{
    ++numTests;

    return  (PlaneClassify(rect, leftPlane_)     != PLANE_BACK) &&
            (PlaneClassify(rect, rightPlane_)    != PLANE_BACK) &&
            (PlaneClassify(rect, topPlane_)      != PLANE_BACK) &&
            (PlaneClassify(rect, bottomPlane_)   != PLANE_BACK) &&
            (PlaneClassify(rect, nearPlane_)     != PLANE_BACK) &&
            (PlaneClassify(rect, farPlane_)      != PLANE_BACK);
}

//---------------------------------------------------------
// Desc:   test if inter sphere is contained or intersected by the frustum
//---------------------------------------------------------
bool Frustum::TestSphere(const Sphere& sphere) const
{
#if 0
    return
        leftPlane_.SignedDistance(sphere.center)     >= -sphere.radius &&
        rightPlane_.SignedDistance(sphere.center)    >= -sphere.radius &&
        topPlane_.SignedDistance(sphere.center)      >= -sphere.radius &&
        bottomPlane_.SignedDistance(sphere.center)   >= -sphere.radius &&
        nearPlane_.SignedDistance(sphere.center)     >= -sphere.radius &&
        farPlane_.SignedDistance(sphere.center)      >= -sphere.radius;
#else

    ++numTests;

    if ((PlaneClassify(sphere, leftPlane_)     == PLANE_BACK) ||
        (PlaneClassify(sphere, rightPlane_)    == PLANE_BACK) ||
        (PlaneClassify(sphere, topPlane_)      == PLANE_BACK) ||
        (PlaneClassify(sphere, bottomPlane_)   == PLANE_BACK) ||
        (PlaneClassify(sphere, nearPlane_)     == PLANE_BACK) ||
        (PlaneClassify(sphere, farPlane_)      == PLANE_BACK))
    {
        return false;
    }

    return true;

#endif
}

//---------------------------------------------------------
// Desc:   get frustum's corner points (in view space)
//         (T-top, B-bottom, R-right, L-left)
//---------------------------------------------------------
void Frustum::GetPointsInView(
    Vec3& nearTL,
    Vec3& nearBL,
    Vec3& nearTR,
    Vec3& nearBR,
    Vec3& farTL,
    Vec3& farBL,
    Vec3& farTR,
    Vec3& farBR) const
{
    // near plane
    const float xn = nearPlaneWidth_ * 0.5f;
    const float yn = nearPlaneHeight_ * 0.5f;
    const float zn = -nearZ_;

    // far plane
    const float xf = farPlaneWidth_ * 0.5f;
    const float yf = farPlaneHeight_ * 0.5f;
    const float zf = farZ_;

    nearTL = Vec3(-xn,  yn, zn);
    nearBL = Vec3(-xn, -yn, zn);
    nearTR = Vec3( xn,  yn, zn);
    nearBR = Vec3( xn, -yn, zn);

    farTL  = Vec3(-xf,  yf, zf);
    farBL  = Vec3(-xf, -yf, zf);
    farTR  = Vec3( xf,  yf, zf);
    farBR  = Vec3( xf, -yf, zf);
}

//---------------------------------------------------------
// Desc:  get frustum's corner points in world space
//        (T-top, B-bottom, R-right, L-left)
//---------------------------------------------------------
void Frustum::GetPointsInWorld(
    Vec3& nearTL,
    Vec3& nearBL,
    Vec3& nearTR,
    Vec3& nearBR,
    Vec3& farTL,
    Vec3& farBL,
    Vec3& farTR,
    Vec3& farBR,
    const Matrix* invView) const
{
    assert(invView);

    GetPointsInView(nearTL, nearBL, nearTR, nearBR, farTL, farBL, farTR, farBR);

    // transform each point into world space
    MatrixMulVec3(nearTL, *invView, nearTL);
    MatrixMulVec3(nearBL, *invView, nearBL);
    MatrixMulVec3(nearTR, *invView, nearTR);
    MatrixMulVec3(nearBR, *invView, nearBR);

    MatrixMulVec3(farTL, *invView, farTL);
    MatrixMulVec3(farBL, *invView, farBL);
    MatrixMulVec3(farTR, *invView, farTR);
    MatrixMulVec3(farBR, *invView, farBR);
}

//---------------------------------------------------------
// Desc:  get axis-aligned bounding box (in view space!!!) around the frustum volume
//---------------------------------------------------------
Rect3d Frustum::GetBoundBoxInView(void) const
{
    const float xf = farPlaneWidth_ * 0.5f;
    const float yf = farPlaneHeight_ * 0.5f;

    return Rect3d(-xf, +xf, -yf, +yf, -nearZ_, farZ_);
}

//---------------------------------------------------------
// Desc:  get axis-aligned bounding box (in world space) around the frustum volume
// Args:  - inverse view matrix (to convert AABB from view space into world space)
//---------------------------------------------------------
Rect3d Frustum::GetBoundBoxInWorld(const Matrix* pInvView) const
{
    Vec3 p[8];  // 8 corner points of the frustum volume
    GetPointsInWorld(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], pInvView);

    return Rect3d{ p, 8 };
}
