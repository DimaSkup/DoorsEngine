#include "MathHelper.h"
#include <cmath>
#include <DirectXMath.h>

using namespace DirectX;

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY(const float x, const float y)
{
	// get an angle in radians by the input x and y coords 
	// relatively to the Origin [0,0]

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

struct Quaternion 
{
	Quaternion(const DirectX::XMVECTOR& quat)
	{
		// real part 
		w = quat.m128_f32[3];   

		// imaginary part
		x = quat.m128_f32[0];   
		y = quat.m128_f32[1];
		z = quat.m128_f32[2];
	}

	float w, x, y, z;
};

struct EulerAngles {
	float roll  = 0.0f;
	float pitch = 0.0f;
	float yaw   = 0.0f;
};

///////////////////////////////////////////////////////////

// return roll/pitch/yaw packed into FLOAT3
DirectX::XMFLOAT3 MathHelper::QuatToRollPitchYaw(const DirectX::XMVECTOR quat)
{
	// this implementation assumes normalized quaternion
	// converts to Euler angles in 3-2-1 sequence

	Quaternion q(quat);

	// x-axis rotation; (forward-backward axis)
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	float roll      = std::atan2(sinr_cosp, cosr_cosp);

	// y-axis rotation; (left-right axis)
	float temp  = 2 * (q.w * q.y - q.x * q.z);
	float sinp  = std::sqrtf(1 + temp);
	float cosp  = std::sqrtf(1 - temp);
	float pitch = 2 * std::atan2(sinp, cosp) - XM_PIDIV2;

	// z-axis rotation; (up-down axis)
	float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	float yaw       = std::atan2(siny_cosp, cosy_cosp);

	return { roll, pitch, yaw };
}

///////////////////////////////////////////////////////////

const XMMATRIX MathHelper::InverseTranspose(const CXMMATRIX& M)
{
	// if we apply a nonuniform scaling transformation A, the transformetd tangent vector
	// uA = v1*A - v0*A doesn't remain orthogonal to the transformed normal vector nA;
	// so the problem is this: given a transformation matrix A that transforms points and
	// vectors (non-normal), we want to find a transformation matrix B that transforms 
	// normal vectors such that the transformed tangent vector is orthogonal to the 
	// transformed normal vector (i.e., uA * nB = 0)

	XMMATRIX A = M;

	// we clear out any translation from the matrix because we use the inverse-transpose
	// to transform vectors, and translations only apply to points
	A.r[3] = { 0, 0, 0, 1 };

	//XMVECTOR det;  // = DirectX::XMMatrixDeterminant(A);

	// return a transformation matrix B
	return XMMatrixTranspose(XMMatrixInverse(nullptr, A));
}