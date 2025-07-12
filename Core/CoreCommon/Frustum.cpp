#include "Frustum.h"
#include <DMath.h>
#include <assert.h>
#include <memory.h>


//---------------------------------------------------------
// Desc:   initialize a view frustum by fields of view and near/far planes
// Args:   - fovX:  horizontal field of view in radians
//         - fovY:  vertical field of view in radians
//         - nearZ: near clip plane
//         - farZ:  far clip plane
//---------------------------------------------------------
void Frustum::Initialize(
    const float fovX,
    const float fovY,
    const float nearZ,
    const float farZ)
{
# if 0
    assert(fovX > 0  && "fovX must be > 0");
    assert(fovY > 0  && "fovY must be > 0");
    assert(nearZ > 0 && "nearZ must be > 0");
    assert(farZ > 0  && "farZ must be > 0");

    fovX_  = fovX;
    fovY_  = fovY;
    nearZ_ = nearZ;
    farZ_  = farZ;

    // each frustum plane is defined by a point and a normal vector;

    // horizontal
    const float sx = sinf(fovX / 2);
    const float cx = cosf(fovX / 2);

    // vertical
    const float sy = sinf(fovY / 2);
    const float cy = cosf(fovY / 2);

    const Vec3 origin = { 0,0,0 };

    // far plane
    planes_[FRUSTUM_FAR].point      = { 0, 0, farZ };
    planes_[FRUSTUM_FAR].normal     = { 0, 0, -1 };

    // near plane
    planes_[FRUSTUM_NEAR].point     = { 0, 0, nearZ };
    planes_[FRUSTUM_NEAR].normal    = { 0,0,1 };

    // left plane
    planes_[FRUSTUM_LEFT].point     = origin;
    planes_[FRUSTUM_LEFT].normal    = { cx, 0, sx };

    // right plane
    planes_[FRUSTUM_RIGHT].point    = origin;
    planes_[FRUSTUM_RIGHT].normal   = { -cx, 0, sx };

    // top plane
    planes_[FRUSTUM_TOP].point      = origin;
    planes_[FRUSTUM_TOP].normal     = { 0, -cy, sy };

    // bottom plane
    planes_[FRUSTUM_BOTTOM].point   = origin;
    planes_[FRUSTUM_BOTTOM].normal  = { 0, cy, sy };
#endif
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
// Desc:    calculate the planes that make-up the viewing frustum
// Args:    viewMatrix: a pointer to arr of 16 floats of viewing matrix
//          projMatrix: a pointer to arr of 16 floats of projection matrix
//---------------------------------------------------------
void Frustum::Initialize(
    const float* viewMatrix,
    const float* projMatrix)
{
    assert(viewMatrix != nullptr);
    assert(projMatrix != nullptr);

    const float* view = viewMatrix;     //view matrix
    const float* proj = projMatrix;     //projection matrix
    float  clip[16]{ 0 };
   
    // Combine the two matrices (multiply projection by modelview)
    clip[0]  = view[0] * proj[0] + view[1] * proj[4] + view[2] * proj[8] + view[3] * proj[12];
    clip[1]  = view[0] * proj[1] + view[1] * proj[5] + view[2] * proj[9] + view[3] * proj[13];
    clip[2]  = view[0] * proj[2] + view[1] * proj[6] + view[2] * proj[10] + view[3] * proj[14];
    clip[3]  = view[0] * proj[3] + view[1] * proj[7] + view[2] * proj[11] + view[3] * proj[15];

    clip[4]  = view[4] * proj[0] + view[5] * proj[4] + view[6] * proj[8] + view[7] * proj[12];
    clip[5]  = view[4] * proj[1] + view[5] * proj[5] + view[6] * proj[9] + view[7] * proj[13];
    clip[6]  = view[4] * proj[2] + view[5] * proj[6] + view[6] * proj[10] + view[7] * proj[14];
    clip[7]  = view[4] * proj[3] + view[5] * proj[7] + view[6] * proj[11] + view[7] * proj[15];

    clip[8]  = view[8] * proj[0] + view[9] * proj[4] + view[10] * proj[8] + view[11] * proj[12];
    clip[9]  = view[8] * proj[1] + view[9] * proj[5] + view[10] * proj[9] + view[11] * proj[13];
    clip[10] = view[8] * proj[2] + view[9] * proj[6] + view[10] * proj[10] + view[11] * proj[14];
    clip[11] = view[8] * proj[3] + view[9] * proj[7] + view[10] * proj[11] + view[11] * proj[15];

    clip[12] = view[12] * proj[0] + view[13] * proj[4] + view[14] * proj[8] + view[15] * proj[12];
    clip[13] = view[12] * proj[1] + view[13] * proj[5] + view[14] * proj[9] + view[15] * proj[13];
    clip[14] = view[12] * proj[2] + view[13] * proj[6] + view[14] * proj[10] + view[15] * proj[14];
    clip[15] = view[12] * proj[3] + view[13] * proj[7] + view[14] * proj[11] + view[15] * proj[15];

    // --------------------------------

    // Extract the NEAR plane
    FrustumPlane& pl = planes_[FRUSTUM_NEAR];

    pl.n.x = clip[3]  + clip[2];
    pl.n.y = clip[7]  + clip[6];
    pl.n.z = clip[11] + clip[10];
    pl.d   = clip[15] + clip[14];

    NormalizePlane(pl);

    // --------------------------------

    // Extract the FAR plane
    pl = planes_[FRUSTUM_FAR];

    pl.n.x = clip[3]  - clip[2];
    pl.n.y = clip[7]  - clip[6];
    pl.n.z = clip[11] - clip[10];
    pl.d   = clip[15] - clip[14];

    NormalizePlane(pl);

    // --------------------------------

    // Extract the LEFT plane
    pl = planes_[FRUSTUM_LEFT];

    pl.n.x = clip[3]  + clip[0];
    pl.n.y = clip[7]  + clip[4];
    pl.n.z = clip[11] + clip[8];
    pl.d   = clip[15] + clip[12];

    NormalizePlane(pl);

    // --------------------------------

    // Extract the RIGHT plane
    pl = planes_[FRUSTUM_RIGHT];

    pl.n.x = clip[3]  - clip[0];
    pl.n.y = clip[7]  - clip[4];
    pl.n.z = clip[11] - clip[8];
    pl.d   = clip[15] - clip[12];

    NormalizePlane(pl);

    // --------------------------------

    // Extract the TOP plane
    pl = planes_[FRUSTUM_TOP];

    pl.n.x = clip[3]  - clip[1];
    pl.n.y = clip[7]  - clip[5];
    pl.n.z = clip[11] - clip[9];
    pl.d   = clip[15] - clip[13];

    NormalizePlane(pl);

    // --------------------------------

     // Extract the BOTTOM plane
    pl = planes_[FRUSTUM_BOTTOM];

    pl.n.x = clip[3]  + clip[1];
    pl.n.y = clip[7]  + clip[5];
    pl.n.z = clip[11] + clip[9];
    pl.d   = clip[15] + clip[13];

    NormalizePlane(pl);
}


//---------------------------------------------------------
// Desc:   setup the frustum with 6 input normalized (!) planes
// Args:   - each arg is a tuple of 4 floats which represents
//           a normalized plane(nx, ny, nz, d)
//---------------------------------------------------------
void Frustum::Initialize(
    const float* nearPlane,
    const float* farPlane,
    const float* rightPlane,
    const float* leftPlane,
    const float* topPlane,
    const float* bottomPlane)
{
    constexpr size_t sz = sizeof(float) * 4;

    memcpy((void*)&planes_[FRUSTUM_NEAR],   nearPlane,   sz);
    memcpy((void*)&planes_[FRUSTUM_FAR],    farPlane,    sz);

    memcpy((void*)&planes_[FRUSTUM_LEFT],   leftPlane,   sz);
    memcpy((void*)&planes_[FRUSTUM_RIGHT],  rightPlane,  sz);

    memcpy((void*)&planes_[FRUSTUM_TOP],    topPlane,    sz);
    memcpy((void*)&planes_[FRUSTUM_BOTTOM], bottomPlane, sz);
}

//---------------------------------------------------------
// Desc:   test a vertex for inclusion in the viewing frustum
// Args:   - x,y,z: vertex position in 3D space
// Ret:    A boolean value: true if the vertex is visible
//---------------------------------------------------------
bool Frustum::VertexTest(const float x, const float y, const float z)
{
    // loop through the frustum planes
    for (int i = 0; i < 6; ++i)
    {
        // dot(n,p) + d: where n is a normal; p is an input point; d = -n*p0
        if ((Dot(planes_[i].n, { x,y,z }) + planes_[i].d) < 0)
            return false;
    }
    return true;
}

//---------------------------------------------------------
// Desc:   check if frustum contains sphere
// Args:   - x, y, z: input sphere center in 3D space
//         - radius: sphere radius
// Ret:    true if frustum intersects or completely contains the sphere
//---------------------------------------------------------
bool Frustum::SphereTest(
    const float x,
    const float y,
    const float z,
    const float radius)
{
    // check the sphere agains each frustum plane
    for (int i = 0; i < 6; ++i)
    {
        if ((Dot(planes_[i].n, { x,y,z }) + planes_[i].d) < -radius)
            return false;
    }

    return true;
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
    const float halfSize)
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
        if (VertexTest(cube[i].x, cube[i].y, cube[i].z))
            return true;
    }

    return false;
}
