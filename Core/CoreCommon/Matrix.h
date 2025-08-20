#ifndef MATRIX_H
#define MATRIX_H

#include <assert.h>
#include <DMath.h>
#include <memory.h>


__declspec(align(16)) struct Matrix
{
public:
    Matrix() {}

    // ----------------------------------------------------
    // Desc:   initialize the 4x4 matrix with arr of 16 floats
    // Args:   - arr: an array of floats
    // ----------------------------------------------------
    Matrix(const float* arr)
    {
        assert(arr != nullptr);
        memcpy((void*)mat, (void*)arr, sizeof(float) * 16);
    }


    inline Matrix& operator*= (const Matrix& mat)
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                this->m[i][j] =
                    this->m[i][0] * mat.m[0][j] +
                    this->m[i][1] * mat.m[1][j] +
                    this->m[i][2] * mat.m[2][j] +
                    this->m[i][3] * mat.m[3][j];
            }
        }

        return *this;
    }

    inline const Vec4& operator[](const int row) const { return r[row]; }

    inline Vec4& operator[](const int row) { return r[row]; }

public:
    union
    {
        float mat[16]{0};
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

//=========================================================

inline Matrix operator*(const Matrix& m1, const Matrix& m2)
{
    // multiply input matrix m1 by m2 and 
    // return the result as a new Matrix

    Matrix result;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.m[i][j] =
                m1.m[i][0] * m2.m[0][j] +
                m1.m[i][1] * m2.m[1][j] +
                m1.m[i][2] * m2.m[2][j] +
                m1.m[i][3] * m2.m[3][j];
        }
    }

    return result;
}


#endif
