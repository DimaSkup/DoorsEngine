#include "frustum.h"
#include <geometry/plane_3d_functions.h>

//==================================================================================
// INLINE METHODS
//==================================================================================

//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
Frustum::Frustum()
{
}

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

Frustum::~Frustum()
{
}

//---------------------------------------------------------
// Desc:   create view frustum plane vectors in camera space in
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

    outFrustum.leftPlane_.Transform(invMT);
    outFrustum.rightPlane_.Transform(invMT);
    outFrustum.topPlane_.Transform(invMT);
    outFrustum.bottomPlane_.Transform(invMT);
    outFrustum.nearPlane_.Transform(invMT);
    outFrustum.farPlane_.Transform(invMT);
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
    return
        leftPlane_.SignedDistance(sphere.center)     >= -sphere.radius &&
        rightPlane_.SignedDistance(sphere.center)    >= -sphere.radius &&
        topPlane_.SignedDistance(sphere.center)      >= -sphere.radius &&
        bottomPlane_.SignedDistance(sphere.center)   >= -sphere.radius &&
        nearPlane_.SignedDistance(sphere.center)     >= -sphere.radius &&
        farPlane_.SignedDistance(sphere.center)      >= -sphere.radius;
}

//---------------------------------------------------------
// Desc:   get frustum's corner points
//---------------------------------------------------------
void Frustum::GetPoints(
    Vec3& nearTopLeft,
    Vec3& nearBottomLeft,
    Vec3& nearTopRight,
    Vec3& nearBottomRight,
    Vec3& farTopLeft,
    Vec3& farBottomLeft,
    Vec3& farTopRight,
    Vec3& farBottomRight)
{
    const float xNear = nearPlaneWidth_ * 0.5f;
    const float yNear = nearPlaneHeight_ * 0.5f;
    const float zNear = -nearPlane_.distance;

    const float xFar = farPlaneWidth_ * 0.5f;
    const float yFar = farPlaneHeight_ * 0.5f;
    const float zFar = farPlane_.distance;

    nearTopLeft     = Vec3(-xNear, yNear, zNear);
    nearBottomLeft  = Vec3(-xNear, -yNear, zNear);
    nearTopRight    = Vec3(xNear, yNear, zNear);
    nearBottomRight = Vec3(xNear, -yNear, zNear);

    farTopLeft      = Vec3(-xFar, yFar, zFar);
    farBottomLeft   = Vec3(-xFar, -yFar, zFar);
    farTopRight     = Vec3(xFar, yFar, zFar);
    farBottomRight  = Vec3(xFar, -yFar, zFar);
}
