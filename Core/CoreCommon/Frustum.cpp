#include "pch.h"
#include "Frustum.h"
#include <DirectXCollision.h>

DirectX::BoundingFrustum dxFrustum;

//---------------------------------------------------------
// Desc:   create view frustum plane vectors in camera space in
//         terms of the focal length, aspect ratio, near and far plane distance
//
//          x-z plane                      y-z plane
//     (e is a focal length)         (e is a focal length)
// 
//        1           1
//  *-----------*-----------*          
//    \         |         /         
//      \     e |       /            
//        \     |     /          
//          \   |   /
//            \ | /
//              *
// 
//---------------------------------------------------------
Frustum::Frustum(
    const float fov,
    const float aspectRatio,
    const float nearZ,
    const float farZ)
{
    // compute focal length
    const float e = 1.0f / tanf(fov/2);

    float denominator1 = sqrtf(SQR(e) + 1);
    float denominator2 = sqrtf(SQR(e) + SQR(aspectRatio));

    // normalized normal vectors
    float horizNx = e / denominator1;
    float horizNz = 1 / denominator1;
    float vertNy  = e / denominator2;
    float vertNz  = aspectRatio / denominator2;


    nearClipPlane_   = { 0,0,+1,nearZ };
    farClipPlane_    = { 0,0,-1,farZ };

    rightClipPlane_  = { -horizNx, 0, horizNz, 0 };
    leftClipPlane_   = { +horizNx, 0, horizNz, 0 };

    topClipPlane_    = { 0, -vertNy, vertNz, 0 };
    bottomClipPlane_ = { 0, vertNy, vertNz, 0 };

    // test if frustum works correctly
    if (!TestFrustum())
    {
        LogErr(LOG, "frustum works incorrectly");
    }
}

//---------------------------------------------------------
// Desc:   setup the fructum clipping planes using input proj matrix
//---------------------------------------------------------
Frustum::Frustum(const Matrix& m)
{
    const Vec4 row1(m[0][0], m[0][1], m[0][2], m[0][3]);
    const Vec4 row2(m[1][0], m[1][1], m[1][2], m[1][3]);
    const Vec4 row3(m[2][0], m[2][1], m[2][2], m[2][3]);
    const Vec4 row4(m[3][0], m[3][1], m[3][2], m[3][3]);

    nearClipPlane_   = { row3 + row4 };
    farClipPlane_    = { row3 - row4 };
    rightClipPlane_  = { row1 - row4 };
    leftClipPlane_   = { row1 + row4 };
    topClipPlane_    = { row2 - row4 };
    bottomClipPlane_ = { row2 + row4 };
}


//---------------------------------------------------------
// Desc:   setup the frustum with 6 input normalized (!) planes
// Args:   - each arg is a tuple of 4 floats which represents
//           a normalized plane(nx, ny, nz, d)
//---------------------------------------------------------
Frustum::Frustum(
    const float* nearPlane,
    const float* farPlane,
    const float* rightPlane,
    const float* leftPlane,
    const float* topPlane,
    const float* bottomPlane)
{
    constexpr size_t sz = sizeof(float) * 4;

    memcpy((void*)&nearClipPlane_,   nearPlane,   sz);
    memcpy((void*)&farClipPlane_,    farPlane,    sz);

    memcpy((void*)&rightClipPlane_,  rightPlane,  sz);
    memcpy((void*)&leftClipPlane_,   leftPlane,   sz);

    memcpy((void*)&topClipPlane_,    topPlane,    sz);
    memcpy((void*)&bottomClipPlane_, bottomPlane, sz);
}

//---------------------------------------------------------
// Desc:   execute normalization of the plane
// Args:   - pl: frustum plane to normalize
//---------------------------------------------------------
inline void NormalizePlane(FrustumPlane& pl)
{
    float invLen = 1.0f / pl.n.Length();

    pl.n.x *= invLen;
    pl.n.y *= invLen;
    pl.n.z *= invLen;
    pl.d   *= invLen;
}

//---------------------------------------------------------
// Desc:   test a point for inclusion in the viewing frustum
// Args:   - x,y,z: vertex position in 3D space
// Ret:    A boolean value: true if the vertex is visible
//---------------------------------------------------------
bool Frustum::PointTest(const float x, const float y, const float z) const
{
    // dot(n,p) + d: where n is a normal; p is an input point; d = -n*p0

    return
        ((Dot(nearClipPlane_.n,   { x,y,z }) + nearClipPlane_.d)   > 0) &&
        ((Dot(farClipPlane_.n,    { x,y,z }) + farClipPlane_.d)    > 0) &&
        ((Dot(leftClipPlane_.n,   { x,y,z }) + leftClipPlane_.d)   > 0) &&
        ((Dot(rightClipPlane_.n,  { x,y,z }) + rightClipPlane_.d)  > 0) &&
        ((Dot(topClipPlane_.n,    { x,y,z }) + topClipPlane_.d)    > 0) &&
        ((Dot(bottomClipPlane_.n, { x,y,z }) + bottomClipPlane_.d) > 0);
}

//---------------------------------------------------------
// Desc:   check if frustum contains sphere
// Args:   - x, y, z: input sphere center in 3D space
//         - r:       sphere radius
// Ret:    true if frustum intersects or completely contains the sphere
//---------------------------------------------------------
bool Frustum::SphereTest(
    const float x,
    const float y,
    const float z,
    const float r) const
{
    return
       ((Dot(nearClipPlane_.n,   { x,y,z }) + nearClipPlane_.d)   >= -r) &&
       ((Dot(farClipPlane_.n,    { x,y,z }) + farClipPlane_.d)    >= -r) &&
       ((Dot(leftClipPlane_.n,   { x,y,z }) + leftClipPlane_.d)   >= -r) &&
       ((Dot(rightClipPlane_.n,  { x,y,z }) + rightClipPlane_.d)  >= -r) &&
       ((Dot(topClipPlane_.n,    { x,y,z }) + topClipPlane_.d)    >= -r) &&
       ((Dot(bottomClipPlane_.n, { x,y,z }) + bottomClipPlane_.d) >= -r);
}

//---------------------------------------------------------
// Desc:   test the six vertices of the axis-aligned bounding box
//         against the viewing frustum
// Args:   - x,y,z:    center of the cube (NOTE: must be translated into camera space)
//         - halfSize: half of the cube size along each axis
// Ret:    boolean value: true if any part of the cube is in the frustum 
//---------------------------------------------------------
bool Frustum::CubeTest(
    const float x,
    const float y,
    const float z,
    const float halfSize) const
{
    const Vec3 cube[8] =
    {
        // bottom part
        {x-halfSize, y-halfSize, z-halfSize},
        {x+halfSize, y-halfSize, z-halfSize},
        {x-halfSize, y-halfSize, z+halfSize},
        {x+halfSize, y-halfSize, z+halfSize},

        // upper part
        {x-halfSize, y+halfSize, z-halfSize},
        {x+halfSize, y+halfSize, z-halfSize},
        {x-halfSize, y+halfSize, z+halfSize},
        {x+halfSize, y+halfSize, z+halfSize},
    };

    for (int i = 0; i < 8; ++i)
    {
        if (PointTest(cube[i].x, cube[i].y, cube[i].z))
            return true;
    }

    return false;
}


//---------------------------------------------------------
// Desc:  some tests to make sure that frustum works correctly
//---------------------------------------------------------
bool Frustum::TestFrustum()
{
    bool res = false;

    // shapes expected to be completely in the frustum
    const Vec3 sphereCenter0  = { 0,0, 10 };
    const float sphereRadius0 = 2;

    const Vec3 sphereCenter1 = { 0,0,nearClipPlane_.n.z };
    const float sphereRadius1 = 1;

    // shapes expected to be out of frustum
    const Vec3 sphereCenter5 = { 0,0,-4 };
    const float sphereRadius5 = 3;

    const Vec3 sphereCenter6 = { 0,-3,nearClipPlane_.n.z };
    const float sphereRadius6 = 1;


    res = SphereTest(sphereCenter0.x, sphereCenter0.y, sphereCenter0.z, sphereRadius0);
    if (!res)
    {
        LogErr(LOG, "testing of frustum is failed: sphere is expected to be in frustum");
        return false;
    }

    res = SphereTest(sphereCenter1.x, sphereCenter1.y, sphereCenter1.z, sphereRadius1);
    if (!res)
    {
        LogErr(LOG, "testing of frustum is failed: sphere is expected to be in frustum");
        return false;
    }



    res = SphereTest(sphereCenter5.x, sphereCenter5.y, sphereCenter5.z, sphereRadius5);
    if (res == true)
    {
        LogErr(LOG, "testing of frustum is failed: sphere is expected to be out of frustum");
        return false;
    }

    res = SphereTest(sphereCenter6.x, sphereCenter6.y, sphereCenter6.z, sphereRadius6);
    if (res == true)
    {
        LogErr(LOG, "testing of frustum is failed: sphere is expected to be in frustum");
        return false;
    }

    return true;
}
