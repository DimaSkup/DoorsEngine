/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: matrix.h
    Desc:     implementation of 4x4 matrix

\**********************************************************************************/
#pragma once

#include <math/vec4.h>
#include <math/vec3.h>
#include <math/math_constants.h>
#include <math/math_helpers.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>


//==================================================================================
// Structure:  Matrix4x4
//==================================================================================
struct Matrix4x4
{
    union
    {
        float mat[16];
        float m[4][4];  // array for storing data

        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
    };
};

//==================================================================================
// Class:   Matrix
//==================================================================================
__declspec(align(16)) class Matrix
{
public:
    Matrix();
    Matrix(const Matrix& src);

    Matrix(const float* arr);

    Matrix(const float m00, const float m01, const float m02, const float m03,
           const float m10, const float m11, const float m12, const float m13,
           const float m20, const float m21, const float m22, const float m23,
           const float m30, const float m31, const float m32, const float m33);
        
    Matrix(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3);

    Matrix& operator *= (const Matrix& mat);
    Matrix& operator =  (const Matrix& mat);

    inline const Vec4& operator[](const int row) const { return r[row]; }
    inline Vec4& operator[](const int row)             { return r[row]; }

public:
    union
    {
        // intentionally don't set matrix with zeros
        float mat[16];
        float m[4][4];

        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };

        struct
        {
            Vec4 r[4];
        };
    };
};


//==================================================================================
// identity matrices
//==================================================================================
constexpr Matrix4x4 ZERO_MAT_4X4 =
{
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0
};

constexpr Matrix4x4 IDENTITY_MAT_4X4 = 
{
    1,0,0,0,  
    0,1,0,0,  
    0,0,1,0,  
    0,0,0,1
};


//==================================================================================
// matrix constructors
//==================================================================================

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
inline Matrix::Matrix()
{
    // intentionally don't set matrix with zeros here!
}

//---------------------------------------------------------
// Desc:   init the 4x4 matrix with arr of 16 floats
//---------------------------------------------------------
inline Matrix::Matrix(const float* arr)
{
    assert(arr != nullptr);
    memcpy(mat, arr, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   init the 4x4 matrix with 16 floats
//---------------------------------------------------------
inline Matrix::Matrix(const float _m00, const float _m01, const float _m02, const float _m03,
                      const float _m10, const float _m11, const float _m12, const float _m13,
                      const float _m20, const float _m21, const float _m22, const float _m23,
                      const float _m30, const float _m31, const float _m32, const float _m33) :
    m00(_m00), m01(_m01), m02(_m02), m03(_m03),
    m10(_m10), m11(_m11), m12(_m12), m13(_m13),
    m20(_m20), m21(_m21), m22(_m22), m23(_m23),
    m30(_m30), m31(_m31), m32(_m32), m33(_m33)
{
}

//---------------------------------------------------------
// Desc:   copy constructor
//---------------------------------------------------------
inline Matrix::Matrix(const Matrix& src)
{
    memcpy(mat, src.mat, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   init the 4x4 matrix with 4 Vec4 values
//---------------------------------------------------------
inline Matrix::Matrix(const Vec4& r0, const Vec4& r1, const Vec4& r2, const Vec4& r3) :
    r{ r0, r1, r2, r3 }
{
}


//==================================================================================
// matrix functions
//==================================================================================

//---------------------------------------------------------
// Desc:   print out a matrix into the console
//---------------------------------------------------------
inline void MatrixPrint(const Matrix& mat)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            printf("%13.6f ", mat.m[i][j]);
        }
        printf("\n");
    }
}

//---------------------------------------------------------
// Desc:  compare two matrices and return true if they are equal
//---------------------------------------------------------
inline bool MatrixEqual(const Matrix& m1, const Matrix& m2)
{
    bool isEqual = true;

    for (int i = 0; i < 16; ++i)
        isEqual &= (FloatEqual(m1.mat[i], m2.mat[i]));

    return isEqual;
}

//---------------------------------------------------------
// Desc:   fill in the input matrix with zeros
//---------------------------------------------------------
inline void MatrixZero(Matrix& m)
{
    memset(m.mat, 0, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   setup input matrix as identity
//---------------------------------------------------------
inline void MatrixIdentity(Matrix& m)
{
    memcpy(m.mat, IDENTITY_MAT_4X4.m, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   return an identity matrix
//---------------------------------------------------------
inline Matrix MatrixIdentity()
{
    Matrix m;
    MatrixIdentity(m);
    return m;
}

//---------------------------------------------------------
// Desc:   copy values from src matrix into the dest matrix
//---------------------------------------------------------
inline void MatrixCopy(const Matrix& src, Matrix& dst)
{
    memcpy(dst.mat, src.mat, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   transpose input matrix
//---------------------------------------------------------
inline void MatrixTranspose(Matrix& mat)
{
    Matrix mt;
    mt.m00 = mat.m00;  mt.m01 = mat.m10;  mt.m02 = mat.m20;  mt.m03 = mat.m30;
    mt.m10 = mat.m01;  mt.m11 = mat.m11;  mt.m12 = mat.m21;  mt.m13 = mat.m31;
    mt.m20 = mat.m02;  mt.m21 = mat.m12;  mt.m22 = mat.m22;  mt.m23 = mat.m32;
    mt.m30 = mat.m03;  mt.m31 = mat.m13;  mt.m32 = mat.m23;  mt.m33 = mat.m33;

    memcpy(mat.mat, mt.mat, sizeof(Matrix4x4));
}

//---------------------------------------------------------
// Desc:   transpose input matrix and store results into the output matrix
//---------------------------------------------------------
inline void MatrixTranspose(const Matrix& src, Matrix& dst)
{
    dst.m00 = src.m00;  dst.m01 = src.m10;  dst.m02 = src.m20;  dst.m03 = src.m30;
    dst.m10 = src.m01;  dst.m11 = src.m11;  dst.m12 = src.m21;  dst.m13 = src.m31;
    dst.m20 = src.m02;  dst.m21 = src.m12;  dst.m22 = src.m22;  dst.m23 = src.m32;
    dst.m30 = src.m03;  dst.m31 = src.m13;  dst.m32 = src.m23;  dst.m33 = src.m33;
}

//---------------------------------------------------------
// Desc:   calc and return a determinant of the matrix
//---------------------------------------------------------
inline float MatrixDeterminant(const Matrix& mat)
{ 
    return mat.m00 * (mat.m11*mat.m22 - mat.m21*mat.m12) -
           mat.m01 * (mat.m10*mat.m22 - mat.m20*mat.m12) +
           mat.m02 * (mat.m10*mat.m21 - mat.m20*mat.m11);
}

//---------------------------------------------------------
// Desc:   compute the inverse of the input matrix mat and store the result in invMat
// Ret:    0 if the matrix is invertible
//---------------------------------------------------------
inline int MatrixInverse(Matrix& invMat, float* determinant, const Matrix& mat)
{
    // compute the determinant to see if there is an inverse
    float det = MatrixDeterminant(mat);

    if (determinant != nullptr)
        *determinant = det;

    if (fabs(det) < EPSILON_E5)
    {
        return 0;
    }

    // compute inverse to save divides
    const float detInv = 1.0f / det;

    // compute inverse matrix == adjoint(m) / det(m)
    invMat.m00 =  detInv * (mat.m11*mat.m22 - mat.m12*mat.m21);
    invMat.m01 = -detInv * (mat.m01*mat.m22 - mat.m02*mat.m21);
    invMat.m02 =  detInv * (mat.m01*mat.m12 - mat.m02*mat.m11);
    invMat.m03 = 0;

    invMat.m10 = -detInv * (mat.m10*mat.m22 - mat.m12*mat.m20);
    invMat.m11 =  detInv * (mat.m00*mat.m22 - mat.m02*mat.m20);
    invMat.m12 = -detInv * (mat.m00*mat.m12 - mat.m02*mat.m10);
    invMat.m13 = 0;

    invMat.m20 =  detInv * (mat.m10*mat.m21 - mat.m11*mat.m20);
    invMat.m21 = -detInv * (mat.m00*mat.m21 - mat.m01*mat.m20);
    invMat.m22 =  detInv * (mat.m00*mat.m11 - mat.m01*mat.m10);
    invMat.m23 = 0;

    invMat.m30 = -(mat.m30 * invMat.m00 + mat.m31 * invMat.m10 + mat.m32 * invMat.m20);
    invMat.m31 = -(mat.m30 * invMat.m01 + mat.m31 * invMat.m11 + mat.m32 * invMat.m21);
    invMat.m32 = -(mat.m30 * invMat.m02 + mat.m31 * invMat.m12 + mat.m32 * invMat.m22);
    invMat.m33 = 1.0f;  // always 1

    // return great success
    return 1;
}

//---------------------------------------------------------
// Desc:   compute the inverse of the input matrix mat and return the inverse matrix
// Ret:    if the input matrix is invertible we set det to zero
//---------------------------------------------------------
inline Matrix MatrixInverse(float* det, const Matrix& mat)
{
    Matrix invMat;
    MatrixInverse(invMat, det, mat);
    return invMat;
}

//---------------------------------------------------------
// Desc:   multiply two matrices together and return the result in outMat
//---------------------------------------------------------
inline void MatrixMul(const Matrix& ma, const Matrix& mb, Matrix& outMat)
{
    for (int i = 0; i < 4; i++)
    {
        outMat.m[i][0] = ma.m[i][0] * mb.m[0][0] +
                         ma.m[i][1] * mb.m[1][0] +
                         ma.m[i][2] * mb.m[2][0] +
                         ma.m[i][3] * mb.m[3][0];

        outMat.m[i][1] = ma.m[i][0] * mb.m[0][1] +
                         ma.m[i][1] * mb.m[1][1] +
                         ma.m[i][2] * mb.m[2][1] +
                         ma.m[i][3] * mb.m[3][1];

        outMat.m[i][2] = ma.m[i][0] * mb.m[0][2] +
                         ma.m[i][1] * mb.m[1][2] +
                         ma.m[i][2] * mb.m[2][2] +
                         ma.m[i][3] * mb.m[3][2];

        outMat.m[i][3] = ma.m[i][0] * mb.m[0][3] +
                         ma.m[i][1] * mb.m[1][3] +
                         ma.m[i][2] * mb.m[2][3] +
                         ma.m[i][3] * mb.m[3][3];
    }
}

//---------------------------------------------------------
// Desc:   multiply a Vec3 by 4x4 matrix and return the result in outVec;
// 
//         the function assumes that the vector refers to a 4D homogenous vector, thus the
//         function assumes that w = 1 to carry out the multiply, also the function
//         does not carry out the last column multiply since we are assuming w = 1;
//
//         this function can be used both for 3D points/vectors transformation using 4x4 matrices;
//---------------------------------------------------------
inline void MatrixMulVec3(const Vec3& vec, const Matrix& mat, Vec3& outVec)
{
    Vec3 tmp(0,0,0);

    // multiply vec by matrix
    for (int col = 0; col < 3; col++)
    {
        tmp.xyz[col] += (vec.xyz[0] * mat.m[0][col]);
        tmp.xyz[col] += (vec.xyz[1] * mat.m[1][col]);
        tmp.xyz[col] += (vec.xyz[2] * mat.m[2][col]);

        // add in last element in column or w*pM[3][col]
        tmp.xyz[col] += mat.m[3][col];
    }

    outVec = tmp;
}

//---------------------------------------------------------
// Desc:   multiply a Vec4 by 4x4 matrix and return the result in outVec;
//---------------------------------------------------------
inline void MatrixMulVec4(const Vec4& vec, const Matrix& mat, Vec4& outVec)
{
    // multiply vec by matrix
    for (int col = 0; col < 4; col++)
    {
        outVec.xyzw[col] = 0.0f;
        outVec.xyzw[col] += (vec.xyzw[0] * mat.m[0][col]);
        outVec.xyzw[col] += (vec.xyzw[1] * mat.m[1][col]);
        outVec.xyzw[col] += (vec.xyzw[2] * mat.m[2][col]);
        outVec.xyzw[col] += (vec.xyzw[3] * mat.m[3][col]);
    }
}

//---------------------------------------------------------
// Desc:   return a scaling matrix
//---------------------------------------------------------
inline Matrix MatrixScaling(const float sx, const float sy, const float sz)
{
    return Matrix{ sx,  0,   0,   0,
                   0,   sy,  0,   0,
                   0,   0,   sz,  0,
                   0,   0,   0,   1 };
}

//---------------------------------------------------------
// Desc:   generate a row-major translation matrix
//---------------------------------------------------------
inline Matrix MatrixTranslation(const float tx, const float ty, const float tz)
{
    return Matrix{ 1,   0,   0,   0,
                   0,   1,   0,   0,
                   0,   0,   1,   0,
                   tx,  ty,  tz,  1.0f };
}

//---------------------------------------------------------
// Desc:   generate a row-major rotation matrix around X-axis 
//         (angle goes in COUNTER CLOCKWISE order; the angle in radians)
//---------------------------------------------------------
inline Matrix MatrixRotationX(const float angle)
{
    const float s = sinf(angle);
    const float c = cosf(angle);

#if 0
    // column-major
    return Matrix{  1,  0,  0,  0,
                    0,  c, -s,  0,
                    0,  s,  c,  0,
                    0,  0,  0,  1  };
#else
    // row-major
    return Matrix{  1,  0,  0,  0,
                    0,  c,  s,  0,
                    0, -s,  c,  0,
                    0,  0,  0,  1  };
#endif
}

//---------------------------------------------------------
// Desc:   generate a row-major rotation matrix around Y-axis 
//         (angle goes in CLOCKWISE order; the angle in radians)
//---------------------------------------------------------
inline Matrix MatrixRotationY(const float angle)
{
    const float s = sinf(angle);
    const float c = cosf(angle);

#if 0
    // column-major
    return Matrix{  c,  0,  s,  0,
                    0,  1,  0,  0,
                   -s,  0,  c,  0,
                    0,  0,  0,  1  };
#else
    // row-major
    return Matrix{  c,  0, -s,  0,
                    0,  1,  0,  0,
                    s,  0,  c,  0,
                    0,  0,  0,  1  };
#endif
}

//---------------------------------------------------------
// Desc:   generate a row-major rotation matrix around Z-axis 
//         (angle goes in COUNTER CLOCKWISE order; the angle in radians)
//---------------------------------------------------------
inline Matrix MatrixRotationZ(const float angle)
{
    const float s = sinf(angle);
    const float c = cosf(angle);

#if 0
    // column-major
    return Matrix{  c, -s,  0,  0,
                    s,  c,  0,  0,
                    0,  0,  1,  0,
                    0,  0,  0,  1  };
#else
    // row-major
    return Matrix{  c,  s,  0,  0,
                   -s,  c,  0,  0,
                    0,  0,  1,  0,
                    0,  0,  0,  1  };
#endif
}

//---------------------------------------------------------
// Desc:   return a row-major rotation matrix about an arbitrary axis
// Args:   - axis:  vector describing the axis of rotation
//         - angle: angle of rotation in radians. Angles are measured clockwise when
//                  looking along the rotation axis toward the origin
//---------------------------------------------------------
inline Matrix MatrixRotationAxis(Vec3 axis, const float angle)
{
    // first of all we normalize the input axis vector
    const float invLen = 1.0f / sqrtf(SQR(axis.x) + SQR(axis.y) + SQR(axis.z));
    const float nx = axis.x * invLen;
    const float ny = axis.y * invLen;
    const float nz = axis.z * invLen;

    // precompute sine and cosine of the input angle
    const float s = sinf(angle);
    const float c = cosf(angle);

    // return actual rotation matrix
    return Matrix {
        c + (1-c)*SQR(nx),   (1-c)*nx*ny + s*nz,   (1-c)*nx*nz - s*ny,   0,
        (1-c)*nx*ny - s*nz,  c+(1-c)*SQR(ny),      (1-c)*ny*nz + s*nx,   0,
        (1-c)*nx*nz + s*ny,  (1-c)*ny*nz - s*nx,   c+(1-c)*SQR(nz),      0,
        0,                   0,                    0,                    1
    };
}

//---------------------------------------------------------
// Desc:   computer and return a projection matrix for left-handed system
// Args:   - fov:         field of view in radians
//         - aspectRatio: sc
//---------------------------------------------------------
inline Matrix MatrixProjectionLH(
    const float fov, 
    const float aspectRatio,
    const float nearZ,
    const float farZ)
{
    assert(fov > 0.0f);
    assert(aspectRatio > 0.0f);
    assert(nearZ > 0.0f && farZ > 0.0f);
    assert(nearZ < farZ);

    // | (h/w)*1/tan(fov/2)               0                  0      0 |
    // |                  0   1/tanf(fov/2)                  0      0 |
    // |                  0               0         zf/(zf-zn)      1 | 
    // |                  0               0   (-zf*zn)/(zf-zn)      0 |
    Matrix M;
    memset(M.m, 0, sizeof(Matrix));

#if 0
    // compute focal length
    const float e = 1.0f / tanf(fov * 0.5f);

    m.m00 = aspectRatio * e;
    m.m11 = e;
    m.m22 = farZ / (farZ - nearZ);
    m.m23 = 1.0f;
    m.m32 = (-farZ * nearZ) / (farZ - nearZ);
#else
    // compute focal length
    const float e = 1.0f / tanf(fov * 0.5f);

    float Height = e;
    float Width = Height / aspectRatio;
    float fRange = farZ / (farZ - nearZ);

    M.m[0][0] = Width;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = Height;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 1.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = -fRange * nearZ;
    M.m[3][3] = 0.0f;
#endif


    return M;
}

//==================================================================================
// operators
//==================================================================================

//---------------------------------------------------------
// Desc:   multiply the current matrix by input one and
//         store the result into the current matrix
//---------------------------------------------------------
inline Matrix& Matrix::operator *= (const Matrix& mat)
{
    Matrix tmp;
    
    MatrixMul(*this, mat, tmp);
    MatrixCopy(tmp, *this);

    return *this;
}

//---------------------------------------------------------
// Desc:   multiply input matrix m1 by matrix m2 and 
//         return the result as a new Matrix
//---------------------------------------------------------
inline Matrix operator * (const Matrix& m1, const Matrix& m2)
{
    Matrix result;
    MatrixMul(m1, m2, result);

    return result;
}

//---------------------------------------------------------
// Desc:   matrix assignment
//---------------------------------------------------------
inline Matrix& Matrix::operator = (const Matrix& mat)
{
    // prevent self assignment
    if (this == &mat)
        return (*this);

    MatrixCopy(mat, *this);

    return (*this);
}
