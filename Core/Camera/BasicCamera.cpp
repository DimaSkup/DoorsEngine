// ********************************************************************************
// Filename: BasicCamera.cpp
// 
// Created:  04.04.22
// ********************************************************************************
#include "BasicCamera.h"
#include "../Common/Log.h"    
#include "../Common/Assert.h"

using namespace DirectX;


BasicCamera::BasicCamera()
	: posVector_{0, 0, 0},
	  rotVector_{0, 0, 0} 
{
	// create a view matrix for default params of the camera
	UpdateViewMatrix();
}

BasicCamera::~BasicCamera()
{
}

void BasicCamera::Initialize(const CameraInitParams& params)
{
	walkSpeed_ = params.speed_;         // a camera walk speed
	runSpeed_ = 4.0f * walkSpeed_;
	rotationSpeed_ = params.sensitivity_;   // a camera rotation speed

	SetProjectionValues(
		params.fovDegrees_, 
		params.aspectRatio_,
		params.nearZ_, 
		params.farZ_);
}


// ********************************************************************************
// 
//                              PUBLIC GETTTERS
// 
// ********************************************************************************


XMFLOAT3 BasicCamera::GetRotationFloat3InDegrees()
{
	// get rotation float3 (in degrees)
	return {
		DirectX::XMConvertToDegrees(rotVector_.m128_f32[0]),  // rotation around X-axis
		DirectX::XMConvertToDegrees(rotVector_.m128_f32[1]),  // rotation around Y-axis
		DirectX::XMConvertToDegrees(rotVector_.m128_f32[2])   // rotation around Z-axis
	};
}

/////////////////////////////////////////////////

void BasicCamera::UpdateReflectionViewMatrix(const DirectX::XMFLOAT3 & reflectionPlanePos,
	const DirectX::XMFLOAT3 & relfectionPlaneRotation)
{
	// NOTE: this function only works for the Y axis plane.
	//
	// this function builds a reflection view matrix the same way as
	// the regular UpdateViewMatrix function builds a view matrix. The main difference is
	// that we take as input the height of the object that will act as the Y axis plane
	// and then we use that height to invert the position.y variable for reflection. 
	// We also need to inver the pitch. This will build the reflection view matrix that we
	// can then use in the reflection shader.
	

	XMVECTOR positionVec(XMLoadFloat3(&reflectionPlanePos));
	XMVECTOR lookAtVec;               // where the camera is looking at
	XMVECTOR upVec;                   // upward direction
	XMMATRIX rotationMatrix;

	// local timer							
	DWORD dwTimeCur = GetTickCount();
	static DWORD dwTimeStart = dwTimeCur;

	// update the local timer
	float t = (dwTimeCur - dwTimeStart) / 1000.0f;

#if 0
	// set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotation in radians
	float pitch = relfectionPlaneRotation.x;//-XM_PIDIV2;
	float yaw   = relfectionPlaneRotation.y;
	float roll  = relfectionPlaneRotation.z;
#endif

#if 1
	float pitch = 0;
	float yaw = -XM_PI;                    // around y-axis
	float roll  = rotVector_.m128_f32[3];  // around z-axis
#endif
	
	// create the rotation matrix from the yaw, pitch, and roll values
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// transform the lookAtVec and up vector by the rotation matrix so the view is correcly
	// rotated at the origin
	const DEFAULT_VECTORS defaultVectors = defaultVectors_;
	lookAtVec = XMVector3TransformCoord(defaultVectors.DEFAULT_FORWARD_VECTOR_, rotationMatrix);
	upVec = XMVector3TransformCoord(defaultVectors.DEFAULT_UP_VECTOR_, rotationMatrix);

	// translate the rotated camera position to the location of the viewer
	lookAtVec = XMVectorAdd(positionVec, lookAtVec);

	// finally create the view matrix from the three updated vectors
	reflectionView_ = XMMatrixLookAtLH(positionVec, lookAtVec, upVec);
}




// ********************************************************************************
//
//                               PUBLIC SETTERS
//
// ********************************************************************************

void BasicCamera::SetRotationInDeg(const DirectX::XMVECTOR & newAngle)
{
	rotVector_ = 
	{ 
		DirectX::XMConvertToRadians(XMVectorGetX(newAngle)),  // x
		DirectX::XMConvertToRadians(XMVectorGetY(newAngle)),  // y
	    DirectX::XMConvertToRadians(XMVectorGetZ(newAngle))   // z
	};
}

///////////////////////////////////////////////////////////

void BasicCamera::AdjustRotationInDeg(const DirectX::XMVECTOR & angle)
{
	rotVector_ += 
	{ 
		DirectX::XMConvertToRadians(XMVectorGetX(angle)),  // x
		DirectX::XMConvertToRadians(XMVectorGetY(angle)),  // y
		DirectX::XMConvertToRadians(XMVectorGetZ(angle))   // z
	};
}

///////////////////////////////////////////////////////////

void BasicCamera::SetProjectionValues(
	const float fov,                     // field of view (in radians)
	const float aspectRatio,
	const float nearZ, 
	const float farZ)
{
	// set up the projection matrix

	Assert::True((fov > 0.0f) & (aspectRatio > 0.0f) & (nearZ > 0.0f) & (farZ > nearZ), "wrong input params");

	//cameraNear_ = nearZ;
	//cameraDepth_ = farZ;
	fovInRad_ = fov;

	projection_ = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);
}

/////////////////////////////////////////////////

void BasicCamera::UpdateViewMatrix()
{
	//
	// updates the view matrix and also updates the movement vectors
	//


	// calculate camera rotation matrix
	const XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotVector_);

	//calculate unit vector of camera target based on camera forward value transformed by camera rotation matrix
	vecLookAt_ = XMVector3TransformCoord(defaultVectors_.DEFAULT_FORWARD_VECTOR_, camRotationMatrix);

	// translate the rotated camera position to the location of the viewer
	vecLookAt_ = XMVectorAdd(posVector_, vecLookAt_);

	// compute new direction vector
	vecCameraDir_ = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vecLookAt_, posVector_));

	// update some data of this camera	
	view_ = XMMatrixLookAtLH(posVector_, vecLookAt_, defaultVectors_.DEFAULT_UP_VECTOR_);

	// each time when we modify rotation of the camera we have to update
	// its basic direction vectors
	const DirectX::XMMATRIX vecRotMatrix = DirectX::XMMatrixRotationRollPitchYaw(0.0f, XMVectorGetY(rotVector_), 0.0f);
	vecForward_  = XMVector3TransformCoord(defaultVectors_.DEFAULT_FORWARD_VECTOR_,  vecRotMatrix);
	vecBackward_ = XMVector3TransformCoord(defaultVectors_.DEFAULT_BACKWARD_VECTOR_, vecRotMatrix);
	vecLeft_     = XMVector3TransformCoord(defaultVectors_.DEFAULT_LEFT_VECTOR_,     vecRotMatrix);
	vecRight_    = XMVector3TransformCoord(defaultVectors_.DEFAULT_RIGHT_VECTOR_,    vecRotMatrix);

	// compute inverse view matrix
	invView_ = XMMatrixInverse(nullptr, view_);
}

///////////////////////////////////////////////////////////

void* BasicCamera::operator new(size_t i)
{
	// memory allocation (we need it because we use DirectX::XM-objects)
	if (void* ptr = _aligned_malloc(i, 16))
	{
		return ptr;
	}

	Log::Error("can't allocate the memory for object");
	throw std::bad_alloc{};
}

void BasicCamera::operator delete(void* p)
{
	_aligned_free(p);
}

