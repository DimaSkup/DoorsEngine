#include "MathHelper.h"
#include <cmath>
#include <DirectXMath.h>

namespace ECS
{

float MathHelper::AngleFromXY(const float x, const float y)
{
	// get an angle in radians by the input x and y

	float theta = 0.0f;

	// quadrant I or IV
	if (x >= 0.0f)
	{
		// if x == 0, then atanf(y/x) = +pi/2 if y > 0
		//                 atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y/x);            // in [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += DirectX::XM_2PI;  // in [0, 2*pi).
	}

	// quadrant II or III
	else
		theta = atanf(y/x) + DirectX::XM_PI;

	return theta;
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX MathHelper::InverseTranspose(const DirectX::CXMMATRIX& M)
{
	// if we apply a nonuniform scaling transformation A, the transformetd tangent vector
	// uA = v1*A - v0*A doesn't remain orthogonal to the transformed normal vector nA;
	// so the problem is this: given a transformation matrix A that transforms points and
	// vectors (non-normal), we want to find a transformation matrix B that transforms 
	// normal vectors such that the transformed tangent vector is orthogonal to the 
	// transformed normal vector (i.e., uA * nB = 0)

	DirectX::XMMATRIX A = M;

	// we clear out any translation from the matrix because we use the inverse-transpose
	// to transform vectors, and translations only apply to points
	A.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);

	DirectX::XMVECTOR det;// = DirectX::XMMatrixDeterminant(A);

	// return a transformation matrix B
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
}

};  // namespace ECS